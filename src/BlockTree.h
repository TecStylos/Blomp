#pragma once

#include "Blocks.h"
#include "Descriptors.h"

namespace Blomp
{
    namespace BlockTree
    {
        ParentBlockRef fromImage(const Image& img, const BlockTreeDesc& btDesc);

        void serialize(ParentBlockRef pbRef, BitStream& bitStream);

        ParentBlockRef deserialize(BaseDescriptor bd, BitStream& bitStream);
    }
}