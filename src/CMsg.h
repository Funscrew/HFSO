#pragma once
#ifndef CMSG_H
#define CMSG_H

#pragma pack(push, 1)

namespace funscrew::HFSO {

  constexpr uint32_t NON_RELIABLE = 0;
  constexpr uint16_t MAX_DATA_SIZE = 4000;

  // ====================================================================================================================
  enum EMsgType {
    Invalid = 0,
    SyncRequest = 1,
    SyncReply = 2,
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
        uint64_t SessionId;

        /// <summary>
        /// Sequence number specific to sync operations.  All sync numbers must be acked for the connection
        /// to succeed.
        /// </summary>
        int SyncNumber;


        uint16_t DataSize;
        // Byte array of metadata that you might want to exchange.
        // Format of the data is [TOTAL SIZE] [[CODE - 2B] [SIZE - 2B] [DATA - <SIZE>B]]...
        // DataSize == 0 means there is no more metadata!
        uint8_t SyncData[MAX_DATA_SIZE];
      } SyncRequest;

      struct {
        /// <summary>
        /// The last sequence number that we acknowledged.  This is used to ensure that metadata exchange is complete.
        /// Remote will not send new sync requests with differnt numbers before receiving this!
        /// </summary>
        uint16_t SyncNumberAck;

        /// <summary>
        /// Flag to indicate that we are ready and no more sync requests need to be sent!
        /// I don't know if we really need this or not.....
        /// </summary>
        uint8_t Ready;
      } SyncReply;

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
        uint16_t res = sizeof(uint64_t) + sizeof(int) + sizeof(uint16_t) + Content.SyncRequest.DataSize;
        return res;
      }
      case EMsgType::SyncReply:
      {
        uint16_t res = sizeof(SyncReply);
        return res;
      }

      default:
        throw std::exception("Unknown / unsupported header type!");
      }
    }

    // ------------------------------------------------------------------------------------------------------------
    inline bool IsSyncMessage() const { return Header.Type == EMsgType::SyncRequest || Header.Type == EMsgType::SyncReply; }
  };

}

#pragma pack(pop)

#endif