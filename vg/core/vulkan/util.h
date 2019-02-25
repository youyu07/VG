#pragma once
#include "vg.h"
#include <vulkan/vulkan.h>

namespace vg::vk
{
    class Util
    {
    public:
        static const VkFormat convertFormat(Format f)
        {
            switch (f)
            {
            case Format::R8:
                return VK_FORMAT_R8_UNORM;
            case Format::Rg8:
                return VK_FORMAT_R8G8_UNORM;
            case Format::Rgb8:
                return VK_FORMAT_R8G8B8_UNORM;
            case Format::Rgba8:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case Format::Bgra8:
                return VK_FORMAT_B8G8R8_UNORM;
            case Format::D32:
                return VK_FORMAT_D32_SFLOAT;
            case Format::D24_s8:
                return VK_FORMAT_D24_UNORM_S8_UINT;
            case Format::R16f:
                return VK_FORMAT_R16_SFLOAT;
            case Format::Rg16f:
                return VK_FORMAT_R16G16_SFLOAT;
            case Format::Rgb16f:
                return VK_FORMAT_R16G16B16_SFLOAT;
            case Format::Rgba16f:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
            case Format::R32f:
                return VK_FORMAT_R32_SFLOAT;
            case Format::Rg32f:
                return VK_FORMAT_R32G32_SFLOAT;
            case Format::Rgb32f:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case Format::Rgba32f:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case Format::R16u:
                return VK_FORMAT_R16_UINT;
            case Format::Rg16u:
                return VK_FORMAT_R16G16_UINT;
            case Format::Rgb16u:
                return VK_FORMAT_R16G16B16_UINT;
            case Format::Rgba16u:
                return VK_FORMAT_R16G16B16A16_UINT;
            case Format::R32u:
                return VK_FORMAT_R32_UINT;
            case Format::Rg32u:
                return VK_FORMAT_R32G32_UINT;
            case Format::Rgb32u:
                return VK_FORMAT_R32G32B32_UINT;
            case Format::Rgba32u:
                return VK_FORMAT_R32G32B32A32_UINT;
            case Format::R16i:
                return VK_FORMAT_R16_SINT;
            case Format::Rg16i:
                return VK_FORMAT_R16G16_SINT;
            case Format::Rgb16i:
                return VK_FORMAT_R16G16B16_SINT;
            case Format::Rgba16i:
                return VK_FORMAT_R16G16B16A16_SINT;
            case Format::R32i:
                return VK_FORMAT_R32_SINT;
            case Format::Rg32i:
                return VK_FORMAT_R32G32_SINT;
            case Format::Rgb32i:
                return VK_FORMAT_R32G32B32_SINT;
            case Format::Rgba32i:
                return VK_FORMAT_R32G32B32A32_SINT;
            default:
                return VK_FORMAT_UNDEFINED;
            }
        }

        static const Format convertFormat(VkFormat f)
        {
            switch (f)
            {
            case VK_FORMAT_R8_UNORM:
                return Format::R8;
            case VK_FORMAT_R8G8_UNORM:
                return Format::Rg8;
            case VK_FORMAT_R8G8B8_UNORM:
                return Format::Rgb8;
            case VK_FORMAT_R8G8B8A8_UNORM:
                return Format::Rgba8;
            case VK_FORMAT_B8G8R8_UNORM:
                return Format::Bgra8;
            case VK_FORMAT_D32_SFLOAT:
                return Format::D32;
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return Format::D24_s8;
            case VK_FORMAT_R16_SFLOAT:
                return Format::R16f;
            case VK_FORMAT_R16G16_SFLOAT:
                return Format::Rg16f;
            case VK_FORMAT_R16G16B16_SFLOAT:
                return Format::Rgb16f;
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                return Format::Rgba16f;
            case VK_FORMAT_R32_SFLOAT:
                return Format::R32f;
            case VK_FORMAT_R32G32_SFLOAT:
                return Format::Rg32f;
            case VK_FORMAT_R32G32B32_SFLOAT:
                return Format::Rgb32f;
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return Format::Rgba32f;
            case VK_FORMAT_R16_UINT:
                return Format::R16u;
            case VK_FORMAT_R16G16_UINT:
                return Format::Rg16u;
            case VK_FORMAT_R16G16B16_UINT:
                return Format::Rgb16u;
            case VK_FORMAT_R16G16B16A16_UINT:
                return Format::Rgba16u;
            case VK_FORMAT_R32_UINT:
                return Format::R32u;
            case VK_FORMAT_R32G32_UINT:
                return Format::Rg32u;
            case VK_FORMAT_R32G32B32_UINT:
                return Format::Rgb32u;
            case VK_FORMAT_R32G32B32A32_UINT:
                return Format::Rgba32u;
            case VK_FORMAT_R16_SINT:
                return Format::R16i;
            case VK_FORMAT_R16G16_SINT:
                return Format::Rg16i;
            case VK_FORMAT_R16G16B16_SINT:
                return Format::Rgb16i;
            case VK_FORMAT_R16G16B16A16_SINT:
                return Format::Rgba16i;
            case VK_FORMAT_R32_SINT:
                return Format::R32i;
            case VK_FORMAT_R32G32_SINT:
                return Format::Rg32i;
            case VK_FORMAT_R32G32B32_SINT:
                return Format::Rgb32i;
            case VK_FORMAT_R32G32B32A32_SINT:
                return Format::Rgba32i;
            default:
                return Format::Undefined;
            }
        }

