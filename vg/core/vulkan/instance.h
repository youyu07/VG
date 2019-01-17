#pragma once

#if defined(_DEBUG)
#define ENABLE_VALIDATION
#endif

#if defined(WIN32) || defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vector>

#include "core/log.h"

namespace vg::vk
{
template <typename Type> class __VkObject
{
  public:
    operator Type() const
    {
        return handle;
    }

  protected:
    Type handle = VK_NULL_HANDLE;
};


VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        log_warning(callbackData->pMessage);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        log_error(callbackData->pMessage);
    }
    else 
    {
        log_info(callbackData->pMessage);
    }
    
    return VK_FALSE;
}


class Instance : public __VkObject<VkInstance>
{
  public:
    ~Instance()
    {
        if(debug != VK_NULL_HANDLE){
            auto destroyFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(handle, "vkDestroyDebugUtilsMessengerEXT"));
            destroyFunc(handle,debug,nullptr);
        }
        vkDestroyInstance(handle,nullptr);
    }
    
    VkResult setup(bool enableValidation)
    {
        // Fill out VkApplicationInfo struct.
        VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = "vg";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.pEngineName = "vg";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_1;

        // Create VkInstance.
        VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instanceCreateInfo.pApplicationInfo = &appInfo;

        if(enableValidation){
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

            auto isLayerAvailable = [&](const char* name) -> bool{
                for (const auto& layer : layers)
                    if (strcmp(layer.layerName, name) == 0)
                        return true;
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

            auto isExensionAvailable = [&](const char* name) -> bool{
                for (const auto& ex : extensions)
                    if (strcmp(ex.extensionName, name) == 0)
                        return true;
                return false;
            };

            if (enableValidation)
            {
                if(isExensionAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)){
                    extensionName.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                }else{
                    log_warning("Extension ", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, " is not available.");
                }
            }

            instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionName.size());
            instanceCreateInfo.ppEnabledExtensionNames = extensionName.empty() ? nullptr : extensionName.data();
        }

        auto result = vkCreateInstance(&instanceCreateInfo, nullptr, &handle);
        if (result != VK_SUCCESS) return result;

        // Get the number of attached physical devices.
        uint32_t physicalDeviceCount = 0;
        result = vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, nullptr);
        if (result != VK_SUCCESS) return result;

        // Make sure there is at least one physical device present.
        if (physicalDeviceCount > 0)
        {
            m_physicalDevices.resize(physicalDeviceCount);
            result = vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, m_physicalDevices.data());
            if (result != VK_SUCCESS) return result;
        }

        if(enableValidation){
            result = setupDebug();
            if (result != VK_SUCCESS) return result;
        }

        // Return success.
        return VK_SUCCESS;
    }

    VkPhysicalDevice selectPhysicalDevice()
    {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        for (auto device : m_physicalDevices)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);
            
            uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
            std::vector<VkQueueFamilyProperties> properties(count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());

            bool supported = false;
            for(const auto &QueueFamilyProps : properties)
            {
                if ((QueueFamilyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 &&
                    (QueueFamilyProps.queueFlags & VK_QUEUE_COMPUTE_BIT)  != 0)
                {
                    supported = true;
                    break;
                }
            }
            if (supported)
            {
                physicalDevice = device;
                if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                   break;
            }
        }

        if (physicalDevice != VK_NULL_HANDLE)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(physicalDevice, &props);
            log_info("Using physical device : ", props.deviceName);
        }
        else
        {
            log_error("Failed to find suitable physical device");
        }

        return physicalDevice;
    }
  private:
    std::vector<VkPhysicalDevice> m_physicalDevices;
    VkDebugUtilsMessengerEXT debug;

    VkResult setupDebug()
    {
        auto createFunc  = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(handle, "vkCreateDebugUtilsMessengerEXT"));
        if(createFunc == nullptr){
            log_error("Failed to find function vkCreateDebugUtilsMessengerEXT!");
            return VK_ERROR_VALIDATION_FAILED_EXT;
        }

        VkDebugUtilsMessengerCreateInfoEXT DbgMessenger_CI = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        DbgMessenger_CI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        DbgMessenger_CI.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        DbgMessenger_CI.pfnUserCallback = DebugMessengerCallback;
        VkResult err = createFunc(handle, &DbgMessenger_CI, nullptr, &debug);
        
        if(err != VK_SUCCESS){
            log_error("Failed to create debug message util!");
        }
        return err;
    }
};

} // namespace vg::vk