#pragma once

#include <stdint.h>
#include <vector>

#include "IClockSource.hpp"
#include "CHFSOEndpoint.h"


constexpr int MAX_PACKET_SIZE = 4000;

// The very upper limit of players that will be supported!
constexpr int MAX_MAX_PLAYERS = 2;

typedef uint16_t milliseconds;

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
    int Receive(byte* buffer,  sockaddr_in* from);
    // virtual ~CUdpBlaster();

  protected:

    IClockSource* _Clock = nullptr;

    // These need to be the 'internal' versions of send + receive.
    virtual int ReceiveImpl(byte* receiveBuffer, sockaddr_in* from) = 0;
    virtual int SendImpl(byte* sendBuffer, size_t size, struct sockaddr* to) = 0;
  };

  // ============================================================================================================
  class CUdpBlaster : public IUdpBlaster {

  public:
    CUdpBlaster(uint16_t port_, IClockSource* clock_);
    ~CUdpBlaster();

  protected:
    virtual int ReceiveImpl(byte* receiveBuffer, sockaddr_in* from) override;
    virtual int SendImpl(byte* sendBuffer, size_t size, struct sockaddr* to) override;

  private:
    uint16_t _Port = 0;
    Socket _Socket = 0;
  };


  // ============================================================================================================
  enum class EClientState {
    /// <summary>
    /// Client is being being setup.
    /// </summary>
    Init = 0,
    Starting,
    Active,
    Stopping,

    /// <summary>
    /// The client has completed all operations.
    /// </summary>
    Complete
  };

  // ============================================================================================================
  struct CClientOptions {
    uint8_t MaxPlayers;

    /// <summary>
    /// How long do we wait before we time out the connection?
    /// This applies to all connections, players, appliances, etc.
    /// </summary>
    milliseconds ConnectTimeout = 5000;

    /// <summary>
    /// After connection, how long do we wait before disconnecting an idle connection?
    /// This applies to all connections, players, appliances, etc.
    /// </summary>
    milliseconds IdleTimeout = 2000;
  };

  // ============================================================================================================
  // HFPOClient is responsible for managing local and remote connections to other players and services.
  class CHFSOClient
  {
  public:

    CHFSOClient(CClientOptions& options_, IUdpBlaster* udp_);
    ~CHFSOClient();

    void AddPlayer(CPlayer& player);

    /// <summary>
    /// Begin activity on the client.  Connections will be started, etc.
    /// </summary>
    void Begin();

    // virtual void SyncInputs();

    /// <summary>
    /// Send / receive / do maintenance.
    /// </summary>
    virtual void Idle();

    /// <summary>
    /// End client activity.  This will disconnect all other clients, appliances, etc.
    /// </summary>
    void End();


  protected:
    byte ReadBuffer[MAX_PACKET_SIZE];

    bool _InRollback = false;

    CClientOptions _Options;

    IUdpBlaster* _Udp = nullptr;

    EClientState _State = EClientState::Init;

    uint8_t PlayerCount = 0;
    CHFSOEndpoint __AllPlayers[MAX_MAX_PLAYERS];


    // Event Handler Functions.
    void OnEndpointStateChange(void* sender, CStateChangedArgs& args);

  private:
    

    // uint8_t MaxPlayers = 0;

    // bool IsInitialized = false;
    // void CheckInit();

    // Stuff we need...
    // Local Player(s).
    // Remote Player(s).
    // Services?  ReplayClient, Spectator, etc.?

  };



  // ============================================================================================================
  class CSimHFSOClient : public CHFSOClient {

  public:
    CSimHFSOClient(CClientOptions& options_, IUdpBlaster* udp_);

    inline uint8_t GetPlayerCount() { return PlayerCount; }
  };


  // Free functions...
  sockaddr_in CreateAddr(const char* ip, uint16_t port);
  void CreateLocalPlayer(const char* name, uint16_t playerIndex, CPlayer* __target);
  void CreateRemotePlayer(const char* name, uint16_t playerIndex, const char* ip, uint16_t port, CPlayer* __target);
}
