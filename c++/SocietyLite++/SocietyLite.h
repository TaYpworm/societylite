/*
 * Implements pub/sub in single-source multi-dest multicast
 * Can easily implement multi-source multi-dest
 */

#ifndef __SOCIETYLITE_H__
#define __SOCIETYLITE_H__
#include "SocietyPacket.h"

#include <list>
#include <map>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

class SocietyLite {
    public:
        SocietyLite(int port, string serverAddress);
        SocietyLite(int port, string serverAddress, string serviceName, int priority);
        ~SocietyLite();
        int publish(unsigned char *data, int size);
        void subscribe(string *sub);
        int unsubscribe(string *sub);
        int poll();
        int bpoll();
        //int popMessage(string &sub, unsigned char *&data, int &size);
        unsigned char *popMessage(string &sub, int &size);
        int clearMessageQueue(string *sub);
        int clearAllMessages();
        int getNumMessages(string *sub);
        int getMaxMessages();
        void setMaxMessages(int maxMessages);
        string getServiceName();
        void setServiceName(string serviceName);

    private:
        bool canPublish;
        bool nonBlocking;
        bool reuseAddr;
        int societyPort;
        int maxMessages;
        int recvBufferSize;
        int sendBufferSize;
        int sequenceNumber;
        int priority;
        int socketfd;
        int hashSize;
        // make smart ptrs
        unsigned char *recvBuffer;
        unsigned char *sendBuffer;
        unsigned char *serviceHash;
        string serviceName;
        fd_set rset;
        fd_set wset;
        map <string, string> subIndex;
        map <string, string>::iterator subIter;
        map <string, list<SocietyPacket *> > messageQueue;
        map <string, list<SocietyPacket *> >::iterator messageIter;
        struct timeval timeout;
        struct sockaddr_in *ENSAddr;
        socklen_t addrLen;
        enum packetType {
            SUBSCRIBE,
            UNSUBSCRIBE,
            PUBLISH
        };

    private:
        unsigned char *generateHash(string *inString);
        SocietyPacket *recv();
        int send(SocietyPacket *packet);
        int checkReady(timeval *timeout);
        void initVars(int port, string serverAddress, bool nonBlocking);
        void clearSets(); 
};

#endif
