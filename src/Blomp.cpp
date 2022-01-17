#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
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
#include "BlompHelp.h"

#define RETURN_MISSING_VALUE(option) { std::cout << "Missing value for option '" << (option) << "'."; return 1; }

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
    if (!Blomp::endswith(filename, ".blp"))
        return Blomp::Image(filename);

    auto bt = loadBlockTree(filename);

    Blomp::Image img = Blomp::Image(bt->getWidth(), bt->getHeight());
    bt->writeToImg(img);

    return img;
}

Blomp::ParentBlockRef calcMaxV(const Blomp::Image& img, Blomp::BlockTreeDesc& btDesc, int sizeToReach, int nIterations, int& nIterationsUsed, bool verbose)
{
    Blomp::ParentBlockRef bt;
    int64_t sizeComp = 0;
    int64_t prevSizeComp = std::numeric_limits<int>::max();
    btDesc.variationThreshold = 2.0f;
    float thresChange = 2.0f;

    bool autoStop = (nIterations == 0);
    if (autoStop) nIterations = std::numeric_limits<int>::max();

    int numOfCloseComps = 0;

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

        if (autoStop && prevSizeComp == sizeComp)
        {
            if (++numOfCloseComps == 5)
                break;
        }
        else
            numOfCloseComps = 0;

        if (btDesc.variationThreshold > 1.1f)
            break;

        prevSizeComp = sizeComp;
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
        std::cout << Blomp::getHelpText(argc > 2 ? argv[2] : "");
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

                if (maxvIterations < 0)
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
            std::cout << "Invalid value for option '" << arg << "'." << std::endl;
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
    
    // std::cout << "Mode:      " << mode << std::endl;
    // std::cout << "MaxDepth:  " << btDesc.maxDepth << std::endl;
    // std::cout << "VarThres:  " << btDesc.variationThreshold << std::endl;
    // std::cout << "InFile:    " << inFile << std::endl;
    // std::cout << "OutFile:   " << outFile << std::endl;
    // std::cout << "HeatFile:  " << heatmapFile << std::endl;
    // std::cout << "CompFile:  " << compFile << std::endl;
    // std::cout << "GenFile:   " << genFile << std::endl;
    // std::cout << "MaxVIter:  " << maxvIterations << std::endl;
    // std::cout << "BeQuiet:   " << beQuiet << std::endl;
    // std::cout << "FSToReach: " << fsizeToReach << std::endl;

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
                throw std::runtime_error("Missing output file.");

            Blomp::Image img(inFile);
            auto bt = Blomp::BlockTree::fromImage(img, btDesc);

            if (!beQuiet)
                viewBlockTreeInfo(bt, genFile.empty() ? "%TEMP%" : genFile);

            if (!genFile.empty())
                saveBlockTree(bt, btDesc.maxDepth, genFile);

            bt->writeToImg(img);
            img.save(outFile);

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

            int nItersUsed = 0;

            auto bt = calcMaxV(img, btDesc, fsizeToReach, maxvIterations, nItersUsed, !beQuiet);

            std::cout << "MaxV result for '" << inFile << "' after " << nItersUsed << " iterations:" << std::endl;
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

            int nItersUsed = 0;
            for (btDesc.maxDepth = 0; btDesc.maxDepth <= 10; ++btDesc.maxDepth)
            {
                if (!beQuiet)
                    std::cout << "Running MaxV test " << (btDesc.maxDepth + 1) << "/11 ..." << std::endl;

                auto bt = calcMaxV(img, btDesc, fsizeToReach, maxvIterations, nItersUsed, !beQuiet);
                bt->writeToImg(img2);

                float score = calcImgCompScore(img, img2, img1Size, calcEstFileSize(bt));

                if (score > best.score)
                {
                    best.bt = bt;
                    best.btDesc = btDesc;
                    best.score = score;
                }
            }

            std::cout << "Opti result for '" << inFile << "' after " << nItersUsed << " iterations:" << std::endl;
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