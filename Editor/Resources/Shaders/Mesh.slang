// Mesh.slang — KairosEngine Extended PBR Shader
//
// Base BRDF   : Cook-Torrance microfacet (GGX / Smith G2 / Schlick F)
// Diffuse     : Oren-Nayar (sigma=0 collapses to Lambertian)
// Layer 1     : Clear Coat (independent GGX lobe, IOR 1.5, own roughness)
// Layer 2     : Wetness    (darkens diffuse, smooths roughness, adds Fresnel sheen)
//
// Descriptor layout:
//   Set 0 Binding 0 — SceneData UBO  (View, Proj, LightViewProj, Lights...)  [vert+frag]
//   Set 0 Binding 1 — shadowMap      (directional light depth map)           [frag]
//   Set 1 Binding 0 — albedoTex
//   Set 1 Binding 1 — normalTex
//   Set 1 Binding 2 — metallicRoughTex
//   Set 1 Binding 3 — aoTex
//   Set 1 Binding 4 — emissiveTex
//   Set 1 Binding 5 — MaterialParams UBO
//   Push constant   — PushConstantObject (Model mat4)
//
// Light types (GpuLight.Type):
//   0 = Directional  — infinite distance, direction only, no attenuation
//   1 = Point        — position + range, spherical attenuation
//   2 = Spot         — position + direction + range + cone angles (cosines)

static const float PI      = 3.14159265358979323846f;
static const float INV_PI  = 0.31830988618379067153f;
static const int   MAX_LIGHTS = 16;

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
// Set 0 — per-scene data (camera + lights)
// Must match Engine::GpuLight and Engine::SceneData exactly.
// -----------------------------------------------------------------------
struct GpuLight
{
    float4 Position;         // xyz = world pos,       w = unused
    float4 Direction;        // xyz = normalized dir,  w = unused
    float4 ColorIntensity;   // xyz = linear color,    w = intensity
    float  Range;
    float  InnerConeAngle;   // cosine of inner half-angle (Spot only)
    float  OuterConeAngle;   // cosine of outer half-angle (Spot only)
    int    Type;             // 0=Directional 1=Point 2=Spot
};

struct SceneData
{
    float4x4 View;
    float4x4 Projection;
    float4x4 LightViewProj;
    GpuLight Lights[MAX_LIGHTS];
    float4   ShadowParams;    // x = bias, y = texel size
    int      LightCount;
    int      ShadowLightIndex;
    int      ShadowEnabled;
    float    _pad0;
};
[[vk::binding(0, 0)]] ConstantBuffer<SceneData> u_SceneData;
[[vk::binding(1, 0)]] Sampler2D shadowMap;

// -----------------------------------------------------------------------
// Set 1 — material textures + params
// -----------------------------------------------------------------------
[[vk::binding(0, 1)]] Sampler2D albedoTex;
[[vk::binding(1, 1)]] Sampler2D normalTex;
[[vk::binding(2, 1)]] Sampler2D metallicRoughTex;
[[vk::binding(3, 1)]] Sampler2D aoTex;
[[vk::binding(4, 1)]] Sampler2D emissiveTex;

[[vk::binding(5, 1)]] cbuffer MaterialParams
{
    float4 albedoColor;
    float  metallic;
    float  roughness;
    float  emissiveScale;
    float  orenNayarSigma;
    float  clearCoat;
    float  clearCoatRoughness;
    float  wetness;
    float  _pad0;
    float  specularIntensity;
    float  specularTint;
    float  _pad1;
    float  _pad2;
};

// -----------------------------------------------------------------------
// BRDF helpers
// -----------------------------------------------------------------------
float D_GGX(float NdotH, float alpha)
{
    float a2    = alpha * alpha;
    float denom = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * denom * denom);
}

float G1_Smith(float NdotX, float k) { return NdotX / (NdotX * (1.0f - k) + k); }

float G_Smith(float NdotV, float NdotL, float rough)
{
    float r = rough + 1.0f;
    float k = (r * r) * 0.125f;
    return G1_Smith(NdotV, k) * G1_Smith(NdotL, k);
}

