#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include <ByteBanger.hpp>

#define CE_PIN 9
#define CSN_PIN 10

RF24 Radio(CE_PIN, CSN_PIN);
const uint64_t pipes[2] = { 0x123456789ALL, 0xA987654321LL };

// NOTE: the following values are reversed from those in Transmitter.ino
const int readPipe = 1;
const int writePipe = 0;

RadioStream SerialR(Radio);
ByteBanger Xch(SerialR);
Buffer outboundBuffer, inboundBuffer;

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("Echoer");

    Serial.print("Initializing ");
    if(Radio.isPVariant()) {
        Radio.setDataRate(RF24_2MBPS);
        Serial.println("nRF24L01+");
    } else {
        Serial.println("nRF24L01");
    }
    Radio.setAutoAck(1);
    Radio.enableAckPayload();
    Radio.setRetries(0, 15);
    Radio.openWritingPipe(pipes[writePipe]);
    Radio.openReadingPipe(1, pipes[readPipe]);
    Radio.startListening();

    Xch.recycleRecvBuffer(&inboundBuffer);

    Serial.println("Startup complete. Waiting to echo transmission.");
}

typedef struct Message
{
  unsigned long timestamp;
  unsigned long data;
} Message;

void loop()
{
    int recvCount, sendCount;
    Buffer *inBufPtr, *outBufPtr;
    Xch.update(recvCount, inBufPtr, sendCount, outBufPtr);
    
    if(recvCount < 0) {
        Serial.println("Error: " + String(recvCount) + " failed to receive message");
        Xch.abort(&inboundBuffer);
    } else if(recvCount) {
        if(inBufPtr) {
            Message &msg = inBufPtr->startCast<Message>();
            Message &echo = outboundBuffer.startCast<Message>();
            echo = msg;
            outboundBuffer.endCast(echo);
            Xch.sendBuffer(&outboundBuffer);
        }
        Xch.recycleRecvBuffer(&inboundBuffer);
    }
    
    if(sendCount < 0) {
        Serial.println("Error: " + String(sendCount) + " failed to send message");
        Xch.abort(&inboundBuffer);
    }

}
