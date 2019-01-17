#pragma once

#include <iostream>
#include <sstream>

namespace vg
{
    class Log
    {
    public:
        template <typename... TS> static void log(const char* severity, TS... args)
        {
            std::stringstream ss;
            ss << severity;
            int a[] = {((ss << args),0)...};
#if defined(_MSC_VER)
            ss << "\n";
            OutputDebugString(ss.str().c_str());
#else
            std::cout << ss.str() << std::endl;
#endif
        }
    };

    #define log_info(...)  Log::log("INFO : ",##__VA_ARGS__)
    #define log_warning(...) Log::log("WARNING : ",##__VA_ARGS__)
    #define log_error(...) Log::log("ERROR : ",##__VA_ARGS__)
}