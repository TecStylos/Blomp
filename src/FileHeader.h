#pragma once

#include "Descriptors.h"
#include <stdint.h>

namespace Blomp
{
    struct FileHeader
    {
        char identifier[4] = { 'B', 'L', 'M', 'P' };
        BaseDescriptor bd;
    };
}