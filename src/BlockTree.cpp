#include "BlockTree.h"

namespace Blomp
{
    ParentBlockRef CreateBlockTree(const Image &img, const BlockTreeDesc& btDesc)
    {
        ParentBlockDesc pbDesc;
        pbDesc.x = 0;
        pbDesc.y = 0;
        pbDesc.width = img.width();
        pbDesc.height = img.height();
        pbDesc.depth = 0;

        return std::shared_ptr<ParentBlock>(new ParentBlock(pbDesc, btDesc, img));
    }
}