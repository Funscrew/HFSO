#pragma once

#ifndef CCLIENT_H
#define CCLIENT_H

#include <stdint.h>
#include <vector>
#include <random>

#include "IClockSource.hpp"
#include "CHFSOEndpoint.h"
#include "CUdpBlaster.h"


// NOTE: This should be HUGE.  For example, ideal CPS3 input size for two players will be four bytes.
constexpr size_t MAX_INPUT_DATA_SIZE = 0xFF;

// The very upper limit of players that will be supported!
constexpr int MAX_MAX_PLAYERS = 2;

typedef uint16_t milliseconds;

namespace funscrew::HFSO
{


  // ============================================================================================================
  enum class EClientState {
    /// <summary>
    /// Client is being being setup.
    /// </summary>
    Init = 0,

    /// <summary>
    ///  Starting?  Not really sure the difference between start and init?  Init is probably just getting the networking stuff initialized?
    /// </summary>
    Starting,

    /// <summary>
    /// The client is connected to its peer.
    /// </summary>
    Active,

    /// <summary>
    /// The client is stopping / shutting down / disconnecting.
    /// </summary>
    Stopping,

    /// <summary>
    /// The client has completed all operations.  Safe to deconstruct.
    /// </summary>
    Complete
  };

  // ============================================================================================================
  struct CClientOptions {
    /// <summary>
    /// The max number of players in the game.
    /// </summary>
    uint8_t MaxPlayers;

    /// <summary>
    /// Per-Player input data size.
    /// </summary>
    uint8_t InputDataSize = 0xFF;


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

    uint64_t SessionId = 0;
  };

  // ============================================================================================================
  // HFPOClient is responsible for managing local and remote connections to other players and services.
  class CHFSOClient
  {
  public:

    CHFSOClient(CClientOptions& options_, IUdpBlaster* udp_, int rngSeed = -1);
    ~CHFSOClient();

    bool IsSynchronizing() const { return _IsSynchronizing; }
    inline EClientState State()  const { return _State; }
    inline uint64_t SessionId() const { return _Options.SessionId; }
    inline IUdpBlaster* UDP() const { return _Udp; }

    byte* GetInputBuffer(size_t& maxInputDataSize);


    void AddPlayer(CPlayer& player);

    /// <summary>
    /// Begin activity on the client.  Connections will be started, etc.
    /// </summary>
    void Start();

    /// <summary>
    /// Send / receive / do maintenance.
    /// </summary>
    virtual void Idle();

    /// <summary>
    /// Sync the inputs from the local + remote players.
    /// </summary>
    /// <returns></returns>
    virtual bool SyncInput();
    virtual void const IncrementFrame();

    /// <summary>
    /// End client activity.  This will disconnect all other clients, appliances, etc.
    /// </summary>
    void End();

    /// <summary>
    /// Get a random number.  Used by endpoints for certain operations.
    /// </summary>
    uint32_t RandInt();

    /// <summary>
    /// Get the data that is needed for sync.  This is application defined.
    /// By default, player names are exchanged.
    /// </summary>
    virtual void GetSyncMetadata(CHFSOEndpoint* forEndpoint, uint8_t* data, uint16_t& dataSize);

  protected:
    byte ReadBuffer[MAX_PACKET_SIZE];
    byte InputBuffer[MAX_INPUT_DATA_SIZE];
    size_t InputDataSize;

    bool _InRollback = false;
    bool _IsSynchronizing = false;

    CClientOptions _Options;

    IUdpBlaster* _Udp = nullptr;

    EClientState _State = EClientState::Init;

    uint8_t PlayerCount = 0;
    CHFSOEndpoint __AllPlayers[MAX_MAX_PLAYERS];

    std::mt19937 _RNG;

    // uint64_t _SessionId = 0;

    // Event Handler Functions.
    void OnEndpointStateChange(void* sender, CStateChangedArgs& args);
    void SetState(EClientState newState);

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
    CSimHFSOClient(CClientOptions& options_, IUdpBlaster* udp_, int rngSeed);

    inline uint8_t GetPlayerCount() { return PlayerCount; }
  };


  // Free functions...
  void CreateLocalPlayer(const char* name, uint16_t playerIndex, CPlayer* __target);
  void CreateRemotePlayer(const char* name, uint16_t playerIndex, const char* ip, uint16_t port, CPlayer* __target);
}


#endif // !CCLIENT_H
