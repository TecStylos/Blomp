#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>

#include "Blocks.h"
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
  denc         Shortcut for running 'enc' and 'dec'. Doesn't save the *.blp file.
  comp         Compare two images with the same dimensions.
  maxv         Estimate the max. '-v' to generate a blomp file not bigger than the input file.
  info         View information of a blomp file.

Options:
  -d [int]     Block depth. Default: 4, Range: 0-10, Modes: enc/dec/maxv/denc
  -v [float]   Variation threshold. Default: 0.02, Range: 0.0 - 1.0, Modes: enc/denc
  -o [string]  Output filename. Default: [inFile] with changed file extension. Modes: enc/dec/denc
  -m [string]  Heatmap filename. Generate a compression heatmap if specified. Modes: enc/dec/denc
  -i [int]     Number of iterations for mode 'maxv'. Default: 10, Modes: maxv
  -c [string]  Comparison file. Modes: comp
  -q           Quiet. View less information. Modes: [all]

Supported image formats:
  Mode | JPG PNG TGA BMP PSD GIF HDR PIC PNM
   enc    X   X   X   X   X   X   X   X   X
   dec    X   X   X   X
   comp   X   X   X   X   X   X   X   X   X
  For more details see https://github.com/nothings/stb 
)";

uint64_t calcEstFileSize(Blomp::ParentBlockRef bt)
{
    return (sizeof(Blomp::FileHeader) * 8 + sizeof(uint64_t) * 8 + bt->nBlocks() + bt->nColorBlocks() * 3 * 8 + 7) / 8;
}

void viewBlockTreeInfo(Blomp::ParentBlockRef bt)
{
    std::cout << "  Blocks:      " << bt->nBlocks() << std::endl;
    std::cout << "  ColorBlocks: " << bt->nColorBlocks() << std::endl;
    std::cout << "  EstFileSize: " << calcEstFileSize(bt) << " bytes" << std::endl;
}

void autoGenSaveHeatmap(Blomp::ParentBlockRef bt, Blomp::Image& img, const std::string& heatmapFile)
{
    if (heatmapFile.empty())
        return;

    bt->writeHeatmap(img, 10);

    img.save(heatmapFile);
}

Blomp::ParentBlockRef loadBlockTree(const std::string& filename)
{
    Blomp::FileHeader fileHeader;

    std::ifstream ifStream(filename, std::ios::binary | std::ios::in);
    if (!ifStream.is_open())
        throw std::runtime_error("Unable to open blomp file.");

    ifStream.read((char*)&fileHeader, sizeof(fileHeader));
    if (!fileHeader.isValid())
        throw std::runtime_error("Invalid blomp file header.");

    Blomp::BitStream bitStream(ifStream);
    ifStream.close();

    return Blomp::BlockTree::deserialize(fileHeader.bd, bitStream);
}

void saveBlockTree(const Blomp::ParentBlockRef bt, int maxDepth, const std::string& filename)
{
    Blomp::FileHeader fileHeader;

    Blomp::BitStream bitStream;
    Blomp::BlockTree::serialize(bt, bitStream);
    fileHeader.bd.imgWidth = bt->getWidth();
    fileHeader.bd.imgHeight = bt->getHeight();
    fileHeader.bd.maxDepth = maxDepth;

    std::ofstream ofStream(filename, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!ofStream.is_open())
        throw std::runtime_error("Unable to open blomp file.");

    ofStream.write((const char*)&fileHeader, sizeof(fileHeader));
    ofStream << bitStream;
    ofStream.close();
}

Blomp::Image loadImage(const std::string& filename)
{
    Blomp::Image img(1, 1);
    if (Blomp::endswith(filename, ".blp"))
    {
        auto bt = loadBlockTree(filename);

        img = Blomp::Image(bt->getWidth(), bt->getHeight());
        bt->writeToImg(img);
    }
    else
    {
        img = Blomp::Image(filename);
    }

    return img;
}

