#include "vg.h"
#include "core/vulkan/device.h"

namespace vg
{

    IDevice::IDevice()
    {
        printf("ERROR : create device ! \n");
    }

    IDevice* IDevice::create()
    {
        auto dev = new vk::Device();
        dev->setup(true);

        return dev;
    }
}