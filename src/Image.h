#pragma once

#include <string>
#include <vector>

#include "stb_image.h"

namespace Blomp
{
    struct Pixel
    {
        float r, g, b;
    public:
        Pixel() = default;
        Pixel(float r, float g, float b) : r(r), g(g), b(b) {}
    };

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
        Pixel& operator()(int x, int y);
        const Pixel& operator()(int x, int y) const;
    public:
        void save(const std::string& filename) const;
    private:
        int m_width;
        int m_height;
        std::vector<Pixel> m_buffer;
    };
}