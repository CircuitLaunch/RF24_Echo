#ifndef __EXCHANGE_HPP__
#define __EXCHANGE_HPP__

#include <Arduino.h>
#include "Buffer.hpp"
#include "RadioStream.hpp"

#define SIGNATURE 0xABCDEFAB

class ByteBanger
{
    public:
        ByteBanger(Stream &iStream)
        : error(0), stream(iStream),
          lastReadTimestamp(0),
          readSignature(0), readChecksum(0),
          readPtr(nullptr), readIndex(0),
          currentReadBuffer(nullptr), nextReadBuffer(new Buffer()),
          lastWriteTimestamp(0),
          writeSignature(SIGNATURE), writeChecksum(0),
          writePtr(nullptr), writeIndex(0),
          currentWriteBuffer(nullptr), nextWriteBuffer(nullptr) { }
		virtual ~ByteBanger() { }

        virtual operator bool();
        int getError() { return error; }

        bool recycleRecvBuffer(Buffer *iBuffer);
        bool sendBuffer(Buffer *iBuffer);

        void setReadTimestamp() { noInterrupts(); lastReadTimestamp = millis(); interrupts(); }
        void setWriteTimestamp() { noInterrupts(); lastWriteTimestamp = millis(); interrupts(); }

        unsigned long readTimestamp() { return lastReadTimestamp; }
        unsigned long writeTimestamp() { return lastWriteTimestamp; }

        void abort(Buffer *iRecvBuffer);

        void update(int &oRecvCount, Buffer *&oRecvBuffer, int &oSendCount, Buffer *&oSendBuffer);

    protected:
        int recv(Buffer *&oBuffer);
        int send(Buffer *&oBuffer);

    protected:
        int error;
        Stream &stream;
        unsigned long lastReadTimestamp;
        unsigned long readSignature;
        unsigned long readChecksum;
        unsigned char *readPtr;
        unsigned int readIndex;
        Buffer *currentReadBuffer;
        Buffer *nextReadBuffer;
        unsigned long lastWriteTimestamp;
        unsigned long writeSignature;
        unsigned long writeChecksum;
        unsigned char *writePtr;
        unsigned int writeIndex;
        Buffer *currentWriteBuffer;
        Buffer *nextWriteBuffer;
};

#endif
