#pragma once
#include "common.h"

namespace vg::vk
{
	enum class ShaderType
	{
		SPV,
		GLSL,
		SPV_FILE,
		GLSL_FILE
	};

	struct ShaderInfo
	{
		std::string source;
		VkShaderStageFlagBits stage;
		ShaderType type = ShaderType::SPV_FILE;
		std::string entry = "main";
	};

	class Shader : public __VkObject<VkShaderModule>
	{
	public:
		Shader(class Device* device, const ShaderInfo& info);

		~Shader();
	private:
		class Device* device = nullptr;
	};

}