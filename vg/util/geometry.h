#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace vg
{

	class SimpleGeometry
	{
	public:
		std::vector<glm::vec3> position;
		std::vector<glm::vec3> normal;
		std::vector<glm::vec2> texcoord;
		std::vector<uint16_t> indices;


		static SimpleGeometry createCube(uint32_t sizeX,uint32_t sizeY,uint32_t sizeZ);

		static SimpleGeometry createSphere(float radius = 1, uint32_t segmentX = 8, uint32_t segmentY = 6);
	};
}