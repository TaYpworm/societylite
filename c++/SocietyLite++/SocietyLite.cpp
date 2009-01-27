#include "SocietyLite.h"
#include "NetworkException.h"

#include <cstring>
#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdint.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

using namespace std;

SocietyLite::SocietyLite(int port, string serverAddress) {
    // provide a blocking option
    initVars(port, serverAddress, true);
}

// remove priority
SocietyLite::SocietyLite(int port, string serverAddress, string serviceName, int priority) {
    initVars(port, serverAddress, true);
    this->canPublish = true;
    this->serviceName = serviceName;
    this->serviceHash = generateHash(serviceName);
    if (priority > 128) {
        throw NetworkException("Priority is too high");
    } else {
        this->priority = priority;
    }
}

SocietyLite::~SocietyLite() {
    if (recvBuffer != NULL)
        delete[] recvBuffer;
    if (sendBuffer != NULL)
        delete[] sendBuffer;
    if (serviceHash != NULL)
        delete[] serviceHash;

    // unsubscribe to all
    //for (int i=0; i<messageQueue.size(); i++) {
    //    
    //}
    
    close(socketfd);
}

int  SocietyLite::publish(unsigned char *data, int size) {
    if (canPublish) {
        SocietyPacket *packet;

        packet = new SocietyPacket(size, PUBLISH, sequenceNumber, priority, serviceHash);
        packet->appendPayload(data, size);
        send(packet);
        sequenceNumber++;
        delete packet;

        return 0;
    }
    return -1;
}

void SocietyLite::subscribe(string &sub) {
    SocietyPacket *packet;
    unsigned char *subHash;
    string *stringHash;
    list<SocietyPacket *> *queue;

    subHash = generateHash(sub);
    stringHash = new string((char *)subHash, hashSize);
    subIndex[sub] = *stringHash;
    queue = new list<SocietyPacket *>;
    messageQueue[*stringHash] = *queue;

    packet = new SocietyPacket(hashSize, SUBSCRIBE, sequenceNumber, priority, serviceHash);
    packet->appendPayload(subHash, hashSize);
    send(packet);
    sequenceNumber++;
    delete packet;
    delete[] subHash;
    delete stringHash;
}

void SocietyLite::voSubscribe(string &sub, ConnectionType type) {
    int payloadSize;
    SocietyPacket *packet;
    unsigned char *subHash, connType=0;
    string *stringHash;
    list<SocietyPacket *> *queue;

    subHash = generateHash(sub);
    stringHash = new string((char *)subHash, hashSize);
    subIndex[sub] = *stringHash;
    queue = new list<SocietyPacket *>;
    messageQueue[*stringHash] = *queue;

    payloadSize = hashSize + sizeof(connType) + sub.size();
    connType += type;

    packet = new SocietyPacket(payloadSize, VOSUBSCRIBE, sequenceNumber, priority, serviceHash);
    packet->appendPayload(subHash, hashSize);
    packet->appendPayload((unsigned char*)&connType, sizeof(connType));
    packet->appendPayload((unsigned char*)sub.c_str(), sub.size());
    send(packet);
    sequenceNumber++;
    delete packet;
    delete[] subHash;
    delete stringHash;
}

int SocietyLite::unsubscribe(string &sub) {
    if (subIndex.count(sub) > 0) {
        SocietyPacket *packet;
        unsigned char *subHash;
        string *stringHash;

        subHash = generateHash(sub);
        stringHash = new string((char *)subHash, hashSize);
        subIndex.erase(sub);
        messageQueue.erase(*stringHash);

        packet = new SocietyPacket(hashSize, UNSUBSCRIBE, sequenceNumber, priority, serviceHash);
        packet->appendPayload(subHash, hashSize);
        send(packet);
        sequenceNumber++;
        delete packet;
        delete[] subHash;
        delete stringHash;

        return 0;
    }
    return -1;
}

int SocietyLite::poll() {
    return checkReady(&timeout);
}

int SocietyLite::bpoll() {
    return checkReady(NULL);
}

/*
int SocietyLite::popMessage(string &sub, unsigned char *&data, int &size) {
    string stringHash;
    SocietyPacket *packet;

    if (subIndex.find(sub) != subIndex.end()) {
        stringHash = subIndex.find(sub)->second;
        if (messageQueue.find(stringHash) != messageQueue.end()) {
            packet = messageQueue.find(stringHash)->second.front();
            messageQueue.find(stringHash)->second.pop_front();
            size = packet->payloadSize;
            data = new unsigned char[size];
            bzero(data, size);

            memcpy(data, packet->data, size);
            delete packet;
            
            return 0;
        }
    }
    return -1;
}
*/

