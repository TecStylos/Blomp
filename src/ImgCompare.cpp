#include "ImgCompare.h"

#include <cmath>
#include <stdexcept>

#include "Image.h"

namespace Blomp
{
    float compareImages(const Image &img1, const Image &img2)
    {
        if (img1.width() != img2.width() || img1.height() != img2.height())
            throw std::runtime_error("Unable to compare images with different dimensions.");

        float diffSum = 0;
        for (int y = 0; y < img1.height(); ++y)
        {
            for (int x = 0; x < img1.width(); ++x)
            {
                auto pixelDiff = img1.getNC(x, y) - img2.getNC(x, y);
                pixelDiff *= pixelDiff;

                diffSum += (pixelDiff.r + pixelDiff.g + pixelDiff.b) / 3.0f;
            }
        }

        diffSum /= img1.width() * img1.height();
        
        return std::pow(1.0f - float(diffSum), 128);
    }
}