#pragma once

#include "IClockSource.hpp"

#include <stdint.h>


// TODO: Maybe this lives in some kind of 'platform' header?



constexpr int MAX_PACKET_SIZE = 4000;

namespace HFPO
{

  // ============================================================================================================
  struct CNetworkStats {
    int      bytes_sent = 0;
    int      packets_sent = 0;

    int      BytesReceived = 0;
    int      PacketsReceived = 0;
    float    KbpsReceived = 0;

    float    kbps_sent = 0.0f;

    // NEW: how long has client been active for?
    int       Uptime = 0.0;
  };

  // ============================================================================================================
  // Represents a UDP connection.
    // Such connections can be real, or simulated.
  class CUdpBlaster {

  public:
    CNetworkStats Stats;

    CUdpBlaster(IClockSource* clock_);

    //int Receive(byte* receiveBuffer, struct sockaddr_in& from);
    int Send(byte* sendBuffer, size_t size, struct sockaddr* to);

    // virtual ~CUdpBlaster();

  protected:

    IClockSource* _Clock = nullptr;

    // These need to be the 'internal' versions of send + receive.
    virtual int ReceiveImpl(byte* receiveBuffer, struct sockaddr_in& from) = 0;
    virtual int SendImpl(byte* sendBuffer, size_t size, struct sockaddr* to) = 0;
  };

  // ============================================================================================================
  class CUdpClient : public CUdpBlaster {

  public:
    CUdpClient(uint16_t port_, IClockSource* clock_);
    ~CUdpClient();

  protected:
    virtual int ReceiveImpl(byte* receiveBuffer, struct sockaddr_in& from) override;
    virtual int SendImpl(byte* sendBuffer, size_t size, struct sockaddr* to) override;

  private:

    uint16_t _Port = 0;
    Socket _Socket = 0;
  };

  // ============================================================================================================
  // A simulated UDP client for sending / receiving simulated udp traffic.
  // Used in testing scenarios when we want to be able to reliably recreate certain network conditions.
  class CSimUdpClient : CUdpBlaster {

  };

  // ============================================================================================================
  // HFPOClient is responsible for managing local and remote connections to other players and services.
  class CHFSOClient
  {
  public:


    //CHFSOClient();


  private:

    // Stuff we need...
    // Local Player(s).
    // Remote Player(s).
    // Services?  ReplayClient, Spectator, etc.?

  };
}
