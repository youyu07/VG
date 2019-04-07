#include "geometry.h"
#include <cmath>

namespace vg
{

	SimpleGeometry SimpleGeometry::createCube(uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ)
	{
		return {  };
	}

	SimpleGeometry SimpleGeometry::createSphere(float radius, uint32_t segmentX, uint32_t segmentY)
	{
		const float PI = static_cast<float>(std::atan(1.0) * 4);
		
		float phiLength = PI * 2;
		float thetaLength = PI;
		
		// buffers
		std::vector<Vertex> vertex;
		std::vector<uint16_t> indices;

		// generate vertices, normals and uvs
		std::vector<std::vector<uint16_t>> grid;
		uint16_t index = 0;
		for (uint32_t iy = 0; iy <= segmentY; iy++) {

			std::vector<uint16_t> verticesRow;

			float v = static_cast<float>(iy) / static_cast<float>(segmentY);

			for (uint32_t ix = 0; ix <= segmentX; ix++) {

				float u = static_cast<float>(ix) / static_cast<float>(segmentX);

				float x = -radius * std::cos(u * phiLength) * std::sin(v * thetaLength);
				float y = radius * std::cos(v * thetaLength);
				float z = radius * std::sin(u * phiLength) * std::sin(v * thetaLength);
				glm::vec3 p(x, y, z);
				vertex.emplace_back(p, glm::normalize(p),glm::vec2(u, 1 - v));
				verticesRow.push_back(index++);
			}
			grid.push_back(verticesRow);
		}

		// indices
		for(uint32_t iy = 0; iy < segmentY; iy++) {
			for(uint32_t ix = 0; ix < segmentX; ix++) {
				auto a = grid.at(iy).at(ix + 1);
				auto b = grid.at(iy).at(ix);
				auto c = grid.at(iy + 1).at(ix);
				auto d = grid.at(iy + 1).at(ix + 1);

				if (iy != 0) {
					indices.push_back(a);
					indices.push_back(b);
					indices.push_back(d);
				}

				if (iy != segmentY - 1) {
					indices.push_back(b);
					indices.push_back(c);
					indices.push_back(d);
				}
			}
		}

		return { std::move(vertex), std::move(indices) };
	}
}