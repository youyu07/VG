#pragma once

#include <vector>
#include <unordered_map>

namespace vg
{
    


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