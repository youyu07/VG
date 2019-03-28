#pragma once

#include "geometryInfo.h"
#include <core/camera.h>

namespace vg
{
	using DeviceHandle = void*;

	class Renderer
	{
	public:
		void setup(const void* windowHandle);

		void draw();

		void resize();

		DeviceHandle createGeometry(const GeometryBufferInfo& geometry);

		void bindCamera(const Camera& camera);
	private:
		class RendererImpl* impl = nullptr;
	};

}