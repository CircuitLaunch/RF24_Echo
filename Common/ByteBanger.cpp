#include "ByteBanger.hpp"

ByteBanger::operator bool()
{
    return error == 0;
}

bool ByteBanger::recycleRecvBuffer(Buffer *iBuffer)
{
    if(nextReadBuffer) return false;
    nextReadBuffer = iBuffer;
    return true;
}

bool ByteBanger::sendBuffer(Buffer *iBuffer)
{
    if(nextWriteBuffer) return false;
    nextWriteBuffer = iBuffer;
    return true;
}

void ByteBanger::abort(Buffer *iRecvBuffer)
{
    currentReadBuffer = nullptr;
    nextReadBuffer = iRecvBuffer;
    readPtr = nullptr;
    readChecksum = 0;
    readIndex = 0;
    readSignature = 0;

    currentWriteBuffer = nullptr;
    nextWriteBuffer = nullptr;
    writePtr = nullptr;
    writeChecksum = 0;
    writeIndex = 0;

    error = 0;
}

void ByteBanger::update(int &oRecvCount, Buffer *&oRecvBuffer, int &oSendCount, Buffer *&oSendBuffer)
{
    oRecvCount = recv(oRecvBuffer);
    oSendCount = send(oSendBuffer);
}

int ByteBanger::recv(Buffer *&oBuffer)
{
    int readByte;
    oBuffer = nullptr;
    if(!currentReadBuffer) {
        if(nextReadBuffer) {
            currentReadBuffer = nextReadBuffer;
            currentReadBuffer->dataLen = 0;
            nextReadBuffer = nullptr;
        }
    } else {
        if(readPtr == nullptr) {
            readPtr = (unsigned char *) &readSignature;
            readIndex = 0;
        }
        if(readPtr == (unsigned char *) &readSignature) {
            if(readIndex < sizeof(readSignature)) {
                if(stream.available()) {
                    setReadTimestamp();
                    readByte = stream.read();
                    if(readByte >= 0) {
                        readByte &= 0xff;
                        if(((unsigned char) readByte) == ((unsigned char *) &writeSignature)[readIndex]) {
                            readPtr[readIndex] = (unsigned char) readByte;
                            readIndex++;
                        } else {
                            readIndex = 0;
                        }
                    }
                }
            } else {
                if(readSignature != SIGNATURE) {
                    error = -1;
                    oBuffer = currentReadBuffer;
                    readPtr = nullptr;
                    readIndex = 0;
                    currentReadBuffer = nullptr;
                    return error;
                }

                readPtr = (unsigned char *) &currentReadBuffer->dataLen;
                readIndex = 0;
            }
        }
        if(readPtr == (unsigned char *) &currentReadBuffer->dataLen) {
            if(readIndex < sizeof(currentReadBuffer->dataLen)) {
                if(stream.available()) {
                    setReadTimestamp();
                    readByte = stream.read();
                    if(readByte >= 0) {
                        readByte &= 0xff;
                        readPtr[readIndex] = (unsigned char) readByte;
                        readIndex++;
                    }
                }
            } else {
                if(currentReadBuffer->dataLen > MESSAGE_BUFFER_SIZE) {
                    error = -2;
                    oBuffer = currentReadBuffer;
                    readPtr = nullptr;
                    readIndex = 0;
                    currentReadBuffer = nullptr;
                    return error;
                }
                readPtr = (unsigned char *) currentReadBuffer->data;
                readIndex = 0;
            }
        }
        if(readPtr == (unsigned char *) currentReadBuffer->data) {
            if(readIndex < currentReadBuffer->dataLen) {
                if(stream.available() && ((readByte = stream.read()) >= 0)) {
                    setReadTimestamp();
                    readByte &= 0xff;
                    readPtr[readIndex] = (unsigned char) readByte;
                    readIndex++;
                }
            } else {
                readPtr = (unsigned char *) &readChecksum;
                readIndex = 0;
            }
        }
        if(readPtr == (unsigned char *) &readChecksum) {
            if(readIndex < sizeof(readChecksum)) {
                if(stream.available()) {
                    setReadTimestamp();
                    readByte = stream.read();
                    if(readByte >= 0) {
                        readByte &= 0xff;
                        readPtr[readIndex] = (unsigned char) readByte;
                        readIndex++;
                    }
                }
            } else {
                oBuffer = currentReadBuffer;
                readPtr = nullptr;
                readIndex = 0;
                currentReadBuffer = nullptr;

                if(oBuffer->genChecksum() == readChecksum) {
                    return oBuffer->dataLen;
                } else {
                    error = -3;
                    return error;
                }
            }
        }
    }
    return 0;
}

int ByteBanger::send(Buffer *&oBuffer)
{
    oBuffer = nullptr;
    if(!currentWriteBuffer) {
        if(nextWriteBuffer) {
            currentWriteBuffer = nextWriteBuffer;
            nextWriteBuffer = nullptr;
        }
    } else {
        if(writePtr == nullptr) {
            writePtr = (unsigned char *) &writeSignature;
            writeIndex = 0;
        }
        if(writePtr == (unsigned char *) &writeSignature) {
            if(writeIndex < sizeof(writeSignature)) {
                setWriteTimestamp();
                stream.write(writePtr[writeIndex]);
                writeIndex++;
            } else {
                writePtr = (unsigned char *) &currentWriteBuffer->dataLen;
                writeIndex = 0;
            }
        }
        if(writePtr == (unsigned char *) &currentWriteBuffer->dataLen) {
            if(writeIndex < sizeof(currentWriteBuffer->dataLen)) {
                setWriteTimestamp();
                stream.write(writePtr[writeIndex]);
                writeIndex++;
            } else {
                writePtr = (unsigned char *) currentWriteBuffer->data;
                writeIndex = 0;
            }
        }
        if(writePtr == currentWriteBuffer->data) {
            if(writeIndex < currentWriteBuffer->dataLen) {
                setWriteTimestamp();
                stream.write(writePtr[writeIndex]);
                writeIndex++;
            } else {
                writeChecksum = currentWriteBuffer->genChecksum();
                writePtr = (unsigned char *) &writeChecksum;
                writeIndex = 0;
            }
        }
        if(writePtr == (unsigned char *) &writeChecksum) {
            if(writeIndex < sizeof(writeChecksum)) {
                setWriteTimestamp();
                stream.write(writePtr[writeIndex]);
                writeIndex++;
            } else {
                stream.flush();
                oBuffer = currentWriteBuffer;
                writePtr = nullptr;
                writeIndex = 0;
                currentWriteBuffer = nullptr;
                return oBuffer->dataLen;
            }
        }
    }
    return 0;
}
