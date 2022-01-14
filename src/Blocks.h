#pragma once

#include <memory>
#include <vector>

#include "Image.h"
#include "Descriptors.h"

namespace Blomp
{
    class Block
    {
    public:
        Block() = delete;
        Block(int x, int y, int w, int h);
    public:
        virtual void writeToImg(Image& img) const = 0;
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
    protected:
        Pixel m_color;
    };

    class ParentBlock : public Block
    {
    public:
        ParentBlock() = delete;
        ParentBlock(const ParentBlockDesc& pbDesc, const BlockTreeDesc& btDesc, const Image& img);
    public:
        virtual void writeToImg(Image& img) const override;
    protected:
        std::vector<BlockRef> m_subBlocks;
    protected:
        static BlockRef createSubBlock(int x, int y, int parentDepth, const BlockTreeDesc& btDesc, const Image& img);
        static int calcDimChange(int depth);
    };
    typedef std::shared_ptr<ParentBlock> ParentBlockRef;
}