#pragma once

#include <string>
#include "Platform.h"
#include "CEventDriver.h"

#include "CMsg.h"

namespace funscrew::HFSO
{
  class CHFSOClient;
//  class CUdpBlaster;
  class IUdpBlaster;

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
  struct CNetworkStats {
    int      bytes_sent = 0;
    int      packets_sent = 0;

    /// <summary>
    /// The last time that a packet was sent...
    /// </summary>
    int     LastTimeSent = 0;
    int     LastReceiveTime = 0;

    int      BytesReceived = 0;
    int      PacketsReceived = 0;

    float    KbpsReceived = 0;
    float    kbps_sent = 0.0f;

    // NEW: how long has client been active for?
    int       Uptime = 0.0;

    int     NextSequence = 0;
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
    void Init(CPlayer& data_, CHFSOClient* parent_, IUdpBlaster* udp_);

    void OnMessage(const CMsg* msg, const sockaddr_in& from);

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

    bool OnInvalid(CMsg* msg, int msgSize);
    bool OnSyncRequest(CMsg* msg, int msgSize);
    bool OnSyncReply(CMsg* msg, int msgSize);
    bool OnInput(CMsg* msg, int msgSize);
    bool OnQualityReport(CMsg* msg, int msgSize);
    bool OnQualityReply(CMsg* msg, int msgSize);
    bool OnKeepAlive(CMsg* msg, int msgSize);

    //// These might be dropped.....
    //bool OnInputAck(CMsg* msg, int msgSize) ;         
    //bool OnData(CMsg* msg, int msgSize);					     


    CNetworkStats Stats;


  protected:
    CHFSOClient* _Parent = nullptr;
    IUdpBlaster* _UDP = nullptr;

    EEndpointState _State = EEndpointState::Available;

    // Don't write to it!
    CPlayer _Data;
    sockaddr_in _RemoteAddr;

    ConnectData _ConnectData = {};

    void SendSyncMessage();
    void SendMsg(CMsg* msg);

  };

}