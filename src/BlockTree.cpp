#include "BlockTree.h"
#include "Descriptors.h"

namespace Blomp
{
    namespace BlockTree
    {
        ParentBlockRef fromImage(const Image &img, const BlockTreeDesc& btDesc)
        {
            ParentBlockDesc pbDesc;
            pbDesc.x = 0;
            pbDesc.y = 0;
            pbDesc.width = img.width();
            pbDesc.height = img.height();
            pbDesc.depth = -1;

            return ParentBlockRef(new ParentBlock(pbDesc, btDesc, img));
        }

        void serialize(ParentBlockRef pbRef, BitStream& bitStream)
        {
            pbRef->serialize(bitStream);
        }

        ParentBlockRef deserialize(BaseDescriptor bd, BitStream& bitStream)
        {
            ParentBlockDesc pbDesc;
            pbDesc.x = 0;
            pbDesc.y = 0;
            pbDesc.width = bd.imgWidth;
            pbDesc.height = bd.imgHeight;
            pbDesc.depth = -1;
            
            BlockTreeDesc btDesc;
            btDesc.baseWidthExp = bd.baseWidthExp;
            btDesc.baseHeightExp = bd.baseHeightExp;
            btDesc.variationThreshold = 0.0f;

            bitStream.readBit();

            return ParentBlockRef(new ParentBlock(pbDesc, btDesc, bd.imgWidth, bd.imgHeight, bitStream));
        }
    }
}