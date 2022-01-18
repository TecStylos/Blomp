#pragma once

#include "Blocks.h"
#include "Descriptors.h"

namespace Blomp
{
    namespace BlockTree
    {
        BlockRef fromImage(const Image& img, const BlockTreeDesc& btDesc);

        void serialize(BlockRef pbRef, BitStream& bitStream);

        BlockRef deserialize(BaseDescriptor bd, BitStream& bitStream);
    }
}