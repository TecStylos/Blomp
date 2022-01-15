#include "Image.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

#include "stb_image_write.h"

namespace Blomp
{
    void Pixel::toCharArray(uint8_t* pixelData) const
    {
        pixelData[0] = r * 255;
        pixelData[1] = g * 255;
        pixelData[2] = b * 255;
    }

    Pixel Pixel::fromCharArray(const uint8_t* pixelData)
    {
        Pixel pix;
        pix.r = float(pixelData[0]) / 255;
        pix.g = float(pixelData[1]) / 255;
        pix.b = float(pixelData[2]) / 255;
        return pix;
    }

    Pixel& operator+=(Pixel& left, const Pixel& right)
    {
        left.r += right.r;
        left.g += right.g;
        left.b += right.b;
        return left;
    }
    Pixel& operator-=(Pixel& left, const Pixel& right)
    {
        left.r -= right.r;
        left.g -= right.g;
        left.b -= right.b;
        return left;
    }
    Pixel& operator*=(Pixel& left, const Pixel& right)
    {
        left.r *= right.r;
        left.g *= right.g;
        left.b *= right.b;
        return left;
    }
    Pixel& operator/=(Pixel& left, const Pixel& right)
    {
        left.r /= right.r;
        left.g /= right.g;
        left.b /= right.b;
        return left;
    }
    Pixel operator+(Pixel left, const Pixel& right)
    {
        return left += right;
    }
    Pixel operator-(Pixel left, const Pixel& right)
    {
        return left -= right;
    }
    Pixel operator*(Pixel left, const Pixel& right)
    {
        return left *= right;
    }
    Pixel operator/(Pixel left, const Pixel& right)
    {
        return left /= right;
    }

    enum class ImageType
    {
        UNKNOWN, PNG, BMP, TGA, JPG
    };

    ImageType ImgTypeFromFilename(std::string filename)
    {
        std::transform(filename.begin(), filename.end(), filename.begin(),
            [](unsigned char c){ return std::tolower(c); }
        );

        auto endswith = [&](const std::string sub) -> bool
        {
            return filename.find(sub) == filename.size() - sub.size();
        };

        if (endswith(".png")) return ImageType::PNG;
        if (endswith(".bmp")) return ImageType::BMP;
        if (endswith(".tga")) return ImageType::TGA;
        if (endswith(".jpg")) return ImageType::JPG;
        if (endswith(".jpeg")) return ImageType::JPG;

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
            m_buffer.push_back(Pixel::fromCharArray((const uint8_t*)data + i * nChannels));

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

        int result;
        switch (type)
        {
        case ImageType::PNG: result = stbi_write_png(filename.c_str(), m_width, m_height, 3, data.data(), m_width * 3); break;
        case ImageType::BMP: result = stbi_write_bmp(filename.c_str(), m_width, m_height, 3, data.data()); break;
        case ImageType::TGA: result = stbi_write_tga(filename.c_str(), m_width, m_height, 3, data.data()); break;
        case ImageType::JPG: result = stbi_write_jpg(filename.c_str(), m_width, m_height, 3, data.data(), 90); break;
        case ImageType::UNKNOWN: throw std::runtime_error("Unknown filetype!");
        }
    }
}