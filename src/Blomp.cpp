#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>

#include "Tools.h"
#include "BitStream.h"
#include "BlockTree.h"
#include "Descriptors.h"
#include "Image.h"
#include "FileHeader.h"
#include "ImgCompare.h"

const char *helpText =
R"(Usage:
  blomp [mode] [options] [inFile]

Modes:
  help         View this help.
  enc          Convert an image file to a blomp file.
  dec          Convert a blomp file to an image file.
  comp         Compare two images with the same dimensions.

Options:
  -d [int]     Block depth. Default: 4, Range: 0-10 (Used in mode 'enc')
  -v [float]   Variation threshold. Default: 0.02, Range: 0.0 - 1.0 (Used in mode 'enc')
  -o [string]  Output filename. Default: [inFile] with changed file extension. (Used in modes 'enc' and 'dec')
  -m [string]  Heatmap filename. Generate a compression heatmap if specified. (Used in modes 'enc' and 'dec')
  -c [string]  Comparison file. (Used in mode 'comp')

Supported image formats:
  Mode | JPG PNG TGA BMP PSD GIF HDR PIC PNM
   enc    X   X   X   X   X   X   X   X   X
   dec    X   X   X   X
   comp   X   X   X   X   X   X   X   X   X
  For more details see https://github.com/nothings/stb 
)";

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

int main(int argc, const char** argv, const char** env)
{
    Blomp::FileHeader fileHeader;

    Blomp::BlockTreeDesc btDesc;
    btDesc.maxDepth = 4;
    btDesc.variationThreshold = 0.02f;
    std::string inFile = "";
    std::string outFile = "";
    std::string heatmapFile = "";
    std::string compFile = "";

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
                std::cout << "Missing value after option '-m'." << std::endl;
                return 1;
            }

            heatmapFile = argv[i];
        }
        else if (arg == "-c")
        {
            ++i;
            if (i >= argc)
            {
                std::cout << "Missing value after option '-c'." << std::endl;
                return 1;
            }

            compFile = argv[i];
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

    try
    {
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
            if (!ofStream.is_open())
                throw std::runtime_error("Unable to open blomp file.");

            ofStream.write((const char*)&fileHeader, sizeof(fileHeader));
            ofStream << bitStream;
            ofStream.close();

            autoGenSaveHeatmap(bt, img, heatmapFile);
        }
        else if (mode == "dec")
        {
            std::cout << "Loading block tree data..." << std::endl;
            std::ifstream ifStream(inFile, std::ios::binary | std::ios::in);
            if (!ifStream.is_open())
                throw std::runtime_error("Unable to open blomp file.");

            ifStream.read((char*)&fileHeader, sizeof(fileHeader));
            if (!fileHeader.isValid())
                throw std::runtime_error("Invalid blomp file header.");

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
        else if (mode == "comp")
        {
            std::vector<Blomp::Image> images;
            std::vector<std::string> files = { compFile, inFile };
            for (auto& filename : files)
            {
                images.push_back(Blomp::Image(1, 1));
                if (Blomp::endswith(filename, ".blp"))
                {
                    std::cout << "Loading block tree..." << std::endl;
                    std::ifstream ifStream(filename, std::ios::binary | std::ios::in);
                    if (!ifStream.is_open())
                        throw std::runtime_error("Unable to open blomp file.");

                    ifStream.read((char*)&fileHeader, sizeof(fileHeader));
                    if (!fileHeader.isValid())
                        throw std::runtime_error("Invalid blomp file header.");

                    Blomp::BitStream bitStream(ifStream);
                    ifStream.close();
                    auto bt = Blomp::BlockTree::deserialize(fileHeader.bd, bitStream);

                    images.back() = Blomp::Image(bt->getWidth(), bt->getHeight());
                    bt->writeToImg(images.back());
                }
                else
                {
                    std::cout << "Loading image..." << std::endl;
                    images.back() = Blomp::Image(filename);
                }
            }

            std::cout << "Comparing images..." << std::endl;
            float similarity = Blomp::compareImages(images[0], images[1]);

            int64_t sizeComp = std::filesystem::file_size(files[0]);
            int64_t sizeIn = std::filesystem::file_size(files[1]);
            float dataRatio = float(sizeIn) / sizeComp;

            std::cout << "Results:" << std::endl;
            std::cout << "  Similarity: " << similarity << std::endl;
            std::cout << "  Data ratio: " << dataRatio << std::endl;
            std::cout << "  Score:      " << ((similarity) * (1.0f - dataRatio)) << std::endl;
        }
        else
        {
            std::cout << "Unknown mode selected.";
            return 1;
        }
    }
    catch (std::exception& e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}