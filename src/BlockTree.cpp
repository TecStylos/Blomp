#include "BlockTree.h"

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

            return std::shared_ptr<ParentBlock>(new ParentBlock(pbDesc, btDesc, img));
        }
    }
}