float3 F_Schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float F_Schlick1(float cosTheta, float f0)
{
    return f0 + (1.0f - f0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float3 OrenNayar(float3 baseColor, float NdotL, float NdotV,
                  float3 N, float3 L, float3 V, float sigma)
{
    float s2 = sigma * sigma;
    float A  = 1.0f - (s2 / (2.0f * (s2 + 0.33f)));
    float B  = (0.45f * s2) / (s2 + 0.09f);

    float3 VpN = normalize(V - N * NdotV);
    float3 LpN = normalize(L - N * NdotL);
    float  gamma = clamp(dot(VpN, LpN), 0.0f, 1.0f);

    float sinI = sqrt(max(0.0f, 1.0f - NdotL * NdotL));
    float sinR = sqrt(max(0.0f, 1.0f - NdotV * NdotV));
    float tanR = (NdotV > 1e-4f) ? (sinR / NdotV) : 0.0f;

    float maxSinTan = (sinI > sinR)
        ? sinI * tanR
        : sinR * (sinI / max(NdotL, 1e-4f));

    float oren = A + B * gamma * maxSinTan;
    return baseColor * INV_PI * oren;
}

float3 CookTorranceSpecular(float NdotL, float NdotV, float NdotH, float HdotV,
                             float alpha, float rough, float3 F0)
{
    float  D = D_GGX(NdotH, alpha);
    float  G = G_Smith(NdotV, NdotL, rough);
    float3 F = F_Schlick(HdotV, F0);
    return (D * G * F) / max(4.0f * NdotV * NdotL, 1e-4f);
}

float ComputeDirectionalShadow(float3 worldPos, float3 N, float3 L)
{
    if (u_SceneData.ShadowEnabled == 0)
        return 1.0f;

    float4 shadowClip = mul(u_SceneData.LightViewProj, float4(worldPos, 1.0f));
    float3 shadowNdc  = shadowClip.xyz / max(shadowClip.w, 1e-5f);
    float2 shadowUv   = shadowNdc.xy * 0.5f + 0.5f;
    float  shadowZ    = shadowNdc.z;

    if (shadowUv.x <= 0.0f || shadowUv.x >= 1.0f || shadowUv.y <= 0.0f || shadowUv.y >= 1.0f)
        return 1.0f;
    if (shadowZ <= 0.0f || shadowZ >= 1.0f)
        return 1.0f;

    float bias = max(u_SceneData.ShadowParams.x * (1.0f - dot(N, L)), u_SceneData.ShadowParams.x);
    float texel = max(u_SceneData.ShadowParams.y, 1e-5f);

    float visibility = 0.0f;
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            float2 offset = float2((float)x, (float)y) * texel;
            float depthSample = shadowMap.Sample(shadowUv + offset).r;
            visibility += (shadowZ - bias) <= depthSample ? 1.0f : 0.0f;
        }
    }

    return visibility / 9.0f;
}

// -----------------------------------------------------------------------
// Per-light BRDF contribution
// Returns Lo for one light. No ambient — accumulate and add ambient after.
// -----------------------------------------------------------------------
float3 EvaluateLight(
    int lightIdx,
    float3 worldPos,
    float3 N, float3 V,
    float3 baseColor,
    float  mat_metallic, float mat_rough, float alpha,
    float3 F0)
{
    GpuLight light = u_SceneData.Lights[lightIdx];

    float3 L        = float3(0, 0, 0);
    float3 radiance = float3(0, 0, 0);

    if (light.Type == 0)
    {
        // Directional — infinite, no attenuation
        L        = normalize(-light.Direction.xyz);
        radiance = light.ColorIntensity.rgb * light.ColorIntensity.w;
    }
    else if (light.Type == 1)
    {
        // Point — inverse-square attenuation clamped at range
        float3 toLight = light.Position.xyz - worldPos;
        float  dist    = length(toLight);
        if (dist >= light.Range) return float3(0, 0, 0);
        L = toLight / dist;

        float  atten   = 1.0f / (dist * dist + 1e-4f);
        float  window  = pow(max(0.0f, 1.0f - (dist / light.Range)), 4.0f);
        radiance = light.ColorIntensity.rgb * light.ColorIntensity.w * atten * window;
    }
    else // Spot (Type == 2)
    {
        float3 toLight = light.Position.xyz - worldPos;
        float  dist    = length(toLight);
        if (dist >= light.Range) return float3(0, 0, 0);
        L = toLight / dist;

        // Cone attenuation — angles stored as cosines
        float cosTheta   = dot(-L, normalize(light.Direction.xyz));
        float cosInner   = light.InnerConeAngle;
        float cosOuter   = light.OuterConeAngle;
        float coneAtten  = smoothstep(cosOuter, cosInner, cosTheta);
        if (coneAtten <= 0.0f) return float3(0, 0, 0);

        float atten  = 1.0f / (dist * dist + 1e-4f);
        float window = pow(max(0.0f, 1.0f - (dist / light.Range)), 4.0f);
        radiance = light.ColorIntensity.rgb * light.ColorIntensity.w * atten * window * coneAtten;
    }

    float NdotL = max(dot(N, L), 0.0f);
    if (NdotL <= 0.0f) return float3(0, 0, 0);

    float NdotV = max(dot(N, V), 1e-4f);
    float3 H    = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0f);
    float HdotV = max(dot(H, V), 0.0f);

    float3 specBase = CookTorranceSpecular(NdotL, NdotV, NdotH, HdotV, alpha, mat_rough, F0)
                      * specularIntensity;

    float3 F_base = F_Schlick(HdotV, F0);
    float3 kD     = (1.0f - F_base) * (1.0f - mat_metallic);
    float3 diffBase = kD * OrenNayar(baseColor, NdotL, NdotV, N, L, V, orenNayarSigma);

    float3 Lo = (diffBase + specBase) * radiance * NdotL;

    // Clear coat — independent lobe
    if (clearCoat > 0.0f)
    {
        float  coatAlpha = clamp(clearCoatRoughness, 0.04f, 1.0f);
        coatAlpha        = coatAlpha * coatAlpha;
        float3 F0_coat   = float3(0.04f, 0.04f, 0.04f);
        float3 specCoat  = CookTorranceSpecular(NdotL, NdotV, NdotH, HdotV,
                                                coatAlpha, clearCoatRoughness, F0_coat);
        float  coatF     = F_Schlick1(HdotV, 0.04f);
        float  coatAtten = 1.0f - coatF * clearCoat;
        Lo = Lo * coatAtten + specCoat * radiance * NdotL * clearCoat;
    }

    if (light.Type == 0 && lightIdx == u_SceneData.ShadowLightIndex)
    {
        float visibility = ComputeDirectionalShadow(worldPos, N, L);
        Lo *= visibility;
    }

    return Lo;
}

