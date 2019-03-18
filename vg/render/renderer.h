#pragma once
#if defined(WIN32)
#include <Windows.h>
#endif

#include <vector>
#include <unordered_map>
#include "geometryInfo.h"

namespace vg
{
	using DeviceHandle = void*;

	class Renderer
	{
	public:
		void setup(HWND windowHandle);

		void draw();

		DeviceHandle createGeometry(const GeometryBufferInfo& geometry);
	private:
		class RenderImpl* impl = nullptr;
	};

}