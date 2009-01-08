#ifndef PACKET_H
#define PACKET_H

/* This defines the structure of the message packet that is used to send     */
/* messages between nodes.                                                   */

#include "Message.h"

#define PKTHDRSIZE (4 * sizeof (int))
#define PKTLNKSIZE (sizeof (void *) + sizeof (int))
#define MSGS_PER_PACKET ((1024 - (PKTLNKSIZE + PKTHDRSIZE)) / sizeof (Message))

typedef struct _Packet
{
  struct _Packet *link;    
  int ID;                  /* for debug, packet serial # */
                 /* Following is what is actually transmitted */
  int nMsgs;               /* Number of messages in packet */
  int Node;                /* Node which sent this packet */
  int TimeSent;            /* Time of earliest message in packet */
  int LastTime;            /* Time of latest message in packet */
  Message Msgs [MSGS_PER_PACKET];      /* Space for messages */
} PACKET;

#endif 
