#pragma once

#ifdef WIN32
#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <core/log.h>
#include <vector>
#include <array>
#include <assert.h>
#include <memory>

#include <vulkan/vulkan.h>

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)

namespace vg::vk
{
	class Instance_T;
	using Instance = std::unique_ptr<Instance_T>;
	class DebugCallback_T;
	using DebugCallback = std::unique_ptr<DebugCallback_T>;
	class Surface_T;
	using Surface = std::unique_ptr<Surface_T>;
	class Device_T;
	using Device = std::unique_ptr<Device_T>;
	class Queue_T;
	using Queue = std::unique_ptr<Queue_T>;
	class DescriptorPool_T;
	using DescriptorPool = std::unique_ptr<DescriptorPool_T>;
	class Image_T;
	using Image = std::unique_ptr<Image_T>;
	class Swapchain_T;
	using Swapchain = std::unique_ptr<Swapchain_T>;
	class RenderPass_T;
	using RenderPass = std::unique_ptr<RenderPass_T>;
	class Pipeline_T;
	using Pipeline = std::unique_ptr<Pipeline_T>;
	class DescriptorSetLayout_T;
	using DescriptorSetLayout = std::unique_ptr<DescriptorSetLayout_T>;
	class DescriptorSet_T;
	using DescriptorSet = std::unique_ptr<DescriptorSet_T>;
	class PipelineLayout_T;
	using PipelineLayout = std::unique_ptr<PipelineLayout_T>;
	class FrameBuffer_T;
	using FrameBuffer = std::unique_ptr<FrameBuffer_T>;
	class Fence_T;
	using Fence = std::unique_ptr<Fence_T>;
	class Semaphore_T;
	using Semaphore = std::unique_ptr<Semaphore_T>;
	class Buffer_T;
	using Buffer = std::unique_ptr<Buffer_T>;
	class Sampler_T;
	using Sampler = std::unique_ptr<Sampler_T>;
	class CommandPool_T;
	using CommandPool = std::unique_ptr<CommandPool_T>;
	class CommandBuffer_T;
	using CommandBuffer = std::unique_ptr<CommandBuffer_T>;

#define VK_CHECK_RESULT(result) assert(VK_SUCCESS == result);

	template <typename T> class ArrayProxy
	{
	public:
		constexpr ArrayProxy(std::nullptr_t) : m_count(0), m_ptr(nullptr) {}
		ArrayProxy(T& ptr) : m_count(1), m_ptr(&ptr) {}
		ArrayProxy(uint32_t count, T* ptr) : m_count(count), m_ptr(ptr) {}
		template <size_t N>
		ArrayProxy(std::array<typename std::remove_const<T>::type, N>& data) : m_count(N), m_ptr(data.data()) {}
		template <size_t N>
		ArrayProxy(std::array<typename std::remove_const<T>::type, N> const& data) : m_count(N), m_ptr(data.data()) {}

		template <class Allocator = std::allocator<typename std::remove_const<T>::type>>
		ArrayProxy(std::vector<typename std::remove_const<T>::type, Allocator> & data) : m_count(static_cast<uint32_t>(data.size())), m_ptr(data.data()) {}

		template <class Allocator = std::allocator<typename std::remove_const<T>::type>>
		ArrayProxy(std::vector<typename std::remove_const<T>::type, Allocator> const& data) : m_count(static_cast<uint32_t>(data.size())), m_ptr(data.data()) {}

		ArrayProxy(std::initializer_list<T> const& data) : m_count(static_cast<uint32_t>(data.end() - data.begin())), m_ptr(data.begin()) {}

		const T* begin() const { return m_ptr; }
		const T* end() const { return m_ptr + m_count; }
		const T& front() const { return *m_ptr; }
		const T& back() const { return *(m_ptr + m_count - 1); }
		bool empty() const { return (m_count == 0); }
		uint32_t size() const { return m_count; }
		T* data() const { return m_ptr; }
	private:
		uint32_t m_count;
		T* m_ptr;
	};

	template<typename Type> class Handle_T
	{
	public:
		Handle_T() {}
		Handle_T(Type handle) :handle_(handle) {}
		inline operator Type() const { return handle_; }
		inline Type get() const { return handle_; }
	protected:
		Type handle_ = VK_NULL_HANDLE;
	};

	class Instance_T : public Handle_T<VkInstance>
	{
	public:
		Instance_T(VkInstance instance) : Handle_T(instance) {}
		~Instance_T() { vkDestroyInstance(handle_, nullptr); }

		std::vector<VkPhysicalDevice> getPhysicalDevice()
		{
			uint32_t count = 0;
			vkEnumeratePhysicalDevices(handle_, &count, nullptr);
			std::vector<VkPhysicalDevice> physicalDevices(count);
			vkEnumeratePhysicalDevices(handle_, &count, physicalDevices.data());
			return std::move(physicalDevices);
		}

		Surface createSurface(const void* windowHandle) const;

		DebugCallback createDebugCallBack() const { return std::make_unique<DebugCallback_T>(this); }

		PFN_vkVoidFunction getProcAddr(const char* name) const {
			return vkGetInstanceProcAddr(handle_, name);
		}
	};

	class DebugCallback_T : public Handle_T<VkDebugReportCallbackEXT>
	{
	public:
		DebugCallback_T(const Instance_T* instance) : instance_(instance) {
			VkDebugReportCallbackCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
			info.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
			info.pfnCallback = &debugCallback;
			auto creator = (PFN_vkCreateDebugReportCallbackEXT)instance_->getProcAddr("vkCreateDebugReportCallbackEXT");
			creator(instance_->get(), &info,nullptr, &handle_);
		}

