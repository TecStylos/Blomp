#include <iostream>

#include "BlockTree.h"
#include "Descriptors.h"
#include "Image.h"

int main(int argc, const char** argv, const char** env)
{
    std::cout << "Loading the image..." << std::endl;
    Blomp::Image img("stockImg.jpg");

    std::cout << "Dimensions: " << img.width() << "x" << img.height() << std::endl;

    Blomp::BlockTreeDesc btDesc;
    btDesc.baseWidthExp = 7;
    btDesc.baseHeightExp = 7;
    btDesc.variationThreshold = 0.01f;

    std::cout << "Creating blockTree from image..." << std::endl;
    auto bt = Blomp::BlockTree::fromImage(img, btDesc);

    std::cout << "Creating img2 structure..." << std::endl;
    Blomp::Image img2(bt->getWidth(), bt->getHeight());

    std::cout << "Converting blockTree to image..." << std::endl;
    bt->writeToImg(img2);

    std::cout << "Storing the img2 to disk..." << std::endl;
    img2.save("image2.png");

    std::cout << "DONE!" << std::endl;
    return 0;
}