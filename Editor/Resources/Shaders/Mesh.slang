// Mesh.slang — KairosEngine Extended PBR Shader
//
// Base BRDF   : Cook-Torrance microfacet (GGX / Smith G2 / Schlick F)
// Diffuse     : Oren-Nayar (sigma=0 collapses to Lambertian)
// Layer 1     : Clear Coat (independent GGX lobe, IOR 1.5, own roughness)
// Layer 2     : Wetness    (darkens diffuse, smooths roughness, adds Fresnel sheen)
// SSS         : deferred — add later
//
// Descriptor layout (unchanged from base PBR):
//   Set 0 Binding 0 — FrameData UBO  (View, Projection)          [vert+frag]
//   Set 1 Binding 0 — albedoTex      sRGB RGBA
//   Set 1 Binding 1 — normalTex      linear RGB tangent-space
//   Set 1 Binding 2 — metallicRoughTex  linear G=rough B=metal   (glTF)
//   Set 1 Binding 3 — aoTex          linear R
//   Set 1 Binding 4 — emissiveTex    sRGB RGB
//   Set 1 Binding 5 — MaterialParams UBO  (3 rows × 16 bytes)
//   Push constant   — PushConstantObject  (Model mat4)
//
// Tonemap : Reinhard + gamma — remove when HDR post-process lands.
// CamPos  : reconstructed from View inverse — no C++ struct change.
// -----------------------------------------------------------------------

static const float PI      = 3.14159265358979323846f;
static const float INV_PI  = 0.31830988618379067153f;

// -----------------------------------------------------------------------
// Structs
// -----------------------------------------------------------------------
struct VSInput
{
    float3 Position  : POSITION;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
    float2 TexCoord  : TEXCOORD0;
};

struct VSOutput
{
    float4 ClipPosition : SV_Position;
    float3 WorldPos     : TEXCOORD0;
    float3 Normal       : TEXCOORD1;
    float2 TexCoord     : TEXCOORD2;
    float3 Tangent      : TEXCOORD3;
    float3 Bitangent    : TEXCOORD4;
};

// -----------------------------------------------------------------------
// Push constant
// -----------------------------------------------------------------------
struct PushConstantObject { float4x4 Model; };
[[vk::push_constant]] PushConstantObject u_PushConstants;

// -----------------------------------------------------------------------
// Set 0 — per-frame camera
// -----------------------------------------------------------------------
struct FrameData { float4x4 View; float4x4 Projection; };
[[vk::binding(0, 0)]] ConstantBuffer<FrameData> u_FrameData;

// -----------------------------------------------------------------------
// Set 1 — material textures
// -----------------------------------------------------------------------
[[vk::binding(0, 1)]] Sampler2D albedoTex;
[[vk::binding(1, 1)]] Sampler2D normalTex;
[[vk::binding(2, 1)]] Sampler2D metallicRoughTex;
[[vk::binding(3, 1)]] Sampler2D aoTex;
[[vk::binding(4, 1)]] Sampler2D emissiveTex;

// Set 1 Binding 5 — MaterialParams UBO (3×16 bytes, std140)
// Row 0 : albedoColor (vec4)
// Row 1 : metallic, roughness, emissiveScale, orenNayarSigma
// Row 2 : clearCoat, clearCoatRoughness, wetness, _pad
[[vk::binding(5, 1)]] cbuffer MaterialParams
{
    float4 albedoColor;
    float  metallic;
    float  roughness;
    float  emissiveScale;
    float  orenNayarSigma;      // 0 = Lambertian, 1 = maximally rough Oren-Nayar
    float  clearCoat;           // 0-1 coat intensity
    float  clearCoatRoughness;  // independent coat lobe roughness
    float  wetness;             // 0=dry 1=wet
    float  _pad0;
    float  specularIntensity;   // 0=no specular, 1=default, >1=boosted
    float  specularTint;        // 0=achromatic F0, 1=tinted toward baseColor
    float  _pad1;
    float  _pad2;
};

// -----------------------------------------------------------------------
// BRDF: GGX NDF
// -----------------------------------------------------------------------
float D_GGX(float NdotH, float alpha)
{
    float a2    = alpha * alpha;
    float denom = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * denom * denom);
}

// Smith G1
float G1_Smith(float NdotX, float k) { return NdotX / (NdotX * (1.0f - k) + k); }

// Smith G2 height-correlated (Disney direct-lighting remapping)
float G_Smith(float NdotV, float NdotL, float rough)
{
    float r = rough + 1.0f;
    float k = (r * r) * 0.125f;
    return G1_Smith(NdotV, k) * G1_Smith(NdotL, k);
}

