#pragma once
#include "common.h"
#include "shader.h"

VK_DEFINE_HANDLE(VmaAllocator);
VK_DEFINE_HANDLE(VmaAllocation);

namespace vg::vk
{
	class Buffer;
	class Image;
	class ImageView;
	class Swapchain;
	class Semaphore;
	class Fence;
	class RenderPass;
	class FrameBuffer;
	class Surface;
	class PipelineLayout;
	class Pipeline;

	enum class MemoryUsage
	{
		UNKNOWN,
		GPU,
		CPU,
		CPU_GPU,
		GPU_CPU
	};

	struct BufferInfo
	{
		uint32_t size = 0;
		VkBufferUsageFlags usage;
		MemoryUsage memoryUsage;
	};

	struct SamplerInfo
	{

	};

	struct ImageInfo
	{
		uint32_t width;
		uint32_t height;
		VkFormat format = VK_FORMAT_UNDEFINED;
		uint32_t mipLevels = 1;
		uint32_t layers = 1;
		VkImageType type = VK_IMAGE_TYPE_2D;
		VkImageUsageFlags usage;
	};

	struct SwapchainInfo
	{
		Surface* surface;
	};

	struct RenderPassInfo
	{
		std::vector<VkAttachmentDescription> attachment;
		std::vector<VkSubpassDescription> subpass;
		std::vector<VkSubpassDependency> dependency;
	};

	struct FrameBufferInfo
	{
		uint32_t width = 0;
		uint32_t height = 0;
		VkRenderPass renderPass;
		std::vector<VkImageView> attachments;
	};

	struct PipelineLayoutInfo
	{
		std::vector<VkDescriptorSetLayout> setLayouts;
		std::vector<VkPushConstantRange> pushConstant;
	};

	struct PipelineInfo
	{
		VkRenderPass renderPass;
		VkPipelineLayout layout;
		std::vector<ShaderInfo> shaders;
		std::vector<VkVertexInputBindingDescription> vertexBindings;
		std::vector<VkVertexInputAttributeDescription> vertexAttributes;
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	};

	/*
	* device
	*/
    class Device : public __VkObject<VkDevice>
    {
    public:
        Device();
        
        class CommandPool* createCommandPool();

		Image* createImage(const ImageInfo& info);

		Swapchain* createSwapchain(const SwapchainInfo& info);

		class CommandBuffer* createCommandBuffer();

#if defined(VK_USE_PLATFORM_WIN32_KHR)
		Surface* createSurface(HWND hWnd);
#endif

		RenderPass* createRenderPass(const RenderPassInfo& info);

		FrameBuffer* createFrameBuffer(const FrameBufferInfo& info);

		Semaphore* createSemaphore();

		Fence* createFence();

		PipelineLayout* createPipelineLayout(const PipelineLayoutInfo& info);

		Pipeline* createPipeline(const PipelineInfo& info);

		inline VmaAllocator getAllocator() const {
			return allocator;
		}

		VkPhysicalDevice getPhysicalDevice();

		inline uint32_t getQueueFamilyIndex() const
		{
			return queueFamilyIndex;
		}

		inline VkQueue getGraphicsQueue() const
		{
			return graphicsQueue;
		}
    private:
        class Instance* instance;
        class PhysicalDevice* physicalDevice;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
		uint32_t queueFamilyIndex = 0;

		VmaAllocator allocator;
		class CommandPool* commandPool = nullptr;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    };


	/*
	* surface
	*/
	class Surface : public __VkObject<VkSurfaceKHR>
	{
	public:
		Surface(Device* device, VkSurfaceKHR surface) : __VkObject(surface), device(device) {}

	private:
		class Device* device = nullptr;
	};


	/*
	* buffer
	*/
	class Buffer : public __VkObject<VkBuffer>
	{
	public:
		Buffer(Device* device, const BufferInfo& info);

		~Buffer();

		void* map();
		void unmap();

		void update(uint32_t size, const void* data);

		const BufferInfo& getInfo() const
		{
			return info;
		}
	protected:
		class Device* device = nullptr;
		BufferInfo info = {};
		VmaAllocation allocation = VK_NULL_HANDLE;
	};

	/*
	* vertex buffer
	*/
	class VertexBuffer : public Buffer
	{
		using Buffer::Buffer;
	public:
		VertexBuffer(Device* device, uint32_t size, const std::vector<std::vector<VkFormat>>& formats);

		std::vector<VkVertexInputBindingDescription> getBindings()
		{
			return bindings;
		}

		std::vector<VkVertexInputAttributeDescription> getAttributes()
		{
			return attributes;
		}
	private:
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;
	};

