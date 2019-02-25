#pragma once

#include "vg.h"
#include "physicalDevice.h"
#include "util.h"

VK_DEFINE_HANDLE(VmaAllocator);
VK_DEFINE_HANDLE(VmaAllocation);

namespace vg::vk
{
	class Semaphore;
	class Image;
	class ImageView;
	class CommandPool;
	class CommandBuffer;
	class Buffer;
	
    class Device : public IDevice, public __VkObject<VkDevice>
    {
    public:
		~Device();

		bool setup(bool enableValidation);

		virtual ISwapchain* createSwapchain(void* nativeHandle) override;

        virtual IBuffer* createBuffer(const BufferDesc& desc,const void* data = nullptr) override
        {
            return nullptr;
        }

		virtual IImage* createImage(const ImageDesc& desc, const void* data = nullptr) override;

		virtual IRenderContext* createRenderContext() override;

		virtual IImageView* createAttachment(const AttachmentDesc& desc) override;

	public:
		Semaphore* createSemaphore();

		CommandBuffer* createCommandBuffer();

		inline const PhysicalDevice& getPhysicalDevice() const
		{
			return physicalDevice;
		}

		inline const VkQueue getGraphicsQueue() const
		{
			return graphicsQueue;
		}

		inline const VkDescriptorPool getDescriptorPool() const
		{
			return descriptorPool;
		}
    private:
        Instance instance;
        PhysicalDevice physicalDevice;
		VkQueue graphicsQueue = VK_NULL_HANDLE;
		uint32_t queueFamilyIndex = 0;

		VmaAllocator allocator;
		CommandPool* commandPool = nullptr;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

		void transitionImageLayout(Image& img, VkImageLayout oldLayout, VkImageLayout newLayout);
    };

	class Image : public IImage, public __VkObject<VkImage>
	{
		using IImage::IImage;
	public:
		~Image();

		inline void setInfo(const ImageDesc& desc)
		{
			width = desc.width;
			height = desc.height;
			mipLevel = desc.mipLevel;
			layer = desc.layer;
			format = desc.format;
		}

		inline void setup(const ImageDesc& desc, VkImage img)
		{
			setInfo(desc);
			setHandle(img);
		}

		bool setup(const ImageDesc& desc, VmaAllocator allocator, VkImageUsageFlags usage);

		virtual IImageView* createView(const ImageViewDesc& desc) override;

		virtual void update(uint32_t offset, uint32_t size, const void* data) override;
	private:
		VmaAllocator allocator;
		VmaAllocation allocation;
	};

	class ImageView : public IImageView, public __VkObject<VkImageView>
	{
		using IImageView::IImageView;
	public:
		~ImageView()
		{
			VkDevice device = getDevice<Device>();
			if (handle != VK_NULL_HANDLE) {
				vkDestroyImageView(device, handle, nullptr);
			}
			if (deviceSampler != VK_NULL_HANDLE) {
				vkDestroySampler(device, deviceSampler, nullptr);
			}
		}

		bool setup(Image* img, const ImageViewDesc& desc);

		inline const VkDescriptorImageInfo* getDescriptorInfo()
		{
			setupSampler();
			imageInfo.imageView = handle;
			imageInfo.sampler = deviceSampler;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			return &imageInfo;
		}

		inline VkDescriptorType getDescriptorType() const
		{
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
	private:
		VkSampler deviceSampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo imageInfo = {};

		void setupSampler()
		{
			VkSamplerCreateInfo info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
			info.addressModeU = info.addressModeV = info.addressModeW = Util::convertAddressMode(sampler.addressMode);
			info.anisotropyEnable = sampler.anisotropy>0.f;
			info.maxAnisotropy = sampler.anisotropy;
			info.minFilter = info.magFilter = Util::convertFilter(sampler.filter);
			info.minLod = -1000.f;
			info.maxLod = 1000.f;
			info.borderColor = Util::convertBorderColor(sampler.borderColor);
			info.compareEnable = sampler.compareEnable;
			info.compareOp = Util::convertCompareOp(sampler.compareOption);

			VkDevice device = getDevice<Device>();
			vkCreateSampler(device,&info,nullptr,&deviceSampler);
		}
	};

