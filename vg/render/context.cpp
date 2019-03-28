#include "context.h"

#if defined(WIN32)
#include <Windows.h>
#endif

#include <shaderc/shaderc.h>
#include <fstream>

#include <vku.hpp>
#include <core/log.h>

namespace vg
{
	Context::Context(const void* windowHandle) {
		vku::InstanceMaker im{};
		instance = im.defaultLayers().createUnique();

		auto gpus = instance->enumeratePhysicalDevices();
		physicalDevice = gpus[0];
		memoryProperties = physicalDevice.getMemoryProperties();

		{
			auto queueProps = physicalDevice.getQueueFamilyProperties();
			vk::QueueFlags search = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;
			for (uint32_t qi = 0; qi != queueProps.size(); ++qi) {
				auto &qprop = queueProps[qi];
				std::cout << vk::to_string(qprop.queueFlags) << "\n";
				if ((qprop.queueFlags & search) == search) {
					graphicsQueueFamilyIndex = qi;
					computerQueueFamilyIndex = qi;
					break;
				}
			}

			if (graphicsQueueFamilyIndex == ~0 || computerQueueFamilyIndex == ~0) {
				log_error("oops, missing a queue\n");
				return;
			}
		}

		{
			vku::DeviceMaker dm{};
			dm.defaultLayers().queue(graphicsQueueFamilyIndex);
			if (computerQueueFamilyIndex != graphicsQueueFamilyIndex) {
				dm.queue(computerQueueFamilyIndex);
			}
			dm.extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
			vk::PhysicalDeviceFeatures feature;
			feature.wideLines = VK_TRUE;
			dm.features(feature);
			device = dm.createUnique(physicalDevice);

			graphicsQueue = device->getQueue(graphicsQueueFamilyIndex, 0);
			if (computerQueueFamilyIndex != graphicsQueueFamilyIndex) {
				computerQueue = graphicsQueue;
			}
		}


		{
			std::vector<vk::DescriptorPoolSize> poolSizes = {
				{vk::DescriptorType::eUniformBuffer,256},
				{vk::DescriptorType::eCombinedImageSampler,256},
				{vk::DescriptorType::eStorageBuffer,256}
			};
			auto info = vk::DescriptorPoolCreateInfo(
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 256, static_cast<uint32_t>(poolSizes.size()), poolSizes.data()
			);
			descriptorPool = device->createDescriptorPoolUnique(info);
		}

		{
			auto info = vk::Win32SurfaceCreateInfoKHR({}, GetModuleHandle(NULL), (HWND)windowHandle);
			surface = instance->createWin32SurfaceKHRUnique(info);
		}
		commandPool = device->createCommandPoolUnique({ vk::CommandPoolCreateFlagBits::eResetCommandBuffer,graphicsQueueFamilyIndex });

		createSwapchain();
	}

	void Context::createSwapchain()
	{
		if (physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, surface.get()) != VK_TRUE) {
			log_error("graphics queue not support present\n");
			return;
		}

		auto formats = physicalDevice.getSurfaceFormatsKHR(surface.get());

		auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface.get());
		vk::SwapchainCreateInfoKHR info;

		info.imageExtent = capabilities.currentExtent;
		
		info.preTransform = capabilities.currentTransform;
		info.surface = surface.get();
		info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		info.queueFamilyIndexCount = 1;
		info.pQueueFamilyIndices = &graphicsQueueFamilyIndex;
		info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		info.presentMode = vk::PresentModeKHR::eFifo;
		info.minImageCount = 3;
		info.imageFormat = getSwapchainFormat();
		info.imageColorSpace = vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear;
		info.imageArrayLayers = 1;
		info.clipped = VK_TRUE;
		info.oldSwapchain = swapchain.get();

		swapchain = device->createSwapchainKHRUnique(info);

		swapchainImages = device->getSwapchainImagesKHR(swapchain.get());

		swapchainImageViews.clear();
		for (auto& img : swapchainImages)
		{
			auto viewInfo = vk::ImageViewCreateInfo({}, img, vk::ImageViewType::e2D, info.imageFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
			swapchainImageViews.emplace_back(device->createImageViewUnique(viewInfo));
		}

		extent = info.imageExtent;
	}


	static shaderc_shader_kind getShadercKind(vk::ShaderStageFlagBits stage)
	{
		switch (stage)
		{
		case vk::ShaderStageFlagBits::eVertex:					return shaderc_vertex_shader;
		case vk::ShaderStageFlagBits::eTessellationControl:		return shaderc_tess_control_shader;
		case vk::ShaderStageFlagBits::eTessellationEvaluation:	return shaderc_tess_evaluation_shader;
		case vk::ShaderStageFlagBits::eGeometry:				return shaderc_geometry_shader;
		case vk::ShaderStageFlagBits::eFragment:				return shaderc_fragment_shader;
		case vk::ShaderStageFlagBits::eCompute:					return shaderc_compute_shader;
		}
		return shaderc_shader_kind();
	}

	static std::string readFile(const std::string& path)
	{
		std::string code = "";

		std::ifstream filestream(path, std::ios::in | std::ios::binary);

		if (!filestream.good()) {
			log_error("Faild to read file : ", path);
		}
		else {
			code = std::string((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>());
		}

		filestream.close();

		return std::move(code);
	}

	vku::ShaderModule Context::compileGLSLToSpv(vk::ShaderStageFlagBits stage,const std::string& src) const
	{
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		auto result = shaderc_compile_into_spv(
			compiler, src.c_str(), src.size(),
			getShadercKind(stage), "", "main", nullptr);

		auto status = shaderc_result_get_compilation_status(result);
		if (status != shaderc_compilation_status_success) {
			log_error("SHADER : ", shaderc_result_get_error_message(result));
		}

		auto length = shaderc_result_get_length(result);
		auto bytes = (uint32_t*)shaderc_result_get_bytes(result);

		std::vector<uint32_t> data(bytes, bytes + length/4);
		auto module = vku::ShaderModule(getDevice(), data.begin(), data.end());

		// Do stuff with compilation results.
		shaderc_result_release(result);
		shaderc_compiler_release(compiler);

		return std::move(module);
	}


	vk::SampleCountFlagBits Context::sample = vk::SampleCountFlagBits::e4;
	vk::Format Context::swapchainFormat = vk::Format::eB8G8R8A8Unorm;
	vk::Format Context::depthFormat = vk::Format::eD24UnormS8Uint;
}