#pragma once

#include <string>

namespace Blomp
{
    inline bool endswith(const std::string& full, const std::string sub)
    {
        return full.find(sub) == full.size() - sub.size();
    };
}