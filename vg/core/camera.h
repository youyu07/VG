#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace vg
{

	class Camera
	{
		float fov = 45.f;
		float minZ = 0.1f;
		float maxZ = 1000.0f;

		enum class Type
		{
			Undefined,
			Perspective,
			Orthographic
		}type;

		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 target = glm::vec3(0.0f);

	public:
		inline static Camera Perspactive(float fov,float minZ = 0.1f,float maxZ = 1000.0f)
		{
			return { fov,minZ,maxZ };
		}

		inline glm::mat4 getProjectionMatrix(float aspect) const {
			return glm::perspective(glm::radians(fov), aspect, minZ, maxZ);
		}

		inline glm::mat4 getViewMatrix() const {
			return glm::lookAt(position, target, glm::vec3(0,1,0));
		}

		inline void setPosition(glm::vec3 pos)
		{
			position = pos;
		}

		inline void zoom(float length)
		{
			if (length == 0.0f)return;
			auto dir = position - target;
			
			auto dirn = glm::normalize(dir);
			auto move = dirn * length;
			if (glm::distance(dir, glm::vec3(0)) <= glm::distance(move,glm::vec3(0))) {
				return;
			}

			position -= move;
		}

		inline void mouseDownRotate()
		{

		}

		inline void mouseMoveRotate(int x,int y)
		{
			auto end = glm::vec2(x, y);
			//auto delta = (end - start) * rotateSpeed;

			//rotateLeft(2 * Math.PI * rotateDelta.x / element.clientHeight); // yes, height

			//rotateUp(2 * Math.PI * rotateDelta.y / element.clientHeight);

			//rotateStart.copy(rotateEnd);

			//scope.update();
		}

		operator bool() const
		{
			return type != Type::Undefined;
		}
	private:
		Camera(float fov, float minZ, float maxZ) : fov(fov),minZ(minZ),maxZ(maxZ),type(Type::Perspective){}

	};

}