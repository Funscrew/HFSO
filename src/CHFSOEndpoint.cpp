#include "CHFSOEndpoint.h"
//#include "CClient.h"




namespace funscrew::HFSO {

  typedef bool (CHFSOEndpoint::* DispatchFn)(CUdpMsg* msg, int msgSize);

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
  bool CHFSOEndpoint::OnInvalid(CUdpMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnSyncRequest(CUdpMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnSyncReply(CUdpMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnInput(CUdpMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnQualityReport(CUdpMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnQualityReply(CUdpMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }
  // -------------------------------------------------------------------------------------------------------------
  bool CHFSOEndpoint::OnKeepAlive(CUdpMsg* msg, int msgSize)
  {
    throw std::runtime_error("not supported!");
  }


  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::Init(CPlayer& data_) {
    if (_State != EEndpointState::Available) {
      throw std::runtime_error("Can't initialize an endpoint that isn't available!");
    }

    _Data = data_;
    _State = EEndpointState::Disconnected;
  }

  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::OnMessage(const CUdpMsg* msg, const sockaddr_in& from) {

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
  void CHFSOEndpoint::Update() {
    // Local / open endpoints don't need to update.
    if (_Data.IsLocal || _State == EEndpointState::Available) { return; }

    switch (_State) {
    case EEndpointState::Connecting:
      // Continue to exchange sync packets:
      // Once we are synced we can proceed to metadata exchange...

    case EEndpointState::MetadataExchange:

    case EEndpointState::Connected:


    case EEndpointState::Disconnected:
      // DO NOTHING:  We aren't connected!
      break;

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
  void  CHFSOEndpoint::BeginConnect() {
    if (_State != EEndpointState::Disconnected) {
      throw std::runtime_error("invalid state to begin connection!");
    }

    // TODO: How do we want to care about eventing and callbacks?
    // We can have an assigned single callback, or we can hook up a whole bunch....
    // Whatever it is, this is certainly one that would be passed up to the parent....
    // So maybe instead of callback we signal the parent directly?

  //  throw std::runtime_error("Figure it out!");

    CStateChangedArgs args;
    args.OldState = _State;
    args.NewState = EEndpointState::Connecting;

    _State = EEndpointState::Connecting;

    OnStateChanged.Trigger(this, args);

    // Log
    // NOTE: I think that the usual update loop will take care of connecting to peers and all that jazz....
  }


}