#pragma once
#include "common.h"

namespace vg
{
class InstanceImpl : public __Object<VkInstance>
{
  public:
    VkResult setup()
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

#if defined(ENABLE_VALIDATION)

#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)

#endif

        auto result = vkCreateInstance(&instanceCreateInfo, nullptr, &handle);
        if (result != VK_SUCCESS)
            return result;

        // Get the number of attached physical devices.
        uint32_t physicalDeviceCount = 0;
        result = vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        // Make sure there is at least one physical device present.
        if (physicalDeviceCount > 0)
        {
            physicalDevices.resize(physicalDeviceCount);
            result = vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, physicalDevices.data());
            if (result != VK_SUCCESS)
                return result;
        }

        // Return success.
        return VK_SUCCESS;
    }

  private:
    std::vector<VkPhysicalDevice> physicalDevices;
};

} // namespace vg