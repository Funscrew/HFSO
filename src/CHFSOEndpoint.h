#pragma once

#include <string>
#include "Platform.h"
#include "CEventDriver.h"

#include "CMsg.h"
#include "EZQ.hpp"


namespace funscrew::HFSO
{
  class CHFSOClient;
  class IUdpBlaster;

  // ============================================================================================================
  struct QueueEntry {

    /// <summary>
    /// When should the message be sent?  This is really only used when simulating latency / out of order packets.
    /// </summary>
    int SendTime;
    CMsg* Msg;

    QueueEntry() {}

    QueueEntry(int time,  CMsg* msg) 
      : SendTime(time)
      , Msg(msg) 
      { }
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
    Initializing = 0,
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
    uint16_t LocalMagic = 0;

    /// <summary>
    /// Magic number for the remote client.
    /// NOTE: We don't assign this until sync is complete.  It is meant for security, but is a pretty weak device.
    /// Better way would be to have the clients already know what the magic numbers should be (via HTTPS or other encrypted channel)
    /// This is similar to how we make sure that the player names are known BEFORE connecting so we don't get bad data (some player advertised as 'dave' connecting as 'fuckyu' or something worse!)
    /// </summary>
    uint16_t RemoteMagic = 0;

    /// <summary>
    /// Last acked sequence number for connections.
    /// </summary>
    int LastSyncNumber = -1;

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
    inline CPlayer* Data() { return &_PlayerData; }


    // Functions ===
    void Init(CPlayer& data_, CHFSOClient* parent_, IUdpBlaster* udp_);

    void OnMessage(const CMsg* msg, size_t msgSize, const sockaddr_in& from);

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

    bool OnInvalid(const CMsg* msg, int msgSize);
    bool OnSyncRequest(const CMsg* msg, int msgSize);
    bool OnSyncReply(const CMsg* msg, int msgSize);
    bool OnInput(const CMsg* msg, int msgSize);
    bool OnQualityReport(const CMsg* msg, int msgSize);
    bool OnQualityReply(const CMsg* msg, int msgSize);
    bool OnKeepAlive(const CMsg* msg, int msgSize);

    //// These might be dropped.....
    //bool OnInputAck(CMsg* msg, int msgSize) ;         
    //bool OnData(CMsg* msg, int msgSize);					     


    CNetworkStats Stats;

    EZQ<QueueEntry, 64> SendQueue;

  protected:
    CHFSOClient* _Parent = nullptr;
    IUdpBlaster* _UDP = nullptr;

    EEndpointState _State = EEndpointState::Initializing;

    // Don't write to it!
    CPlayer _PlayerData;
    sockaddr_in _RemoteAddr;

    ConnectData _ConnectData = {};

    void SendSyncRequest();
    void SendMsg(CMsg* msg);


    uint16_t _RecvSequence = 0;
    uint16_t _SendSequence = 0;

  private:
    void PumpSendQueue();
    void ClearSendQueue();
  };




}