#pragma once

namespace Blomp
{
    struct BlockTreeDesc
    {
        int baseWidthExp, baseHeightExp;
        float variationThreshold;
    };

    struct ParentBlockDesc
    {
        int x, y;
        int width, height;
        int depth;
    };

    struct BaseDescriptor
    {
        int imgWidth, imgHeight;
        int baseWidthExp, baseHeightExp;
    };
}