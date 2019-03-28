#pragma once

#include "geometryInfo.h"
#include <core/camera.h>

namespace vg
{
	class Renderer
	{
	public:
		void setup(const void* windowHandle);

		void draw();

		void resize();

		void addGeometry(uint64_t id, const GeometryBufferInfo& info);

		void bindCamera(const Camera& camera);
	private:
		class RendererImpl* impl = nullptr;
	};

}