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
		std::vector<glm::vec3> position;
		std::vector<glm::vec3> normal;
		std::vector<glm::vec2> texcoord;
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
				
				position.push_back(p);
				normal.push_back(glm::normalize(p));
				texcoord.push_back({ u,1 - v });
				verticesRow.push_back(index++);
			}
			grid.push_back(verticesRow);
		}

		// indices
		for(uint32_t iy = 0; iy < segmentY; iy++) {
			for(uint32_t ix = 0; ix < segmentX; ix++) {
				auto a = grid[iy][ix + 1];
				auto b = grid[iy][ix];
				auto c = grid[iy + 1][ix];
				auto d = grid[iy + 1][ix + 1];

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

		return { std::move(position),std::move(normal),std::move(texcoord),std::move(indices) };
	}
}