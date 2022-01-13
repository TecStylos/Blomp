#pragma once

#include <string>
#include <vector>

#include "stb_image.h"

namespace Blomp
{
    struct Pixel
    {
        float r, g, b;
    };

    class Image
    {
    public:
        Image(int width, int height);
        Image(const std::string& filepath);
    public:
        int width() const;
        int height() const;
        Pixel& get(int x, int y);
        const Pixel& get(int x, int y) const;
    public:
        void save(const std::string& filepath);
    private:
        int m_width;
        int m_height;
        std::vector<Pixel> m_buffer;
    };
}