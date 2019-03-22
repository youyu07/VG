#pragma once
#if defined(WIN32)
#include <Windows.h>
#endif

#include <vector>
#include <unordered_map>
#include "geometryInfo.h"
#include <core/camera.h>

namespace vg
{
	using DeviceHandle = void*;

	class Renderer
	{
	public:
		void setup(HWND windowHandle);

		void draw();

		DeviceHandle createGeometry(const GeometryBufferInfo& geometry);

		void bindCamera(const Camera& camera);
	private:
		class RenderImpl* impl = nullptr;
	};

}