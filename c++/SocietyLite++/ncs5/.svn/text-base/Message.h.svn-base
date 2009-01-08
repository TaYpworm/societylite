#ifndef MESSAGE_H
#define MESSAGE_H

/* New definitions of messages.  The basic message is a union of several     */
/* different types of message (as for example XWindows events).              */

/* Notice that although messages are conceptually independent entities that  */
/* are operated on as linked lists &c, in the implementation they are        */
/* allocated & passed between machines as packets (see Packet.h) containing  */
/* many messages.  This is done to reduce allocation and copy overhead, and  */
/* is why the link field in the Message struct is passed, even though it is  */
/* meaningless on a different machine.                                       */

#include <stdlib.h>

#include "defines.h"

/**
 * The generic message type.  All messages must contain these fields.
 */
struct ANY_MSG
{
        /// Integer Code indicating the message type
  int Type;
        /// Pointer to allow messages to form linked lists
  ANY_MSG *link;
        /// Time when the message must be delivered
  int Time;
        /// Destination address (function to receive msg)
  void *Addr;
};
typedef struct ANY_MSG ANY_MSG;


/**
 * Spike message is what gets passed between cells when a synapse fires
 */
struct SPIKE_MSG
{
        /// Integer Code indicating the message type
  int Type; 
        /// Pointer to allow messages to form linked lists
  ANY_MSG *link;  
        /// Time when the message must be delivered
  int Time; 
        /// Destination address (function to receive msg)
  void *Addr;
        /// Destintion synapse
  int SynID;
};
typedef struct SPIKE_MSG SPIKE_MSG;

/**
 * Generic Stimulus Message.
 * Stim message comes from outside stimulus objects to cells.  Currently not passed 
 * between nodes.  There are two types, CURRENT and VOLTAGE, that reflect
 * the MODE type in Stimulus.cpp.                                        
 */
typedef struct    /* This is a generic stimulus, used so the Stimulus module */
{                 /* doesn't have to do an if (type) test for every message  */
        /// Integer Code indicating the message type
  int Type; 
        /// Pointer to allow messages to form linked lists 
  ANY_MSG *link;
        /// Destination address (function to receive msg)
  int Time; 
        /// Destination address (function to receive msg)
  void *Addr;
        /// Message Content - either voltage or current
  float Value; 
} STIM_MSG;

/**
 * Stimulus Message for VOLTAGE Mode
 */
struct STIMV_MSG
{
        /// Integer Code indicating the message type
  int Type; 
        /// Pointer to allow messages to form linked lists 
  ANY_MSG *link;
        /// Destination address (function to receive msg)
  int Time; 
        /// Destination address (function to receive msg)
  void *Addr;
        /// Message Content - Destination compartment should set membrane voltage
  float Voltage;
};
typedef struct STIMV_MSG STIMV_MSG;

/**
 * Stimulus Message for CURRENT Mode
 */
struct STIMC_MSG
{
        /// Integer Code indicating the message type
  int Type; 
        /// Pointer to allow messages to form linked lists 
  ANY_MSG *link;
        /// Destination address (function to receive msg)
  int Time; 
        /// Destination address (function to receive msg)
  void *Addr;
        /// Message Content - Destination compartment should integrate current
  float Current;
};
typedef struct STIMC_MSG STIMC_MSG;

/**
 * Union to combines all message types.
 * This is the structure that will get sent between nodes since it
 * determines the memory size.
 */
union Message
{
        /// Integer Code indicating the message type
  int Type;
        /// Generic Message
  ANY_MSG   Any;
        /// Spike Message
  SPIKE_MSG Spike;
        /// Generic Stimulus Message
  STIM_MSG  Stim;
        /// Stimulus Voltage Message
  STIMV_MSG StimV;
        /// Stimulus Current message
  STIMC_MSG StimC;
};
typedef union Message Message;

#endif

