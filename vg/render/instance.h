#pragma once

#include "common.h"


namespace vg::vk
{
    class DebugUtil : public __VkObject<VkDebugUtilsMessengerEXT>
    {
        VkInstance instance = VK_NULL_HANDLE;
    public:
        DebugUtil(VkInstance ins, PFN_vkDebugUtilsMessengerCallbackEXT callback) : instance(ins)
        {
            auto createFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
			if (createFunc == nullptr) {
				log_error("Failed to find function vkCreateDebugUtilsMessengerEXT!");
			}else{
                VkDebugUtilsMessengerCreateInfoEXT DbgMessenger_CI = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
                DbgMessenger_CI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                DbgMessenger_CI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                DbgMessenger_CI.pfnUserCallback = callback;
                VkResult err = createFunc(instance, &DbgMessenger_CI, nullptr, &handle);
                if (err != VK_SUCCESS) {
                    log_error("Failed to create debug message util!");
                }
            }
        }

        ~DebugUtil()
        {
            auto destroyFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
			destroyFunc(instance, handle, nullptr);
        }
    };


    class Instance : public __VkObject<VkInstance>
    {
    public:
        Instance(bool enableValidation, PFN_vkDebugUtilsMessengerCallbackEXT debugCallBack)
        {
            VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
			appInfo.pApplicationName = "vg";
			appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
			appInfo.pEngineName = "vg";
			appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
			appInfo.apiVersion = VK_API_VERSION_1_1;

			// Create VkInstance.
			VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
			instanceCreateInfo.pApplicationInfo = &appInfo;

			if (enableValidation) {
				const char* layerName[] = {
	#if !defined(__ANDROID__)
				"VK_LAYER_LUNARG_standard_validation"
	#else
				"VK_LAYER_GOOGLE_threading",
				"VK_LAYER_LUNARG_parameter_validation",
				"VK_LAYER_LUNARG_object_tracker",
				"VK_LAYER_LUNARG_core_validation",
				"VK_LAYER_LUNARG_swapchain",
				"VK_LAYER_GOOGLE_unique_objects"

	#endif
				};

				uint32_t count = 0;
				auto res = vkEnumerateInstanceLayerProperties(&count, nullptr);
				std::vector<VkLayerProperties> layers(count);
				res = vkEnumerateInstanceLayerProperties(&count, layers.data());

				auto isLayerAvailable = [&](const char* name) -> bool {
					for (const auto& layer : layers)
                    {
                        if (strcmp(layer.layerName, name) == 0)return true;
                    }
					return false;
				};

				bool ValidationLayersPresent = true;
				for (size_t i = 0; i < _countof(layerName); ++i)
				{
					auto* pLayerName = layerName[i];
					if (!isLayerAvailable(pLayerName))
					{
						ValidationLayersPresent = false;
						log_info("Failed to find ", pLayerName, " layer. Validation will be disabled");
					}
				}
				if (ValidationLayersPresent)
				{
					instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(_countof(layerName));
					instanceCreateInfo.ppEnabledLayerNames = layerName;
				}
			}

			std::vector<const char*> extensionName = {
				VK_KHR_SURFACE_EXTENSION_NAME,
	#if defined(VK_USE_PLATFORM_WIN32_KHR)
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	#endif
			};

			{
				uint32_t count = 0;
				auto res = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
				std::vector<VkExtensionProperties> extensions(count);
				res = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

				auto isExensionAvailable = [&](const char* name) -> bool {
					for (const auto& ex : extensions)
                    {
                        if (strcmp(ex.extensionName, name) == 0)return true;
                    }
					return false;
				};

				if (enableValidation)
				{
					if (isExensionAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
						extensionName.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
					}
					else {
						log_warning("Extension ", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, " is not available.");
					}
				}

				instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionName.size());
				instanceCreateInfo.ppEnabledExtensionNames = extensionName.empty() ? nullptr : extensionName.data();
			}

			auto result = vkCreateInstance(&instanceCreateInfo, nullptr, &handle);
			if (result == VK_SUCCESS && enableValidation) {
                debug = new DebugUtil(handle,debugCallBack);
            }
        }

        virtual ~Instance()
        {
            delete debug;
            vkDestroyInstance(handle,nullptr);
        }
    private:
        DebugUtil* debug;
    };

    class PhysicalDevice  : public __VkObject<VkPhysicalDevice>
    {
    public:
        PhysicalDevice(VkInstance ins)
        {
            // Get the number of attached physical devices.
            uint32_t count = 0;
            vkEnumeratePhysicalDevices(ins, &count, nullptr);
            std::vector<VkPhysicalDevice> physicalDevices(count);
            vkEnumeratePhysicalDevices(ins, &count, physicalDevices.data());

            for (auto device : physicalDevices)
            {
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(device, &props);
                
                uint32_t count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
                std::vector<VkQueueFamilyProperties> properties(count);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());

                uint32_t index = -1;
                for(uint32_t i = 0;i < properties.size();++i)
                {
                    const auto& QueueFamilyProps = properties[i];
                    if ((QueueFamilyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && (QueueFamilyProps.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
                    {
                        index = i;
                        break;
                    }
                }
                if (index != -1)
                {
                    handle = device;
                    queueFamily = index;
                    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                    {
                        break;
                    }
                }
            }

            if (handle != VK_NULL_HANDLE)
            {
                vkGetPhysicalDeviceProperties(handle, &properties);
                log_info("Using physical device : ", properties.deviceName);

                vkGetPhysicalDeviceFeatures(handle, &features);
                vkGetPhysicalDeviceMemoryProperties(handle, &memoryProperties);
            }
            else
            {
                log_error("Failed to find suitable physical device");
            }
        }

        const uint32_t getQueueFamily() const
        {
            return queueFamily;
        }
    private:
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        VkPhysicalDeviceFeatures features;
        uint32_t queueFamily = -1;
    };

} // name
