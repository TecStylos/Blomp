#pragma once

#include <memory>
#include <vector>
#include <math.h>

#include "Image.h"
#include "BitStream.h"
#include "Descriptors.h"

namespace Blomp
{
    struct BlockMetrics
    {
        int width, height;
        float variation = 0.0f;
        Pixel avgColor;
    };
    
    class Block
    {
    public:
        Block() = delete;
        Block(int x, int y, int w, int h);
    public:
        int getWidth() const;
        int getHeight() const;
    public:
        virtual void writeToImg(Image& img) const = 0;
        virtual void writeHeatmap(Image& img, int maxDepth, int depth = -1) const = 0;
        virtual void serialize(BitStream& bitStream) const = 0;
    protected:
        int m_x, m_y;
        int m_w, m_h;
    };
    typedef std::shared_ptr<Block> BlockRef;

    class ColorBlock : public Block
    {
    public:
        ColorBlock() = delete;
        ColorBlock(int x, int y, int w, int h, Pixel color);
    public:
        virtual void writeToImg(Image& img) const override;
        virtual void writeHeatmap(Image& img, int maxDepth, int depth = -1) const override;
        virtual void serialize(BitStream& bitStream) const override;
    protected:
        Pixel m_color;
    };

    class ParentBlock;
    typedef std::shared_ptr<ParentBlock> ParentBlockRef;
    
    class ParentBlock : public Block
    {
    public:
        ParentBlock() = delete;
        ParentBlock(const ParentBlockDesc& pbDesc, const BlockTreeDesc& btDesc, const Image& img);
        ParentBlock(const ParentBlockDesc& pbDesc, const BlockTreeDesc& btDesc, int imgWidth, int imgHeight, BitStream& bitStream);
    public:
        virtual void writeToImg(Image& img) const override;
        virtual void writeHeatmap(Image& img, int maxDepth, int depth = -1) const override;
        virtual void serialize(BitStream& bitStream) const override;
    public:
        int nBlocks() const;
        int nColorBlocks() const;
    protected:
        std::vector<BlockRef> m_subBlocks;
    protected:
        static BlockRef createSubBlock(int x, int y, int newDepth, const BlockTreeDesc& btDesc, const Image& img);
        static BlockRef createSubBlock(int x, int y, int newDepth, const BlockTreeDesc& btDesc, int imgWidth, int imgHeight, BitStream& bitStream);
        static int calcDimVal(int base, int depth);
        static BlockMetrics calcBlockMetrics(int x, int y, int maxDepth, int newDepth, const Image& img);
    };

    inline int Block::getWidth() const
    {
        return m_w;
    }

    inline int Block::getHeight() const
    {
        return m_h;
    }

    inline int ParentBlock::calcDimVal(int base, int depth)
    {
        if (depth > base)
            throw std::runtime_error("FATAL: depth > maxDepth!!!");

        return std::pow(2, base - depth);
    }
}