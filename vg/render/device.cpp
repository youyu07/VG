#include "device.h"
#include "instance.h"
#include "commandBuffer.h"
#include "shader.h"
#include "util.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace vg::vk
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
	{
		if (messageSeverity& VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			log_warning(callbackData->pMessage);
		}
		else if (messageSeverity& VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			log_error(callbackData->pMessage);
		}
		else
		{
			log_info(callbackData->pMessage);
		}

		return VK_FALSE;
	}


	/*
	* device
	*/
    Device::Device()
	{
		instance = new Instance(true,DebugMessengerCallback);
		if(!instance->isValid())return;
		physicalDevice = new PhysicalDevice(*instance);
		if(!physicalDevice->isValid())return;

		float priorities = 1.0f;
		queueFamilyIndex = physicalDevice->getQueueFamily();
		VkDeviceQueueCreateInfo queueInfos = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueInfos.queueCount = 1;
		queueInfos.queueFamilyIndex = queueFamilyIndex;
		queueInfos.pQueuePriorities = &priorities;

		{
			uint32_t count = 0;
			vkEnumerateDeviceExtensionProperties(*physicalDevice, nullptr, &count, nullptr);
			std::vector<VkExtensionProperties> properties(count);
			vkEnumerateDeviceExtensionProperties(*physicalDevice, nullptr, &count, properties.data());
			for (auto& it : properties)
			{
				log_info("Find Enable Extension : ", it.extensionName);
			}
		}
		

		const char* extension[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,VK_KHR_MAINTENANCE1_EXTENSION_NAME };

		//VkPhysicalDeviceFeatures feature;
		//feature.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		info.enabledExtensionCount = 2;
		info.ppEnabledExtensionNames = extension;
		info.queueCreateInfoCount = 1;
		info.pQueueCreateInfos = &queueInfos;
		//info.pEnabledFeatures = &feature;

		if (vkCreateDevice(*physicalDevice, &info, nullptr, &handle) != VK_SUCCESS) {
			log_error("Failed to create device!");
			return;
		}

		{
			vkGetDeviceQueue(handle, queueFamilyIndex, 0, &graphicsQueue);
			if (graphicsQueue == VK_NULL_HANDLE) {
				log_error("Faild to get graphics queue.");
				return;
			}

			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = *physicalDevice;
			allocatorInfo.device = handle;

			vmaCreateAllocator(&allocatorInfo, &allocator);

			commandPool = createCommandPool();

			std::vector<VkDescriptorPoolSize> poolSize = {
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1000},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1000},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1000},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,1000},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,1000}
			};

			VkDescriptorPoolCreateInfo descriptorPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
			descriptorPoolInfo.maxSets = static_cast<uint32_t>(poolSize.size() * 1000);
			descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
			descriptorPoolInfo.pPoolSizes = poolSize.data();
			vkCreateDescriptorPool(handle,&descriptorPoolInfo,nullptr,&descriptorPool);
		}
	}

	CommandPool* Device::createCommandPool()
	{
		return new CommandPool(handle,graphicsQueue,queueFamilyIndex);
	}

	Image* Device::createImage(const ImageInfo& info)
	{
		return new Image(this, info);
	}

	Sampler* Device::createSampler(const SamplerInfo& info)
	{
		return new Sampler(this, info);
	}

	Buffer* Device::createBuffer(const BufferInfo& info)
	{
		return new Buffer(this, info);
	}

	Swapchain* Device::createSwapchain(const SwapchainInfo& info)
	{
		return new Swapchain(this, info);
	}

	CommandBuffer* Device::createCommandBuffer()
	{
		return commandPool->createCommandBuffer();
	}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	Surface* Device::createSurface(HWND hWnd)
	{
		VkWin32SurfaceCreateInfoKHR info = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		info.hinstance = GetModuleHandle(NULL);
		info.hwnd = hWnd;

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		vkCreateWin32SurfaceKHR(*instance, &info, nullptr, &surface);
		return new Surface(this,surface);
	}
