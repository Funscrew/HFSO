#include "CHFSOEndpoint.h"
#include "CClient.h"
#include "CMsg.h"


#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(a)    (sizeof(a) / sizeof((a)[0]))
#endif

namespace funscrew::HFSO {

  typedef bool (CHFSOEndpoint::* DispatchFn)(const CMsg* msg, int msgSize);

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
  bool CHFSOEndpoint::OnInvalid(const CMsg* msg, int msgSize)
  {
    throw std::runtime_error("Invalid message received!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnSyncRequest(const CMsg* msg, int msgSize)
  {

    // Only respond up to a certain point...
    auto& rq = msg->Content.SyncRequest;
    if (rq.SyncNumber == _ConnectData.LastSyncNumber + 1)
    {
      // Acceptable, send the reply!
      CMsg* reply = new CMsg(EMsgType::SyncReply);
      auto& rs = reply->Content.SyncReply;



      // Now we can decide if we are done with the sync operation....
      // This happens when there is no more metadata to exchange.
      printf("handle metadata exchange!");
      const bool METADATA_OK = true;

      rs.Ready = 1;

      SendMsg(reply);
      _ConnectData.LastSyncNumber = rq.SyncNumber;

      if (rq.DataSize == 0 && METADATA_OK) {
        printf("connection is ready!");
        throw std::exception("finish me!");
      }
      else {
        // printf("send another sync request here!");
        SendSyncRequest();
      }

    }

    // TODO: LOG:
    // Log the fact that we didn't receive a sync request with the expected number!

  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnSyncReply(const CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnInput(const CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnQualityReport(const CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnQualityReply(const CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnKeepAlive(const CMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }


  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::Init(CPlayer& data_, CHFSOClient* parent_, IUdpBlaster* udp_) {
    if (_State != EEndpointState::Initializing) {
      throw std::runtime_error("Can't initialize an endpoint that isn't available!");
    }
    _Parent = parent_;
    _UDP = _Parent->UDP();
    _PlayerData = data_;
    _State = EEndpointState::Disconnected;

    _RemoteAddr = CreateAddr(_PlayerData.Host, _PlayerData.Port);

    //if (_Data.IsLocal
    //auto a = _UDP->Addr();
    //if (AreSame(a, _RemoteAddr)) { 
    //  int x=  10;
    //}
    // if (AreSame(_UDP._Addr
  }

  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::OnMessage(const CMsg* msg, size_t msgSize, const sockaddr_in& from) {

    // Local / open endpoints don't handle messages.
    if (_PlayerData.IsLocal || _State == EEndpointState::Initializing) { return; }

    // Make sure that the message is for us.
    auto& _peer_addr = _RemoteAddr;
    bool isForMe = _peer_addr.sin_addr.S_un.S_addr == from.sin_addr.S_un.S_addr &&
      _peer_addr.sin_port == from.sin_port;

    if (!isForMe) { return; }

    // Make sure we are coming from the right place.
    // See notes on 'RemoteMagic' for more information.
    if (!msg->IsSyncMessage()) {
      if (msg->Header.Magic != _ConnectData.RemoteMagic)
      {
        // TODO: LOG:
        // bad magic!
        return;
      }
    }

    // Filter OOP.  Any packet that is received out of order is simply dropped.
    // Other endpoint will have to re-send. TEST:
    auto seq = msg->Header.Sequence;
    uint16_t skipped = seq - _RecvSequence;
    if (skipped > 0x8000)
    {
      // TODO: LOG:  This is def. something that we would want to keep stats on!
      return;
    }
    _RecvSequence = seq;

    uint8_t msgType = (uint8_t)msg->Header.Type;
    if (msgType > ARRAY_SIZE(msgHandlers))
    {
      OnInvalid(msg, msgSize);
    }
    else {
      bool handled = (this->*(msgHandlers[msgType]))(msg, msgSize);
      if (handled) {
        Stats.LastReceiveTime = _UDP->Clock()->GetTime();
        Stats.BytesReceived += msgSize;
      }
    }

    // printf("I will handle a message!");
  }

  // -------------------------------------------------------------------------------------------------------------
  // NOTE: OnLoopPoll
  void CHFSOEndpoint::Update() {
    // Local / open endpoints don't need to update.
    if (_PlayerData.IsLocal || _State == EEndpointState::Initializing) { return; }

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

    _PlayerData = {};
    _State = EEndpointState::Initializing;
    _ConnectData = {};

    ClearSendQueue();
  }

  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::BeginConnect() {

    if (_State != EEndpointState::Disconnected) {
      throw std::runtime_error("Invalid state to begin connection!");
    }

    // Local players are already connected!
    if (_PlayerData.IsLocal) {
      _State = EEndpointState::Connected;
      return;
    }

    _ConnectData = {};
    _ConnectData.LastSyncNumber = -1;

    // TODO: How do we want to care about eventing and callbacks?
    // We can have an assigned single callback, or we can hook up a whole bunch....
    // Whatever it is, this is certainly one that would be passed up to the parent....
    // So maybe instead of callback we signal the parent directly?

    CStateChangedArgs args;
    args.OldState = _State;
    args.NewState = EEndpointState::Connecting;

    // TODO: Run this through SetState() so we have a way to always trigger the thunk!
    _State = EEndpointState::Connecting;
    OnStateChanged.Trigger(this, args);

    // Log
    // NOTE: I think that the usual update loop will take care of connecting to peers and all that jazz....
    _ConnectData.LocalMagic = _Parent->RandInt();

    // We probably need to send a packet here, and think about metadata exchange type stuff.
    SendSyncRequest();

    // throw std::exception("please complete this code path!");
  }


  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::SendSyncRequest() {
    CMsg* msg = new CMsg(EMsgType::SyncRequest);

    auto& rq = msg->Content.SyncRequest;
    rq.SessionId = _Parent->SessionId();
    rq.SyncNumber = 0;

    // NOTE: Custom / multipart data exchange needs to be thought about / examined.....
    // This has a lot to do with sequence numbers too.
    // Probably make some test cases for this up front!
    // Sync number will def need to be a part of it....
    // Maybe the parent isn't really resposible for that either....?
    _Parent->GetSyncMetadata(this, rq.SyncData, rq.DataSize);

    SendMsg(msg);
  }


  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::SendMsg(CMsg* msg) {

    // TEMP:  Fill in real logic later!
    // TODO: We will use a proper session ID!
    msg->Header.Magic = 0;
    msg->Header.Sequence = _SendSequence;
    _SendSequence++;

    QueueEntry e;
    e.Msg = msg;
    e.SendTime = 0;

    SendQueue.Push(e);

    PumpSendQueue();
  }


  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::PumpSendQueue() {

    while (!SendQueue.Empty()) {

      auto& entry = SendQueue.First();
      auto now = _UDP->Clock()->GetTime();

      // TODO: Reimplement!
      // NOTE: If latency is set, then messages may not be sent this time around.
      // Like that of '_oop_percent' (below) this is probably useful for testing scenarios
      // to simulate jitter and out of order packets.
      //if (_send_latency) {
      //  // should really come up with a gaussian distribution based on the configured
      //  // value, but this will do for now.
      //  int jitter = (_send_latency * 2 / 3) + ((rand() % _send_latency) / 3);
      //  if (Platform::GetCurrentTimeMS() < _send_queue.front().queue_time + jitter) {
      //    break;
      //  }
      //}

      // TODO: Reimplement!
      // NOTE: I think there might be a cleaner way to do this.... probably just assign send_time to some nonzero value....
      //if (_oop_percent && !_oo_packet.msg && ((rand() % 100) < _oop_percent)) {
      //  int delay = rand() % (_send_latency * 10 + 1000);
      //  Utils::LogIt(CATEGORY_TEST, "creating rogue oop (seq: %d  delay: %d)", entry.msg->header.sequence_number, delay);
      //  _oo_packet.send_time = Platform::GetCurrentTimeMS() + delay;
      //  _oo_packet.msg = entry.msg;
      //  _oo_packet.dest_addr = entry.dest_addr;
      //}

          // TODO: Check out other stats that we might like to track.
      CMsg* msg = entry.Msg;

      auto size = msg->PacketSize();

      Stats.packets_sent++;
      Stats.LastTimeSent = _UDP->Clock()->GetTime();
      Stats.bytes_sent += size;

      _UDP->Send((byte*)(msg), size, (struct sockaddr*)&_RemoteAddr);

      delete entry.Msg;

      SendQueue.Pop();
    }


  }

  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::ClearSendQueue() {
    SendQueue.Clear();
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