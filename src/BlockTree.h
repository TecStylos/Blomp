#pragma once

#include "Blocks.h"
#include "Descriptors.h"

namespace Blomp
{
    namespace BlockTree
    {
        ParentBlockRef fromImage(const Image& img, const BlockTreeDesc& btDesc);

        ParentBlockRef fromFile(const std::string& filename);

        void toFile(const std::string& filename);
    }
}