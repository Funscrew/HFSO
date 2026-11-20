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
  enum class EEndpointState
  {
    /// <summary>
    /// This endpoint is ready to be connected (local, remote, etc.)
    /// </summary>
    Available = 0,
    Connecting,
    Connected,
    Disconnecting,
    Disconnected
  };


  // ============================================================================================================
  struct CEndpointStateChangedArgs {
    EEndpointState OldState;
    EEndpointState NewState;
  };


  // ============================================================================================================
  /// <summary>
  /// Keeps track of the current connection attempt / sync.
  /// REFACTOR: This is becoming more of a way to keep track of message / state times.  Rename it and update the
  /// comments to reflect this.
  /// </summary>
  struct CConnectData {
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
    int AckedSyncNumber = -1;

    /// <summary>
    /// The time that any message was last sent.
    /// NOTE: This is not the same as when it was put on the wire, rather last time that SendMsg was called.
    /// </summary>
    int LastMsg = 0;

    int ConnectStart = 0;
    int LastConnectMsg = 0;

    int DisconnectStart = 0;
    int LastDisconnectMsg = 0;
  };

  // ============================================================================================================
  enum class EDisconnectReason : uint8_t { 
    Invalid = 0,
    /// <summary>
    /// This is a normal disconnect.  We will wait around for an ACK response
    /// from the remote.
    /// </summary>
    Normal,
    
    /// <summary>
    /// Connection attempt timed out.  Disconnect immediately.
    /// </summary>
    ConnectTimeout,

    /// <summary>
    /// Did not receive disconnect ACK in a timely fashion.  Disconnect immediately.
    /// </summary>
    DisconnectTimeout,

    /// <summary>
    /// Remote stopped responding.  Disconnect immediately.
    /// </summary>
    IdleTimeout
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
    CEvent<void*, CEndpointStateChangedArgs> OnStateChanged;

    // Properties =
    inline EEndpointState State() const { return _State; }
    inline const CPlayer* PlayerData() const { return &_PlayerData; }

    inline CConnectData& ConnectData() { return _ConnectData; }
    inline bool IsLocalPlayer() const { return _PlayerData.IsLocal; }
    inline bool IsRemotePlayer() const { return !IsLocalPlayer(); }

    // Functions ===
    void Init(CPlayer& data_, CHFSOClient* parent_, IUdpBlaster* udp_);

    virtual void OnMessage(const CMsg* msg, size_t msgSize, const sockaddr_in& from);

    void Update();

    /// <summary>
    /// Clear player data + mark as available.
    /// </summary>
    void Complete();

    /// <summary>
    /// Attempt to connect to the other players.
    /// </summary>
    void BeginConnect();

    /// <summary>
    /// This is called when the connection is completed.
    /// </summary>
    void Connected();

    /// <summary>
    /// Disconnect us from the remote.
    /// </summary>
    void Disconnect(EDisconnectReason reason);



    // Message Handlers..
    bool OnInvalid(const CMsg* msg, int msgSize);
    bool OnSyncRequest(const CMsg* msg, int msgSize);
    bool OnSyncReply(const CMsg* msg, int msgSize);
    bool OnDisconnect(const CMsg* msg, int msgSize);
    bool OnDisconnectAck(const CMsg* msg, int msgSize);

    bool OnInput(const CMsg* msg, int msgSize);
    bool OnQualityReport(const CMsg* msg, int msgSize);
    bool OnQualityReply(const CMsg* msg, int msgSize);
    bool OnKeepAlive(const CMsg* msg, int msgSize);

    //// These might be dropped.....
    //bool OnInputAck(CMsg* msg, int msgSize) ;         
    //bool OnData(CMsg* msg, int msgSize);					     



  protected:
    CHFSOClient* _Parent = nullptr;
    IUdpBlaster* _UDP = nullptr;

    EEndpointState _State = EEndpointState::Available;

    // Don't write to it!
    CPlayer _PlayerData;
    sockaddr_in _RemoteAddr;

    CConnectData _ConnectData = {};

    void SendSyncRequest();
    void SendDisconnectMessage();
    void SendDisconnectAck();

    void SendMsg(CMsg* msg);
    void SetState(EEndpointState newState);

    uint16_t _RecvSequence = 0;
    uint16_t _SendSequence = 0;

    EZQ<QueueEntry, 64> SendQueue;

    int Now() const;

  private:
    void PumpSendQueue();
    void ClearSendQueue();
  };




}