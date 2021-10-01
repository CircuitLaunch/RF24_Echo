#ifndef __RADIOSTREAM_HPP__
#define __RADIOSTREAM_HPP__

#include <RF24.h>

class RadioStream : public Stream
{
    public:
        RadioStream(RF24 &iRadio)
        : Stream(), radio(iRadio) { }

        virtual int available() {
            return radio.available();
        }

        virtual int read() {
            uint8_t buf;
            byte pipeNo;
            if(radio.available(&pipeNo)) {
                radio.read(&buf, 1);
                return buf;
            }
            return -1;
        }

        virtual int peek() {
            return 0;
        }

        virtual void flush() {

        }

        virtual size_t write(uint8_t iBuf) {
            size_t result = 1;
            radio.stopListening();
            if(!radio.write(&iBuf, 1)) {
                result = -1;
            }
            radio.startListening();
            return result;
        }

    protected:
        RF24 &radio;
};

#endif
