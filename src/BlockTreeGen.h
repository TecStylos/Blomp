#pragma once

#include "Blocks.h"

namespace Blomp
{
    namespace BlockTree
    {
        BlockRef createSubBlock(BlockDim dimensions, int newDepth, const BlockTreeDesc& btDesc, const Image& img);
        
        int calcDimVal(int base, int depth);
        
        BlockMetrics calcSubBlockMetrics(const std::vector<BlockRef>& subBlocks);





        inline int calcDimVal(int base, int depth)
        {
            if (depth > base)
                throw std::runtime_error("FATAL: depth > maxDepth!!!");

            return std::pow(2, base - depth);
        }

        inline BlockMetrics calcSubBlockMetrics(const std::vector<BlockRef>& subBlocks)
        {
            BlockMetrics bm;

            for (auto& block : subBlocks)
                bm.avgColor += block->getColor();

            bm.avgColor /= subBlocks.size();

            for (auto& block : subBlocks)
            {
                Pixel diff = bm.avgColor - block->getColor();
                diff *= diff;
                bm.variation += diff.r + diff.g + diff.b;
            }

            bm.variation /= subBlocks.size();

            return bm;
        }
    }
}