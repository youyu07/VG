#pragma once
#include <iostream>

namespace vg
{
    class __declspec(dllexport) __IObject
    {
    public:
        __IObject() {};

        template<typename T> T& getHandle()
        {
            return *static_cast<T*>(this);
        }

        __IObject(const __IObject&) = delete;
        void operator=(const __IObject&) = delete;
    };

    class ISwapchain;
    class IBuffer;
    class IImage;
    class IImageView;
    class IPipeline;
    class IRenderContext;

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

    class __declspec(dllexport) IDevice : public __IObject
    {
    public:
        IDevice();

        static IDevice* create();

        virtual ~IDevice() {};

        virtual ISwapchain* createSwapchain(void* nativeHandle, uint32_t imageCount = 2) = 0;

        virtual IBuffer* createBuffer(const BufferDesc& desc,const void* data = nullptr) = 0;
        virtual IImage* createImage(const ImageDesc& desc,const void* data = nullptr) = 0;
        virtual IRenderContext* createRenderContext() = 0;
    };

    class __IObjectWithDevice : public __IObject
    {
    public:
        __IObjectWithDevice(IDevice* device) : m_device(device) {}

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
        virtual bool updateData(uint32_t size,uint32_t offset,const void* data) = 0;

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

    enum class ShaderLanguageType : uint8_t
    {
        GLSL,
        SPRV
    };

    enum class ShaderType : uint8_t
    {
        VERTEX,
        FRAGMENT,
        COMPUTER,
        GEOMETRY
    };

    class IPipeline : public __IObjectWithDevice
    {
    public:
        virtual void setShader(const ShaderType& type,const std::string& path,ShaderLanguageType language = ShaderLanguageType::SPRV);
    };

    struct DrawAttribute
    {
        enum class ValueType
        {
            vt_16,
            vt_32
        }indexType;
        union
        {
            uint32_t numVertex = 0;
            uint32_t numIndices;
        };
        uint32_t numInstance = 0;
        bool isIndex = false;
    };

    class IRenderContext : public __IObjectWithDevice
    {
    public:
        void setSize(uint32_t width, uint32_t height);
        virtual void bindPipeline(IPipeline* pipeline);
        virtual void bindVertexBuffers(uint32_t count, IBuffer* buf);
        virtual void bindIndexBuffer(IBuffer* buf);
        virtual void bindImageView(uint32_t bind, IImageView* view);
        virtual void draw(const DrawAttribute& attribute);
        virtual void submit();
    };
}