#include "ImgCompare.h"

#include <cmath>

#include "Image.h"

namespace Blomp
{
    float compareImages(const Image &img1, const Image &img2)
    {
        float diffSum = 0;
        for (int y = 0; y < img1.height(); ++y)
        {
            for (int x = 0; x < img1.width(); ++x)
            {
                auto pixelDiff = img1(x, y) - img2(x, y);
                pixelDiff *= pixelDiff;

                diffSum += (pixelDiff.r + pixelDiff.g + pixelDiff.b) / 3.0f;
            }
        }

        diffSum /= img1.width() * img1.height();
        
        return std::pow(1.0f - float(diffSum), 128);
    }
}