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

		glm::vec3 rotation = glm::vec3();
		glm::vec3 position = glm::vec3();

		float rotateSpeed = 0.2f;

	public:
		inline static Camera Perspactive(float fov,float minZ = 0.1f,float maxZ = 1000.0f)
		{
			return { fov,minZ,maxZ };
		}

		inline glm::mat4 getProjectionMatrix(float aspect) const {
			return glm::perspective(glm::radians(fov), aspect, minZ, maxZ);
		}

		inline glm::mat4 getViewMatrix() const {
			glm::mat4 rotM = glm::mat4(1.0f);
			glm::mat4 transM;

			rotM = glm::rotate(rotM, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			transM = glm::translate(glm::mat4(1.0f), position);
			return transM * rotM;
		}

		inline void setPosition(glm::vec3 pos)
		{
			position = pos;
		}

		void translate(glm::vec3 delta)
		{
			this->position += delta;
		}

		inline void rotate(glm::vec3 delta)
		{
			this->rotation += delta;
		}

		inline float getRotateSpeed() const
		{
			return rotateSpeed;
		}

		operator bool() const
		{
			return type != Type::Undefined;
		}
	private:
		Camera(float fov, float minZ, float maxZ) : fov(fov),minZ(minZ),maxZ(maxZ),type(Type::Perspective){}

	};

}