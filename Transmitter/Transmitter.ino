#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include <ByteBanger.hpp>

#define CE_PIN 9
#define CSN_PIN 10

RF24 Radio(CE_PIN, CSN_PIN);
const uint64_t pipes[2] = { 0x123456789ALL, 0xA987654321LL };

// NOTE: the following values are reversed from those in Echoer.ino
const int readPipe = 0;
const int writePipe = 1;

RadioStream SerialR(Radio);
ByteBanger Xch(SerialR);
Buffer outboundBuffer, inboundBuffer;

unsigned long timer;

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("Transmitter");

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

    Serial.println("Startup complete. Commencing transmission.");

    timer = millis();
}

typedef struct Message
{
	unsigned long timestamp;
	unsigned long data;
} Message;

unsigned long counter;

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
            Serial.println("Received echo. Timestamp: " + String(msg.timestamp) + ", counter: " + String(msg.data));
        }
        Xch.recycleRecvBuffer(&inboundBuffer);
    }
    
    if(sendCount < 0) {
        Serial.println("Error: " + String(sendCount) + " failed to send message");
        Xch.abort(&inboundBuffer);
    }

    if((millis() - timer) > 125) {
        timer = millis();
        Message &msg = outboundBuffer.startCast<Message>();
        msg.timestamp = millis();
				msg.data = counter;
        outboundBuffer.endCast(msg);
        Xch.sendBuffer(&outboundBuffer);
        counter++;
    }
}
