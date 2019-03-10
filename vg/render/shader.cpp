#include "shader.h"
#include <shaderc/shaderc.h>
#include "device.h"
#include <fstream>

namespace vg::vk
{
	static shaderc_shader_kind getShadercKind(VkShaderStageFlagBits stage)
	{
		switch (stage)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:
			return shaderc_vertex_shader;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return shaderc_tess_control_shader;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return shaderc_tess_evaluation_shader;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return shaderc_geometry_shader;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return shaderc_fragment_shader;
		case VK_SHADER_STAGE_COMPUTE_BIT:
			return shaderc_compute_shader;
		}
		return shaderc_shader_kind();
	}

	static std::string readFile(const std::string& path)
	{
		std::string code = "";

		std::ifstream filestream(path, std::ios::in | std::ios::binary);
		
		if (!filestream.good()) {
			log_error("Faild to read file : ",path);
		}
		else {
			code = std::string((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>());
		}

		filestream.close();

		return std::move(code);
	}

	static VkShaderModule createModule(VkDevice device,const uint32_t* code,size_t size)
	{
		VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = size;
		createInfo.pCode = code;

		VkShaderModule handle = VK_NULL_HANDLE;
		vkCreateShaderModule(device, &createInfo, nullptr, &handle);
		return handle;
	}

	Shader::Shader(Device* device, const ShaderInfo& info) : device(device)
	{
		if (info.type == ShaderType::GLSL) {
			shaderc_compiler_t compiler = shaderc_compiler_initialize();
			auto result = shaderc_compile_into_spv(
				compiler, info.source.c_str(), info.source.size(),
				getShadercKind(info.stage), "", info.entry.c_str(), nullptr);

			auto status = shaderc_result_get_compilation_status(result);
			if (status != shaderc_compilation_status_success) {
				log_error("SHADER : ", shaderc_result_get_error_message(result));
			}

			auto length = shaderc_result_get_length(result);
			auto bytes = shaderc_result_get_bytes(result);

			handle = createModule(*device, (uint32_t*)(bytes), length);

			// Do stuff with compilation results.
			shaderc_result_release(result);
			shaderc_compiler_release(compiler);
		}
		else if (info.type == ShaderType::SPV_FILE) {
			auto code = readFile(info.source);
			handle = createModule(*device, (uint32_t*)(code.data()), code.size());
		}
	}

	Shader::~Shader()
	{
		vkDestroyShaderModule(*device, handle, nullptr);
	}
}