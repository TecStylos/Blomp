#include <iostream>

#include "Image.h"

int main(int argc, const char** argv, const char** env)
{
    std::cout << "Creating image structure..." << std::endl;
    //Blomp::Image img(100, 100);
    Blomp::Image img("image.png");

    std::cout << "W x H: " << img.width() << "x" << img.height() << std::endl;

    std::cout << "Writing image data..." << std::endl;
    for (int x = 0; x < img.width(); ++x)
    {
        for (int y = 0; y < img.height(); ++y)
        {
            auto& pix = img(x, y);
            pix.r *= 0.5f;
            pix.g *= 0.5f;
            pix.b *= 0.5f;
            //img(x, y) = Blomp::Pixel((float)x / 100.0f, (float)y / 100.0f, 0.3f);
        }
    }

    std::cout << "Storing the image to disk..." << std::endl;
    img.save("image.png");

    std::cout << "DONE!" << std::endl;
    return 0;
}