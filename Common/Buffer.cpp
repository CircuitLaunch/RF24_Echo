#include "Buffer.hpp"
#include <Arduino_CRC32.h>

Arduino_CRC32 crc;

void Buffer::setData(unsigned char *iData, unsigned int iLen)
{
    dataLen = 0;
    if(iLen > MESSAGE_BUFFER_SIZE) iLen = MESSAGE_BUFFER_SIZE;
    dataLen = iLen;
    while(iLen--) data[iLen] = iData[iLen];
}

unsigned long Buffer::genChecksum()
{
    return crc.calc(data, dataLen);
}

bool Buffer::check(unsigned long iChecksum)
{
    return crc.calc(data, dataLen) == iChecksum;
}