int main(int argc, const char** argv, const char** env)
{
    Blomp::BlockTreeDesc btDesc;
    btDesc.maxDepth = 4;
    btDesc.variationThreshold = 0.02f;
    std::string inFile = "";
    std::string outFile = "";
    std::string heatmapFile = "";
    std::string compFile = "";
    int maxvIterations = 10;
    bool beQuiet = false;

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
        else if (arg == "-i")
        {
            ++i;
            if (i >= argc)
            {
                std::cout << "Missing value after option '-i'." << std::endl;
                return 1;
            }
            try 
            {
                maxvIterations = std::stof(argv[i]);

                if (maxvIterations < 1)
                    invalidValue = true;
            }
            catch (std::exception e)
            {
                invalidValue = true;
            }
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
        else if (arg == "-q")
        {
            beQuiet = true;
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
        std::string fileext = "";
        if (mode == "enc")
            fileext = ".blp";
        else if (mode == "dec")
            fileext = ".png";
        else if (mode == "denc")
            fileext = "_DENC.png";
        else
            fileext = ".UNKNOWN";
        outFile = inFile.substr(0, inFile.find_last_of(".")) + fileext;
    }

    try
    {
        if (mode == "enc")
        {
            Blomp::Image img(inFile);
            auto bt = Blomp::BlockTree::fromImage(img, btDesc);

            if (!beQuiet)
                viewBlockTreeInfo(bt);

            saveBlockTree(bt, btDesc.maxDepth, outFile);

            autoGenSaveHeatmap(bt, img, heatmapFile);
        }
        else if (mode == "dec")
        {
            auto bt = loadBlockTree(inFile);

            if (!beQuiet)
                viewBlockTreeInfo(bt);

            Blomp::Image img(bt->getWidth(), bt->getHeight());

            bt->writeToImg(img);
            img.save(outFile);

            autoGenSaveHeatmap(bt, img, heatmapFile);
        }
        else if (mode == "denc")
        {
            Blomp::Image img(inFile);
            auto bt = Blomp::BlockTree::fromImage(img, btDesc);

            if (!beQuiet)
                viewBlockTreeInfo(bt);

            bt->writeToImg(img);
            img.save(outFile);

            autoGenSaveHeatmap(bt, img, heatmapFile);
        }
        else if (mode == "comp")
        {
            std::vector<Blomp::Image> images;
            std::vector<std::string> files = { compFile, inFile };
            for (auto& filename : files)
                images.push_back(loadImage(filename));

            float similarity = Blomp::compareImages(images[0], images[1]);

            int64_t sizeComp = std::filesystem::file_size(files[0]);
            int64_t sizeIn = std::filesystem::file_size(files[1]);
            float dataRatio = float(sizeIn) / sizeComp;

            std::cout << "Results:" << std::endl;
            std::cout << "  Similarity: " << similarity << std::endl;
            std::cout << "  Data ratio: " << dataRatio << std::endl;
            std::cout << "  Score:      " << ((similarity) * (1.0f - dataRatio)) << std::endl;
        }
        else if (mode == "maxv")
        {
            btDesc.variationThreshold = 2.0f;
            float thresChange = 2.0f;

            auto img = loadImage(inFile);
            int64_t sizeOrig = std::filesystem::file_size(inFile);
            int64_t sizeComp = 0;

            Blomp::ParentBlockRef bt;

            for (int i = 0; i < maxvIterations; ++i)
            {
                if (!beQuiet)
                    std::cout << "Iteration " << (i + 1) << "/" << maxvIterations << "  ->  ";

                thresChange /= 2.0f;

                if (sizeComp < sizeOrig)
                    btDesc.variationThreshold -= thresChange;
                else
                    btDesc.variationThreshold += thresChange;

                bt = Blomp::BlockTree::fromImage(img, btDesc);

                sizeComp = calcEstFileSize(bt);

                if (!beQuiet)
                    std::cout << "v:" << btDesc.variationThreshold << " fs:" << sizeComp << " bytes" << std::endl;
            }

            std::cout << "Result:" << std::endl;
            std::cout << "  v:" << btDesc.variationThreshold << " -> fs: " << sizeComp << " bytes" << std::endl;
        }
        else if (mode == "info")
        {
            auto bt = loadBlockTree(inFile);

            std::cout << "Info for '" << inFile << "':" << std::endl;
            viewBlockTreeInfo(bt);
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