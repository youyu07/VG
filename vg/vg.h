#pragma once
#include <iostream>
#include <vector>

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
        Rgba8,
		Bgra8,
		D32,
		D24_s8,
        R16f,
        Rg16f,
        Rgb16f,
        Rgba16f,
        R32f,
        Rg32f,
        Rgb32f,
        Rgba32f,
        R16u,
        Rg16u,
        Rgb16u,
        Rgba16u,
        R32u,
        Rg32u,
        Rgb32u,
        Rgba32u,
        R16i,
        Rg16i,
        Rgb16i,
        Rgba16i,
        R32i,
        Rg32i,
        Rgb32i,
        Rgba32i,
    };

    enum class BufferUsage : uint8_t
    {
        Undefined,
        Uniform,
        Uniform_Dynamic,
        Storage,
        Storage_Dynamic,
        Vertex,
        Vertex_Dynamic,
        Index,
        Index_Dynamic
    };

    struct BufferDesc
    {
        BufferUsage usage;
        uint32_t size;
    };

    enum class ImageType : uint8_t
    {
		Image_1d,
        Image_2d,
		Image_3d
    };

	enum class ImageViewType : uint8_t
	{
		View_1d,
		View_1d_Array,
		View_2d,
		View_2d_Array,
		View_Cube,
		View_Cube_Array
	};

	enum class AttachmentType : uint8_t
	{
		Color,
		Depth
	};
    
    struct ImageDesc
    {
        ImageType type;
        Format format;
        uint32_t mipLevel;
        uint32_t layer;
		uint32_t width;
		uint32_t height;
    };

	struct ImageViewDesc
	{
		ImageViewType type;
	};

	struct AttachmentDesc
	{
		AttachmentType type;
		Format format;
		uint32_t width;
		uint32_t height;
	};

    class __declspec(dllexport) IDevice : public __IObject
    {
    public:
        IDevice();

        static IDevice* create();

        virtual ~IDevice() {};

        virtual ISwapchain* createSwapchain(void* nativeHandle) = 0;

        virtual IBuffer* createBuffer(const BufferDesc& desc,const void* data = nullptr) = 0;
        virtual IImage* createImage(const ImageDesc& desc,const void* data = nullptr) = 0;
        virtual IRenderContext* createRenderContext() = 0;
		virtual IImageView* createAttachment(const AttachmentDesc& desc) = 0;
    };

    class __declspec(dllexport) __IObjectWithDevice : public __IObject
    {
    public:
        __IObjectWithDevice(IDevice* device) : m_device(device) {}

        template<typename T> T& getDevice(){
            return *static_cast<T*>(m_device);
        }
    protected:
        IDevice* m_device = nullptr;
    };

    class __declspec(dllexport) ISwapchain : public __IObjectWithDevice
    {
		using __IObjectWithDevice::__IObjectWithDevice;
    public:
        virtual bool resize() = 0;
        virtual bool present(IImage* image) = 0;
    private:
        uint32_t imageCount = 2;
    };

    class __declspec(dllexport) IBuffer : public __IObjectWithDevice
    {
		using __IObjectWithDevice::__IObjectWithDevice;
    public:
        virtual bool updateData(uint32_t size,uint32_t offset,const void* data) = 0;

        inline void setInfo(const BufferDesc& desc)
        {
            size = desc.size;
            usage = desc.usage;
        }

        inline const uint32_t getSize() const
        {
            return size;
        }

        inline const BufferUsage getUsage() const
        {   
            return usage;
        }
    private:
        uint32_t size = 0;
        BufferUsage usage;
    };

    enum class CompareOption : uint8_t
    {
        Never,
        Less,
        Equal,
        Less_or_equal,
        Greater,
        Not_equal,
        Greater_or_equal,
        Always
    };

    struct Sampler
    {
        enum class Filter : uint8_t
        {
            Nearest,
            Linear
        }filter;

        enum class AddressMode : uint8_t
        {
            Repeat,
            Mirrored_repead,
            Clamp_edge,
            Clamp_border,
            Mirror_clamp_edge
        }addressMode;

        enum class BorderColor : uint8_t
        {
            Float_transparent_black,
            Int_transparent_black,
            Float_opaque_black,
            Int_opaque_black,
            Float_opaque_white,
            Int_opaque_white
        }borderColor;

        float anisotropy = 0.0f;

        //soft shadow mapping https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch11.html
        bool compareEnable = false;
        CompareOption compareOption;
    };

    class __declspec(dllexport) IImage : public __IObjectWithDevice
    {
		using __IObjectWithDevice::__IObjectWithDevice;
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

		virtual void update(uint32_t offset, uint32_t size,const void* data) = 0;

        virtual IImageView* createView(const ImageViewDesc& desc) = 0;

    protected:
        uint32_t width = 0, height = 0;
        uint32_t mipLevel = 1,layer = 1;
        Format format = Format::Undefined;
    };

    class __declspec(dllexport) IImageView : public __IObjectWithDevice
    {
		using __IObjectWithDevice::__IObjectWithDevice;
	public:
		template<typename T> T& getImage() const
		{
			return *static_cast<T*>(image);
		}

        Sampler sampler;
	protected:
        IImage* image = nullptr;
    };

    enum class ShaderLanguageType : uint8_t
    {
        GLSL,
        SPRV
    };

    enum class ShaderType : uint8_t
    {
        Vertex,
		Tessellation_control,
		Tessellation_evaluation,
		Geometry,
		Fragment,
		Computer,
    };

    enum class PrimitiveType
    {
        Point,
        Line,
        Line_strip,
        Triangles,
        Triangles_strip,
        Triangles_fan
    };

    class __declspec(dllexport) IPipeline : public __IObjectWithDevice
    {
		using __IObjectWithDevice::__IObjectWithDevice;
    public:
        virtual void setShader(const ShaderType& type,const std::string& src,ShaderLanguageType language = ShaderLanguageType::SPRV) = 0;

        virtual void setVertexAttribute(uint32_t binding, const std::vector<Format>& formats) = 0;

        inline void setPrimitiveType(const PrimitiveType& type)
        {
            primitiveType = type;
        }

		inline void setSize(uint32_t w,uint32_t h)
		{
			width = w;
			height = h;
		}
    protected:
        PrimitiveType primitiveType = PrimitiveType::Triangles;
		uint32_t width, height;
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

    class __declspec(dllexport) IRenderContext : public __IObjectWithDevice
    {
		using __IObjectWithDevice::__IObjectWithDevice;
    public:
		inline void setSize(uint32_t width, uint32_t height) {
			this->width = width;
			this->height = height;
		}
		virtual void bindVertexBuffers(uint32_t count, IBuffer** buf) = 0;
		virtual void bindIndexBuffer(IBuffer* buf) = 0;
		virtual void bindBuffer(uint32_t bind, IBuffer* buf) = 0;
		virtual void bindImageView(uint32_t bind, IImageView* view) = 0;
		virtual void bindShader(const ShaderType& type, const std::string& src, ShaderLanguageType language = ShaderLanguageType::SPRV) = 0;

		virtual bool setColorAttachment(uint32_t location, IImageView* view) = 0;
		virtual bool setDepthAttachment(IImageView* view) = 0;
		virtual void setVertexAttribute(uint32_t binding, const std::vector<Format>& formats) = 0;
        
        virtual void draw(const DrawAttribute& attribute) = 0;
        virtual bool submit() = 0;

	protected:
		uint32_t width, height;
    };
}