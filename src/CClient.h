#pragma once

#include <stdint.h>

#include "IClockSource.hpp"
#include <vector>

constexpr int MAX_PACKET_SIZE = 4000;

namespace funscrew::HFSO
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
  class IUdpBlaster {

  public:
    CNetworkStats Stats;

    IUdpBlaster(IClockSource* clock_);

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
  class CUdpBlaster : public IUdpBlaster {

  public:
    CUdpBlaster(uint16_t port_, IClockSource* clock_);
    ~CUdpBlaster();

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
  class CSimUdpBlaster : IUdpBlaster {

  };

  // ============================================================================================================
  struct CPlayer {

    /// <summary>
    /// The name of the player.
    /// </summary>
    std::string Name;

    /// <summary>
    /// Zero based index for each player.  0 == player 1, 1 == player 2 and so on.
    /// Negative numbers can be used for implementation specific reasons.
    /// </summary>
    int PlayerIndex = -1;

    bool IsLocal = false;

    // These are used for remote players.
    // NOTE: If/when we care about ipv6 support (and we should) this is where we will do the updates.
    sockaddr_in RemoteAddr;
  };

  // ============================================================================================================
  // HFPOClient is responsible for managing local and remote connections to other players and services.
  class CHFSOClient
  {
  private:
    uint8_t MaxPlayers;

  public:

    CHFSOClient(uint8_t maxPlayers_);
    ~CHFSOClient();

    void Init(IUdpBlaster* udp_);

    void AddPlayer(CPlayer& player);

  protected:
    IUdpBlaster* _Udp = nullptr;

    std::vector<CPlayer> _AllPlayers;

  private:


    bool IsInitialized = false;
    void CheckInit();

    // Stuff we need...
    // Local Player(s).
    // Remote Player(s).
    // Services?  ReplayClient, Spectator, etc.?

  };


  // ============================================================================================================
  class CSimHFSOClient : public CHFSOClient {

  public:
    CSimHFSOClient(uint8_t maxPlayers);

    inline uint8_t GetPlayerCount() { return _AllPlayers.size(); }
  };


  // Free functions...
  sockaddr_in CreateAddr(const char* ip, uint16_t port);
  void CreateLocalPlayer(const char* name, uint16_t playerIndex, CPlayer* __target);
  void CreateRemotePlayer(const char* name, uint16_t playerIndex, const char* ip, uint16_t port, CPlayer* __target);
}
