#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace vg
{

	class Camera
	{
		float fov = 45.f;
		float aspect = 1.0f;
		float minZ = 0.1f;
		float maxZ = 1000.0f;

		enum class Type
		{
			Undefined,
			Perspective,
			Orthographic
		}type;

		glm::vec3 position;
		glm::vec3 rotation;

	public:
		inline static Camera Perspactive(float fov,float aspect,float minZ = 0.1f,float maxZ = 1000.0f)
		{
			return { fov,aspect,minZ,maxZ };
		}

		inline glm::mat4 getProjectionMatrix() {
			return glm::perspective(fov, aspect, minZ, maxZ);
		}

		operator bool() const
		{
			return type != Type::Undefined;
		}
	private:
		Camera(float fov, float aspect, float minZ, float maxZ) : fov(fov),aspect(aspect),minZ(minZ),maxZ(maxZ),type(Type::Perspective){}

	};

}