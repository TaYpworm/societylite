#include "SocietyPacket.h"

#include <cstdio>
#include <cstring>
#include <arpa/inet.h>

int SocietyPacket::headerSize = 37;

SocietyPacket::SocietyPacket() {
    initCommonVars();
    dataInitialized = false;
}

SocietyPacket::SocietyPacket(int pSize, int type, int sNum, int prior, unsigned char *src) {
    initCommonVars();
    packetSize = pSize + headerSize;

    ident = identifier;
    packetType = type;
    sequenceNumber = sNum;
    priority = prior;
    moreFragments = 0;
    fragmentOffset = 0;
    payloadSize = pSize;
    memcpy((void *)source, (void *) src, 20);
    data = new unsigned char[payloadSize];
    dataInitialized = true;
}

SocietyPacket::~SocietyPacket() {
    delete[] data;
}

int SocietyPacket::size() {
    return packetSize;
}

unsigned char *SocietyPacket::serialize() {
    int offset = 0;
    uint16_t tmp16;
    uint32_t tmp32;
    unsigned char *buffer = new unsigned char[packetSize];
    bzero(buffer, packetSize);
    
    buffer[offset] = ident;
    offset++;
    
    tmp16 = htons(packetType);
    memcpy(&buffer[offset], &tmp16, 2);
    offset += 2;
    
    tmp32 = htonl(sequenceNumber);
    memcpy(&buffer[offset], &tmp32, 2);
    offset += 4;

    buffer[offset] = priority;
    offset++;

    buffer[offset] = moreFragments;
    offset++;

    tmp32 = htonl(fragmentOffset);
    memcpy(&buffer[offset], &tmp32, 4);
    offset += 4;

    tmp32 = htonl(payloadSize);
    memcpy(&buffer[offset], &tmp32, 4);
    offset += 4;

    memcpy(&buffer[offset], &source, 20);
    offset += 20;

    memcpy(&buffer[offset], data, packetSize - headerSize);
    offset += (packetSize - headerSize);

    dataOffset = packetSize - headerSize;
    
    return buffer;
}

void SocietyPacket::deserialize(unsigned char *packetData, int size) {
    int offset = 0;
    uint16_t tmp16;
    uint32_t tmp32;

    ident = packetData[offset];
    offset++;

    if (ident != 0xAA)
        printf("there's your problem\n");

    memcpy(&tmp16, &packetData[offset], 2);
    packetType = ntohs(tmp16);
    offset += 2;

    memcpy(&tmp32, &packetData[offset], 2);
    sequenceNumber = ntohl(tmp32);
    offset += 4;

    priority = packetData[offset];
    offset++;

    moreFragments = packetData[offset];
    offset++;

    memcpy(&tmp32, &packetData[offset], 4);
    fragmentOffset = ntohl(tmp32);
    offset += 4;

    memcpy(&tmp32, &packetData[offset], 4);
    payloadSize = ntohl(tmp32);
    offset += 4;
    printf("PAYLOADSIZE: %d\n", payloadSize);

    memcpy(&source, &packetData[offset], 20);
    offset += 20;
    
    if (!dataInitialized) {
        data = new unsigned char[payloadSize];
        dataInitialized = true;
    }

    memcpy(data, &packetData[offset], size - headerSize);
    offset += size - headerSize;
    dataOffset += size - headerSize;
}

bool SocietyPacket::isSocietyPacket() {
    return ident == identifier;
}

void SocietyPacket::initCommonVars() {
    dataInitialized = false;
    headerSize = 37;
    dataOffset = 0;
    payloadSize = 0;
    identifier = 0xAA;
}

int SocietyPacket::appendPayload(unsigned char *packetData, int size) {
    if (dataInitialized) {
        printf("dataOffset: %d\tsizeAppend: %d\n", dataOffset, size);
        memcpy(&data[dataOffset], packetData, size);
        dataOffset += size;
    }
    
    return 0;
}

