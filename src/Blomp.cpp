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

#define RETURN_MISSING_VALUE(option) { std::cout << "Missing value after option '" << (option) << "'."; return 1; }

// TODO: Add long option names to help
const char *helpText =
R"(Usage:
  blomp [mode] [options] [inFile]

Modes:
  help         View this help.
  enc          Convert an image file to a blomp file.
  dec          Convert a blomp file to an image file.
  denc         Shortcut for running 'enc' and 'dec'. Doesn't save the *.blp file.
  comp         Compare two images with the same dimensions.
  maxv         Optimize the '-v' options to reach a specified file size or the input file size.
  opti         Optimize the '-d' and '-v' options to reach a specified file size or the input file size.
  info         View information of a blomp file.

Options:
  -d [int]     Block depth. Default: 4, Range: 0-10, Modes: enc/dec/maxv/denc
  -v [float]   Variation threshold. Default: 0.02, Range: 0.0 - 1.0, Modes: enc/denc
  -o [string]  Output filename. Default: [inFile] with changed file extension. Modes: enc/dec/denc/maxv/opti
  -m [string]+ Heatmap filename. Generate a compression heatmap if specified. Modes: enc/dec/denc/maxv/opti
  -i [int]     Number of iterations. Default: 10, Modes: maxv/opti
  -c [string]  Comparison file. Modes: comp
  -s [int]     File size to reach. Modes: maxv/opti
  -g [string]+ Regenerated image filename. Modes: maxv/opti
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

