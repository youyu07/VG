#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace vg
{

	class SimpleGeometry
	{
	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 texcoord;

			Vertex(glm::vec3 p, glm::vec3 n, glm::vec2 t) :position(p), normal(n), texcoord(t) {}
		};
		std::vector<Vertex> vertex;
		std::vector<uint16_t> indices;

		static SimpleGeometry createCube(uint32_t sizeX,uint32_t sizeY,uint32_t sizeZ);

		static SimpleGeometry createSphere(float radius = 1, uint32_t segmentX = 8, uint32_t segmentY = 6);
	};
}