// -----------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------
[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput o;
    float4 worldPos    = mul(u_PushConstants.Model, float4(input.Position, 1.0f));
    o.ClipPosition     = mul(u_SceneData.Projection, mul(u_SceneData.View, worldPos));
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
    // ---- Camera world pos from View inverse (rigid body) ----------------
    float3 Rx    = float3(u_SceneData.View[0][0], u_SceneData.View[1][0], u_SceneData.View[2][0]);
    float3 Ry    = float3(u_SceneData.View[0][1], u_SceneData.View[1][1], u_SceneData.View[2][1]);
    float3 Rz    = float3(u_SceneData.View[0][2], u_SceneData.View[1][2], u_SceneData.View[2][2]);
    float3 Vt    = float3(u_SceneData.View[0][3], u_SceneData.View[1][3], u_SceneData.View[2][3]);
    float3 camPos = -(Rx * Vt.x + Ry * Vt.y + Rz * Vt.z);

    // ---- Textures -------------------------------------------------------
    float4 albedoSample = albedoTex.Sample(input.TexCoord);
    float3 baseColor    = albedoSample.rgb * albedoColor.rgb;
    float2 mr           = metallicRoughTex.Sample(input.TexCoord).bg;
    float  mat_metallic = clamp(mr.x * metallic,  0.0f, 1.0f);
    float  mat_rough    = clamp(mr.y * roughness,  0.04f, 1.0f);
    float  ao           = aoTex.Sample(input.TexCoord).r;
    float3 emissive     = emissiveTex.Sample(input.TexCoord).rgb * emissiveScale;

    // ---- Normal map → world space ---------------------------------------
    float3 tn = normalTex.Sample(input.TexCoord).rgb * 2.0f - 1.0f;
    float3 N  = normalize(input.Normal);
    float3 T  = normalize(input.Tangent - dot(input.Tangent, N) * N);
    float3 B  = cross(N, T);
    N         = normalize(T * tn.x + B * tn.y + N * tn.z);

    // ---- Wetness layer --------------------------------------------------
    float  wet      = clamp(wetness, 0.0f, 1.0f);
    baseColor       = lerp(baseColor, sqrt(baseColor), wet);
    mat_rough       = lerp(mat_rough, 0.04f, wet * 0.8f);
    float  alpha    = mat_rough * mat_rough;

    // ---- View vector / F0 -----------------------------------------------
    float3 V = normalize(camPos - input.WorldPos);

    float  lum       = dot(baseColor, float3(0.2126f, 0.7152f, 0.0722f));
    float3 tintColor = (lum > 0.f) ? baseColor / lum : float3(1.f, 1.f, 1.f);
    float3 F0_dielectric = lerp(float3(0.04f, 0.04f, 0.04f), tintColor * 0.04f, specularTint);
    float3 F0_base = lerp(F0_dielectric, baseColor, mat_metallic);

    float  NdotV = max(dot(N, V), 1e-4f);
    float  wetF  = wet * F_Schlick1(NdotV, 0.02f);
    float3 F0    = lerp(F0_base, max(F0_base, float3(wetF, wetF, wetF)), wet);

    // ---- Accumulate all lights ------------------------------------------
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    int numLights = min(u_SceneData.LightCount, MAX_LIGHTS);
    for (int i = 0; i < numLights; ++i)
    {
        Lo += EvaluateLight(i, input.WorldPos, N, V,
                             baseColor, mat_metallic, mat_rough, alpha, F0);
    }

    // ---- Ambient (constant IBL placeholder) -----------------------------
    // TODO: replace with SH irradiance when IBL lands.
    float3 ambient = float3(0.03f, 0.03f, 0.04f) * baseColor * ao;

    // ---- Combine --------------------------------------------------------
    float3 color = ambient + Lo + emissive;

    // Reinhard + gamma — remove when HDR post-process lands.
    color = color / (color + 1.0f);
    color = pow(color, 1.0f / 2.2f);

    return float4(color, albedoSample.a);
}