	/*
	* index buffer
	*/
	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer();

	};

	/*
	* image
	*/
	class Image : public __VkObject<VkImage>
	{
	public:
		Image(VkImage img, const ImageInfo& info) : __VkObject(img), info(info) {}
		Image(Device* device, const ImageInfo& info);

		void transitionImageLayout(VkImage img, VkImageLayout oldLayout, VkImageLayout newLayout);

		const ImageInfo& getInfo() const
		{
			return info;
		}
	private:
		Device* device = nullptr;
		ImageInfo info = {};
		VmaAllocation allocation = VK_NULL_HANDLE;
	};


	/*
	* image view
	*/
	class ImageView : public __VkObject<VkImageView>
	{
	public:
		ImageView(Device* device,Image* img, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D);
	private:
		Device* device = nullptr;
		Image* image = nullptr;
		VkImageViewType type;
	};


	/*
	* semaphore
	*/
	class Semaphore : public __VkObject<VkSemaphore>
	{
	public:
		Semaphore(Device* device) : device(device)
		{
			VkSemaphoreCreateInfo info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			vkCreateSemaphore(*device, &info, nullptr, &handle);
		}
	private:
		Device* device = nullptr;
	};


	/*
	* fence
	*/
	class Fence : public __VkObject<VkFence>
	{
	public:
		Fence(Device* device) : device(device)
		{
			VkFenceCreateInfo info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
			info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			vkCreateFence(*device, &info, nullptr, &handle);
		}

		inline void wait() {
			vkWaitForFences(*device, 1, &handle, VK_TRUE, UINT64_MAX);
			vkResetFences(*device, 1, &handle);
		}
	private:
		Device* device = nullptr;
	};


	/*
	* swapchain
	*/
	class Swapchain : public __VkObject<VkSwapchainKHR>
	{
	public:
		Swapchain(Device* device, const SwapchainInfo& info) : device(device), info(info)
		{
			reCreate();
		}

		bool reCreate();

		inline const std::vector<ImageView*>& getImageViews() const
		{
			return views;
		}

		inline uint32_t getFrameIndex() const
		{
			return imageIndex;
		}

		inline const VkExtent2D getExtent() const
		{
			return swapchainExtent;
		}

		inline const uint32_t getWidth() const
		{
			return swapchainExtent.width;
		}

		inline const uint32_t getHeight() const
		{
			return swapchainExtent.height;
		}

		inline const VkFormat getColorFormat() const
		{
			return colorFormat;
		}

		inline VkSemaphore getAcquireSemaphore() const
		{
			return *acquireSemaphore;
		}

		void present(VkSemaphore wait = VK_NULL_HANDLE);
		void acquireNextImage();
	private:
		Device* device = nullptr;
		SwapchainInfo info = {};
		VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		uint32_t imageIndex = 0;
		VkExtent2D swapchainExtent = {};
		Semaphore* acquireSemaphore = nullptr;
		VkQueue presentQueue = VK_NULL_HANDLE;
		std::vector<Image*> images;
		std::vector<ImageView*> views;
		
		void destroyImageView();
	};
	

	/*
	* render pass
	*/
	class RenderPass : public __VkObject<VkRenderPass>
	{
	public:
		RenderPass(Device* device,const RenderPassInfo& info) : device(device),info(info)
		{
			VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
			createInfo.attachmentCount = static_cast<uint32_t>(info.attachment.size());
			createInfo.pAttachments = info.attachment.data();
			createInfo.subpassCount = static_cast<uint32_t>(info.subpass.size());
			createInfo.pSubpasses = info.subpass.data();
			createInfo.dependencyCount = static_cast<uint32_t>(info.dependency.size());
			createInfo.pDependencies = info.dependency.data();

			vkCreateRenderPass(*device, &createInfo, nullptr, &handle);
		}

	private:
		Device* device = nullptr;
		RenderPassInfo info = {};
	};

	/*
	* frame buffer
	*/
	class FrameBuffer : public __VkObject<VkFramebuffer>
	{
	public:
		FrameBuffer(Device* device, const FrameBufferInfo& info) : device(device), info(info)
		{
			VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			createInfo.width = info.width;
			createInfo.height = info.height;
			createInfo.renderPass = info.renderPass;
			createInfo.layers = 1;
			createInfo.attachmentCount = static_cast<uint32_t>(info.attachments.size());
			createInfo.pAttachments = info.attachments.data();

			vkCreateFramebuffer(*device, &createInfo, nullptr, &handle);
		}
	private:
		Device* device = nullptr;
		FrameBufferInfo info = {};
	};

	/*
	* pipeline layout
	*/
	class PipelineLayout : public __VkObject<VkPipelineLayout>
	{
	public:
		PipelineLayout(Device* device, const PipelineLayoutInfo& info);
	private:
		class Device* device = nullptr;
	};

	/*
	* pipeline
	*/
	class Pipeline : public __VkObject<VkPipeline>
	{
	public:
		Pipeline(Device* device, const PipelineInfo& info);
	private:
		class Device* device = nullptr;
		PipelineInfo info = {};
	};
}
