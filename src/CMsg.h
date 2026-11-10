#pragma once
#ifndef CMSG_H
#define CMSG_H

namespace funscrew::HFSO {

  constexpr uint32_t NON_RELIABLE = 0;
  constexpr uint16_t MAX_DATA_SIZE = 4000;

  // ====================================================================================================================
  enum EMsgType {
    Invalid = 0,
    SyncRequest = 1,
    //SyncReply = 2,
    //Input = 3,
    //QualityReport = 4,
    //QualityReply = 5,
    //KeepAlive = 6,
    //InputAck = 7,
    //Datagram = 8,
    //Heartbeat = 9
  };

  // ====================================================================================================================
  struct CSyncMetadata {
    uint16_t Code;
    uint16_t DataSize;
    uint16_t SyncData;
  };

  // ====================================================================================================================
  struct CMsg {

    struct {
      // Magic + Sequence numbers are set during the send operation(s)
      uint16_t Magic;
      uint16_t Sequence;

      // Corresonds to EMsgType
      uint8_t Type;
    } Header;

    union {
      struct {
        /// <summary>
        /// The last sequence number that we acknowledged.  This is used to ensure that metadata exchange is complete.
        /// </summary>
        uint16_t LastAckSequence;

        uint64_t SessionId;

        uint16_t DataSize;
        // Byte array of metadata that you might want to exchange.
        // Format of the data is [TOTAL SIZE] [[CODE - 2B] [SIZE - 2B] [DATA - <SIZE>B]]...
        // DataSize == 0 means there is no more metadata!
        uint8_t SyncData[MAX_DATA_SIZE];
      } SyncRequest;
    } Content;

    // ------------------------------------------------------------------------------------------------------------
    CMsg() {}

    // ------------------------------------------------------------------------------------------------------------
    CMsg(EMsgType type) {
      Header.Type = (uint8_t)type;
    }

    // ------------------------------------------------------------------------------------------------------------
    size_t PacketSize() {
      size_t res = sizeof(Header) + DataSize();
      return res;
    }

    // ------------------------------------------------------------------------------------------------------------
    uint16_t DataSize() {
      switch (Header.Type) {
      case EMsgType::SyncRequest:
      {
        uint16_t res = sizeof(uint64_t) + sizeof(uint16_t) + sizeof(uint16_t) + Content.SyncRequest.DataSize;
        return res;
      }

      default:
        throw std::exception("Unknown / unsupported header type!");
      }
    }
  };

}

#endif