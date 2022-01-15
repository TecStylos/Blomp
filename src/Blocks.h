#pragma once

#include <memory>
#include <vector>

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
        int getX() const;
        int getY() const;
        int getWidth() const;
        int getHeight() const;
    public:
        virtual void writeToImg(Image& img) const = 0;
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
    public:
        virtual void writeToImg(Image& img) const override;
        virtual void serialize(BitStream& bitStream) const override;
    public:
        int nBlocks() const;
        int nColorBlocks() const;
    protected:
        std::vector<BlockRef> m_subBlocks;
    public:
        static ParentBlockRef deserialize(BitStream& bitStream);
    protected:
        static BlockRef createSubBlock(int x, int y, int newDepth, const BlockTreeDesc& btDesc, const Image& img);
        static int calcDimVal(int base, int depth);
        static BlockMetrics calcBlockMetrics(int x, int y, int baseWidthExp, int baseHeightExp, int newDepth, const Image& img);
    };
}