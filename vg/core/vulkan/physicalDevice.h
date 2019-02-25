#pragma once
#include "instance.h"

namespace vg::vk
{
    class PhysicalDevice  : public __VkObject<VkPhysicalDevice>
    {
    public:
        bool setup(VkInstance ins)
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
            }
            else
            {
                log_error("Failed to find suitable physical device");
                return false;
            }

            vkGetPhysicalDeviceFeatures(handle, &features);
            vkGetPhysicalDeviceMemoryProperties(handle, &memoryProperties);

            return true;
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

}