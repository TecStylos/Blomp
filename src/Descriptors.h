#pragma once

namespace Blomp
{
    struct BlockTreeDesc
    {
        int maxDepth;
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
        int maxDepth;
    };
}