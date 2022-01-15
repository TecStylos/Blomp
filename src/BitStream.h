#pragma once

#include <stdint.h>
#include <iostream>
#include <vector>

namespace Blomp
{
    class BitStream
    {
    public:
        BitStream() = default;
        BitStream(uint64_t nBits);
        BitStream(std::istream& iStream);
    public:
        bool readBit();
        void writeBit(bool value);
        void read(void* dest, uint64_t nBits, uint64_t destOffset = 0);
        void write(const void* src, uint64_t nBits, uint64_t srcOffset = 0);
        template <typename T> void read(T& dest);
        template <typename T> void write(const T& src);
    public:
        void resize(uint64_t nBits);
        void reset() noexcept;
    public:
        uint64_t size() const;
        void* data();
        const void* data() const;
    private:
        bool getBit(const char* data, uint64_t offset) const;
        void setBit(char* data, uint64_t offset, bool value);
    public:
        static uint64_t minBytes(uint64_t nBits);
    private:
        static void splitOffset(uint64_t& byteOut, uint64_t& bitOut, uint64_t offsetIn);
    private:
        uint64_t m_readOffset = 0;
        uint64_t m_writeOffset = 0;
        uint64_t m_size = 0;
        std::vector<char> m_data;
    };

    std::istream& operator>>(std::istream& iStream, BitStream& bs);
    std::ostream& operator<<(std::ostream& oStream, const BitStream& bs);

    template <typename T>
    void BitStream::read(T& dest)
    {
        read(&dest, sizeof(dest) * 8);
    }

    template <typename T>
    void BitStream::write(const T& src)
    {
        write(&src, sizeof(src) * 8);
    }
}