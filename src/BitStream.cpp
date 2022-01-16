#include "BitStream.h"
#include <stdexcept>

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

    uint64_t BitStream::minBytes(uint64_t nBits)
    {
        return (nBits + 7) / 8;
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