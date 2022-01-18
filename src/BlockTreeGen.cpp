#include "BlockTreeGen.h"

#include "BlockPool.h"

namespace Blomp
{
    namespace BlockTree
    {
        BlockRef createSubBlock(BlockDim dimensions, int newDepth, const BlockTreeDesc& btDesc, const Image& img)
        {
            if (dimensions.w == 1 && dimensions.h == 1)
                return ColorBlockPool.take(dimensions, img(dimensions.x, dimensions.y));

            int subBlockDim = BlockTree::calcDimVal(btDesc.maxDepth, newDepth + 1);

            std::vector<BlockRef> subBlocks;
            subBlocks.reserve(4);
            bool hasParentSubBlock = false;
            
            for (int ry = dimensions.y; ry < (dimensions.y + dimensions.h) && ry < img.height(); ry += subBlockDim)
            {
                for (int rx = dimensions.x; rx < (dimensions.x + dimensions.w) && rx < img.width(); rx += subBlockDim)
                {
                    subBlocks.push_back(
                        BlockTree::createSubBlock(
                            BlockDim(
                                rx, ry,
                                std::min(img.width() - rx, subBlockDim),
                                std::min(img.height() - ry, subBlockDim)
                            ),                            
                            newDepth + 1,
                            btDesc, img
                        )
                    );
                    if (subBlocks.back()->getType() == BlockType::Parent)
                        hasParentSubBlock = true;
                }
            }

            if (hasParentSubBlock)
                return BlockRef(new ParentBlock(dimensions, subBlocks));

            BlockMetrics bm = calcSubBlockMetrics(subBlocks);

            if (bm.variation > btDesc.variationThreshold)
                return BlockRef(new ParentBlock(dimensions, subBlocks));

            for (auto& block : subBlocks)
                if (block->getType() == BlockType::Color)
                    ColorBlockPool.give(std::static_pointer_cast<ColorBlock>(block));

            return ColorBlockPool.take(dimensions, bm.avgColor);
        }
    }
}