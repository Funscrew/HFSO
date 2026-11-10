#include "CHFSOEndpoint.h"
#include "CClient.h"
#include "CMsg.h"


namespace funscrew::HFSO {

  typedef bool (CHFSOEndpoint::* DispatchFn)(CMsg* msg, int msgSize);

  // NOTE:  A table of function pointers is used so that the values from UdpMsg::MsgType
  // can be used to index into this.
  static const DispatchFn msgHandlers[] = {
     &CHFSOEndpoint::OnInvalid,             /* Invalid */
     &CHFSOEndpoint::OnSyncRequest,         /* SyncRequest */
     &CHFSOEndpoint::OnSyncReply,           /* SyncReply */
     &CHFSOEndpoint::OnInput,               /* Input */
     &CHFSOEndpoint::OnQualityReport,       /* QualityReport */
     &CHFSOEndpoint::OnQualityReply,        /* QualityReply */
     &CHFSOEndpoint::OnKeepAlive,           /* KeepAlive */
     //&CHFSOEndpoint::OnInputAck,            /* InputAck */
     //&CHFSOEndpoint::OnData					       /* Data Exchange - A chat message or other type of data was received */
  };

  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnInvalid(CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnSyncRequest(CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnSyncReply(CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnInput(CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnQualityReport(CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnQualityReply(CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnKeepAlive(CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }


  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::Init(CPlayer& data_, CHFSOClient* parent_, IUdpBlaster* udp_) {
    if (_State != EEndpointState::Available) {
      throw std::runtime_error("Can't initialize an endpoint that isn't available!");
    }
    _Parent = parent_;
    _UDP = _Parent->UDP();
    _Data = data_;
    _State = EEndpointState::Disconnected;

    _RemoteAddr = CreateAddr(_Data.Host, _Data.Port);
  }

  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::OnMessage(const CMsg* msg, const sockaddr_in& from) {

    // Local / open endpoints don't handle messages.
    if (_Data.IsLocal || _State == EEndpointState::Available) { return; }

    // Make sure that the message is for us.
    auto& _peer_addr = _RemoteAddr;
    bool isForMe = _peer_addr.sin_addr.S_un.S_addr == from.sin_addr.S_un.S_addr &&
      _peer_addr.sin_port == from.sin_port;

    if (!isForMe) { return; }

    // TODO: This is where we will handle sequence tracking + updates.

    printf("I will handle a message!");
  }

  // -------------------------------------------------------------------------------------------------------------
  // NOTE: OnLoopPoll
  void CHFSOEndpoint::Update() {
    // Local / open endpoints don't need to update.
    if (_Data.IsLocal || _State == EEndpointState::Available) { return; }

    switch (_State) {
    case EEndpointState::Connecting:
      // Continue to exchange sync packets:
      // Once we are synced we can proceed to metadata exchange...

      break;
      // case EEndpointState::MetadataExchange:

      // case EEndpointState::Connected:


      //case EEndpointState::Disconnected:
      //  // DO NOTHING:  We aren't connected!
      //  break;

    default:
      throw std::runtime_error("This state is not supported!");
      // case EEndpointState::Available:
    }
  }

  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::Complete() {
    if (_State != EEndpointState::Disconnected) {
      throw std::runtime_error("Can't complete an endpoint that isn't disconnected!");
    }

    _Data = {};
    _State = EEndpointState::Available;
  }

  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::BeginConnect() {
    if (_State != EEndpointState::Disconnected) {
      throw std::runtime_error("invalid state to begin connection!");
    }

    // TODO: How do we want to care about eventing and callbacks?
    // We can have an assigned single callback, or we can hook up a whole bunch....
    // Whatever it is, this is certainly one that would be passed up to the parent....
    // So maybe instead of callback we signal the parent directly?

    CStateChangedArgs args;
    args.OldState = _State;
    args.NewState = EEndpointState::Connecting;

    _State = EEndpointState::Connecting;

    OnStateChanged.Trigger(this, args);

    // Log
    // NOTE: I think that the usual update loop will take care of connecting to peers and all that jazz....
    _ConnectData.LocalMagic = _Parent->RandInt();

    // We probably need to send a packet here, and think about metadata exchange type stuff.
    SendSyncMessage();

    // throw std::exception("please complete this code path!");
  }


  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::SendSyncMessage() {
    CMsg msg(EMsgType::SyncRequest);



    auto& rq = msg.Content.SyncRequest;
    rq.SessionId = _Parent->SessionId();
    
    // TEMP:
    rq.LastAckSequence = 0;

    _Parent->GetSyncMetadata(this, rq.SyncData, rq.DataSize);

    SendMsg(&msg);
  }


  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::SendMsg(CMsg* msg) {

    // TEMP:  Fill in real logic later!
    msg->Header.Magic = 0;
    msg->Header.Sequence = 0;

    // TODO: Relocate network stats to this class!
    Stats.packets_sent++;
    Stats.LastTimeSent = _UDP->Clock()->GetTime(); // _Clock->GetTime();

    auto size = msg->PacketSize();
    Stats.bytes_sent += size;

    _UDP->Send((byte*)(msg), size, (struct sockaddr*)&_RemoteAddr);
    //_NetStats.PacketsSent++;
    //_NetStats.LastSendTime = _Parent.Get



  }

  //_packets_sent++;
  //_last_send_time = Platform::GetCurrentTimeMS();
  //_bytes_sent += msg->PacketSize();

  //msg->header.magic = _magic_number;
  //msg->header.sequence_number = _next_send_seq++;

  //_send_queue.push(QueueEntry(Platform::GetCurrentTimeMS(), _peer_addr, msg));

  //Utils::LogMsg(EMsgDirection::Send, msg);
  //PumpSendQueue();


}