unsigned char * SocietyLite::popMessage(string &sub, int &size) {
    string stringHash;
    SocietyPacket *packet;
    unsigned char *data;

    if (subIndex.find(sub) != subIndex.end()) {
        stringHash = subIndex.find(sub)->second;
        if (messageQueue.find(stringHash) != messageQueue.end()) {
            packet = messageQueue.find(stringHash)->second.front();
            messageQueue.find(stringHash)->second.pop_front();
            size = packet->payloadSize;
            data = new unsigned char[size];
            bzero(data, size);

            memcpy(data, packet->data, size);
            delete packet;
            
            return data;
        }
    }
    return NULL;
}

int SocietyLite::clearMessageQueue(string &sub) {
    int size;
    string stringHash;
    
    if (subIndex.find(sub) != subIndex.end()) {
        stringHash = subIndex.find(sub)->second;
        if (messageQueue.find(stringHash) != messageQueue.end()) {
            size = messageQueue.find(stringHash)->second.size();
            messageQueue.find(stringHash)->second.clear();
            return size;
        }
    }
    return -1;
}

int SocietyLite::clearAllMessages() {
    int totalMessages = 0;
    for (messageIter = messageQueue.begin(); messageIter != messageQueue.end(); messageIter++) {
        totalMessages += (*messageIter).second.size();
        (*messageIter).second.clear();
    }
    return totalMessages;
}

int SocietyLite::getNumMessages(string &sub) {
    string stringHash;
    
    if (subIndex.find(sub) != subIndex.end()) {
        stringHash = subIndex.find(sub)->second;
        return messageQueue.find(stringHash)->second.size();
    }
    return -1;
}

int SocietyLite::getMaxMessages() {
    return maxMessages;
}

void SocietyLite::setMaxMessages(int maxMessages) {
    this->maxMessages = maxMessages;
}

string SocietyLite::getServiceName() {
    return serviceName;
}

void SocietyLite::setServiceName(string serviceName) {
    this->serviceName = serviceName;
    this->canPublish = true;
    this->serviceHash = generateHash(serviceName);
}

unsigned char *SocietyLite::generateHash(string &inString) {
    unsigned char *hash = new unsigned char[20];
    SHA1((unsigned char*)inString.c_str(), inString.size(), hash);
    return hash;
}

SocietyPacket *SocietyLite::recv() {
    int bytesRecv, bytesToRecv, total;
    SocietyPacket *packet;

    clearSets();
    bzero(recvBuffer, recvBufferSize);
    bytesRecv = 0;

    select(socketfd+1, &rset, NULL, NULL, NULL);
    if (FD_ISSET(socketfd, &rset)) {
        if ((bytesRecv = read(socketfd, recvBuffer, SocietyPacket::getHeaderSize())) < 0) {
            if (errno != EWOULDBLOCK)
                throw NetworkException("Error reading Society header");
        } else if (bytesRecv == 0) {
            throw NetworkException("Connection before header received");
        } else if (bytesRecv == SocietyPacket::getHeaderSize()){
            packet = new SocietyPacket();
            packet->deserialize(recvBuffer, SocietyPacket::getHeaderSize());
        } else {
            // Create case if < Society header size.
            printf("partial header recv\n");
        }
    }
    // debugging
    total = 0;
    printf("recvBufferSize: %d\n", recvBufferSize);
    bytesToRecv = packet->payloadSize;
    while (bytesToRecv != 0) {
        clearSets();
        bzero(recvBuffer, recvBufferSize);
        bytesRecv = 0;

        select(socketfd+1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(socketfd, &rset)) {
            if (bytesToRecv > recvBufferSize) { 
                bytesRecv = read(socketfd, recvBuffer, recvBufferSize);
            } else {
                bytesRecv = read(socketfd, recvBuffer, bytesToRecv);
            }

            if (bytesRecv < 0) {
                if (errno != EWOULDBLOCK)
                    throw NetworkException("Error reading from socket");
            } else if (bytesRecv == 0) {
                throw NetworkException("Connection terminated prematurely");
            } else {
                //debugging
                printf("bytesReceived: %d\tbytesToRecv: %d\n", bytesRecv, bytesToRecv);
                for (int i=0; i<bytesRecv; i++)
                    if (recvBuffer[i] != ((total+i)%26+97))
                        printf("error in recv: %d\t%d\t%c\t%c\n", total, i, (total+i)%26+97, recvBuffer[i]);
                total += bytesRecv;

                packet->appendPayload(recvBuffer, bytesRecv);
                bytesToRecv -= bytesRecv;
            }
        }
    }

    return packet;
}

