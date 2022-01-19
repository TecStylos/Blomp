#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#include "stb_image.h"

namespace Blomp
{
    struct Pixel
    {
        float r, g, b;
    public:
        Pixel() : Pixel(0.0f) {}
        Pixel(float c) : Pixel(c, c, c) {}
        Pixel(float r, float g, float b) : r(r), g(g), b(b) {}
    public:
        operator float() const;
    public:
        void toCharArray(uint8_t* pixelData) const;
        static Pixel fromCharArray(const uint8_t* pixelData);
    };

    Pixel& operator+=(Pixel& left, const Pixel& right);
    Pixel& operator-=(Pixel& left, const Pixel& right);
    Pixel& operator*=(Pixel& left, const Pixel& right);
    Pixel& operator/=(Pixel& left, const Pixel& right);
    Pixel operator+(Pixel left, const Pixel& right);
    Pixel operator-(Pixel left, const Pixel& right);
    Pixel operator*(Pixel left, const Pixel& right);
    Pixel operator/(Pixel left, const Pixel& right);

    class Image
    {
    public:
        Image(int width, int height);
        Image(const std::string& filename);
    public:
        int width() const;
        int height() const;
        Pixel& get(int x, int y);
        const Pixel& get(int x, int y) const;
        Pixel& getNC(int x, int y);
        const Pixel& getNC(int x, int y) const;
        Pixel& operator()(int x, int y);
        const Pixel& operator()(int x, int y) const;
    public:
        void save(const std::string& filename) const;
    private:
        int m_width;
        int m_height;
        std::vector<Pixel> m_buffer;
    };

    inline void Pixel::toCharArray(uint8_t* pixelData) const
    {
        pixelData[0] = r * 255;
        pixelData[1] = g * 255;
        pixelData[2] = b * 255;
    }

    inline Pixel Pixel::fromCharArray(const uint8_t* pixelData)
    {
        Pixel pix;
        pix.r = float(pixelData[0]) / 255;
        pix.g = float(pixelData[1]) / 255;
        pix.b = float(pixelData[2]) / 255;
        return pix;
    }

    inline Pixel& operator+=(Pixel& left, const Pixel& right)
    {
        left.r += right.r;
        left.g += right.g;
        left.b += right.b;
        return left;
    }
    inline Pixel& operator-=(Pixel& left, const Pixel& right)
    {
        left.r -= right.r;
        left.g -= right.g;
        left.b -= right.b;
        return left;
    }
    inline Pixel& operator*=(Pixel& left, const Pixel& right)
    {
        left.r *= right.r;
        left.g *= right.g;
        left.b *= right.b;
        return left;
    }
    inline Pixel& operator/=(Pixel& left, const Pixel& right)
    {
        left.r /= right.r;
        left.g /= right.g;
        left.b /= right.b;
        return left;
    }
    inline Pixel operator+(Pixel left, const Pixel& right)
    {
        return left += right;
    }
    inline Pixel operator-(Pixel left, const Pixel& right)
    {
        return left -= right;
    }
    inline Pixel operator*(Pixel left, const Pixel& right)
    {
        return left *= right;
    }
    inline Pixel operator/(Pixel left, const Pixel& right)
    {
        return left /= right;
    }

    inline int Image::width() const
    {
        return m_width;
    }

    inline int Image::height() const
    {
        return m_height;
    }

    inline Pixel& Image::get(int x, int y)
    {
        if (x < 0 || width() <= x || y < 0 || height() <= y)
            throw std::runtime_error("Cannot read out-of-bounds pixel of image.");
        return getNC(x, y);
    }

    inline const Pixel& Image::get(int x, int y) const
    {
        if (x < 0 || width() <= x || y < 0 || height() <= y)
            throw std::runtime_error("Cannot read out-of-bounds pixel of image.");
        return getNC(x, y);
    }

    inline Pixel& Image::getNC(int x, int y)
    {
        return m_buffer[y * width() + x];
    }

    inline const Pixel& Image::getNC(int x, int y) const
    {
        return m_buffer[y * width() + x];
    }

    inline Pixel& Image::operator()(int x, int y)
    {
        return get(x, y);
    }

    inline const Pixel& Image::operator()(int x, int y) const
    {
        return get(x, y);
    }
}