#pragma once

#include "Blocks.h"
#include "Descriptors.h"

namespace Blomp
{
    ParentBlockRef CreateBlockTree(const Image& img, const BlockTreeDesc& btDesc);
}