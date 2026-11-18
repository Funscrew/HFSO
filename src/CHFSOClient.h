#pragma once

#ifndef CCLIENT_H
#define CCLIENT_H

#include <stdint.h>
#include <vector>
#include <random>

#include "IClockSource.hpp"
#include "CHFSOEndpoint.h"
#include "CUdpBlaster.h"
#include "CThunk.h"

// NOTE: This should be HUGE.  For example, ideal CPS3 input size for two players will be four bytes.
constexpr size_t MAX_INPUT_DATA_SIZE = 0xFF;

// The very upper limit of players that will be supported!
constexpr int MAX_MAX_PLAYERS = 2;

typedef uint16_t milliseconds;


//using thunk = CThunk<CHFSOClient, void, void*, CStateChangedArgs&>;
//auto fn = thunk::Bind(this, &CHFSOClient::OnEndpointStateChange);
//endpoint.OnStateChanged.Add(fn);

namespace funscrew::HFSO
{
  constexpr int DEFAULT_CONNECT_TIMEOUT = 5000;
  constexpr int DEFAULT_IDLE_TIMEOUT = 5000;
  constexpr int DEFAULT_QUALITY_REPORT_INTERVAL = 1000;
  constexpr int DEFAULT_KEEPALIVE_INTERVAL = 500;



  // ============================================================================================================
  enum class EEventType {
    Invalid = 0,
    ClientConnected,
    ClientDisconnected,
  };

  // ============================================================================================================
  // Used in internal event handling.
  struct CEndpointEvent {
    EEventType Type;

    union {
      struct {
        CHFSOEndpoint* Endpoint;
      } Connected;
      struct {
        CHFSOEndpoint* Endpoint;
      } Disconnected;
    } Content;

    CEndpointEvent(EEventType type_) { Type = type_; }
  };

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
  struct CClientStateChangedArgs {
    EClientState OldState;
    EClientState NewState;
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
    milliseconds ConnectTimeout = DEFAULT_CONNECT_TIMEOUT;

    /// <summary>
    /// After connection, how long do we wait before disconnecting an idle connection?
    /// Idle connections happen when we don't receive data for a certain amount of time.
    /// </summary>
    milliseconds IdleTimeout = DEFAULT_IDLE_TIMEOUT;

    /// <summary>
    /// How often do we resend packets if we haven't heard anything in a while.
    /// NOTE: This is specific to sending game inputs.  We resend the ones that we have queued over and over again....
    /// We will probably end up relocating this to somewhere else....
    /// </summary>
    milliseconds ResendInterval = 200;


    milliseconds QualityReportInterval = DEFAULT_QUALITY_REPORT_INTERVAL;
    milliseconds KeepaliveInterval = DEFAULT_KEEPALIVE_INTERVAL;

    uint64_t SessionId = 0;
  };

  // ============================================================================================================
  // HFPOClient is responsible for managing local and remote connections to other players and services.
  class CHFSOClient
  {
    friend class CHFSOEndpoint;

  public:

    CHFSOClient(CClientOptions& options_, IUdpBlaster* udp_, int rngSeed = -1);
    ~CHFSOClient();

    // Events ======
    // CEventDriver<CClientStateChangedArgs> OnStateChanged;

    CEvent<void*, CClientStateChangedArgs> OnStateChanged; 

    bool IsSynchronizing() const { return _IsSynchronizing; }
    inline EClientState State()  const { return _State; }
    inline uint64_t SessionId() const { return _Options.SessionId; }
    inline IUdpBlaster* UDP() const { return _Udp; }

    inline int PlayerCount() const { return _PlayerCount; }
    const CHFSOEndpoint* GetPlayer(uint8_t playerIndex) const;

    uint8_t* GetInputBuffer(size_t& maxInputDataSize);

    /// <summary>
    /// Returns the local player endpoint.
    /// Throws an exception if it doesn't yet exist.
    /// </summary>
    CHFSOEndpoint& GetLocalEndpoint();

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
    /// Format for the data is:
    /// Datacount (1B) - [code (1B) - datasize (2B) - data (datasize:B)]...
    /// Multiple data entries can be included in the metadata.
    /// </summary>
    /// <returns>
    /// Integer value representing the sync number.
    /// </returns>
    virtual int GetSyncMetadata(CHFSOEndpoint* forEndpoint, uint8_t* data, uint16_t& dataSize, uint16_t maxDataSize);

    /// <summary>
    /// Handle sync metadata that was sent from another client.
    /// By default, we will confirm that the player name is what we expect.
    /// </summary>
    virtual bool HandleSyncMetadata(CHFSOEndpoint* forEndpoint, const CMsg* syncMsg);



  protected:
    uint8_t ReadBuffer[MAX_PACKET_SIZE];
    uint8_t InputBuffer[MAX_INPUT_DATA_SIZE];
    size_t InputDataSize;

    bool _InRollback = false;
    bool _IsSynchronizing = false;

    CClientOptions _Options;

    IUdpBlaster* _Udp = nullptr;

    EClientState _State = EClientState::Init;

    uint8_t _PlayerCount = 0;
    CHFSOEndpoint __AllPlayers[MAX_MAX_PLAYERS];

    std::mt19937 _RNG;

    // uint64_t _SessionId = 0;

    // Event Handler Functions.
    void OnEndpointStateChange(void* sender, CEndpointStateChangedArgs& args);
    void SetState(EClientState newState);

    void HandleEvents();
    // Can this be at friend scope?
    void AddEndpointEvent(CEndpointEvent* event);

  private:

    EZQ<CEndpointEvent*, 32> __EndpointEvents;

  };



  // ============================================================================================================
  class CSimHFSOClient : public CHFSOClient {

  public:
    CSimHFSOClient(CClientOptions& options_, IUdpBlaster* udp_, int rngSeed);

    inline uint8_t GetPlayerCount() { return _PlayerCount; }
  };


  // Free functions...
  void CreateLocalPlayer(const char* name, uint16_t playerIndex, CPlayer* __target);
  void CreateRemotePlayer(const char* name, uint16_t playerIndex, const char* ip, uint16_t port, CPlayer* __target);
}


#endif // !CCLIENT_H
