#include "kepch.h"
#include "PrimitiveMeshFactory.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Engine
{
	// -------------------------------------------------------------------------
	// Cube
	// -------------------------------------------------------------------------
	Ref<Mesh> PrimitiveMeshFactory::CreateCube(float h)
	{
		// 24 unique vertices (4 per face) — correct normals per face
		std::vector<Vertex> verts;
		verts.reserve(24);

		auto AddFace = [&](glm::vec3 n, glm::vec3 right, glm::vec3 up)
		{
			glm::vec3 tangent   = right;
			glm::vec3 bitangent = up;
			glm::vec3 center    = n * h;

			glm::vec3 p0 = center - right * h - up * h;
			glm::vec3 p1 = center + right * h - up * h;
			glm::vec3 p2 = center + right * h + up * h;
			glm::vec3 p3 = center - right * h + up * h;

			verts.push_back({ p0, n, tangent, bitangent, { 0.f, 1.f } });
			verts.push_back({ p1, n, tangent, bitangent, { 1.f, 1.f } });
			verts.push_back({ p2, n, tangent, bitangent, { 1.f, 0.f } });
			verts.push_back({ p3, n, tangent, bitangent, { 0.f, 0.f } });
		};

		AddFace({ 0,  0,  1}, { 1, 0, 0}, { 0, 1, 0});  // +Z
		AddFace({ 0,  0, -1}, {-1, 0, 0}, { 0, 1, 0});  // -Z
		AddFace({ 1,  0,  0}, { 0, 0,-1}, { 0, 1, 0});  // +X
		AddFace({-1,  0,  0}, { 0, 0, 1}, { 0, 1, 0});  // -X
		AddFace({ 0,  1,  0}, { 1, 0, 0}, { 0, 0,-1});  // +Y
		AddFace({ 0, -1,  0}, { 1, 0, 0}, { 0, 0, 1});  // -Y

		std::vector<uint32_t> indices;
		indices.reserve(36);
		for (uint32_t f = 0; f < 6; ++f)
		{
			uint32_t b = f * 4;
			indices.insert(indices.end(), { b, b+1, b+2, b+2, b+3, b });
		}

		return Mesh::Create(std::move(verts), std::move(indices));
	}

	// -------------------------------------------------------------------------
	// Plane (XZ plane, Y-up normal)
	// -------------------------------------------------------------------------
	Ref<Mesh> PrimitiveMeshFactory::CreatePlane(float h, uint32_t sub)
	{
		sub = (std::max)(sub, 1u);
		uint32_t vertCount = (sub + 1) * (sub + 1);
		std::vector<Vertex> verts;
		verts.reserve(vertCount);

		float step = (2.0f * h) / static_cast<float>(sub);
		for (uint32_t row = 0; row <= sub; ++row)
		{
			for (uint32_t col = 0; col <= sub; ++col)
			{
				float x = -h + col * step;
				float z = -h + row * step;
				float u = static_cast<float>(col) / sub;
				float v = static_cast<float>(row) / sub;
				verts.push_back({ {x, 0.f, z}, {0,1,0}, {1,0,0}, {0,0,1}, {u, v} });
			}
		}

		std::vector<uint32_t> indices;
		indices.reserve(sub * sub * 6);
		uint32_t stride = sub + 1;
		for (uint32_t row = 0; row < sub; ++row)
		{
			for (uint32_t col = 0; col < sub; ++col)
			{
				uint32_t tl = row * stride + col;
				uint32_t tr = tl + 1;
				uint32_t bl = tl + stride;
				uint32_t br = bl + 1;
				indices.insert(indices.end(), { tl, bl, tr, tr, bl, br });
			}
		}

		return Mesh::Create(std::move(verts), std::move(indices));
	}

	// -------------------------------------------------------------------------
	// Sphere (UV sphere)
	// -------------------------------------------------------------------------
	Ref<Mesh> PrimitiveMeshFactory::CreateSphere(float radius, uint32_t rings, uint32_t sectors)
	{
		rings   = (std::max)(rings,   2u);
		sectors = (std::max)(sectors, 3u);

		std::vector<Vertex> verts;
		verts.reserve((rings + 1) * (sectors + 1));

		const float PI  = glm::pi<float>();
		const float TWO_PI = glm::two_pi<float>();

		for (uint32_t r = 0; r <= rings; ++r)
		{
			float phi = PI * r / rings;                 // 0 .. PI
			float y   = radius * std::cos(phi);
			float sinPhi = std::sin(phi);

			for (uint32_t s = 0; s <= sectors; ++s)
			{
				float theta = TWO_PI * s / sectors;     // 0 .. 2PI
				float x = radius * sinPhi * std::cos(theta);
				float z = radius * sinPhi * std::sin(theta);

				glm::vec3 pos = { x, y, z };
				glm::vec3 n   = glm::normalize(pos);
				glm::vec3 t   = glm::normalize(glm::vec3(-std::sin(theta), 0, std::cos(theta)));
				glm::vec3 bt  = glm::cross(n, t);
				glm::vec2 uv  = { static_cast<float>(s) / sectors,
				                  static_cast<float>(r) / rings };

				verts.push_back({ pos, n, t, bt, uv });
			}
		}

		std::vector<uint32_t> indices;
		indices.reserve(rings * sectors * 6);
		uint32_t stride = sectors + 1;
		for (uint32_t r = 0; r < rings; ++r)
		{
			for (uint32_t s = 0; s < sectors; ++s)
			{
				uint32_t tl = r * stride + s;
				uint32_t tr = tl + 1;
				uint32_t bl = tl + stride;
				uint32_t br = bl + 1;
				indices.insert(indices.end(), { tl, bl, tr, tr, bl, br });
			}
		}

		return Mesh::Create(std::move(verts), std::move(indices));
	}
}
