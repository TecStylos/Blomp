#include <fstream>
#include <iostream>
#include <string>

#include "BitStream.h"
#include "BlockTree.h"
#include "Descriptors.h"
#include "Image.h"
#include "FileHeader.h"

const char* helpText = 
"Usage:\n"
"  blomp [mode] [options] [inFile]\n"
"\n"
"Modes:\n"
"  help         View this help.\n"
"  enc          Image -> Blomp\n"
"  dec          Blomp -> Image\n"
"\n"
"Options:\n"
"  -d [int]     Block depth. Default: 4\n"
"  -v [float]   Variation threshold. Default: 0.02\n"
"  -o [string]  Output filename. Default: [inFile] with changed file extension.\n"
"  -m [string]  Heatmap filename. Generate a compression heatmap if specified.\n"
"\n"
"Supported image formats:\n"
"  Mode | JPG PNG TGA BMP PSD GIF HDR PIC PNM\n"
"   enc    X   X   X   X   X   X   X   X   X \n"
"   dec    X   X   X   X                     \n"
"  For more details see https://github.com/nothings/stb \n"
"";


void viewBlockTreeInfo(Blomp::ParentBlockRef bt)
{
    int nBlocks = bt->nBlocks();
    int nColorBlocks = bt->nColorBlocks();
    int estFileSizeBits = sizeof(Blomp::FileHeader) * 8 + sizeof(uint64_t) * 8 + nBlocks + nColorBlocks * 3 * 8;
    int estFileSizeBytes = (estFileSizeBits + 7) / 8;
    std::cout << "  Blocks:      " << nBlocks << std::endl;
    std::cout << "  ColorBlocks: " << nColorBlocks << std::endl;
    std::cout << "  EstFileSize: " << estFileSizeBytes << " bytes" << std::endl;
}

void autoGenSaveHeatmap(Blomp::ParentBlockRef bt, Blomp::Image& img, const std::string& heatmapFile)
{
    if (heatmapFile.empty())
        return;

    std::cout << "Generating heatmap..." << std::endl;
    bt->writeHeatmap(img, 10);

    std::cout << "Saving heatmap..." << std::endl;
    img.save(heatmapFile);

}

//
// TODO: ERROR CHECKING!!!
//
int main(int argc, const char** argv, const char** env)
{
    Blomp::FileHeader fileHeader;

    Blomp::BlockTreeDesc btDesc;
    btDesc.maxDepth = 4;
    btDesc.variationThreshold = 0.02f;
    std::string inFile = "";
    std::string outFile = "";
    std::string heatmapFile = "";

    if (argc < 2)
    {
        std::cout << "Missing mode." << std::endl;
        std::cout << "For help run 'blomp help'." << std::endl;;
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "help")
    {
        std::cout << helpText;
        return 1;
    }

    if (mode != "enc" && mode != "dec")
    {
        std::cout << "Unknown mode selected." << std::endl;
        return 1;
    }

    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];
        bool invalidValue = false;

        if (arg == "-d")
        {
            ++i;
            if (i >= argc)
            {
                std::cout << "Missing value after option '-d'." << std::endl;
                return 1;
            }
            try 
            {
                btDesc.maxDepth = std::stoi(argv[i]);

                if (btDesc.maxDepth < 0 || 10 < btDesc.maxDepth)
                    invalidValue = true;
            }
            catch (std::exception e)
            {
                invalidValue = true;
            }
        }
        else if (arg == "-v")
        {
            ++i;
            if (i >= argc)
            {
                std::cout << "Missing value after option '-v'." << std::endl;
                return 1;
            }
            try 
            {
                btDesc.variationThreshold = std::stof(argv[i]);

                if (btDesc.variationThreshold < 0.0f || 1.0f < btDesc.variationThreshold)
                    invalidValue = true;
            }
            catch (std::exception e)
            {
                invalidValue = true;
            }
        }
        else if (arg == "-o")
        {
            ++i;
            if (i >= argc)
            {
                std::cout << "Missing value after option '-o'." << std::endl;
                return 1;
            }

            outFile = argv[i];
        }
        else if (arg == "-m")
        {
            ++i;
            if (i >= argc)
            {
                std::cout << "Missing value after option '-o'." << std::endl;
                return 1;
            }

            heatmapFile = argv[i];
        }
        else
        {
            inFile = arg;
        }

        if (invalidValue)
        {
            std::cout << "Invalid value after option '" << arg << "'." << std::endl;
            return 1;
        }
    }

    if (inFile.empty())
    {
        std::cout << "Missing input file." << std::endl;
        return 1;
    }

    if (outFile.empty())
    {
        outFile = inFile.substr(0, inFile.find_last_of(".")) + (mode == "enc" ? ".blp" : ".png");
    }

    if (mode == "enc")
    {
        std::cout << "Loading source image..." << std::endl;
        Blomp::Image img(inFile);
        std::cout << "Generating block tree..." << std::endl;
        auto bt = Blomp::BlockTree::fromImage(img, btDesc);

        viewBlockTreeInfo(bt);

        std::cout << "Saving block tree..." << std::endl;
        Blomp::BitStream bitStream;
        Blomp::BlockTree::serialize(bt, bitStream);
        fileHeader.bd.imgWidth = img.width();
        fileHeader.bd.imgHeight = img.height();
        fileHeader.bd.maxDepth = btDesc.maxDepth;

        std::ofstream ofStream(outFile, std::ios::binary | std::ios::out | std::ios::trunc);
        ofStream.write((const char*)&fileHeader, sizeof(fileHeader));
        ofStream << bitStream;
        ofStream.close();

        autoGenSaveHeatmap(bt, img, heatmapFile);
    }
    else if (mode == "dec")
    {
        std::cout << "Loading block tree data..." << std::endl;
        std::ifstream ifStream(inFile, std::ios::binary | std::ios::in);
        ifStream.read((char*)&fileHeader, sizeof(fileHeader));
        Blomp::BitStream bitStream(ifStream);
        ifStream.close();
        auto bt = Blomp::BlockTree::deserialize(fileHeader.bd, bitStream);

        viewBlockTreeInfo(bt);

        std::cout << "Generating image..." << std::endl;
        Blomp::Image img(bt->getWidth(), bt->getHeight());
        bt->writeToImg(img);

        std::cout << "Saving image..." << std::endl;
        img.save(outFile);

        autoGenSaveHeatmap(bt, img, heatmapFile);
    }
    else
    {
        std::cout << "Unknown mode selected.";
        return 1;
    }
    
    return 0;
}