#pragma once

namespace Blomp
{
    struct BlockTreeDesc
    {
        int baseWidth, baseHeight;
        float variation;
    };

    struct ParentBlockDesc
    {
        int x, y;
        int width, height;
        int depth;
    };
}