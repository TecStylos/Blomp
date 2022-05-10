#include "Image.h"

#include <algorithm>
#include <cctype>
#include <cmath>

#include "Tools.h"

#include "stb_image_write.h"

namespace Blomp
{
    enum class ImageType
    {
        UNKNOWN, PNG, BMP, TGA, JPG
    };

    ImageType ImgTypeFromFilename(std::string filename)
    {
        std::transform(filename.begin(), filename.end(), filename.begin(),
            [](unsigned char c){ return std::tolower(c); }
        );

        if (endswith(filename, ".png")) return ImageType::PNG;
        if (endswith(filename, ".bmp")) return ImageType::BMP;
        if (endswith(filename, ".tga")) return ImageType::TGA;
        if (endswith(filename, ".jpg")) return ImageType::JPG;
        if (endswith(filename, ".jpeg")) return ImageType::JPG;

        return ImageType::UNKNOWN;
    }

    Image::Image(int width, int height)
        : m_width(width), m_height(height), m_buffer(width * height)
    {}

    Image::Image(const std::string& filename)
    {
        int nChannels;
        auto data = stbi_load(filename.c_str(), &m_width, &m_height, &nChannels, 3);
    
        if (!data)
            throw std::runtime_error("Unable to load file!");

        //if (nChannels != 3)
        //    throw std::runtime_error("Wrong channel count!");

        m_buffer.clear();
        m_buffer.reserve(m_width * m_height);

        for (int i = 0; i < m_width * m_height; ++i)
            m_buffer.push_back(Pixel::fromCharArray((const uint8_t*)data + i * nChannels));

        stbi_image_free(data);
    }

    void Image::save(const std::string& filename) const
    {
        auto type = ImgTypeFromFilename(filename);

        if (type == ImageType::UNKNOWN)
            throw std::runtime_error("Unknown filetype!");

        std::vector<stbi_uc> data;
        data.reserve(m_width * m_height);

        for (auto& pix : m_buffer)
        {
            data.push_back(stbi_uc(std::min(1.0f, std::max(0.0f, pix.r)) * 255.0f));
            data.push_back(stbi_uc(std::min(1.0f, std::max(0.0f, pix.g)) * 255.0f));
            data.push_back(stbi_uc(std::min(1.0f, std::max(0.0f, pix.b)) * 255.0f));
        }

        int result;
        switch (type)
        {
        case ImageType::PNG: result = stbi_write_png(filename.c_str(), m_width, m_height, 3, data.data(), m_width * 3); break;
        case ImageType::BMP: result = stbi_write_bmp(filename.c_str(), m_width, m_height, 3, data.data()); break;
        case ImageType::TGA: result = stbi_write_tga(filename.c_str(), m_width, m_height, 3, data.data()); break;
        case ImageType::JPG: result = stbi_write_jpg(filename.c_str(), m_width, m_height, 3, data.data(), 90); break;
        case ImageType::UNKNOWN: throw std::runtime_error("Unknown filetype!");
        }

        if (result == 0)
            throw std::runtime_error("Unable to write image file.");
    }
}