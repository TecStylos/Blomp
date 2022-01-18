#include "Blocks.h"
#include "BlockPool.h"
#include "Descriptors.h"
#include <algorithm>
#include <memory>
#include <stdexcept>

#include "BlockTreeGen.h"

namespace Blomp
{
    Block::Block(BlockDim dimensions)
        : m_dim(dimensions)
    {}

    BlockType Block::getType() const
    {
        return BlockType::None;
    }

    Pixel Block::getColor() const
    {
        return Pixel(1.0f, 0.0f, 1.0f);
    }

    ColorBlock::ColorBlock(BlockDim dimensions, Pixel color)
        : Block(dimensions), m_color(color)
    {}

    BlockType ColorBlock::getType() const
    {
        return BlockType::Color;
    }

    Pixel ColorBlock::getColor() const
    {
        return m_color;
    }

    void ColorBlock::writeToImg(Image &img) const
    {
        if (m_dim.x + m_dim.w > img.width() || m_dim.y + m_dim.h > img.height())
            throw std::runtime_error("Image dimensions too small.");

        for (int x = m_dim.x; x < m_dim.x + m_dim.w; ++x)
            for (int y = m_dim.y; y < m_dim.y + m_dim.h; ++y)
                img(x, y) = m_color;
    }

    void ColorBlock::writeHeatmap(Image &img, int maxDepth, int depth) const
    {
        if (m_dim.x + m_dim.w > img.width() || m_dim.y + m_dim.h > img.height())
            throw std::runtime_error("Image dimensions too small.");

        Pixel color = 1.0f / maxDepth * depth;
        for (int x = m_dim.x; x < m_dim.x + m_dim.w; ++x)
            for (int y = m_dim.y; y < m_dim.y + m_dim.h; ++y)
                img(x, y) = color;
    }

    void ColorBlock::serialize(BitStream &bitStream) const
    {
        bitStream.writeBit(false);
        uint8_t pixelData[3];
        m_color.toCharArray(pixelData);
        bitStream.write(pixelData, 3 * 8);
    }

    int ColorBlock::nBlocks() const
    {
        return 1;
    }

    int ColorBlock::nColorBlocks() const
    {
        return 1;
    }

    ParentBlock::ParentBlock(BlockDim dimensions, const std::vector<BlockRef>& subBlocks)
        : Block(dimensions), m_subBlocks(subBlocks)
    {}

    ParentBlock::ParentBlock(const ParentBlockDesc& pbDesc, const BlockTreeDesc& btDesc, int imgWidth, int imgHeight, BitStream& bitStream)
        : Block(BlockDim(pbDesc.x, pbDesc.y, pbDesc.width, pbDesc.height))
    {
        int newDepth = pbDesc.depth + 1;
        int blockDim = BlockTree::calcDimVal(btDesc.maxDepth, newDepth);

        for (int y = m_dim.y; y < (m_dim.y + pbDesc.height) && y < imgHeight; y += blockDim)
            for (int x = m_dim.x; x < (m_dim.x + pbDesc.width) && x < imgWidth; x += blockDim)
                m_subBlocks.push_back(createSubBlock(x, y, newDepth, btDesc, imgWidth, imgHeight, bitStream));
    }

    ParentBlock::~ParentBlock()
    {
        for (auto& block : m_subBlocks)
            if (block->getType() == BlockType::Color)
                ColorBlockPool.give(std::static_pointer_cast<ColorBlock>(block));
    }

    BlockType ParentBlock::getType() const
    {
        return BlockType::Parent;
    }

    void ParentBlock::writeToImg(Image &img) const
    {
        for (auto& block : m_subBlocks)
            block->writeToImg(img);
    }

    void ParentBlock::writeHeatmap(Image &img, int maxDepth, int depth) const
    {
        for (auto& block : m_subBlocks)
            block->writeHeatmap(img, maxDepth, depth + 1);
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
            count += block->nBlocks();

        return count;
    }

    int ParentBlock::nColorBlocks() const
    {
        int count = 0;

        for (auto block : m_subBlocks)
            count += block->nColorBlocks();

        return count;
    }

    BlockRef ParentBlock::createSubBlock(int x, int y, int newDepth, const BlockTreeDesc& btDesc, int imgWidth, int imgHeight, BitStream& bitStream)
    {
        bool isParent = bitStream.readBit();

        int maxDim = BlockTree::calcDimVal(btDesc.maxDepth, newDepth);
        int blockWidth = std::min(imgWidth - x, maxDim);
        int blockHeight = std::min(imgHeight - y, maxDim);

        if (!isParent)
        {
            uint8_t pixelData[3];
            bitStream.read(pixelData, 3 * 8);
            return BlockRef(new ColorBlock(BlockDim(x, y, blockWidth, blockHeight), Pixel::fromCharArray(pixelData)));
        }

        ParentBlockDesc pbDesc;
        pbDesc.x = x;
        pbDesc.y = y;
        pbDesc.width = blockWidth;
        pbDesc.height = blockHeight;
        pbDesc.depth = newDepth;

        return BlockRef(new ParentBlock(pbDesc, btDesc, imgWidth, imgHeight, bitStream));
    }
}