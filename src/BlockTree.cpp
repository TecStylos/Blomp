#include "BlockTree.h"
#include "BlockTreeGen.h"
#include "Descriptors.h"
#include <stdexcept>

namespace Blomp
{
    namespace BlockTree
    {
        BlockRef fromImage(const Image &img, const BlockTreeDesc& btDesc)
        {
            return createSubBlock(BlockDim(0, 0, img.width(), img.height()), -1, btDesc, img);
        }

        void serialize(BlockRef pbRef, BitStream& bitStream)
        {
            pbRef->serialize(bitStream);
        }

        BlockRef deserialize(BaseDescriptor bd, BitStream& bitStream)
        {
            ParentBlockDesc pbDesc;
            pbDesc.x = 0;
            pbDesc.y = 0;
            pbDesc.width = bd.imgWidth;
            pbDesc.height = bd.imgHeight;
            pbDesc.depth = -1;
            
            BlockTreeDesc btDesc;
            btDesc.maxDepth = bd.maxDepth;
            btDesc.variationThreshold = 0.0f;

            if (!bitStream.readBit())
                throw std::runtime_error("Unable to read damaged blomp file.");

            return BlockRef(new ParentBlock(pbDesc, btDesc, bd.imgWidth, bd.imgHeight, bitStream));
        }
    }
}