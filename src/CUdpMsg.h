#pragma once
#ifndef CUDP_MSG_H
#define CUDP_MSG_H

namespace funscrew::HFSO
{

  constexpr uint32_t NON_RELIABLE = 0;


  // ============================================================================================================
  struct CUdpMsg {
    enum class EMsgType {
      Invalid = 0,
      SyncRequest = 1,
      SyncReply = 2,
      Input = 3,
      QualityReport = 4,
      QualityReply = 5,
      KeepAlive = 6,

      //InputAck = 7,           // Used only on replay appliance and will proably be dropped.
      //Datagram = 8,
      //Heartbeat = 9

      // NOTE: All message types > Heartbeat can be considered "custom"
      // --> We have to means to support custom messages, but food for thought.
    };


    // TODO: Can we use 16 bit numbers to save a bit of space?
    struct  {
      uint32_t Sequence = NON_RELIABLE; // Sequence number.  We keep it all reliable and ordered.  Use NON_RELIABLE (zero) for messages where we don't care about delivery of the packet.
      uint32_t Ack = 0;                 // The highest acked sequence on the other side.
      uint32_t AckBits = 0;             // Bitfield to indicate the status of the last 32 packets before Ack.  This is used for missing / oop retransmission.  Large ping times can really goof this stuff up...
      uint8_t MsgType = 0;              // Type of message that this indicates.      
    } Header;


    union {
      struct { 
        uint16_t Magic;
      } SyncRequest;
    };

  };

}


#endif