		~DebugCallback_T() {
			auto destroy = (PFN_vkDestroyDebugReportCallbackEXT)instance_->getProcAddr("vkDestroyDebugReportCallbackEXT");
			destroy(instance_->get(), handle_, nullptr);
		}
	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
			uint64_t object, size_t location, int32_t messageCode,
			const char* pLayerPrefix, const char* pMessage, void* pUserData) {
			log_error(pMessage);
			return VK_FALSE;
		}
		const Instance_T* instance_;
	};

	class Surface_T : public Handle_T<VkSurfaceKHR>
	{
	public:
		Surface_T(const Instance_T* instance, VkSurfaceKHR surface) : instance_(instance), Handle_T(surface) {}
		~Surface_T() { vkDestroySurfaceKHR(*instance_, handle_, nullptr); }
	private:
		const Instance_T* instance_;
	};

	class Device_T : public Handle_T<VkDevice>
	{
	public:
		Device_T(VkPhysicalDevice physicalDevice, VkDevice device);
		~Device_T() { vkDestroyDevice(handle_, nullptr); }
		VmaAllocator allocator() const { return allocator_; }
		operator VkPhysicalDevice() const { return physicalDevice_; }

		std::vector<VkSurfaceFormatKHR> getSurfaceFormat(VkSurfaceKHR surface) const
		{
			uint32_t count = 0;
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &count, nullptr));
			std::vector<VkSurfaceFormatKHR> formats(count);
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &count, formats.data()));
			return std::move(formats);
		}

		VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR surface) const
		{
			VkSurfaceCapabilitiesKHR capabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface, &capabilities);
			return capabilities;
		}

		VkPhysicalDeviceProperties getPhysicalDeviceProperties() {
			VkPhysicalDeviceProperties prop;
			vkGetPhysicalDeviceProperties(physicalDevice_, &prop);
			return prop;
		}

		CommandPool createCommandPool(uint32_t familyIndex) {
			return std::make_unique<CommandPool_T>(this, familyIndex);
		}

		DescriptorPool createDescriptorPool() {
			return std::make_unique<DescriptorPool_T>(this);
		}

		FrameBuffer createFrameBuffer(const RenderPass& renderPass, uint32_t width, uint32_t height, ArrayProxy<const VkImageView> attachments) {
			return std::make_unique<FrameBuffer_T>(this, renderPass.get(), width, height, attachments);
		}

		Swapchain createSwapchain(const Surface& surface) {
			return std::make_unique<Swapchain_T>(this, surface.get());
		}

		Fence createFence() {
			return std::make_unique<Fence_T>(this);
		}

		Semaphore createSemaphore() {
			return std::make_unique<Semaphore_T>(this);
		}

		Queue getQueue(uint32_t familyIndex,uint32_t index = 0) {
			VkQueue queue;
			vkGetDeviceQueue(handle_, familyIndex, index, &queue);
			return std::make_unique<Queue_T>(queue);
		}

		void waitForFences(ArrayProxy<const VkFence> fences,VkBool32 reset = VK_TRUE) {
			vkWaitForFences(handle_, fences.size(), fences.data(), VK_TRUE, std::numeric_limits<uint64_t>::max());
			if (reset) {
				vkResetFences(handle_, fences.size(), fences.data());
			}
		}

		void waitIdle() {
			VK_CHECK_RESULT(vkDeviceWaitIdle(handle_));
		}

		VkResult acquireNextImage(VkSwapchainKHR swapchain,uint32_t* index,VkSemaphore semaphore,VkFence fence = VkFence()) {
			return vkAcquireNextImageKHR(handle_, swapchain, std::numeric_limits<uint64_t>::max(), semaphore, fence, index);
		}

		Image createTexture2D(uint32_t width, uint32_t height, uint32_t mipLevels = 1, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
		Image createDepthStencilAttachment(uint32_t width, uint32_t height, VkSampleCountFlagBits sample = VK_SAMPLE_COUNT_1_BIT, VkFormat format = VK_FORMAT_D24_UNORM_S8_UINT);
		Image createColorAttachment(uint32_t width, uint32_t height, VkSampleCountFlagBits sample = VK_SAMPLE_COUNT_1_BIT, VkFormat format = VK_FORMAT_B8G8R8A8_UNORM);

		Buffer createUniformBuffer(VkDeviceSize size, VkBool32 dynamic = false);
		Buffer createVertexBuffer(VkDeviceSize size, VkBool32 dynamic = false);
		Buffer createIndexBuffer(VkDeviceSize size, VkBool32 dynamic = false);
	private:
		VkPhysicalDevice physicalDevice_;
		VmaAllocator allocator_;
	};

	class Queue_T : public Handle_T<VkQueue>
	{
	public:
		Queue_T(VkQueue queue) : Handle_T(queue) {}

		void submit(ArrayProxy<const VkCommandBuffer> cmds, ArrayProxy<const VkSemaphore> wait = nullptr, ArrayProxy<const VkSemaphore> signal = nullptr, VkFence fence = VkFence(), VkPipelineStageFlags waitStage = 0);

		void submit(CommandBuffer& cmd);

		VkResult present(ArrayProxy<const VkSwapchainKHR> swapchain, uint32_t* imageIndex, ArrayProxy<const VkSemaphore> wait);

		void waitIdle() { vkQueueWaitIdle(handle_); }

		Queue clone() const { return std::make_unique<Queue_T>(handle_); }
	};

	class RenderPass_T : public Handle_T<VkRenderPass>
	{
	public:
		RenderPass_T(const Device_T* device, const VkRenderPassCreateInfo& info) : device_(device){
			VK_CHECK_RESULT(vkCreateRenderPass(*device_, &info, nullptr, &handle_));
		}
		~RenderPass_T() { vkDestroyRenderPass(*device_,handle_,nullptr); }
	private:
		const Device_T* device_;
	};

	class DescriptorPool_T : public Handle_T<VkDescriptorPool>
	{
	public:
		DescriptorPool_T(const Device_T* device) : device_(device) {
			constexpr uint32_t per = 256;
			std::vector<VkDescriptorPoolSize> size = {
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,per},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,per},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,per},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,per},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,per}
			};
			VkDescriptorPoolCreateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
			info.poolSizeCount = static_cast<uint32_t>(size.size());
			info.pPoolSizes = size.data();
			info.maxSets = info.poolSizeCount * per;
			VK_CHECK_RESULT(vkCreateDescriptorPool(*device_, &info, nullptr, &handle_));
		}
		~DescriptorPool_T() { vkDestroyDescriptorPool(*device_, handle_, nullptr); }

		DescriptorSet createDescriptorSet(ArrayProxy<const VkDescriptorSetLayout> layouts) {
			return std::make_unique<DescriptorSet_T>(device_, this, layouts);
		}
	private:
		const Device_T* device_;
	};

	class Image_T : public Handle_T<VkImage>
	{
	public:
		Image_T(const Device_T* device, VkImageCreateInfo& info, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
		Image_T(const Device_T* device, VkImageCreateInfo& info, VkImage image, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
		~Image_T();
		inline VkImageView view() const { return view_; }

		void setLayout(CommandBuffer& cmd, VkImageLayout newLayout);

		void upload(CommandBuffer& cmd, const Buffer& staging);

		void upload(CommandPool& pool, Queue& queue, const void* value);
	private:
		void setupView(VkImageViewType viewType);

		const Device_T* device_;
		VkImageView view_ = VK_NULL_HANDLE;
		VmaAllocation allocation_ = VK_NULL_HANDLE;
		VkImageLayout layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageCreateInfo info_;
	};

	class Swapchain_T : public Handle_T<VkSwapchainKHR>
	{
	public:
		Swapchain_T(const Device_T* device, const Surface_T* surface);
		~Swapchain_T() { destroy(); }

		void destroy() {
			images_.swap(std::vector<Image>());
			vkDestroySwapchainKHR(*device_, handle_, nullptr);
		}

		bool reCreate();

		VkFormat getColorFormat() const { return colorFormat; }
		VkExtent2D getExtent() const { return extent; }

		uint32_t getImageCount() const { return static_cast<uint32_t>(images_.size()); }
		VkImageView getView(uint32_t index) const { return images_.at(index)->view(); }
	private:
		const Device_T* device_;
		const Surface_T* surface_;
		VkExtent2D extent = {};
		std::vector<Image> images_;
		VkFormat colorFormat;
	};

	class Pipeline_T : public Handle_T<VkPipeline>
	{
	public:
		Pipeline_T(const Device_T* device, const VkGraphicsPipelineCreateInfo& info) : device_(device) {
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(*device_, VkPipelineCache(), 1, &info, nullptr, &handle_));
		}
		Pipeline_T(const Device_T* device, const VkComputePipelineCreateInfo& info) : device_(device) {
			VK_CHECK_RESULT(vkCreateComputePipelines(*device_, VkPipelineCache(), 1, &info, nullptr, &handle_));
		}
		~Pipeline_T() { vkDestroyPipeline(*device_, handle_, nullptr); }
	private:
		const Device_T* device_;
	};

	class DescriptorSetLayout_T : public Handle_T<VkDescriptorSetLayout>
	{
	public:
		DescriptorSetLayout_T(const Device_T* device, const VkDescriptorSetLayoutCreateInfo& info) : device_(device) {
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*device_, &info, nullptr, &handle_));
		}
		~DescriptorSetLayout_T() { vkDestroyDescriptorSetLayout(*device_, handle_, nullptr); }
	private:
		const Device_T* device_;
	};

	class DescriptorSet_T : public Handle_T<VkDescriptorSet>
	{
	public:
		DescriptorSet_T(const Device_T* device, const DescriptorPool_T* pool, ArrayProxy<const VkDescriptorSetLayout> layouts) : device_(device), pool_(pool) {
			VkDescriptorSetAllocateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			info.descriptorPool = *pool_;
			info.descriptorSetCount = 1;
			info.pSetLayouts = layouts.data();
			VK_CHECK_RESULT(vkAllocateDescriptorSets(*device_, &info, &handle_));
		}
		~DescriptorSet_T() { vkFreeDescriptorSets(*device_, *pool_,1, &handle_); }
	private:
		const Device_T* device_;
		const DescriptorPool_T* pool_;
	};

	class PipelineLayout_T : public Handle_T<VkPipelineLayout>
	{
	public:
		PipelineLayout_T(const Device_T* device,const VkPipelineLayoutCreateInfo& info) : device_(device) {
			VK_CHECK_RESULT(vkCreatePipelineLayout(*device_, &info, nullptr, &handle_));
		}
		~PipelineLayout_T() { vkDestroyPipelineLayout(*device_, handle_, nullptr); }
	private:
		const Device_T* device_;
	};

	class FrameBuffer_T : public Handle_T<VkFramebuffer>
	{
	public:
		FrameBuffer_T(const Device_T* device,const RenderPass_T* renderPass,uint32_t width,uint32_t height,ArrayProxy<const VkImageView> attachments) : device_(device) {
			VkFramebufferCreateInfo info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			info.renderPass = *renderPass;
			info.width = width;
			info.height = height;
			info.layers = 1;
			info.attachmentCount = attachments.size();
			info.pAttachments = attachments.data();

			VK_CHECK_RESULT(vkCreateFramebuffer(*device_, &info, nullptr, &handle_));
		}
		~FrameBuffer_T() { vkDestroyFramebuffer(*device_, handle_, nullptr); }
	private:
		const Device_T* device_;
	};

	class Fence_T : public Handle_T<VkFence>
	{
	public:
		Fence_T(const Device_T* device) : device_(device) {
			VkFenceCreateInfo info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
			info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			VK_CHECK_RESULT(vkCreateFence(*device_, &info, nullptr, &handle_));
		}
		~Fence_T() { vkDestroyFence(*device_, handle_, nullptr); }
	private:
		const Device_T* device_;
	};

	class Semaphore_T : public Handle_T<VkSemaphore>
	{
	public:
		Semaphore_T(const Device_T* device) : device_(device) {
			VkSemaphoreCreateInfo info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			VK_CHECK_RESULT(vkCreateSemaphore(*device_, &info, nullptr, &handle_));
		}
		~Semaphore_T() { vkDestroySemaphore(*device_, handle_, nullptr); }
	private:
		const Device_T* device_;
	};

	class Buffer_T : public Handle_T<VkBuffer>
	{
	public:
		enum class Type
		{
			UNKNOWN,
			GPU_ONLY,
			CPU_ONLY,
			CPU_TO_GPU,
			GPU_TO_CPU
		};

		Buffer_T(const Device_T* device,VkDeviceSize size, VkBufferUsageFlags usage, Type type);
		~Buffer_T();

		void uploadLocal(const void* value);
		void upload(CommandBuffer& cmd, const Buffer& staging);
		void upload(CommandPool& pool, Queue& queue, const void* value);

		void* map();
		void unmap();
		void flush();

		VkDeviceSize size() const { return size_; }
	private:
		const Device_T* device_;
		VmaAllocation allocation_;
		VkDeviceSize size_ = 0;
	};

	class Sampler_T : public Handle_T<VkSampler>
	{
	public:
		Sampler_T(const Device_T* device, const VkSamplerCreateInfo& info) :device_(device) {
			VK_CHECK_RESULT(vkCreateSampler(*device_, &info, nullptr, &handle_));
		}
		~Sampler_T() { vkDestroySampler(*device_, handle_, nullptr); }
	private:
		const Device_T* device_;
	};

	class CommandBuffer_T : public Handle_T<VkCommandBuffer>
	{
	public:
		CommandBuffer_T(const Device_T* device, const CommandPool_T* pool);
		~CommandBuffer_T();

		void pipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
			ArrayProxy<const VkMemoryBarrier> memoryBarriers,
			ArrayProxy<const VkBufferMemoryBarrier> bufferMemoryBarriers,
			ArrayProxy<const VkImageMemoryBarrier> imageMemoryBarriers)
		{
			vkCmdPipelineBarrier(handle_, srcStageMask, dstStageMask, dependencyFlags,
				memoryBarriers.size(), memoryBarriers.data(),
				bufferMemoryBarriers.size(), bufferMemoryBarriers.data(),
				imageMemoryBarriers.size(), imageMemoryBarriers.data());
		}

		void begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) {
			VkCommandBufferBeginInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			info.flags = usage;
			VK_CHECK_RESULT(vkBeginCommandBuffer(handle_, &info));
		}
		void end() {
			VK_CHECK_RESULT(vkEndCommandBuffer(handle_));
		}
		void beginRenderPass(const RenderPass& renderPass,const FrameBuffer& frameBuffer, VkRect2D area, ArrayProxy<const VkClearValue> clear) {
			VkRenderPassBeginInfo info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
			info.renderArea = area;
			info.clearValueCount = clear.size();
			info.pClearValues = clear.data();
			info.renderPass = renderPass->get();
			info.framebuffer = frameBuffer->get();
			vkCmdBeginRenderPass(handle_, &info, VK_SUBPASS_CONTENTS_INLINE);
		}
		void endRenderPass() {
			vkCmdEndRenderPass(handle_);
		}

		void bindPipeline(const Pipeline& pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS)
		{
			vkCmdBindPipeline(handle_, bindPoint, *pipeline);
		}

		void bindVertexBuffer(uint32_t first, ArrayProxy<const VkBuffer> buffer, ArrayProxy<const VkDeviceSize> offset) {
			vkCmdBindVertexBuffers(handle_, 0, buffer.size(), buffer.data(), offset.data());
		}

		void bindIndexBuffer(VkBuffer buffer, uint32_t offset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT16) {
			vkCmdBindIndexBuffer(handle_, buffer, offset, indexType);
		}

		void bindDescriptorSet(PipelineLayout& layout, uint32_t first, ArrayProxy<const VkDescriptorSet> sets, ArrayProxy<uint32_t> offset = nullptr, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) {
			vkCmdBindDescriptorSets(handle_, bindPoint, *layout, first, sets.size(), sets.data(), offset.size(), offset.data());
		}

		void pushContants(PipelineLayout& layout, VkShaderStageFlags stage, uint32_t offset, uint32_t size, const void* value) {
			vkCmdPushConstants(handle_, *layout, stage, offset, size, value);
		}

		template<typename T> void pushContants(PipelineLayout& layout, VkShaderStageFlags stage, uint32_t offset, const T& value) {
			vkCmdPushConstants(handle_, *layout, stage, offset, sizeof(T), &value);
		}

		template<typename T1, typename T2, typename T3, typename T4> void viewport(T1 x, T2 y, T3 width, T4 height) {
			VkViewport value = { static_cast<float>(x),static_cast<float>(y), static_cast<float>(width), static_cast<float>(height),0.0f,1.0f };
			vkCmdSetViewport(handle_, 0, 1, &value);
		}

		template<typename T1, typename T2, typename T3, typename T4> void scissor(T1 x, T2 y, T3 width, T4 height) {
			VkRect2D value = { {static_cast<int32_t>(x),static_cast<int32_t>(y)},{static_cast<uint32_t>(width),static_cast<uint32_t>(height)} };
			vkCmdSetScissor(handle_, 0, 1, &value);
		}

		void lineWidth(float width)
		{
			vkCmdSetLineWidth(handle_, width);
		}

		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0) {
			vkCmdDraw(handle_, vertexCount, instanceCount, firstVertex, firstInstance);
		}

		void drawIndexd(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) {
			vkCmdDrawIndexed(handle_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}

		void copyBuffer(VkBuffer src, VkBuffer dst, ArrayProxy<const VkBufferCopy> region) {
			vkCmdCopyBuffer(handle_, src, dst, region.size(), region.data());
		}

		void copyBufferToImage(VkBuffer src, VkImage dst, VkImageLayout layout, ArrayProxy<const VkBufferImageCopy> region) {
			vkCmdCopyBufferToImage(handle_, src, dst, layout, region.size(), region.data());
		}
	private:
		const Device_T* device_;
		const CommandPool_T* pool_;
	};

	class CommandPool_T : public Handle_T<VkCommandPool>
	{
	public:
		CommandPool_T(const Device_T* device, uint32_t familyIndex) : device_(device), familyIndex_(familyIndex) {
			VkCommandPoolCreateInfo info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.queueFamilyIndex = familyIndex_;
			VK_CHECK_RESULT(vkCreateCommandPool(*device_, &info, nullptr, &handle_));
		}
		~CommandPool_T() { vkDestroyCommandPool(*device_, handle_, nullptr); }

		inline CommandBuffer createCommandBuffer() {
			return std::make_unique<CommandBuffer_T>(device_, this);
		}
	private:
		const Device_T* device_;
		uint32_t familyIndex_;
	};

	class InstanceMaker
	{
	public:
		InstanceMaker()
		{
			layers_.emplace_back("VK_LAYER_LUNARG_standard_validation");
			extensions_.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#ifdef WIN32
			extensions_.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
			extensions_.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

			app_info_.pApplicationName = "vg";
			app_info_.apiVersion = VK_API_VERSION_1_1;
			app_info_.engineVersion = VK_MAKE_VERSION(0, 1, 0);
			app_info_.pEngineName = "vg";
			app_info_.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
		}
		InstanceMaker& layer(const char* layer)
		{
			layers_.push_back(layer);
			return *this;
		}

		InstanceMaker& extension(const char* layer)
		{
			extensions_.push_back(layer);
			return *this;
		}

		InstanceMaker& applicationName(const char* pApplicationName_)
		{
			app_info_.pApplicationName = pApplicationName_;
			return *this;
		}

		InstanceMaker& applicationVersion(uint32_t applicationVersion_)
		{
			app_info_.applicationVersion = applicationVersion_;
			return *this;
		}

		InstanceMaker& engineName(const char* pEngineName_)
		{
			app_info_.pEngineName = pEngineName_;
			return *this;
		}

		InstanceMaker& engineVersion(uint32_t engineVersion_)
		{
			app_info_.engineVersion = engineVersion_;
			return *this;
		}

		InstanceMaker& apiVersion(uint32_t apiVersion_)
		{
			app_info_.apiVersion = apiVersion_;
			return *this;
		}

		Instance create()
		{
			VkInstanceCreateInfo instance_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
			instance_info.pApplicationInfo = &app_info_;
			instance_info.enabledLayerCount = static_cast<uint32_t>(layers_.size());
			instance_info.ppEnabledLayerNames = layers_.data();
			instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions_.size());
			instance_info.ppEnabledExtensionNames = extensions_.data();
			VkInstance instance = VK_NULL_HANDLE;
			VK_CHECK_RESULT(vkCreateInstance(&instance_info, nullptr, &instance));
			return std::make_unique<Instance_T>(instance);
		}

	private:
		std::vector<const char*> layers_;
		std::vector<const char*> extensions_;
		VkApplicationInfo app_info_;
	};

	class DeviceMaker
	{
	public:
		DeviceMaker()
		{
			layers_.emplace_back("VK_LAYER_LUNARG_standard_validation");
			extensions_.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		DeviceMaker& features(const VkPhysicalDeviceFeatures& feature)
		{
			features_ = feature;
			return *this;
		}

		DeviceMaker& extension(const char* layer)
		{
			extensions_.emplace_back(layer);
			return *this;
		}

		DeviceMaker& queue(uint32_t familyIndex, float priority = 0.0f, uint32_t n = 1)
		{
			queue_priorities_.emplace_back(n, priority);

			VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			info.queueFamilyIndex = familyIndex;
			info.queueCount = n;
			info.pQueuePriorities = queue_priorities_.back().data();
			qci_.emplace_back(info);

			return *this;
		}

		Device create(VkPhysicalDevice physical_device)
		{
			VkDeviceCreateInfo device_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
			device_info.queueCreateInfoCount = static_cast<uint32_t>(qci_.size());
			device_info.pQueueCreateInfos = qci_.data();
			device_info.enabledExtensionCount = static_cast<uint32_t>(extensions_.size());
			device_info.ppEnabledExtensionNames = extensions_.data();
			device_info.enabledLayerCount = static_cast<uint32_t>(layers_.size());
			device_info.ppEnabledLayerNames = layers_.data();
			device_info.pEnabledFeatures = &features_;
			VkDevice device = VK_NULL_HANDLE;
			VK_CHECK_RESULT(vkCreateDevice(physical_device, &device_info, nullptr, &device));

			return std::make_unique<Device_T>(physical_device, device);
		}

	private:
		std::vector<const char*> layers_;
		std::vector<const char*> extensions_;
		VkPhysicalDeviceFeatures features_;
		std::vector<std::vector<float>> queue_priorities_;
		std::vector<VkDeviceQueueCreateInfo> qci_;
	};

	class RenderpassMaker {
	public:
		RenderpassMaker() {}

		void attachmentBegin(VkFormat format,VkImageLayout finalLayout, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED) {
			VkAttachmentDescription desc = {};
			desc.format = format;
			desc.finalLayout = finalLayout;
			desc.initialLayout = initialLayout;
			desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			s.attachmentDescriptions.emplace_back(desc);
		}

		void attachmentSamples(VkSampleCountFlagBits value) { s.attachmentDescriptions.back().samples = value; };
		void attachmentLoadOp(VkAttachmentLoadOp value) { s.attachmentDescriptions.back().loadOp = value; };
		void attachmentStoreOp(VkAttachmentStoreOp value) { s.attachmentDescriptions.back().storeOp = value; };
		void attachmentStencilLoadOp(VkAttachmentLoadOp value) { s.attachmentDescriptions.back().stencilLoadOp = value; };
		void attachmentStencilStoreOp(VkAttachmentStoreOp value) { s.attachmentDescriptions.back().stencilStoreOp = value; };

		void subpassBegin(VkPipelineBindPoint bp = VK_PIPELINE_BIND_POINT_GRAPHICS) {
			VkSubpassDescription desc{};
			desc.pipelineBindPoint = bp;
			s.subpassDescriptions.emplace_back(desc);
		}

		void subpassColorAttachment(VkImageLayout layout, uint32_t attachment) {
			VkSubpassDescription& subpass = s.subpassDescriptions.back();
			auto* p = getAttachmentReference();
			p->layout = layout;
			p->attachment = attachment;
			if (subpass.colorAttachmentCount == 0) {
				subpass.pColorAttachments = p;
			}
			subpass.colorAttachmentCount++;
		}

		void subpassResolveAttachment(VkImageLayout layout, uint32_t attachment) {
			VkSubpassDescription& subpass = s.subpassDescriptions.back();
			auto* p = getAttachmentReference();
			p->layout = layout;
			p->attachment = attachment;
			subpass.pResolveAttachments = p;
		}

		void subpassDepthStencilAttachment(VkImageLayout layout, uint32_t attachment) {
			VkSubpassDescription& subpass = s.subpassDescriptions.back();
			auto* p = getAttachmentReference();
			p->layout = layout;
			p->attachment = attachment;
			subpass.pDepthStencilAttachment = p;
		}

		RenderPass create(const Device& device) const {
			VkRenderPassCreateInfo info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
			info.attachmentCount = (uint32_t)s.attachmentDescriptions.size();
			info.pAttachments = s.attachmentDescriptions.data();
			info.subpassCount = (uint32_t)s.subpassDescriptions.size();
			info.pSubpasses = s.subpassDescriptions.data();
			info.dependencyCount = (uint32_t)s.subpassDependencies.size();
			info.pDependencies = s.subpassDependencies.data();
			return std::make_unique<RenderPass_T>(device.get(), info);
		}

		void dependencyBegin(uint32_t srcSubpass, uint32_t dstSubpass) {
			VkSubpassDependency desc{};
			desc.srcSubpass = srcSubpass;
			desc.dstSubpass = dstSubpass;
			s.subpassDependencies.emplace_back(desc);
		}

		void dependencySrcSubpass(uint32_t value) { s.subpassDependencies.back().srcSubpass = value; };
		void dependencyDstSubpass(uint32_t value) { s.subpassDependencies.back().dstSubpass = value; };
		void dependencySrcStageMask(VkPipelineStageFlags value) { s.subpassDependencies.back().srcStageMask = value; };
		void dependencyDstStageMask(VkPipelineStageFlags value) { s.subpassDependencies.back().dstStageMask = value; };
		void dependencySrcAccessMask(VkAccessFlags value) { s.subpassDependencies.back().srcAccessMask = value; };
		void dependencyDstAccessMask(VkAccessFlags value) { s.subpassDependencies.back().dstAccessMask = value; };
		void dependencyDependencyFlags(VkDependencyFlags value) { s.subpassDependencies.back().dependencyFlags = value; };
	private:
		constexpr static int max_refs = 64;

		VkAttachmentReference* getAttachmentReference() {
			return (s.num_refs < max_refs) ? &s.attachmentReferences[s.num_refs++] : nullptr;
		}

		struct State {
			std::vector<VkAttachmentDescription> attachmentDescriptions;
			std::vector<VkSubpassDescription> subpassDescriptions;
			std::vector<VkSubpassDependency> subpassDependencies;
			std::array<VkAttachmentReference, max_refs> attachmentReferences;
			int num_refs = 0;
			bool ok_ = false;
		}s;
	};

	class PipelineMaker {
	public:
		PipelineMaker(const Device& device) : device_(device.get()) 
		{
			inputAssemblyState_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			rasterizationState_.lineWidth = 1.0f;

			depthStencilState_.depthTestEnable = VK_FALSE;
			depthStencilState_.depthWriteEnable = VK_FALSE;
			depthStencilState_.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilState_.depthBoundsTestEnable = VK_FALSE;
			depthStencilState_.back.failOp = VK_STENCIL_OP_KEEP;
			depthStencilState_.back.passOp = VK_STENCIL_OP_KEEP;
			depthStencilState_.back.compareOp = VK_COMPARE_OP_ALWAYS;
			depthStencilState_.stencilTestEnable = VK_FALSE;
			depthStencilState_.front = depthStencilState_.back;
			depthStencilState_.maxDepthBounds = 1.0f;
		}

		Pipeline create(const PipelineLayout& pipelineLayout, const RenderPass& renderPass) {
			auto count = (uint32_t)colorBlendAttachments_.size();
			colorBlendState_.attachmentCount = count;
			colorBlendState_.pAttachments = count ? colorBlendAttachments_.data() : nullptr;

			VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport_;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor_;

			VkPipelineVertexInputStateCreateInfo vertexInputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
			vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexAttributeDescriptions_.size();
			vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescriptions_.data();
			vertexInputState.vertexBindingDescriptionCount = (uint32_t)vertexBindingDescriptions_.size();
			vertexInputState.pVertexBindingDescriptions = vertexBindingDescriptions_.data();

			VkPipelineDynamicStateCreateInfo dynState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			dynState.dynamicStateCount = (uint32_t)dynamicState_.size();
			dynState.pDynamicStates = dynamicState_.data();

			VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
			pipelineInfo.pVertexInputState = &vertexInputState;
			pipelineInfo.stageCount = (uint32_t)modules_.size();
			pipelineInfo.pStages = modules_.data();
			pipelineInfo.pInputAssemblyState = &inputAssemblyState_;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizationState_;
			pipelineInfo.pMultisampleState = &multisampleState_;
			pipelineInfo.pColorBlendState = &colorBlendState_;
			pipelineInfo.pDepthStencilState = &depthStencilState_;
			pipelineInfo.layout = pipelineLayout->get();
			pipelineInfo.renderPass = renderPass->get();
			pipelineInfo.pDynamicState = dynamicState_.empty() ? nullptr : &dynState;
			pipelineInfo.subpass = subpass_;

			auto result = std::make_unique<Pipeline_T>(device_, pipelineInfo);

			for (auto& shader : modules_)
			{
				vkDestroyShaderModule(*device_, shader.module, nullptr);
			}

			return std::move(result);
		}

		PipelineMaker& shader(VkShaderStageFlagBits stage,size_t size,const uint32_t* code) {
			VkShaderModuleCreateInfo shaderInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			shaderInfo.codeSize = size;
			shaderInfo.pCode = code;
			VkShaderModule module;
			VK_CHECK_RESULT(vkCreateShaderModule(*device_, &shaderInfo, nullptr, &module));

			VkPipelineShaderStageCreateInfo info = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
			info.module = module;
			info.pName = "main";
			info.stage = stage;
			modules_.emplace_back(info);

			return *this;
		}

		PipelineMaker& shaderGLSL(VkShaderStageFlagBits stage, const std::string& src);

		PipelineMaker& subPass(uint32_t subpass) { subpass_ = subpass; return *this; }

		PipelineMaker& defaultBlend(VkBool32 enable) {
			VkPipelineColorBlendAttachmentState blend = {};
			blend.blendEnable = enable;
			blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			blend.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			blend.colorBlendOp = VK_BLEND_OP_ADD;
			blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blend.alphaBlendOp = VK_BLEND_OP_ADD;
			blend.colorWriteMask = 0xf;
			colorBlendAttachments_.push_back(blend);
			return *this;
		}
		PipelineMaker& defaultDynamic(ArrayProxy<const VkDynamicState> dynamic = nullptr) {
			dynamicState_.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
			dynamicState_.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
			dynamicState_.insert(dynamicState_.end(),dynamic.begin(), dynamic.end());
			return *this;
		}

		PipelineMaker& blendBegin(VkBool32 enable) {
			colorBlendAttachments_.emplace_back();
			auto& blend = colorBlendAttachments_.back();
			blend.blendEnable = enable;
			blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blend.colorBlendOp = VK_BLEND_OP_ADD;
			blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blend.alphaBlendOp = VK_BLEND_OP_ADD;
			blend.colorWriteMask = 0xf;
			return *this;
		}

		PipelineMaker& blendEnable(VkBool32 value) { colorBlendAttachments_.back().blendEnable = value; return *this;}

		PipelineMaker& blendSrcColorBlendFactor(VkBlendFactor value) { colorBlendAttachments_.back().srcColorBlendFactor = value; return *this;}

		PipelineMaker& blendDstColorBlendFactor(VkBlendFactor value) { colorBlendAttachments_.back().dstColorBlendFactor = value; return *this;}

		PipelineMaker& blendColorBlendOp(VkBlendOp value) { colorBlendAttachments_.back().colorBlendOp = value; return *this;}

		PipelineMaker& blendSrcAlphaBlendFactor(VkBlendFactor value) { colorBlendAttachments_.back().srcAlphaBlendFactor = value; return *this;}

		PipelineMaker& blendDstAlphaBlendFactor(VkBlendFactor value) { colorBlendAttachments_.back().dstAlphaBlendFactor = value; return *this;}

		PipelineMaker& blendAlphaBlendOp(VkBlendOp value) { colorBlendAttachments_.back().alphaBlendOp = value; return *this;}

		PipelineMaker& blendColorWriteMask(VkColorComponentFlags value) { colorBlendAttachments_.back().colorWriteMask = value; return *this;}

		PipelineMaker& vertexAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset) {
			vertexAttributeDescriptions_.push_back({ location, binding, format, offset });
			return *this;
		}

		PipelineMaker& vertexAttribute(ArrayProxy<const VkVertexInputAttributeDescription> value) { 
			vertexAttributeDescriptions_.insert(vertexAttributeDescriptions_.end(),value.begin(),value.end());
			return *this; 
		}

		PipelineMaker& vertexBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX) {
			vertexBindingDescriptions_.push_back({ binding, stride, inputRate });
			return *this;
		}

		PipelineMaker& vertexBinding(ArrayProxy<const VkVertexInputBindingDescription> value) {
			vertexBindingDescriptions_.insert(vertexBindingDescriptions_.end(), value.begin(), value.end());
			return *this; 
		}

		PipelineMaker& topology(VkPrimitiveTopology topology) { inputAssemblyState_.topology = topology; return *this; }

		PipelineMaker& primitiveRestartEnable(VkBool32 primitiveRestartEnable) { inputAssemblyState_.primitiveRestartEnable = primitiveRestartEnable; return *this; }

		PipelineMaker& viewport(const VkViewport& value) { viewport_ = value; return *this; }

		PipelineMaker& scissor(const VkRect2D& value) { scissor_ = value; return *this; }

		PipelineMaker& depthClampEnable(VkBool32 value) { rasterizationState_.depthClampEnable = value; return *this; }
		PipelineMaker& rasterizerDiscardEnable(VkBool32 value) { rasterizationState_.rasterizerDiscardEnable = value; return *this; }
		PipelineMaker& polygonMode(VkPolygonMode value) { rasterizationState_.polygonMode = value; return *this; }
		PipelineMaker& cullMode(VkCullModeFlags value) { rasterizationState_.cullMode = value; return *this; }
		PipelineMaker& frontFace(VkFrontFace value) { rasterizationState_.frontFace = value; return *this; }
		PipelineMaker& depthBiasEnable(VkBool32 value) { rasterizationState_.depthBiasEnable = value; return *this; }
		PipelineMaker& depthBiasConstantFactor(float value) { rasterizationState_.depthBiasConstantFactor = value; return *this; }
		PipelineMaker& depthBiasClamp(float value) { rasterizationState_.depthBiasClamp = value; return *this; }
		PipelineMaker& depthBiasSlopeFactor(float value) { rasterizationState_.depthBiasSlopeFactor = value; return *this; }
		PipelineMaker& lineWidth(float value) { rasterizationState_.lineWidth = value; return *this; }


		PipelineMaker& rasterizationSamples(VkSampleCountFlagBits value) { multisampleState_.rasterizationSamples = value; return *this; }
		PipelineMaker& sampleShadingEnable(VkBool32 value) { multisampleState_.sampleShadingEnable = value; return *this; }
		PipelineMaker& minSampleShading(float value) { multisampleState_.minSampleShading = value; return *this; }
		PipelineMaker& pSampleMask(const VkSampleMask* value) { multisampleState_.pSampleMask = value; return *this; }
		PipelineMaker& alphaToCoverageEnable(VkBool32 value) { multisampleState_.alphaToCoverageEnable = value; return *this; }
		PipelineMaker& alphaToOneEnable(VkBool32 value) { multisampleState_.alphaToOneEnable = value; return *this; }

		PipelineMaker& depthTestEnable(VkBool32 value) { depthStencilState_.depthTestEnable = value; return *this; }
		PipelineMaker& depthWriteEnable(VkBool32 value) { depthStencilState_.depthWriteEnable = value; return *this; }
		PipelineMaker& depthCompareOp(VkCompareOp value) { depthStencilState_.depthCompareOp = value; return *this; }
		PipelineMaker& depthBoundsTestEnable(VkBool32 value) { depthStencilState_.depthBoundsTestEnable = value; return *this; }
		PipelineMaker& stencilTestEnable(VkBool32 value) { depthStencilState_.stencilTestEnable = value; return *this; }
		PipelineMaker& front(VkStencilOpState value) { depthStencilState_.front = value; return *this; }
		PipelineMaker& back(VkStencilOpState value) { depthStencilState_.back = value; return *this; }
		PipelineMaker& minDepthBounds(float value) { depthStencilState_.minDepthBounds = value; return *this; }
		PipelineMaker& maxDepthBounds(float value) { depthStencilState_.maxDepthBounds = value; return *this; }

		PipelineMaker& logicOpEnable(VkBool32 value) { colorBlendState_.logicOpEnable = value; return *this; }
		PipelineMaker& logicOp(VkLogicOp value) { colorBlendState_.logicOp = value; return *this; }
		PipelineMaker& blendConstants(float value[4]) { memcpy(colorBlendState_.blendConstants,value,sizeof(float)*4); return *this; }
		 
		PipelineMaker& dynamicState(VkDynamicState value) { dynamicState_.emplace_back(value); return *this; }
	private:
		const Device_T* device_;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState_ = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
		VkViewport viewport_;
		VkRect2D scissor_;
		VkPipelineRasterizationStateCreateInfo rasterizationState_ = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
		VkPipelineMultisampleStateCreateInfo multisampleState_ = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
		VkPipelineDepthStencilStateCreateInfo depthStencilState_ = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
		VkPipelineColorBlendStateCreateInfo colorBlendState_ = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments_;
		std::vector<VkPipelineShaderStageCreateInfo> modules_;
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions_;
		std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions_;
		std::vector<VkDynamicState> dynamicState_;
		uint32_t subpass_ = 0;
	};

	class PipelineLayoutMaker
	{
	public:
		void pushConstant(VkShaderStageFlags stage, uint32_t offset, uint32_t size) {
			pushConstants_.push_back({ stage,offset,size });
		}
		void setLayout(const DescriptorSetLayout& layout) {
			setLayouts_.emplace_back(layout->get());
		}
		PipelineLayout create(const Device& device) {
			VkPipelineLayoutCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
			info.pushConstantRangeCount = static_cast<uint32_t>(pushConstants_.size());
			info.pPushConstantRanges = pushConstants_.data();
			info.setLayoutCount = static_cast<uint32_t>(setLayouts_.size());
			info.pSetLayouts = setLayouts_.data();
			return std::make_unique<PipelineLayout_T>(device.get(), info);
		}
	private:
		std::vector<VkPushConstantRange> pushConstants_;
		std::vector<VkDescriptorSetLayout> setLayouts_;
	};

	class DescriptorSetLayoutMaker
	{
	public:
		void binding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1)
		{
			bindings_.push_back({ binding, descriptorType, descriptorCount, stageFlags, nullptr });
		}

		DescriptorSetLayout create(const Device& device) {
			VkDescriptorSetLayoutCreateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			info.bindingCount = static_cast<uint32_t>(bindings_.size());
			info.pBindings = bindings_.data();
			return std::make_unique<DescriptorSetLayout_T>(device.get(), info);
		}
	private:
		std::vector<VkDescriptorSetLayoutBinding> bindings_;
	};

	class SamplerMaker {
	public:
		SamplerMaker() {
			info.magFilter = info.minFilter = VK_FILTER_NEAREST;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.addressModeU = info.addressModeV  = info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.mipLodBias = 0.0f;
			info.anisotropyEnable = 0;
			info.maxAnisotropy = 0.0f;
			info.compareEnable = 0;
			info.compareOp = VK_COMPARE_OP_NEVER;
			info.minLod = 0;
			info.maxLod = 0;
			info.borderColor = {};
			info.unnormalizedCoordinates = 0;
		}

		SamplerMaker &magFilter(VkFilter value) { info.magFilter = value; return *this; }

		SamplerMaker &minFilter(VkFilter value) { info.minFilter = value; return *this; }

		SamplerMaker &mipmapMode(VkSamplerMipmapMode value) { info.mipmapMode = value; return *this; }
		SamplerMaker &addressModeU(VkSamplerAddressMode value) { info.addressModeU = value; return *this; }
		SamplerMaker &addressModeV(VkSamplerAddressMode value) { info.addressModeV = value; return *this; }
		SamplerMaker &addressModeW(VkSamplerAddressMode value) { info.addressModeW = value; return *this; }
		SamplerMaker &mipLodBias(float value) { info.mipLodBias = value; return *this; }
		SamplerMaker &anisotropyEnable(VkBool32 value) { info.anisotropyEnable = value; return *this; }
		SamplerMaker &maxAnisotropy(float value) { info.maxAnisotropy = value; return *this; }
		SamplerMaker &compareEnable(VkBool32 value) { info.compareEnable = value; return *this; }
		SamplerMaker &compareOp(VkCompareOp value) { info.compareOp = value; return *this; }
		SamplerMaker &minLod(float value) { info.minLod = value; return *this; }
		SamplerMaker &maxLod(float value) { info.maxLod = value; return *this; }
		SamplerMaker &borderColor(VkBorderColor value) { info.borderColor = value; return *this; }
		SamplerMaker &unnormalizedCoordinates(VkBool32 value) { info.unnormalizedCoordinates = value; return *this; }

		Sampler create(const Device& device) const {
			return std::make_unique<Sampler_T>(device.get(), info);
		}
	private:
		VkSamplerCreateInfo info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	};

	class DescriptorSetUpdater {
	public:
		DescriptorSetUpdater(int maxBuffers = 10, int maxImages = 10) {
			bufferInfo_.resize(maxBuffers);
			imageInfo_.resize(maxImages);
		}

		void beginDescriptorSet(const DescriptorSet& dstSet) {
			dstSet_ = dstSet->get();
		}

		void beginImages(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) {
			VkWriteDescriptorSet wdesc = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
			wdesc.dstSet = dstSet_;
			wdesc.dstBinding = dstBinding;
			wdesc.dstArrayElement = dstArrayElement;
			wdesc.descriptorCount = 0;
			wdesc.descriptorType = descriptorType;
			wdesc.pImageInfo = imageInfo_.data() + numImages_;
			descriptorWrites_.push_back(wdesc);
		}

		void image(Sampler& sampler, VkImageView imageView, VkImageLayout imageLayout) {
			if (!descriptorWrites_.empty() && numImages_ != imageInfo_.size() && descriptorWrites_.back().pImageInfo) {
				descriptorWrites_.back().descriptorCount++;
				imageInfo_[numImages_++] = { sampler->get(), imageView, imageLayout };
			}
			else {
				ok_ = false;
			}
		}

		void beginBuffers(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) {
			VkWriteDescriptorSet wdesc = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			wdesc.dstSet = dstSet_;
			wdesc.dstBinding = dstBinding;
			wdesc.dstArrayElement = dstArrayElement;
			wdesc.descriptorCount = 0;
			wdesc.descriptorType = descriptorType;
			wdesc.pBufferInfo = bufferInfo_.data() + numBuffers_;
			descriptorWrites_.push_back(wdesc);
		}

		void buffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range) {
			if (!descriptorWrites_.empty() && numBuffers_ != bufferInfo_.size() && descriptorWrites_.back().pBufferInfo) {
				descriptorWrites_.back().descriptorCount++;
				bufferInfo_[numBuffers_++] = { buffer->get(), offset, range };
			}
			else {
				ok_ = false;
			}
		}

		void update(const Device& device) const {
			vkUpdateDescriptorSets(*device, static_cast<uint32_t>(descriptorWrites_.size()), descriptorWrites_.data(), 0, nullptr);
		}

		bool ok() const { return ok_; }
	private:
		std::vector<VkDescriptorBufferInfo> bufferInfo_;
		std::vector<VkDescriptorImageInfo> imageInfo_;
		std::vector<VkWriteDescriptorSet> descriptorWrites_;
		VkDescriptorSet dstSet_;
		int numBuffers_ = 0;
		int numImages_ = 0;
		bool ok_ = true;
	};

	static std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice physicalDevice) {
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
		std::vector<VkQueueFamilyProperties> props(count);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, props.data());
		return std::move(props);
	}

	static VkBool32 getSurfaceSupport(VkPhysicalDevice physicalDevice,uint32_t familyIndex, VkSurfaceKHR surface)
	{
		VkBool32 support;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &support));
		return support;
	}
} // namespace vg::vk