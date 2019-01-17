#pragma once
#include "vg.h"

namespace vg
{

    class __declspec(dllexport) Entry
    {
    public:
        inline bool setup()
        {
            device = IDevice::create();
            return true;
        }

    private:
        IDevice* device = nullptr;
    };

}