        static const uint32_t getFormatSize(Format f)
        {
            switch (f)
            {
            case Format::R8:
                return 1;
            case Format::Rg8:
            case Format::R16f:
            case Format::R16u:
                return 2;
            case Format::Rgb8:
                return 3;
            case Format::Rgba8:
            case Format::Bgra8:
            case Format::D32:
            case Format::D24_s8:
            case Format::Rg16f:
            case Format::Rg16u:
            case Format::R16i:
            case Format::R32f:
            case Format::R32u:
            case Format::R32i:
                return 4;
            case Format::Rgb16f:
            case Format::Rgb16u:
            case Format::Rgb16i:
                return 6;
            case Format::Rgba16f:
            case Format::Rgba16u:
            case Format::Rgba16i:
            case Format::Rg16i:
            case Format::Rg32f:
            case Format::Rg32u:
            case Format::Rg32i:
                return 8;
            case Format::Rgb32f:
            case Format::Rgb32u:
            case Format::Rgb32i:
                return 12;
            case Format::Rgba32f:
            case Format::Rgba32u:
            case Format::Rgba32i:
                return 16;
            default:
                return 0;
            }
        }

        static const VkImageViewType convertViewType(ImageViewType type)
        {
            switch (type)
            {
            case vg::ImageViewType::View_1d:
                return VK_IMAGE_VIEW_TYPE_1D;
            case vg::ImageViewType::View_1d_Array:
                return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            case vg::ImageViewType::View_2d:
                return VK_IMAGE_VIEW_TYPE_2D;
            case vg::ImageViewType::View_2d_Array:
                return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            case vg::ImageViewType::View_Cube:
                return VK_IMAGE_VIEW_TYPE_CUBE;
            case vg::ImageViewType::View_Cube_Array:
                return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            }
            return VK_IMAGE_VIEW_TYPE_2D;
        }

        static const VkImageType convertImageType(ImageType type)
        {
            switch (type)
            {
            case ImageType::Image_1d:
                return VK_IMAGE_TYPE_1D;
            case ImageType::Image_2d:
                return VK_IMAGE_TYPE_2D;
            case ImageType::Image_3d:
                return VK_IMAGE_TYPE_3D;
            }
            return VK_IMAGE_TYPE_2D;
        }

        static const VkImageAspectFlags getImageAspectFlags(VkFormat format)
        {
            switch (format)
            {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D32_SFLOAT:
                return VK_IMAGE_ASPECT_DEPTH_BIT;
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            default:
                return VK_IMAGE_ASPECT_COLOR_BIT;
            }
        }

        static const VkImageAspectFlags getImageAspectFlags(Format format)
        {
            return convertFormat(format);
        }

        static const VkSamplerAddressMode convertAddressMode(Sampler::AddressMode mode)
        {
            switch (mode)
			{	
			case Sampler::AddressMode::Mirrored_repead:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case Sampler::AddressMode::Clamp_edge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case Sampler::AddressMode::Clamp_border:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case Sampler::AddressMode::Mirror_clamp_edge:
                return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			}
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }

        static const VkFilter convertFilter(Sampler::Filter filter)
        {
            if(filter == Sampler::Filter::Linear){
                return VK_FILTER_LINEAR;
            }
            return VK_FILTER_NEAREST;
        }

        static const VkBorderColor convertBorderColor(Sampler::BorderColor borderColer)
        {
            switch (borderColer)
            {
            case Sampler::BorderColor::Int_transparent_black:
                return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
            case Sampler::BorderColor::Float_opaque_black:
                return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            case Sampler::BorderColor::Int_opaque_black:
                return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            case Sampler::BorderColor::Float_opaque_white:
                return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            case Sampler::BorderColor::Int_opaque_white:
                return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
            }

            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;;
        }

        static const VkCompareOp convertCompareOp(CompareOption compareOp)
        {
            switch (compareOp)
            {
            case CompareOption::Less:
                return VK_COMPARE_OP_LESS;
            case CompareOption::Equal:
                return VK_COMPARE_OP_EQUAL;
            case CompareOption::Less_or_equal:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareOption::Greater:
                return VK_COMPARE_OP_GREATER;
            case CompareOption::Not_equal:
                return VK_COMPARE_OP_NOT_EQUAL;
            case CompareOption::Greater_or_equal:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareOption::Always:
                return VK_COMPARE_OP_ALWAYS;
            }
            return VK_COMPARE_OP_NEVER;
        }

		static const VkPrimitiveTopology convertPrimitiveType(PrimitiveType type)
		{
			switch (type)
			{
			case vg::PrimitiveType::Point:
				return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case vg::PrimitiveType::Line:
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case vg::PrimitiveType::Line_strip:
				return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case vg::PrimitiveType::Triangles_strip:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case vg::PrimitiveType::Triangles_fan:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			}
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		static const VkShaderStageFlagBits convertShaderType(ShaderType type)
		{
			switch (type)
			{
			case vg::ShaderType::Vertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case vg::ShaderType::Tessellation_control:
				return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			case vg::ShaderType::Tessellation_evaluation:
				return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			case vg::ShaderType::Geometry:
				return VK_SHADER_STAGE_GEOMETRY_BIT;
			case vg::ShaderType::Fragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			case vg::ShaderType::Computer:
				return VK_SHADER_STAGE_COMPUTE_BIT;
			}
			return VK_SHADER_STAGE_ALL;
		}
    };

    
}
