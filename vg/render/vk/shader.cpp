#include "vktypes.h"
#include <core/log.h>
#include <shaderc/shaderc.h>
#include <fstream>

namespace vg
{
    static shaderc_shader_kind getShadercKind(ShaderStage stage)
	{
		switch (stage)
		{
		case SHADER_VERT:
			return shaderc_vertex_shader;
		case SHADER_TESC:
			return shaderc_tess_control_shader;
		case SHADER_TESE:
			return shaderc_tess_evaluation_shader;
		case SHADER_GEOM:
			return shaderc_geometry_shader;
		case SHADER_FRAG:
			return shaderc_fragment_shader;
		case SHADER_COMP:
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

    Shader* createShader(VkDevice device,const ShaderDesc& desc)
    {
        VkShaderModule module = VK_NULL_HANDLE;

        if (desc.type == ShaderType::GLSL) {
			shaderc_compiler_t compiler = shaderc_compiler_initialize();
			auto result = shaderc_compile_into_spv(
				compiler, desc.source.c_str(), desc.source.size(),
				getShadercKind(desc.stage), "", desc.entry.c_str(), nullptr);

			auto status = shaderc_result_get_compilation_status(result);
			if (status != shaderc_compilation_status_success) {
				log_error("SHADER : ", shaderc_result_get_error_message(result));
			}

			auto length = shaderc_result_get_length(result);
			auto bytes = shaderc_result_get_bytes(result);

			module = createModule(device, (uint32_t*)(bytes), length);

			// Do stuff with compilation results.
			shaderc_result_release(result);
			shaderc_compiler_release(compiler);
		}
		else if (desc.type == ShaderType::SPV_FILE) {
			auto code = readFile(desc.source);
			module = createModule(device, (uint32_t*)(code.data()), code.size());
		}

        return new Shader{desc.stage,module};
    }

    void destroyShader(VkDevice device,Shader* shader)
    {
        vkDestroyShaderModule(device, shader->module, nullptr);
        delete shader;
    }

} // vg
