#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

#ifndef MESSAGE_BUFFER_SIZE
#define MESSAGE_BUFFER_SIZE 1024
#endif

class Buffer
{
    friend class ByteBanger;

    public:
        Buffer() : dataLen(0) { }

        void setData(unsigned char *iData, unsigned int iLen);
        const unsigned char *getData() { return data; }
        unsigned int length() { return dataLen; }

        unsigned long genChecksum();
        bool check(unsigned long iChecksum);

        template <class T> T &startCast() {
            return (T &) data;
        }

        template <class T> void endCast(T &iType) {
            dataLen = sizeof(T);
        }

    protected:
        unsigned int dataLen;
        unsigned char data[MESSAGE_BUFFER_SIZE];
};

#endif