void viewBlockTreeInfo(Blomp::ParentBlockRef bt, const std::string& filename = "")
{
    if (!filename.empty())
        std::cout << "BlockTree Info for '" << filename << "':" << std::endl;
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

Blomp::ParentBlockRef calcMaxV(const Blomp::Image& img, Blomp::BlockTreeDesc& btDesc, int sizeToReach, int nIterations, bool verbose)
{
    Blomp::ParentBlockRef bt;
    int64_t sizeComp = 0;
    btDesc.variationThreshold = 2.0f;
    float thresChange = 2.0f;

    for (int i = 0; i < nIterations; ++i)
    {
        if (verbose)
            std::cout << "Iteration " << (i + 1) << "/" << nIterations << "  ->  ";

        thresChange /= 2.0f;

        if (sizeComp < sizeToReach)
            btDesc.variationThreshold -= thresChange;
        else
            btDesc.variationThreshold += thresChange;

        bt = Blomp::BlockTree::fromImage(img, btDesc);

        sizeComp = calcEstFileSize(bt);

        if (verbose)
            std::cout << "v:" << btDesc.variationThreshold << " fs:" << sizeComp << " bytes" << std::endl;
    }

    return bt;
}

float calcImgCompScore(const Blomp::Image& img1, const Blomp::Image& img2, uint64_t img1Size, uint64_t img2Size, float* pSimilarity = nullptr, float* pDataRatio = nullptr)
{
    float temp1, temp2;
    if (!pSimilarity) pSimilarity = &temp1;
    if (!pDataRatio) pDataRatio = &temp2;

    *pSimilarity = Blomp::compareImages(img1, img2);
    *pDataRatio = float(img2Size) / img1Size;
    return *pSimilarity / *pDataRatio;
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
    std::string genFile = "";
    int maxvIterations = 10;
    bool beQuiet = false;
    uint64_t fsizeToReach = 0;

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

        if (arg == "-d" || arg == "--depth")
        {
            ++i;
            if (i >= argc)
                RETURN_MISSING_VALUE(arg);

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
        else if (arg == "-v" || arg == "--variation")
        {
            ++i;
            if (i >= argc)
                RETURN_MISSING_VALUE(arg);
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
        else if (arg == "-o" || arg == "--output")
        {
            ++i;
            if (i >= argc)
                RETURN_MISSING_VALUE(arg);

            outFile = argv[i];
        }
        else if (arg == "-m" || arg == "--heatmap")
        {
            ++i;
            if (i >= argc)
                RETURN_MISSING_VALUE(arg);
            
            heatmapFile = argv[i];
        }
        else if (arg == "-i" || arg == "--iterations")
        {
            ++i;
            if (i >= argc)
                RETURN_MISSING_VALUE(arg);

            try 
            {
                maxvIterations = std::stoi(argv[i]);

                if (maxvIterations < 1)
                    invalidValue = true;
            }
            catch (std::exception e)
            {
                invalidValue = true;
            }
        }
        else if (arg == "-c" || arg == "--compfile")
        {
            ++i;
            if (i >= argc)
                RETURN_MISSING_VALUE(arg);

            compFile = argv[i];
        }
        else if (arg == "-s" || arg == "--size")
        {
            ++i;
            if (i >= argc)
                RETURN_MISSING_VALUE(arg);

            try 
            {
                fsizeToReach = std::stoi(argv[i]);

                if (fsizeToReach < 1)
                    invalidValue = true;
            }
            catch (std::exception e)
            {
                invalidValue = true;
            }
        }
        else if (arg == "-g" || arg == "--genoutput")
        {
            ++i;
            if (i >= argc)
                RETURN_MISSING_VALUE(arg);
           
            genFile = argv[i];
        }
        else if (arg == "-q" || arg == "--quiet")
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


    while (true)
    {
        std::string outExt = "";
        std::string genExt = "";
        if (mode == "enc")
            outExt = ".blp";
        else if (mode == "dec")
            outExt = ".png";
        else if (mode == "denc")
        {
            outExt = "_DENC.png";
            if (genFile == "+")
                genExt = "_DENC.png";
        }
        else if (mode == "comp")
            break;
        else if (mode == "maxv")
        {
            outExt = ".blp";
            if (genFile == "+")
                genExt = "_MAXV.png";
        }
        else if (mode == "opti")
        {
            std::cout << "MODE____OPTI" << std::endl;
            outExt = ".blp";
            if (genFile == "+")
                genExt = "_OPTI.png";
        }
        else if (mode == "info")
            break;
        else
            break;

        if (!outExt.empty() && outFile.empty())
            outFile = inFile.substr(0, inFile.find_last_of(".")) + outExt;
        
        if (!genExt.empty() && genFile == "+")
            genFile = inFile.substr(0, inFile.find_last_of(".")) + genExt;

        break;
    }

    if (heatmapFile == "+")
        heatmapFile = inFile.substr(0, inFile.find_last_of(".")) + "_HEAT.png";
    
    std::cout << "Mode:      " << mode << std::endl;
    std::cout << "MaxDepth:  " << btDesc.maxDepth << std::endl;
    std::cout << "VarThres:  " << btDesc.variationThreshold << std::endl;
    std::cout << "InFile:    " << inFile << std::endl;
    std::cout << "OutFile:   " << outFile << std::endl;
    std::cout << "HeatFile:  " << heatmapFile << std::endl;
    std::cout << "CompFile:  " << compFile << std::endl;
    std::cout << "GenFile:   " << genFile << std::endl;
    std::cout << "MaxVIter:  " << maxvIterations << std::endl;
    std::cout << "BeQuiet:   " << beQuiet << std::endl;
    std::cout << "FSToReach: " << fsizeToReach << std::endl;

    try
    {
        if (inFile.empty())
            throw std::runtime_error("Missing input file.");

        if (mode == "enc")
        {
            if (outFile.empty())
                throw std::runtime_error("Missing output file.");

            Blomp::Image img(inFile);
            auto bt = Blomp::BlockTree::fromImage(img, btDesc);

            if (!beQuiet)
                viewBlockTreeInfo(bt, outFile);

            saveBlockTree(bt, btDesc.maxDepth, outFile);

            autoGenSaveHeatmap(bt, img, heatmapFile);
        }
        else if (mode == "dec")
        {
            if (outFile.empty())
                throw std::runtime_error("Missing output file.");

            auto bt = loadBlockTree(inFile);

            if (!beQuiet)
                viewBlockTreeInfo(bt, inFile);

            Blomp::Image img(bt->getWidth(), bt->getHeight());

            bt->writeToImg(img);
            img.save(outFile);

            autoGenSaveHeatmap(bt, img, heatmapFile);
        }
        else if (mode == "denc")
        {
            if (outFile.empty())
                throw std::runtime_error("Mising output file.");

            Blomp::Image img(inFile);
            auto bt = Blomp::BlockTree::fromImage(img, btDesc);

            if (!beQuiet)
                viewBlockTreeInfo(bt, "%TEMP%");

            if (!genFile.empty())
                saveBlockTree(bt, btDesc.maxDepth, genFile);

            if (!outFile.empty())
            {
                bt->writeToImg(img);
                img.save(outFile);
            }

            autoGenSaveHeatmap(bt, img, heatmapFile);
        }
        else if (mode == "comp")
        {
            if (compFile.empty())
                throw std::runtime_error("Missing comparison file.");

            Blomp::Image compImg = loadImage(compFile);
            Blomp::Image inImg = loadImage(inFile);

            float similarity, dataRatio;
            float score = calcImgCompScore(
                compImg, inImg,
                std::filesystem::file_size(compFile),
                std::filesystem::file_size(inFile),
                &similarity, &dataRatio
            );

            std::cout << "Comp Results ('" << compFile << "' vs. '" << inFile << "'):" << std::endl;
            std::cout << "  Similarity: " << similarity << std::endl;
            std::cout << "  Data ratio: " << dataRatio << std::endl;
            std::cout << "  Score:      " << (similarity / dataRatio) << std::endl;
        }
        else if (mode == "maxv")
        {
            if (outFile.empty())
                throw std::runtime_error("Missing output file.");

            auto img = loadImage(inFile);
            if (!fsizeToReach)
                fsizeToReach = std::filesystem::file_size(inFile);

            auto bt = calcMaxV(img, btDesc, fsizeToReach, maxvIterations, !beQuiet);

            std::cout << "MaxV result for '" << inFile << "':" << std::endl;
            std::cout << "  v:" << btDesc.variationThreshold << " -> fs: " << calcEstFileSize(bt) << " bytes" << std::endl;

            saveBlockTree(bt, btDesc.maxDepth, outFile);

            if (!genFile.empty())
            {
                bt->writeToImg(img);
                img.save(genFile);
            }

            autoGenSaveHeatmap(bt, img, heatmapFile);
        }
        else if (mode == "opti")
        {
            if (outFile.empty())
                throw std::runtime_error("Missing output file.");

            auto img = loadImage(inFile);
            uint64_t img1Size = std::filesystem::file_size(inFile);
            auto img2 = Blomp::Image(img.width(), img.height());

            if (!fsizeToReach)
                fsizeToReach = std::filesystem::file_size(inFile);

            struct BestResult
            {
                Blomp::ParentBlockRef bt;
                Blomp::BlockTreeDesc btDesc;
                float score = 0.0f;
            } best;

            for (btDesc.maxDepth = 0; btDesc.maxDepth <= 10; ++btDesc.maxDepth)
            {
                if (!beQuiet)
                    std::cout << "Running MaxV test " << (btDesc.maxDepth + 1) << "/11 ..." << std::endl;

                auto bt = calcMaxV(img, btDesc, fsizeToReach, maxvIterations, !beQuiet);
                bt->writeToImg(img2);

                float score = calcImgCompScore(img, img2, img1Size, calcEstFileSize(bt));

                if (score > best.score)
                {
                    best.bt = bt;
                    best.btDesc = btDesc;
                    best.score = score;
                }
            }

            std::cout << "Opti result for '" << inFile << "':" << std::endl;
            std::cout << "  d:" << best.btDesc.maxDepth << " v:" << best.btDesc.variationThreshold << std::endl;
            std::cout << "  -> fs: " << calcEstFileSize(best.bt) << " bytes" << std::endl;

            saveBlockTree(best.bt, best.btDesc.maxDepth, outFile);

            if (!genFile.empty())
            {
                best.bt->writeToImg(img2);
                img2.save(genFile);
            }

            autoGenSaveHeatmap(best.bt, img2, heatmapFile);
        }
        else if (mode == "info")
        {
            auto bt = loadBlockTree(inFile);

            viewBlockTreeInfo(bt, inFile);
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