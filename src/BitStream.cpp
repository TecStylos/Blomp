#include "BitStream.h"

namespace Blomp
{
    BitStream::BitStream(uint64_t nBits)
    {
        resize(nBits);
    }

    BitStream::BitStream(std::istream& iStream)
    {
        iStream >> *this;
    }

    bool BitStream::readBit()
    {
        ++m_readOffset;
        if (m_readOffset > m_size)
            return false;

        return getBit(m_data.data(), m_readOffset - 1);
    }

    void BitStream::writeBit(bool value)
    {
        ++m_writeOffset;

        if (m_readOffset > m_size)
            resize(m_writeOffset);

        setBit(m_data.data(), m_writeOffset - 1, value);
    }

    void BitStream::read(void* dest, uint64_t nBits, uint64_t destOffset)
    {
        for (uint64_t i = 0; i < nBits; ++i)
            setBit((char*)dest, destOffset + i, readBit());
    }

    void BitStream::write(const void* src, uint64_t nBits, uint64_t srcOffset)
    {
        for (uint64_t i = 0; i < nBits; ++i)
            writeBit(getBit((const char*)src, srcOffset + i));
    }

    void BitStream::resize(uint64_t nBits)
    {
        m_size = nBits;
        m_data.resize(BitStream::minBytes(nBits));
    }

    void BitStream::reset() noexcept
    {
        m_readOffset = 0;
        m_writeOffset = 0;
        m_size = 0;
        m_data.clear();
    }

    uint64_t BitStream::size() const
    {
        return m_size;
    }

    void* BitStream::data()
    {
        return m_data.data();
    }

    const void* BitStream::data() const
    {
        return m_data.data();
    }

    bool BitStream::getBit(const char* data, uint64_t offset) const
    {
        uint64_t byte;
        uint64_t bit;
        splitOffset(byte, bit, offset);
        
        return (data[byte] >> bit) & 1;
    }

    void BitStream::setBit(char* data, uint64_t offset, bool value)
    {
        uint64_t byte;
        uint64_t bit;
        splitOffset(byte, bit, offset);

        if (value)
            data[byte] |= (1 << bit);
        else
            data[byte] &= ~(1 << bit);
    }

    uint64_t BitStream::minBytes(uint64_t nBits)
    {
        return (nBits + 7) / 8;
    }

    void BitStream::splitOffset(uint64_t& byteOut, uint64_t& bitOut, uint64_t offsetIn)
    {
        byteOut = offsetIn / 8;
        bitOut = offsetIn % 8;
    }

    std::istream& operator>>(std::istream& iStream, BitStream& bs)
    {
        uint64_t nBits;
        iStream.read((char*)&nBits, sizeof(nBits));

        bs.reset();
        bs.resize(nBits);

        iStream.read((char*)bs.data(), BitStream::minBytes(nBits));

        return iStream;
    }
    std::ostream& operator<<(std::ostream& oStream, const BitStream& bs)
    {
        uint64_t nBits = bs.size();
        oStream.write((const char*)&nBits, sizeof(nBits));
        oStream.write((char*)bs.data(), BitStream::minBytes(nBits));

        return oStream;
    }
}