	class Semaphore : public __VkObject<VkSemaphore>
	{
		VkDevice device = VK_NULL_HANDLE;
	public:
		Semaphore(VkDevice dev) : device(dev)
		{
			VkSemaphoreCreateInfo info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			VkSemaphore semaphore = VK_NULL_HANDLE;
			vkCreateSemaphore(device, &info, nullptr, &handle);
		}

		~Semaphore()
		{
			if(handle != VK_NULL_HANDLE)vkDestroySemaphore(device, handle, nullptr);
		}
	};

	class CommandPool : public __VkObject<VkCommandPool>
	{
		VkDevice device = VK_NULL_HANDLE;
		VkQueue queue = VK_NULL_HANDLE;
		uint32_t queueFamilyIndex = 0;
	public:
		CommandPool(VkDevice dev,VkQueue queue,uint32_t index) : device(dev), queue(queue),queueFamilyIndex(index)
		{
			VkCommandPoolCreateInfo info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			info.queueFamilyIndex = index;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			vkCreateCommandPool(device, &info, nullptr, &handle);
		}

		~CommandPool()
		{
			if(handle != VK_NULL_HANDLE)vkDestroyCommandPool(device, handle, nullptr);
		}

		inline const uint32_t getFamilyIndex() const
		{
			return queueFamilyIndex;
		}

		inline const VkQueue getQueue() const
		{
			return queue;
		}
	};

	class CommandBuffer : public __VkObject<VkCommandBuffer>
	{
		VkDevice device = VK_NULL_HANDLE;
		CommandPool* pool = VK_NULL_HANDLE;
	public:
		CommandBuffer(VkDevice dev,CommandPool* pool,VkCommandBufferLevel level) : device(dev),pool(pool)
		{
			VkCommandBufferAllocateInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
			info.commandPool = *pool;
			info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			info.commandBufferCount = 1;

			vkAllocateCommandBuffers(device, &info, &handle);
		}

		~CommandBuffer()
		{
			if(handle != VK_NULL_HANDLE)vkFreeCommandBuffers(device,*pool,1,&handle);
		}

		const CommandPool* getPool() const
		{
			return pool;
		}

		void begin(VkCommandBufferUsageFlags flag)
		{
			VkCommandBufferBeginInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
			info.flags = flag;
			vkBeginCommandBuffer(handle,&info);
		}

		void end()
		{
			vkEndCommandBuffer(handle);
		}

		void beginRenderPass(const std::vector<VkClearValue>& clearValue,VkRenderPass renderPass,VkRect2D area,VkFramebuffer frameBuffer)
		{
			VkRenderPassBeginInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
			info.clearValueCount = static_cast<uint32_t>(clearValue.size());
			info.pClearValues = clearValue.data();
			info.renderPass = renderPass;
			info.renderArea = area;
			info.framebuffer = frameBuffer;
			vkCmdBeginRenderPass(handle,&info,VK_SUBPASS_CONTENTS_INLINE);
		}

		void endRenderPass()
		{
			vkCmdEndRenderPass(handle);
		}
	};

	class Buffer : public IBuffer, public __VkObject<VkBuffer>
	{
		using IBuffer::IBuffer;
	public:


		inline const VkDescriptorBufferInfo* getDescriptorInfo()
		{
			bufferInfo.buffer = handle;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;
			return &bufferInfo;
		}

		inline VkDescriptorType getDescriptorType() const
		{
			switch(getUsage())
            {
            case BufferUsage::Uniform:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case BufferUsage::Uniform_Dynamic:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            case BufferUsage::Storage:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case BufferUsage::Storage_Dynamic:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            }
		}
	private:
		VkBufferUsageFlags usage;
		VkDescriptorBufferInfo bufferInfo = {};
	};
}