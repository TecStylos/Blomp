#include "Blocks.h"
#include "Descriptors.h"
#include <math.h>

namespace Blomp
{
    Block::Block(int x, int y, int w, int h)
        : m_x(x), m_y(y), m_w(w), m_h(h)
    {}

    ColorBlock::ColorBlock(int x, int y, int w, int h, Pixel color)
        : Block(x, y, w, h), m_color(color)
    {}

    void ColorBlock::writeToImg(Image &img) const
    {
        for (int x = m_x; x < m_x + m_w; ++x)
            for (int y = m_y; y < m_y + m_h; ++y)
                img(x, y) = m_color;
    }

    ParentBlock::ParentBlock(const ParentBlockDesc& pbDesc, const BlockTreeDesc& btDesc, const Image& img)
        : Block(pbDesc.x, pbDesc.y, pbDesc.width, pbDesc.height)
    {
        int dimensionChange = calcDimChange(pbDesc.depth);

        int blockWidth = btDesc.baseWidth / dimensionChange;
        int blockHeight = btDesc.baseHeight / dimensionChange;

        for (int y = pbDesc.y; y < pbDesc.y + pbDesc.height && y < img.height(); y += blockHeight)
        {
            for (int x = pbDesc.x; x < pbDesc.x + pbDesc.width && x < img.width(); x += blockWidth)
            {
                m_subBlocks.push_back(createSubBlock(x, y, pbDesc.depth, btDesc, img));
            }
        }
    }

    void ParentBlock::writeToImg(Image &img) const
    {
        for (auto& block : m_subBlocks)
            block->writeToImg(img);
    }

    BlockRef ParentBlock::createSubBlock(int x, int y, int parentDepth, const BlockTreeDesc& btDesc, const Image& img)
    {
        // TODO: Implementation

        ParentBlockDesc pbDesc;
        pbDesc.x = x;
        pbDesc.y = y;
        pbDesc.width /= 2;
        pbDesc.height /= 2;
        pbDesc.depth = parentDepth;

        return nullptr;
    }

    int ParentBlock::calcDimChange(int depth)
    {
        return std::pow(2, depth);
    }
}