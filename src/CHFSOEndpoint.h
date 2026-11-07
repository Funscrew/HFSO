#pragma once

#include <string>
#include "Platform.h"
#include "CEventDriver.h"

#include "CUdpMsg.h"

namespace funscrew::HFSO
{
  class CHFSOClient;

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

    std::string Host;
    uint16_t Port;

    // These are used for remote players.
    // NOTE: If/when we care about ipv6 support (and we should) this is where we will do the updates.
    // struct sockaddr_in RemoteAddr;

    //CHFSOEndpoint* Endpoint = nullptr;
  };


  // ============================================================================================================
  enum class EEndpointState
  {
    Available = 0,
    Connecting,

    /// <summary>
    /// Optional state for exchanging game or other specific data between clients.
    /// </summary>
    MetadataExchange,
    Connected,
    Disconnecting,
    Disconnected
  };


  // ============================================================================================================
  struct CStateChangedArgs {
    EEndpointState OldState;
    EEndpointState NewState;
  };


  // ============================================================================================================
  /// <summary>
  /// Keeps track of the current connection attempt / sync.
  /// </summary>
  struct ConnectData {
    /// <summary>
    /// Magic number for this client.  Remote client must ACK with this number to prove exchange.
    /// </summary>
    uint32_t LocalMagic = 0;

    /// <summary>
    /// Magic number for the remote client.
    /// </summary>
    uint32_t RemoteMagic = 0;

    /// <summary>
    /// Last acked sequence number for connections.
    /// </summary>
    uint32_t LastAck = 0;

    /// <summary>
    /// The next time that we will send a connection packet.
    /// </summary>
    uint32_t NextSendTime = 0;
  };

  // ============================================================================================================
  /// <summary>
  /// Represents and endpoint to be used with CHFSO client.  This can be a player, a remote service (ReplayAppliance / LiveSpectate), or anything else that
  /// you may want to plug-in.
  /// </summary>
  /// <remarks>
  /// There is no real plugin support, or plans for pluggable endpoints at this time, despite what the description says.
  /// NOTE: Still in development, this may end up being exclusive to player endpoints...
  /// </remarks>
  class CHFSOEndpoint {

  public:
        // Events ====
    CEventDriver<CStateChangedArgs> OnStateChanged;

    // Properties =
    inline EEndpointState State() { return _State; }
    inline CPlayer* Data() { return &_Data; }


    // Functions ===
    void Init(CPlayer& data_, CHFSOClient* parent_);

    void OnMessage(const CUdpMsg* msg, const sockaddr_in& from);

    void Update();

    /// <summary>
    /// Clear player data + mark as available.
    /// </summary>
    void Complete();

    /// <summary>
    /// Attempt to connect to the other players.
    /// </summary>
    void BeginConnect();



    // Message Handlers..

    bool OnInvalid(CUdpMsg* msg, int msgSize);
    bool OnSyncRequest(CUdpMsg* msg, int msgSize);
    bool OnSyncReply(CUdpMsg* msg, int msgSize);
    bool OnInput(CUdpMsg* msg, int msgSize);
    bool OnQualityReport(CUdpMsg* msg, int msgSize);
    bool OnQualityReply(CUdpMsg* msg, int msgSize);
    bool OnKeepAlive(CUdpMsg* msg, int msgSize);

    //// These might be dropped.....
    //bool OnInputAck(CUdpMsg* msg, int msgSize) ;         
    //bool OnData(CUdpMsg* msg, int msgSize);					     



  protected:
    CHFSOClient* _Parent = nullptr;

    EEndpointState _State = EEndpointState::Available;

    // Don't write to it!
    CPlayer _Data;
    sockaddr_in _RemoteAddr;

    ConnectData _ConnectData = {};

  };

}