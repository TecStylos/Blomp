#include "Blocks.h"
#include "Descriptors.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

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
        if (m_x + m_w > img.width() || m_y + m_h > img.height())
            throw std::runtime_error("Image dimensions too small.");

        for (int y = m_y; y < m_y + m_h; ++y)
            for (int x = m_x; x < m_x + m_w; ++x)
                img.getNC(x, y) = m_color;
    }

    void ColorBlock::writeHeatmap(Image &img, int maxDepth, int depth) const
    {
        if (m_x + m_w > img.width() || m_y + m_h > img.height())
            throw std::runtime_error("Image dimensions too small.");

        Pixel color = 1.0f / maxDepth * depth;
        for (int y = m_y; y < m_y + m_h; ++y)
            for (int x = m_x; x < m_x + m_w; ++x)
                img.getNC(x, y) = color;
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
        int blockDim = calcDimVal(btDesc.maxDepth, newDepth);

        for (int y = m_y; y < (m_y + pbDesc.height) && y < img.height(); y += blockDim)
            for (int x = m_x; x < (m_x + pbDesc.width) && x < img.width(); x += blockDim)
                m_subBlocks.push_back(createSubBlock(x, y, newDepth, btDesc, img));
    }

    ParentBlock::ParentBlock(const ParentBlockDesc& pbDesc, const BlockTreeDesc& btDesc, int imgWidth, int imgHeight, BitStream& bitStream)
        : Block(pbDesc.x, pbDesc.y, pbDesc.width, pbDesc.height)
    {
        int newDepth = pbDesc.depth + 1;
        int blockDim = calcDimVal(btDesc.maxDepth, newDepth);

        for (int y = m_y; y < (m_y + pbDesc.height) && y < imgHeight; y += blockDim)
            for (int x = m_x; x < (m_x + pbDesc.width) && x < imgWidth; x += blockDim)
                m_subBlocks.push_back(createSubBlock(x, y, newDepth, btDesc, imgWidth, imgHeight, bitStream));
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
        BlockMetrics bm = calcBlockMetrics(x, y, btDesc.maxDepth, newDepth, img);

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

        int maxDim = calcDimVal(btDesc.maxDepth, newDepth);
        int blockWidth = std::min(imgWidth - x, maxDim);
        int blockHeight = std::min(imgHeight - y, maxDim);

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

    BlockMetrics ParentBlock::calcBlockMetrics(int x, int y, int maxDepth, int newDepth, const Image& img)
    {
        BlockMetrics bm;

        int maxDim = calcDimVal(maxDepth, newDepth);

        bm.width = std::min(img.width() - x, maxDim);
        bm.height = std::min(img.height() - y, maxDim);
        int nPixels = bm.width * bm.height;

        for (int ry = y; ry < y + bm.height; ++ry)
           for (int rx = x; rx < x + bm.width; ++rx)
               bm.avgColor += img.getNC(rx, ry);

        bm.avgColor /= nPixels;

        for (int ry = y; ry < y + bm.height; ++ry)
        {
           for (int rx = x; rx < x + bm.width; ++rx)
           {
               Pixel diff = bm.avgColor - img.getNC(rx, ry);
               diff *= diff;
               bm.variation += diff.r + diff.g + diff.b;
           }
        }

        bm.variation /= nPixels;

        // v = (a - p1)^2 + (a - p2)^2 + ...
        // v = 2a^2 - 2a(p1 + p2 + ...) + p1^2 + p2^2 + ...

        // Pixel pxSum, px2Sum;
        // for (int ry = y; ry < y + bm.height; ++ry)
        // {
        //     for (int rx = x; rx < x + bm.width; ++rx)
        //     {
        //         const Pixel& px = img.getNC(rx, ry);
        //         bm.avgColor += px;
        //         pxSum += px;
        //         px2Sum += px * px;
        //     }
        // }

        // bm.avgColor /= Pixel(nPixels);

        // Pixel avg22 = Pixel(2.0f) * bm.avgColor * bm.avgColor;
        // Pixel pxSumAvg2 = Pixel(2.0f) * bm.avgColor * pxSum;
        // Pixel result = avg22 - pxSumAvg2 + px2Sum;
        // result /= nPixels;
        // bm.variation = result.r + result.g + result.b;
        // //bm.variation /= nPixels;

        // std::cout << "AvgCol:    " << bm.avgColor.r << "  " << bm.avgColor.g << "  " << bm.avgColor.b << std::endl;
        // std::cout << "PxSum:     " << pxSum.r << "  " << pxSum.g << "  " << pxSum.b << std::endl;
        // std::cout << "Px2Sum:    " << px2Sum.r << "  " << px2Sum.g << "  " << px2Sum.b << std::endl;
        // std::cout << "Avg22:     " << avg22.r << "  " << avg22.g << "  " << avg22.b << std::endl;
        // std::cout << "PxSumAvg2: " << pxSumAvg2.r << "  " << pxSumAvg2.g << "  " << pxSumAvg2.b << std::endl;
        // std::cout << "Result:    " << result.r << "  " << result.g << "  " << result.b << std::endl;
        // std::cout << "Dimens:    " << bm.width << "  " << bm.height << std::endl;
        // std::cout << "Variat:    " << bm.variation << std::endl;
        // std::cout << "-----" << std::endl;

        return bm;
    }
}