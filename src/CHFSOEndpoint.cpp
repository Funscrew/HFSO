#include "CHFSOEndpoint.h"
//#include "CClient.h"

namespace funscrew::HFSO {

  // -------------------------------------------------------------------------------------------------------------
  void CHFSOEndpoint::Init(CPlayer& data_) {
    if (_State != EEndpointState::Available) {
      throw std::runtime_error("Can't initialize an endpoint that isn't available!");
    }

    _Data = data_;
    _State = EEndpointState::Disconnected;
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