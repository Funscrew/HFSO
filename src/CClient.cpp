#include "CClient.h"

#include <stdexcept>
#include "errno.h"
#include <cassert>

#include "CThunk.h"
#include "CUdpBlaster.h"

using namespace funscrew::HFSO;

const int MIN_PLAYERS = 2;


// ------------------------------------------------------------------------------------------------
CHFSOClient::CHFSOClient(CClientOptions& options_, IUdpBlaster* udp_, int rngSeed)
  : _Options(options_)
  , _Udp(udp_)
{
  // Validate options.
  if (_Options.MaxPlayers < MIN_PLAYERS) {
    throw new std::runtime_error("You must have a minimum of two players!");
  }
  if (_Options.MaxPlayers > MAX_MAX_PLAYERS) {
    // TODO: Formatted string...
    throw new std::runtime_error("This client can only support a max of MAX_MAX_PLAYERS (2) players!");
  }

  memset(InputBuffer, 0, MAX_INPUT_DATA_SIZE);
  InputDataSize = _Options.MaxPlayers * _Options.InputDataSize;
  if (InputDataSize > MAX_INPUT_DATA_SIZE) { 
    throw std::runtime_error("Input data size exceeds memory size!");
  }

  if (rngSeed != -1) {
    _RNG.seed(rngSeed);
  }
  else { 
    std::random_device rd;
    _RNG.seed(rd());
  }
}


// ------------------------------------------------------------------------------------------------
CHFSOClient::~CHFSOClient() {}


//// ------------------------------------------------------------------------------------------------
//void CHFSOClient::Init(IUdpBlaster* udp_)
//{
//  if (IsInitialized) { throw std::runtime_error("Client has already been initialized!"); }
//
//  _Udp = udp_;
//  IsInitialized = true;
//}

//// ------------------------------------------------------------------------------------------------
//void CHFSOClient::CheckInit() {
//  if (!IsInitialized) {
//    throw std::runtime_error("Client is not initialized!");
//  }
//}

// ------------------------------------------------------------------------------------------------
void CHFSOClient::OnEndpointStateChange(void* sender, CStateChangedArgs& args) {
  printf("Yo!  I am a handler for an event!\n");
}


// ------------------------------------------------------------------------------------------------
void CHFSOClient::AddPlayer(CPlayer& player)
{
  bool stateOK = _State == EClientState::Init ||
    _State == EClientState::Active ||
    _State == EClientState::Starting;

  if (!stateOK) {
    throw std::runtime_error("Players can't be added in the current state!");
  }

  if (PlayerCount >= _Options.MaxPlayers) {
    throw std::runtime_error("Max players exceeded!");
  }

  int availableIndex = -1;
  for (size_t i = 0; i < _Options.MaxPlayers; i++)
  {
    auto& p = __AllPlayers[i];
    if (p.State() != EEndpointState::Initializing)
    {
      if (player.PlayerIndex == p.Data()->PlayerIndex) {
        throw std::runtime_error("Player index is already in use!");
      }
    }
    else {
      if (availableIndex == -1) { availableIndex = i; }
    }

  }

  // TODO: Some better way to do assert like stuff w/ messages..... like the one we had in BigBlock?
  assert(availableIndex != -1);

  auto& endpoint = __AllPlayers[availableIndex];
  endpoint.Init(player, this, this->_Udp);

  // Hook the event handlers....
  // NOTE: We want to keep that thunk instance somewhere....
  using thunk = CThunk<CHFSOClient, void, void*, CStateChangedArgs&>;
  auto fn = thunk::Bind(this, &CHFSOClient::OnEndpointStateChange);
  endpoint.OnStateChanged.Add(fn);

  ++PlayerCount;

  //// I'm thinking that the best way is to do 'BeginConnect' when we are adding the remote player?
  //// What about 3+ player games tho?  I don't really want to get too deep in the weeds, but I don't
  //// want to box us out?  fuck it, we are 'starting' when we added the local player, and we only 
  //// try to connect after we are hooking up the remote player.
  //if (_State == EClientState::Active || _State == EClientState::Starting) {
  //  // TODO: Start the connection process....
  //  endpoint.BeginConnect();
  //}

}



// ------------------------------------------------------------------------------------------------
void CHFSOClient::Start()
{
  if (_State != EClientState::Init) {
    throw std::runtime_error("Invalid state!");
  }

  for (size_t i = 0; i < PlayerCount; i++)
  {
    __AllPlayers[i].BeginConnect();
  }

  SetState(EClientState::Starting);

  // Connect all clients:
}

// ------------------------------------------------------------------------------------------------
void CHFSOClient::SetState(EClientState newState)
{
  _State = newState;
}


// ------------------------------------------------------------------------------------------------
void CHFSOClient::Idle() {
  if (_InRollback) { return; }

  // Receive data from blaster + distribute to endpoints.
  while (true)
  {
    sockaddr_in from;
    int msgSize = _Udp->Receive(ReadBuffer, &from);
    if (msgSize > 0) {

      CMsg* msg = (CMsg*)ReadBuffer;

      for (size_t i = 0; i < MAX_MAX_PLAYERS; i++)
      {
        __AllPlayers[i].OnMessage(msg, msgSize, from);
      }
    }
    else {
      break;
    }
  }

  // Process all endpoints now.
  for (size_t i = 0; i < MAX_MAX_PLAYERS; i++)
  {
    __AllPlayers[i].Update();
  }


  // Handle protocol level events.
  // Log.Info
  // TODO: Get the logging into place....
  // Send data out!
  // Receive data from network + process messages.
}

// ------------------------------------------------------------------------------------------------
bool CHFSOClient::SyncInput() { 

  // TODO: Actually implement this...
  return true;
}

// ------------------------------------------------------------------------------------------------
void  const CHFSOClient::IncrementFrame() {
  // TODO: Do the frame increment!
  if (_State != EClientState::Active) { return; }


}

// ------------------------------------------------------------------------------------------------
int IUdpBlaster::Receive(byte* buffer, sockaddr_in* from) {
  int res = ReceiveImpl(buffer, from);
  return res;
}

// ------------------------------------------------------------------------------------------------
void CHFSOClient::End()
{
  throw std::runtime_error("finish me!");
}

// ------------------------------------------------------------------------------------------------
uint32_t CHFSOClient::RandInt() {
  auto res = _RNG();
  return res;
}

// ------------------------------------------------------------------------------------------------
void funscrew::HFSO::CHFSOClient::GetSyncMetadata(CHFSOEndpoint* forEndpoint, uint8_t* data, uint16_t& dataSize)
{
  // TODO: We will send the name of this player....

  dataSize = 0;
}


// ------------------------------------------------------------------------------------------------
CSimHFSOClient::CSimHFSOClient(CClientOptions& ops_, IUdpBlaster* udp_, int rngSeed)
  : CHFSOClient(ops_, udp_, rngSeed)
{ }








// ------------------------------------------------------------------------------------------------------------------------
void funscrew::HFSO::CreateLocalPlayer(const char* name, uint16_t playerIndex, CPlayer* __target)
{
  __target->IsLocal = true;
  __target->Name = name;
  __target->PlayerIndex = playerIndex;
  // __target->Magic = GetMagic();
}

// ------------------------------------------------------------------------------------------------------------------------
void funscrew::HFSO::CreateRemotePlayer(const char* name, uint16_t playerIndex, const char* ip, uint16_t port, CPlayer* __target)
{
  __target->PlayerIndex = playerIndex;
  __target->IsLocal = false;
  __target->Name = name;
  // __target->RemoteAddr = CreateAddr(ip, port);
}