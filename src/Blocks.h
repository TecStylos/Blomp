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
        float variation = 0.0f;
        Pixel avgColor;
    };

    enum class BlockType
    {
        None,
        Color,
        Parent
    };

    struct BlockDim
    {
        int x, y, w, h;
    public:
        BlockDim(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
    };
    
    class Block
    {
    public:
        Block() = delete;
        Block(BlockDim dimensions);
    public:
        int getWidth() const;
        int getHeight() const;
    public:
        virtual BlockType getType() const;
        virtual Pixel getColor() const;
        virtual void writeToImg(Image& img) const = 0;
        virtual void writeHeatmap(Image& img, int maxDepth, int depth = -1) const = 0;
        virtual void serialize(BitStream& bitStream) const = 0;
        virtual int nBlocks() const = 0;
        virtual int nColorBlocks() const = 0;
    protected:
        BlockDim m_dim;
    };
    typedef std::shared_ptr<Block> BlockRef;

    class ColorBlock : public Block
    {
    public:
        ColorBlock() = delete;
        ColorBlock(BlockDim dimensions, Pixel color);
    public:
        virtual BlockType getType() const override;
        virtual Pixel getColor() const override;
        virtual void writeToImg(Image& img) const override;
        virtual void writeHeatmap(Image& img, int maxDepth, int depth = -1) const override;
        virtual void serialize(BitStream& bitStream) const override;
        virtual int nBlocks() const override;
        virtual int nColorBlocks() const override;
    protected:
        Pixel m_color;
    };
    
    class ParentBlock : public Block
    {
    public:
        ParentBlock() = delete;
        ParentBlock(BlockDim dimensions, const std::vector<BlockRef>& subBlocks);
        ParentBlock(const ParentBlockDesc& pbDesc, const BlockTreeDesc& btDesc, int imgWidth, int imgHeight, BitStream& bitStream);
        ~ParentBlock();
    public:
        virtual BlockType getType() const override;
        virtual void writeToImg(Image& img) const override;
        virtual void writeHeatmap(Image& img, int maxDepth, int depth = -1) const override;
        virtual void serialize(BitStream& bitStream) const override;
        virtual int nBlocks() const override;
        virtual int nColorBlocks() const override;
    protected:
        std::vector<BlockRef> m_subBlocks;
    protected:
        static BlockRef createSubBlock(int x, int y, int newDepth, const BlockTreeDesc& btDesc, int imgWidth, int imgHeight, BitStream& bitStream);
    };

    inline int Block::getWidth() const
    {
        return m_dim.w;
    }

    inline int Block::getHeight() const
    {
        return m_dim.h;
    }
}