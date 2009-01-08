#ifndef __SOCIETYPACKET_H__
#define __SOCIETYPACKET_H__

#include <stdint.h>

class SocietyPacket {
    public:
        uint8_t ident;
        uint16_t packetType;
        uint32_t sequenceNumber;
        uint8_t priority;
        uint8_t moreFragments;
        uint32_t fragmentOffset;
        uint32_t payloadSize;
        unsigned char source[20];
        unsigned char* data;

    public:
        SocietyPacket();
        SocietyPacket(int pSize, int type, int sNum, int prior, unsigned char *src);
        //SocietyPacket(int pSize, void *packet);
        ~SocietyPacket();
        
        static int getHeaderSize() {
            return headerSize;
        }

        int size();
        unsigned char *serialize();
        void deserialize(unsigned char *data, int size);
        bool isSocietyPacket();
        int appendPayload(unsigned char *data, int size);
    
    private:
        bool dataInitialized;
        static int headerSize;
        int packetSize;
        int dataOffset;
        uint8_t identifier;

    private:
        void initCommonVars();
};

#endif