// Schlick Fresnel
float3 F_Schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

// Scalar Fresnel (for clear coat / wetness — fixed F0)
float F_Schlick1(float cosTheta, float f0)
{
    return f0 + (1.0f - f0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

// -----------------------------------------------------------------------
// Diffuse: Oren-Nayar (Ramamoorthi-Hanrahan qualitative approximation)
// sigma = 0 → pure Lambertian.  sigma ∈ [0,1] (linear roughness measure).
// -----------------------------------------------------------------------
float3 OrenNayar(float3 baseColor, float NdotL, float NdotV, float3 N, float3 L, float3 V, float sigma)
{
    // Fast analytic approximation — avoids trig on GPU
    float s2 = sigma * sigma;
    float A  = 1.0f - (s2 / (2.0f * (s2 + 0.33f)));
    float B  = (0.45f * s2) / (s2 + 0.09f);

    // cos(phi_i - phi_r) via tangent-plane projection
    float3 VpN = normalize(V - N * NdotV);
    float3 LpN = normalize(L - N * NdotL);
    float  gamma = clamp(dot(VpN, LpN), 0.0f, 1.0f);

    // sin/tan from NdotX
    float sinI = sqrt(max(0.0f, 1.0f - NdotL * NdotL));
    float sinR = sqrt(max(0.0f, 1.0f - NdotV * NdotV));
    float tanR = (NdotV > 1e-4f) ? (sinR / NdotV) : 0.0f;

    float maxSinTan = (sinI > sinR) ? sinI * tanR : sinR * (sinI / max(NdotL, 1e-4f));

    float oren = A + B * gamma * maxSinTan;
    return baseColor * INV_PI * oren;
}

// -----------------------------------------------------------------------
// Cook-Torrance specular lobe (single call, reusable for coat)
// -----------------------------------------------------------------------
float3 CookTorranceSpecular(float NdotL, float NdotV, float NdotH, float HdotV,
                             float alpha, float rough, float3 F0)
{
    float  D = D_GGX(NdotH, alpha);
    float  G = G_Smith(NdotV, NdotL, rough);
    float3 F = F_Schlick(HdotV, F0);
    return (D * G * F) / max(4.0f * NdotV * NdotL, 1e-4f);
}

// -----------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------
[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput o;
    float4 worldPos    = mul(u_PushConstants.Model, float4(input.Position, 1.0f));
    o.ClipPosition     = mul(u_FrameData.Projection, mul(u_FrameData.View, worldPos));
    o.WorldPos         = worldPos.xyz;
    float3x3 nMat      = (float3x3)u_PushConstants.Model;
    o.Normal           = normalize(mul(nMat, input.Normal));
    o.Tangent          = normalize(mul(nMat, input.Tangent));
    o.Bitangent        = normalize(mul(nMat, input.Bitangent));
    o.TexCoord         = input.TexCoord;
    return o;
}

// -----------------------------------------------------------------------
// Fragment Shader
// -----------------------------------------------------------------------
[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    // ---- Camera world pos from rigid-body View inverse ------------------
    float3 Rx    = float3(u_FrameData.View[0][0], u_FrameData.View[1][0], u_FrameData.View[2][0]);
    float3 Ry    = float3(u_FrameData.View[0][1], u_FrameData.View[1][1], u_FrameData.View[2][1]);
    float3 Rz    = float3(u_FrameData.View[0][2], u_FrameData.View[1][2], u_FrameData.View[2][2]);
    float3 Vt    = float3(u_FrameData.View[0][3], u_FrameData.View[1][3], u_FrameData.View[2][3]);
    float3 camPos = -(Rx * Vt.x + Ry * Vt.y + Rz * Vt.z);

    // ---- Sample textures ------------------------------------------------
    float4 albedoSample = albedoTex.Sample(input.TexCoord);
    float3 baseColor    = albedoSample.rgb * albedoColor.rgb;
    float2 mr           = metallicRoughTex.Sample(input.TexCoord).bg; // B=metal G=rough (glTF)
    float  mat_metallic = clamp(mr.x * metallic,   0.0f, 1.0f);
    float  mat_rough    = clamp(mr.y * roughness,  0.04f, 1.0f);
    float  ao           = aoTex.Sample(input.TexCoord).r;
    float3 emissive     = emissiveTex.Sample(input.TexCoord).rgb * emissiveScale;

    // ---- Normal map → world space ---------------------------------------
    float3 tn = normalTex.Sample(input.TexCoord).rgb * 2.0f - 1.0f;
    float3 N  = normalize(input.Normal);
    float3 T  = normalize(input.Tangent - dot(input.Tangent, N) * N);
    float3 B  = cross(N, T);
    N         = normalize(T * tn.x + B * tn.y + N * tn.z);

    // ---- Wetness layer — modulate base params before BRDF ---------------
    // Water absorption darkens diffuse (√ tint trick from Unreal wetness).
    // Water film smooths surface roughness and adds Fresnel sheen.
    float  wet         = clamp(wetness, 0.0f, 1.0f);
    float3 wetColor    = sqrt(baseColor);                           // darkened diffuse
    baseColor          = lerp(baseColor, wetColor, wet);
    float  wetRough    = lerp(mat_rough, 0.04f, wet * 0.8f);       // smooth toward mirror
    mat_rough          = wetRough;
    float  alpha       = mat_rough * mat_rough;                     // perceptual → linear

    // ---- View / light vectors -------------------------------------------
    float3 V      = normalize(camPos - input.WorldPos);
    float3 L      = normalize(float3(1.0f, 2.0f, 1.0f));           // placeholder directional
    float3 H      = normalize(V + L);
    float3 radiance   = float3(2.0f, 2.0f, 2.0f);

    float NdotL = max(dot(N, L), 0.0f);
    float NdotV = max(dot(N, V), 1e-4f);
    float NdotH = max(dot(N, H), 0.0f);
    float HdotV = max(dot(H, V), 0.0f);

    // ---- F0 — base reflectance (Disney specular tint) -------------------
    // SpecularTint lerps dielectric F0 (0.04 white) toward tinted-baseColor version.
    // Metals always use baseColor as F0 — tint doesn't apply to them.
    float  lum       = dot(baseColor, float3(0.2126f, 0.7152f, 0.0722f));
    float3 tintColor = (lum > 0.f) ? baseColor / lum : float3(1.f, 1.f, 1.f);
    float3 F0_dielectric = lerp(float3(0.04f, 0.04f, 0.04f), tintColor * 0.04f, specularTint);
    float3 F0_base = lerp(F0_dielectric, baseColor, mat_metallic);

    // ---- Wetness Fresnel sheen (thin water film, F0 ~0.02) --------------
    // Adds a uniform low-gloss reflection across the wet surface.
    float wetF  = wet * F_Schlick1(NdotV, 0.02f);
    float3 F0   = lerp(F0_base, max(F0_base, float3(wetF, wetF, wetF)), wet);

    // ---- Base layer BRDF ------------------------------------------------
    float3 specBase = CookTorranceSpecular(NdotL, NdotV, NdotH, HdotV, alpha, mat_rough, F0)
                      * specularIntensity;

    // Oren-Nayar diffuse — sigma from material param (0 = Lambertian)
    float3 F_base = F_Schlick(HdotV, F0);
    float3 kD     = (1.0f - F_base) * (1.0f - mat_metallic);
    float3 diffBase = kD * OrenNayar(baseColor, NdotL, NdotV, N, L, V, orenNayarSigma);

    float3 Lo = (diffBase + specBase) * radiance * NdotL;

    // ---- Clear coat layer -----------------------------------------------
    // Independent GGX lobe over the base. IOR 1.5 → F0=0.04.
    // Attenuates base by (1 - coatFresnel) per energy conservation.
    float3 Lo_coat = float3(0.0f, 0.0f, 0.0f);
    float  coatAttenuation = 1.0f;
    if (clearCoat > 0.0f)
    {
        float  coatAlpha  = clamp(clearCoatRoughness, 0.04f, 1.0f);
        coatAlpha         = coatAlpha * coatAlpha;
        float3 F0_coat    = float3(0.04f, 0.04f, 0.04f);   // IOR 1.5 dielectric

        float3 specCoat   = CookTorranceSpecular(NdotL, NdotV, NdotH, HdotV,
                                                  coatAlpha, clearCoatRoughness, F0_coat);
        float  coatF      = F_Schlick1(HdotV, 0.04f);

        Lo_coat        = specCoat * radiance * NdotL * clearCoat;
        coatAttenuation = 1.0f - coatF * clearCoat;  // base attenuated by coat Fresnel
    }

    float3 Lfinal = Lo * coatAttenuation + Lo_coat;

    // ---- Ambient (constant IBL placeholder) -----------------------------
    // TODO: replace with SH irradiance + prefiltered specular when IBL lands.
    float3 ambient = float3(0.03f, 0.03f, 0.04f) * baseColor * ao;

    // ---- Combine --------------------------------------------------------
    float3 color = ambient + Lfinal + emissive;

    // Reinhard tonemap + gamma — remove when HDR post-process pipeline lands.
    color = color / (color + 1.0f);
    color = pow(color, 1.0f / 2.2f);

    return float4(color, albedoSample.a);
}
