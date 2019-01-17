#include "vg.h"
#include "instance.h"

namespace vg::vk
{

    class Device : public IDevice
    {
    public:
        bool setup(bool enableValidation){
            if(VK_SUCCESS != instance.setup(enableValidation)){
                return false;
            }
            physicalDevice = instance.selectPhysicalDevice();

            if(VK_NULL_HANDLE == physicalDevice){
                return false;
            }

            return true;
        }

        virtual ISwapchain* createSwapchain(void* nativeHandle, uint32_t imageCount = 2) override
        {
            return nullptr;
        }

        virtual IBuffer* createBuffer(const BufferDesc& desc,const void* data = nullptr) override
        {
            return nullptr;
        }

        virtual IImage* createImage(const ImageDesc& desc,const void* data = nullptr) override
        {
            return nullptr;
        }

        virtual IRenderContext* createRenderContext() override
        {
            return nullptr;
        }
    private:
        Instance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    };

}