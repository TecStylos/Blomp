#include "Blocks.h"
#include "Descriptors.h"
#include <algorithm>
#include <math.h>

namespace Blomp
{
    Block::Block(int x, int y, int w, int h)
        : m_x(x), m_y(y), m_w(w), m_h(h)
    {}

    int Block::getX() const
    {
        return m_x;
    }

    int Block::getY() const
    {
        return m_y;
    }

    int Block::getWidth() const
    {
        return m_w;
    }

    int Block::getHeight() const
    {
        return m_h;
    }

    ColorBlock::ColorBlock(int x, int y, int w, int h, Pixel color)
        : Block(x, y, w, h), m_color(color)
    {}

    void ColorBlock::writeToImg(Image &img) const
    {
        for (int x = m_x; x < m_x + m_w; ++x)
            for (int y = m_y; y < m_y + m_h; ++y)
                img(x, y) = m_color;
    }

    void ColorBlock::serialize(BitStream &bitStream) const
    {
        bitStream.writeBit(false);
        uint8_t pixelData[3];
        m_color.toCharArray(pixelData);
        bitStream.write(pixelData, 3 * 8);
    }

    ParentBlock::ParentBlock(const ParentBlockDesc& pbDesc, const BlockTreeDesc& btDesc, const Image& img)
        : Block(pbDesc.x, pbDesc.y, pbDesc.width, pbDesc.height)
    {
        int newDepth = pbDesc.depth + 1;
        int blockWidth = calcDimVal(btDesc.baseWidthExp, newDepth);
        int blockHeight = calcDimVal(btDesc.baseHeightExp, newDepth);

        for (int y = m_y; y < (m_y + pbDesc.height) && y < img.height(); y += blockHeight)
            for (int x = m_x; x < (m_x + pbDesc.width) && x < img.width(); x += blockWidth)
                m_subBlocks.push_back(createSubBlock(x, y, newDepth, btDesc, img));
    }

    ParentBlock::ParentBlock(const ParentBlockDesc& pbDesc, const BlockTreeDesc& btDesc, int imgWidth, int imgHeight, BitStream& bitStream)
        : Block(pbDesc.x, pbDesc.y, pbDesc.width, pbDesc.height)
    {
        int newDepth = pbDesc.depth + 1;
        int blockWidth = calcDimVal(btDesc.baseWidthExp, newDepth);
        int blockHeight = calcDimVal(btDesc.baseHeightExp, newDepth);

        for (int y = m_y; y < (m_y + pbDesc.height) && y < imgHeight; y += blockHeight)
            for (int x = m_x; x < (m_x + pbDesc.width) && x < imgWidth; x += blockWidth)
                m_subBlocks.push_back(createSubBlock(x, y, newDepth, btDesc, imgWidth, imgHeight, bitStream));
    }

    void ParentBlock::writeToImg(Image &img) const
    {
        for (auto& block : m_subBlocks)
            block->writeToImg(img);
    }

    void ParentBlock::serialize(BitStream &bitStream) const
    {
        bitStream.writeBit(true);
        for (auto& block : m_subBlocks)
            block->serialize(bitStream);
    }

    int ParentBlock::nBlocks() const
    {
        int count = 1;

        for (auto block : m_subBlocks)
        {
            const ParentBlock* pb = dynamic_cast<const ParentBlock*>(block.get());
            if (pb)
                count += pb->nBlocks();
            else
                ++count;
        }

        return count;
    }

    int ParentBlock::nColorBlocks() const
    {
        int count = 0;

        for (auto block : m_subBlocks)
        {
            const ParentBlock* pb = dynamic_cast<const ParentBlock*>(block.get());
            if (pb)
                count += pb->nColorBlocks();
            else
                ++count;
        }

        return count;
    }

    BlockRef ParentBlock::createSubBlock(int x, int y, int newDepth, const BlockTreeDesc& btDesc, const Image& img)
    {
        BlockMetrics bm = calcBlockMetrics(x, y, btDesc.baseWidthExp, btDesc.baseHeightExp, newDepth, img);

        if (bm.variation <= btDesc.variationThreshold)
            return BlockRef(new ColorBlock(x, y, bm.width, bm.height, bm.avgColor));

        ParentBlockDesc pbDesc;
        pbDesc.x = x;
        pbDesc.y = y;
        pbDesc.width = bm.width;
        pbDesc.height = bm.height;
        pbDesc.depth = newDepth;

        return BlockRef(new ParentBlock(pbDesc, btDesc, img));
    }

    BlockRef ParentBlock::createSubBlock(int x, int y, int newDepth, const BlockTreeDesc& btDesc, int imgWidth, int imgHeight, BitStream& bitStream)
    {
        bool isParent = bitStream.readBit();

        int blockWidth = std::min(imgWidth - x, calcDimVal(btDesc.baseWidthExp, newDepth));
        int blockHeight = std::min(imgHeight - y, calcDimVal(btDesc.baseHeightExp, newDepth));

        if (!isParent)
        {
            uint8_t pixelData[3];
            bitStream.read(pixelData, 3 * 8);
            return BlockRef(new ColorBlock(x, y, blockWidth, blockHeight, Pixel::fromCharArray(pixelData)));
        }

        ParentBlockDesc pbDesc;
        pbDesc.x = x;
        pbDesc.y = y;
        pbDesc.width = blockWidth;
        pbDesc.height = blockHeight;
        pbDesc.depth = newDepth;

        return BlockRef(new ParentBlock(pbDesc, btDesc, imgWidth, imgHeight, bitStream));
    }

    int ParentBlock::calcDimVal(int base, int depth)
    {
        return std::pow(2, base - depth);
    }

    BlockMetrics ParentBlock::calcBlockMetrics(int x, int y, int baseWidthExp, int baseHeightExp, int newDepth, const Image& img)
    {
        BlockMetrics bm;

        bm.width = std::min(img.width() - x, calcDimVal(baseWidthExp, newDepth));
        bm.height = std::min(img.height() - y, calcDimVal(baseHeightExp, newDepth));

        for (int rx = x; rx < x + bm.width; ++rx)
            for (int ry = y; ry < y + bm.height; ++ry)
                bm.avgColor += img(rx, ry);

        bm.avgColor /= (float)(bm.width * bm.height);

        for (int rx = x; rx < x + bm.width; ++rx)
        {
            for (int ry = y; ry < y + bm.height; ++ry)
            {
                Pixel diff = bm.avgColor - img(rx, ry);
                diff *= diff;
                bm.variation += diff.r + diff.g + diff.b;
            }
        }

        bm.variation /= (float)(bm.width * bm.height);

        return bm;
    }
}