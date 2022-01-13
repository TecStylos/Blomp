#include "Image.h"

#include <stdexcept>

#include "stb_image_write.h"

namespace Blomp
{
    enum class ImageType
    {
        UNKNOWN, PNG, BMP, TGA, JPG, HDR
    };

    ImageType ImgTypeFromFilename(const std::string& filename)
    {
        auto endswith = [&](const std::string sub) -> bool
        {
            return filename.find(sub) == filename.size() - sub.size();
        };

        if (endswith(".png")) return ImageType::PNG;
        if (endswith(".bmp")) return ImageType::BMP;
        if (endswith(".tga")) return ImageType::TGA;
        if (endswith(".jpg")) return ImageType::JPG;
        if (endswith(".hdr")) return ImageType::HDR;

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

        if (nChannels != 3)
            throw std::runtime_error("Wrong channel count!");

        m_buffer.clear();
        m_buffer.reserve(m_width * m_height);

        for (int i = 0; i < m_width * m_height; ++i)
        {
            Pixel pix;
            pix.r = float(data[i * nChannels + 0]) / 255;
            pix.g = float(data[i * nChannels + 1]) / 255;
            pix.b = float(data[i * nChannels + 2]) / 255;
            m_buffer.push_back(pix);
        }

        stbi_image_free(data);
    }

    int Image::width() const
    {
        return m_width;
    }

    int Image::height() const
    {
        return m_height;
    }

    Pixel& Image::get(int x, int y)
    {
        return m_buffer[y * width() + x];
    }

    const Pixel& Image::get(int x, int y) const
    {
        return m_buffer[y * width() + x];
    }

    Pixel& Image::operator()(int x, int y)
    {
        return get(x, y);
    }

    const Pixel& Image::operator()(int x, int y) const
    {
        return get(x, y);
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

        // PNG, BMP, TGA, JPG, HDR
        int result;
        switch (type)
        {
        case ImageType::PNG: result = stbi_write_png(filename.c_str(), m_width, m_height, 3, data.data(), m_width * 3); break;
        case ImageType::BMP: break;
        case ImageType::TGA: break;
        case ImageType::JPG: break;
        case ImageType::HDR: break;
        case ImageType::UNKNOWN: throw std::runtime_error("Unknown filetype!");
        }
    }
}