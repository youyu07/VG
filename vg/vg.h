#pragma once
#include <iostream>

namespace vg
{

    class __IObject
    {
    public:
        template<typename T> T& getHandle()
        {
            return *static_cast<T*>(this);
        }
    };

    class ISwapchain;
    class IBuffer;
    class IImage;
    class IImageView;
    class IShader;
    class ICommand;

    enum class Format : uint8_t
    {
        Undefined,
        R8,
        Rg8,
        Rgb8,
        Rgba8
    };

    enum class MemoryUsage : uint8_t
    {
        GPU,
        CPU_GPU,
        CPU,
        GPU_CPU
    };

    enum class BufferType : uint8_t
    {
        Uniform,
        Vertex,
        Index
    };

    enum class ShaderLanguageType : uint8_t
    {
        GLSL,
        SPRV
    };

    struct BufferDesc
    {
        MemoryUsage usage;
        BufferType type;
        uint32_t size;
    };

    enum class ImageType : uint8_t
    {
        t_2d
    };
    
    struct ImageDesc
    {
        MemoryUsage usage;
        ImageType type;
        Format format;
        uint32_t mipLevel;
        uint32_t layer;
    };

    struct SwapchainDesc
    {
        uint32_t imageCount = 2;
    };

    struct ShaderDesc
    {
        ShaderLanguageType language;
    };

    class IDevice : public __IObject
    {
    public:
        IBuffer* createBuffer(const BufferDesc& desc,const void* data = nullptr);
        IImage* createImage(const ImageDesc& desc,const void* data = nullptr);
        ISwapchain* createSwapchain(const SwapchainDesc& desc);
        IShader* createShader(const ShaderDesc& desc,const void* data);
    };

    class __IObjectWithDevice : public __IObject
    {
    public:
        template<typename T> T& getDevice(){
            return *static_cast<T*>(m_device);
        }
    protected:
        IDevice* m_device = nullptr;
    };

    class ISwapchain : public __IObjectWithDevice
    {
    public:
        virtual bool resize() = 0;
        virtual bool present(IImage* image) = 0;
    private:
        uint32_t imageCount = 2;
    };

    class IBuffer : public __IObjectWithDevice
    {
    public:
        virtual bool subData(uint32_t size,uint32_t offset,const void* data) = 0;

        uint32_t getSize() const
        {
            return size;
        }
    private:
        uint32_t size = 0;
    };

    class IImage : public __IObjectWithDevice
    {
    public:
        const Format getFormat() const
        {
            return format;
        }

        const uint32_t getWidth() const
        {
            return width;
        }

        const uint32_t getHeight() const
        {
            return height;
        }

        const uint32_t getMipLevel() const
        {
            return mipLevel;
        }

        const uint32_t getLayer() const
        {
            return layer;
        }

        virtual IImageView* createView() = 0;

    private:
        uint32_t width = 0, height = 0;
        uint32_t mipLevel = 1,layer = 1;
        Format format = Format::Undefined;
    };

    class IImageView : public __IObjectWithDevice
    {
    private:
        IImage* image = nullptr;
    };

    class ICommand : public __IObjectWithDevice
    {
    public:
        virtual void submit() = 0;
    };
}