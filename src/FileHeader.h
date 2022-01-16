#pragma once

#include "Descriptors.h"
#include <stdint.h>

namespace Blomp
{
    struct FileHeader
    {
        static constexpr char DEFAULT_IDENTIFIER[4] = { 'B', 'L', 'M', 'P' };
        char identifier[4] = { 'B', 'L', 'M', 'P' };
        BaseDescriptor bd;
    public:
        bool isValid() const
        {
            for (int i = 0; i < 4; ++i)
                if (identifier[i] != DEFAULT_IDENTIFIER[i])
                    return false;
            return true;
        }
    };
}