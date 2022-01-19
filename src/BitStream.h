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
        void reserve(uint64_t nBits);
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
        uint64_t m_reserved = 0;
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

    inline bool BitStream::readBit()
    {
        ++m_readOffset;

        return getBit(m_data.data(), m_readOffset - 1);
    }

    inline void BitStream::writeBit(bool value)
    {
        ++m_writeOffset;

        if (m_writeOffset > m_size)
            resize(m_writeOffset);

        setBit(m_data.data(), m_writeOffset - 1, value);
    }

    inline void BitStream::read(void* dest, uint64_t nBits, uint64_t destOffset)
    {
        for (uint64_t i = 0; i < nBits; ++i)
            setBit((char*)dest, destOffset + i, readBit());
    }

    inline void BitStream::write(const void* src, uint64_t nBits, uint64_t srcOffset)
    {
        for (uint64_t i = 0; i < nBits; ++i)
            writeBit(getBit((const char*)src, srcOffset + i));
    }

    inline void BitStream::resize(uint64_t nBits)
    {
        m_size = nBits;
        if (m_reserved < nBits)
            reserve(nBits);
    }

    inline void BitStream::reserve(uint64_t nBits)
    {
        m_reserved = nBits + nBits / 2;
        m_data.resize(BitStream::minBytes(m_reserved));
    }

    inline uint64_t BitStream::minBytes(uint64_t nBits)
    {
        return (nBits + 7) / 8;
    }

    inline uint64_t BitStream::size() const
    {
        return m_size;
    }

    inline void* BitStream::data()
    {
        return m_data.data();
    }

    inline const void* BitStream::data() const
    {
        return m_data.data();
    }

    inline bool BitStream::getBit(const char* data, uint64_t offset) const
    {
        if (offset >= m_size)
            throw std::runtime_error("Unable to get out-of-bounds bit of bitstream.");

        uint64_t byte;
        uint64_t bit;
        splitOffset(byte, bit, offset);
        
        return (data[byte] >> bit) & 1;
    }

    inline void BitStream::setBit(char* data, uint64_t offset, bool value)
    {
        if (offset >= m_size)
            throw std::runtime_error("Unable to set out-of-bounds bit of bitstream.");

        uint64_t byte;
        uint64_t bit;
        splitOffset(byte, bit, offset);

        if (value)
            data[byte] |= (1 << bit);
        else
            data[byte] &= ~(1 << bit);
    }

    inline void BitStream::splitOffset(uint64_t& byteOut, uint64_t& bitOut, uint64_t offsetIn)
    {
        byteOut = offsetIn / 8;
        bitOut = offsetIn % 8;
    }
}