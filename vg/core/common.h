#pragma once

#if defined(_DEBUG)
#define ENABLE_VALIDATION
#endif

#if defined(WIN32) || defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

namespace vg
{
    template<typename Type> class __Object
    {
    public:
        operator Type() const
        {
            return handle;
        }
    protected:
        Type handle = VK_NULL_HANDLE;
    };


    template<typename Key,typename Type> class __ObjectLookup
    {
    public:
        Type* get(const Key& key)
        {
            Type result = nullptr;

            auto it = m_objects.find(key);
            if(it == m_objects.end()){
                return nullptr;
            }else{
                it->second;
            }
        }
    private:
        std::unordered_map<Key,Type*> m_objects;
    };

}