int SocietyLite::send(SocietyPacket *packet) {
    unsigned char *sendPtr, *serPacket, *front, *back;
    int bytesSent, totalBytesToSend, bytesToSend;
   
    serPacket = packet->serialize();
    totalBytesToSend = packet->size();
    sendPtr = serPacket;
   
    printf("bytesToSend: %d\n", totalBytesToSend);
    printf("packetSize: %d\n", packet->payloadSize);

    while (totalBytesToSend != 0) {
        clearSets();
        bzero(sendBuffer, sendBufferSize);
        bytesSent = 0;
        
        if (totalBytesToSend < sendBufferSize) {
            memcpy(sendBuffer, sendPtr, totalBytesToSend);
            front = sendBuffer;
            bytesToSend = totalBytesToSend;
            back = sendBuffer + bytesToSend;
        } else {
            memcpy(sendBuffer, sendPtr, sendBufferSize);
            front = sendBuffer;
            back = sendBuffer + sendBufferSize;
            bytesToSend = sendBufferSize;
        }

        while (front != back) {
            int num = select(socketfd+1, NULL, &wset, NULL, NULL);
            if (FD_ISSET(socketfd, &wset)) {
    
                //for (int i=0; i<packet->size(); i++)
                //    printf("%02x", serPacket[i]);
                //printf("\n");

                if ((bytesSent = write(socketfd, sendPtr, bytesToSend)) < 0) {
                    //printf("bytesSent: %d\n", bytesSent);
                    if (errno != EWOULDBLOCK)
                        throw NetworkException("Error writing to socket");
                } else {
                    //printf("bytesSent: %d\n", bytesSent);
                    front += bytesSent;
                    bytesToSend -= bytesSent;
                    totalBytesToSend -= bytesSent;
                    sendPtr += bytesSent;
                }
            }
        }
    }

    delete[] serPacket; 
    return 0;
}

int SocietyLite::checkReady(timeval *time) {
    int numReady;
    string *stringHash;

    clearSets();
    numReady = select(socketfd+1, &rset, NULL, NULL, time);
    SocietyPacket *packet;

    if (numReady > 0) {
        packet = recv();
        // string should not be used here (not null terminated)
        // no guarantee that hash does not contain a null character
        // alternatives:
        // base64 conversion
        // byte array object
        stringHash = new string((char *)packet->source, hashSize);
        if (messageQueue.find(*stringHash)->second.size() >= (unsigned int)maxMessages)
            messageQueue.find(*stringHash)->second.pop_back();
        messageQueue.find(*stringHash)->second.push_front(packet);
        return messageQueue.find(*stringHash)->second.size();
    }

    return 0;


}

void SocietyLite::initVars(int port, string serverAddress, bool nonBlocking) {
    int error, opts;
    struct addrinfo hints, *result;
    socklen_t size;

    this->canPublish = false;
    this->nonBlocking = nonBlocking;
    this->reuseAddr = true;
    this->societyPort = port;
    this->maxMessages = 5;
    this->sequenceNumber = 0;
    this->priority = 1;
    this->hashSize = SHA_DIGEST_LENGTH;

    bzero(&hints, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(serverAddress.c_str(), NULL, &hints, &result);
    
    if (error != 0) {
        char *message = new char[256];
        sprintf(message, "error in getaddrinfo: %s", gai_strerror(error));
        throw NetworkException(message);
    }
    if (result == NULL)
        throw NetworkException("Unable to resolve network address");
    
    ENSAddr = (struct sockaddr_in *)result->ai_addr;

    socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (this->reuseAddr) {
        opts = 1;
        setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));
    }

    if (this->nonBlocking) {
        int flags;
        if ((flags = fcntl(socketfd, F_GETFL, 0)) < 0)
            throw NetworkException("File status flag get failed");
        
        flags |= O_NONBLOCK;
        if (fcntl(socketfd, F_SETFL, flags) < 0)
            throw NetworkException("Unable to set non-blocking mode on socket");
    }

    size = sizeof(int);
    // check errors
    error = getsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char *) &recvBufferSize, &size);
    error = getsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (char *) &sendBufferSize, &size);

    // check scoping
    this->recvBuffer = new unsigned char[recvBufferSize];
    this->sendBuffer = new unsigned char[sendBufferSize];

    ENSAddr->sin_port = htons(societyPort);
    error = connect(socketfd, result->ai_addr, result->ai_addrlen);
    if (error < 0) {
        if (errno == EINPROGRESS) {
            clearSets();
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            error = select(socketfd+1, NULL, &wset, NULL, &timeout);
            if (error < 0 && errno != EINTR) {
                char *message = new char[256];
                sprintf(message, "Unable to connect to Society ENS %s: %s", serverAddress.c_str(), strerror(errno));
                throw NetworkException(message);
            } else if (error > 0) {
                size = sizeof(int);
                if (getsockopt(socketfd, SOL_SOCKET, SO_ERROR, (void *)&opts, &size) < 0)
                    throw NetworkException("Error querying socket");
                if (opts) {
                    char *message = new char[256];
                    sprintf(message, "Unable to connect to Society ENS %s: %s", serverAddress.c_str(), strerror(errno));
                    throw NetworkException(message);
                }
            } else {
                throw NetworkException("Society ENS connection timed out");
            }

        } else {
            char *message = new char[256];
            sprintf(message, "Unable to connect to Society ENS %s: %s", serverAddress.c_str(), strerror(errno));
            throw NetworkException(message);
        }
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    freeaddrinfo(result);
}

void SocietyLite::clearSets() {
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_SET(socketfd, &rset);
    FD_SET(socketfd, &wset);
}