#endif

	RenderPass* Device::createRenderPass(const RenderPassInfo& info)
	{
		return new RenderPass(this, info);
	}

	FrameBuffer* Device::createFrameBuffer(const FrameBufferInfo& info)
	{
		return new FrameBuffer(this, info);
	}

	Semaphore* Device::createSemaphore()
	{
		return new Semaphore(this);
	}

	Fence* Device::createFence()
	{
		return new Fence(this);
	}

	PipelineLayout* Device::createPipelineLayout(const PipelineLayoutInfo& info)
	{
		return new PipelineLayout(this, info);
	}

	Pipeline* Device::createPipeline(const PipelineInfo& info)
	{
		return new Pipeline(this, info);
	}

	DescriptorSetLayout* Device::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		return new DescriptorSetLayout(this, bindings);
	}

	DescriptorSet* Device::createDescriptorSet(const VkDescriptorSetLayout* layout)
	{
		return new DescriptorSet(this, layout);
	}

	VkPhysicalDevice Device::getPhysicalDevice()
	{
		return *physicalDevice;
	}

	/*
	* buffer
	*/
	Buffer::Buffer(Device* device, const BufferInfo& info) : device(device), info(info)
	{
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = info.size;
		bufferInfo.usage = info.usage;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = static_cast<VmaMemoryUsage>(info.memoryUsage);

		vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &handle, &allocation, nullptr);
	}

	Buffer::~Buffer()
	{
		vmaDestroyBuffer(device->getAllocator(), handle, allocation);
	}

	void* Buffer::map()
	{
		void* ptr = nullptr;
		vmaMapMemory(device->getAllocator(), allocation, &ptr);
		return ptr;
	}
	void Buffer::unmap()
	{
		vmaUnmapMemory(device->getAllocator(), allocation);
	}

	void Buffer::update(uint32_t size, const void* data)
	{
		if (info.memoryUsage == MemoryUsage::GPU) {
			const BufferInfo& stagingInfo = {info.size,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,MemoryUsage::CPU };
			auto staging = device->createBuffer(stagingInfo);
			staging->update(size, data);

			auto cmd = device->createCommandBuffer();
			cmd->begin();
			VkBufferCopy region = {0,0,size};
			vkCmdCopyBuffer(*cmd, *staging, handle, 1, &region);
			cmd->end();
			cmd->submit();
			cmd->waitIdle();

			delete staging;
		}
		else {
			auto ptr = map();
			memcpy(ptr, data, size);
			unmap();
		}
	}

	/*
	* image
	*/
	Image::Image(Device* device, const ImageInfo& info) : device(device), info(info)
	{
		VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		createInfo.extent = { info.width,info.height,1 };
		createInfo.format = info.format;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		createInfo.arrayLayers = info.layers;
		createInfo.mipLevels = info.mipLevels;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createInfo.imageType = info.type;
		createInfo.usage = info.usage;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(device->getAllocator(), &createInfo, &allocCreateInfo, &handle, &allocation, nullptr);
	}

	ImageView* Image::createView(VkImageViewType type)
	{
		return new ImageView(device, this, type);
	}

	void Image::update(const void* data)
	{
		const BufferInfo& bufInfo = { info.width * info.height * util::getFormatSize(info.format),VK_BUFFER_USAGE_TRANSFER_SRC_BIT,MemoryUsage::CPU };
		auto buf = device->createBuffer(bufInfo);
		buf->update(bufInfo.size, data);

		auto cmd = device->createCommandBuffer();
		cmd->begin();

		VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = handle;
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,info.mipLevels,0,info.layers };
		vkCmdPipelineBarrier(*cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		VkBufferImageCopy region = {};
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.layerCount = 1;
		region.imageExtent = { info.width,info.height,1 };
		vkCmdCopyBufferToImage(*cmd, *buf, handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkCmdPipelineBarrier(*cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		cmd->end();
		cmd->submit();
		cmd->waitIdle();

		delete buf;
	}


	/*
	* image view
	*/
	ImageView::ImageView(Device* device, Image* img, VkImageViewType type) : device(device),image(img),type(type)
	{
		VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		info.format = img->getInfo().format;
		info.image = *img;
		info.viewType = type;
		info.subresourceRange = {util::getImageAspectFlags(info.format),0,img->getInfo().mipLevels,0,img->getInfo().layers};
		
		vkCreateImageView(*device, &info, nullptr, &handle);
	}

	/*
	* sampler
	*/
	Sampler::Sampler(Device* device, const SamplerInfo& info)
	{
		VkSamplerCreateInfo createInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		createInfo.addressModeU = createInfo.addressModeV = createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		createInfo.minLod = -1000.f;
		createInfo.maxLod = 1000.f;
		createInfo.unnormalizedCoordinates = VK_FALSE;
		createInfo.minFilter = createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		createInfo.anisotropyEnable = VK_FALSE;
		createInfo.maxAnisotropy = 1.0f;
		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp = VK_COMPARE_OP_NEVER;

		vkCreateSampler(*device, &createInfo, nullptr, &handle);
	}

	/*
	* swapchain
	*/
	bool Swapchain::reCreate()
	{
		const VkPhysicalDevice& physicalDevice = device->getPhysicalDevice();

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, *info.surface, &formatCount, NULL);

		if (formatCount == 0) {
			log_error("this surface has no support format.");
			return false;
		}

		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, *info.surface, &formatCount, formats.data());

		VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM || format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				surfaceFormat = format;
				break;
			}
		}

		colorFormat = surfaceFormat.format;

		VkSurfaceCapabilitiesKHR surfCapabilities = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, *info.surface, &surfCapabilities);
		
		swapchainExtent.width = std::max<uint32_t>(surfCapabilities.currentExtent.width, 1u);
		swapchainExtent.height = std::max<uint32_t>(surfCapabilities.currentExtent.height, 1u);

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, *info.surface, &presentModeCount, NULL);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, *info.surface, &presentModeCount, presentModes.data());

		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		uint32_t desiredNumberOfSwapChainImages = 2;
		if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) == presentModes.end())
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			desiredNumberOfSwapChainImages = 3;
		}

		VkBool32 support = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, device->getQueueFamilyIndex(), *info.surface, &support);
		if (support != VK_TRUE) {
			log_error("surface is not support!");
			return false;
		}

		auto preTransform = (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfCapabilities.currentTransform;

		// Find a supported composite alpha mode - one of these is guaranteed to be set
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (uint32_t i = 0; i < _countof(compositeAlphaFlags); i++) {
			if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
				compositeAlpha = compositeAlphaFlags[i];
				break;
			}
		}

		VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		createInfo.surface = *info.surface;
		createInfo.minImageCount = desiredNumberOfSwapChainImages;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageExtent.width = swapchainExtent.width;
		createInfo.imageExtent.height = swapchainExtent.height;
		createInfo.preTransform = preTransform;
		createInfo.compositeAlpha = compositeAlpha;
		createInfo.imageArrayLayers = 1;
		createInfo.presentMode = swapchainPresentMode;
		createInfo.oldSwapchain = handle;
		createInfo.clipped = VK_TRUE;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		// vkCmdClearColorImage() command requires the image to use VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL layout
		// that requires  VK_IMAGE_USAGE_TRANSFER_DST_BIT to be set
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;

		VkSwapchainKHR obj = VK_NULL_HANDLE;
		if (vkCreateSwapchainKHR(*device, &createInfo, NULL, &obj) != VK_SUCCESS) {
			log_error("Failed to create Vulkan swapchain");
			return false;
		}

		if (handle != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(*device, handle, NULL);
		}
		handle = obj;

		uint32_t swapchainImageCount = 0;
		vkGetSwapchainImagesKHR(*device, handle, &swapchainImageCount, nullptr);
		std::vector<VkImage> deviceImage(swapchainImageCount);
		vkGetSwapchainImagesKHR(*device, handle, &swapchainImageCount, deviceImage.data());

		destroyImageView();
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			const ImageInfo info = { swapchainExtent.width,swapchainExtent.height,colorFormat,1,1,VK_IMAGE_TYPE_2D, createInfo.imageUsage };
			auto img = new Image(deviceImage[i], info);
			images.push_back(img);

			auto view = new ImageView(device, img);
			views.push_back(view);
		}

		if (!acquireSemaphore) {
			acquireSemaphore = device->createSemaphore();
		}

		presentQueue = device->getGraphicsQueue();

		return true;
	}

	void Swapchain::present(VkSemaphore wait)
	{
		VkPresentInfoKHR info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		info.pImageIndices = &imageIndex;
		info.pSwapchains = &handle;
		info.swapchainCount = 1;
		info.waitSemaphoreCount = wait == VK_NULL_HANDLE ? 0 : 1;
		info.pWaitSemaphores = wait == VK_NULL_HANDLE ? nullptr : &wait;;

		auto result = vkQueuePresentKHR(presentQueue, &info);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			reCreate();
		}
		else if (result != VK_SUCCESS)
		{
			log_error("Present failed");
		}
	}

	void Swapchain::acquireNextImage()
	{
		VkResult result;
		do {
			// Get the index of the next available swapchain image:
			result = vkAcquireNextImageKHR(*device, handle, UINT64_MAX, *acquireSemaphore, VK_NULL_HANDLE, &imageIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				// demo->swapchain is out of date (e.g. the window was resized) and
				// must be recreated:
				reCreate();
			}
			else if (result == VK_SUBOPTIMAL_KHR) {
				// demo->swapchain is not as optimal as it could be, but the platform's
				// presentation engine will still present the image correctly.
				break;
			}
			else if(result != VK_SUCCESS){
				log_error("Acquire Next Image failed");
			}
		} while (result != VK_SUCCESS);
	}

	void Swapchain::destroyImageView()
	{

	}


	/*
	* pipeline layout
	*/
	PipelineLayout::PipelineLayout(Device* device, const PipelineLayoutInfo& info) : device(device)
	{
		VkPipelineLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		createInfo.setLayoutCount = static_cast<uint32_t>(info.setLayouts.size());
		createInfo.pSetLayouts = info.setLayouts.data();
		createInfo.pushConstantRangeCount = static_cast<uint32_t>(info.pushConstant.size());
		createInfo.pPushConstantRanges = info.pushConstant.data();
		vkCreatePipelineLayout(*device, &createInfo, nullptr, &handle);
	}

	/*
	* pipeline
	*/
	Pipeline::Pipeline(Device* device, const PipelineInfo& info) : device(device),info(info)
	{
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		std::vector<Shader*> shaders;
		for (auto& it : info.shaders)
		{
			auto shader = new Shader(device, it);
			shaders.push_back(shader);
			VkPipelineShaderStageCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			info.stage = it.stage;
			info.module = *shader;
			info.pName = it.entry.c_str();
			stages.push_back(info);
		}

		VkPipelineVertexInputStateCreateInfo vertexInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		{
			vertexInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(info.vertexBindings.size());
			vertexInfo.pVertexBindingDescriptions = info.vertexBindings.data();
			vertexInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(info.vertexAttributes.size());
			vertexInfo.pVertexAttributeDescriptions = info.vertexAttributes.data();
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		inputAssemblyState.topology = info.topology;

		VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencilState.depthTestEnable = info.enableDepthTest;
		depthStencilState.depthWriteEnable = info.enableDepthWrite;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {
			info.enableColorBlend,
			VK_BLEND_FACTOR_SRC_ALPHA,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_FACTOR_ZERO,VK_BLEND_OP_ADD,
			0xf
		};
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachment;

		VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		std::vector<VkDynamicState> dynamics = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR };
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamics.size());
		dynamicState.pDynamicStates = dynamics.data();

		VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		createInfo.layout = info.layout;
		createInfo.renderPass = info.renderPass;
		createInfo.stageCount = static_cast<uint32_t>(stages.size());
		createInfo.pStages = stages.data();
		createInfo.pVertexInputState = &vertexInfo;
		createInfo.pInputAssemblyState = &inputAssemblyState;
		createInfo.pTessellationState = nullptr;
		createInfo.pViewportState = &viewportState;
		createInfo.pRasterizationState = &rasterizationState;
		createInfo.pMultisampleState = &multisampleState;
		createInfo.pDepthStencilState = &depthStencilState;
		createInfo.pColorBlendState = &colorBlendState;
		createInfo.pDynamicState = &dynamicState;

		vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle);

		for (auto& it : shaders)
		{
			delete it;
		}
	}

	/*
	* descriptor set layou
	*/
	DescriptorSetLayout::DescriptorSetLayout(Device* device, const std::vector<VkDescriptorSetLayoutBinding>& bindings) : device(device)
	{
		VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		createInfo.pBindings = bindings.data();

		vkCreateDescriptorSetLayout(*device, &createInfo, nullptr, &handle);
	}

	/*
	* descriptor set
	*/
	DescriptorSet::DescriptorSet(Device* device, const VkDescriptorSetLayout* layout) : device(device)
	{
		VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.descriptorPool = device->getDescriptorPool();
		allocateInfo.pSetLayouts = layout;
		vkAllocateDescriptorSets(*device, &allocateInfo, &handle);
	}

	void DescriptorSet::update(const std::vector<DescriptorSet::WriteInfo>& writeInfo)
	{
		std::vector<VkWriteDescriptorSet> writes;
		for (auto& it : writeInfo)
		{
			VkWriteDescriptorSet desc = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			desc.dstSet = handle;
			desc.descriptorCount = 1;
			desc.descriptorType = it.type;
			desc.pImageInfo = it.imageInfo;
			desc.pBufferInfo = it.bufferInfo;
			writes.push_back(desc);
		}
		vkUpdateDescriptorSets(*device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}
} // 
