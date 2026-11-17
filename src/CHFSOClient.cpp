#include "CHFSOClient.h"

#include <stdexcept>
#include "errno.h"
#include <cassert>

#include "CThunk.h"
#include "CUdpBlaster.h"
#include <cstring>

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


// ------------------------------------------------------------------------------------------------
void CHFSOClient::OnEndpointStateChange(void* sender, CStateChangedArgs& args) {
  printf("Yo!  I am a handler for an event!\n");
}
//
//// ------------------------------------------------------------------------------------------------
//CEndpointEvent::CEndpointEvent(EEventType type_) {
//  Type = type_;
//}

// ------------------------------------------------------------------------------------------------
void CHFSOClient::AddPlayer(CPlayer& player)
{
  bool stateOK = _State == EClientState::Init ||
    _State == EClientState::Active ||
    _State == EClientState::Starting;

  if (!stateOK) {
    throw std::runtime_error("Players can't be added in the current state!");
  }

  if (_PlayerCount >= _Options.MaxPlayers) {
    throw std::runtime_error("Max players exceeded!");
  }

  int availableIndex = -1;
  for (size_t i = 0; i < _Options.MaxPlayers; i++)
  {
    auto& p = __AllPlayers[i];
    if (p.State() != EEndpointState::Initializing)
    {
      if (player.PlayerIndex == p.PlayerData()->PlayerIndex) {
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

  //// Hook the event handlers....
  //// NOTE: We want to keep that thunk instance somewhere....
  //using thunk = CThunk<CHFSOClient, void, void*, CStateChangedArgs&>;
  //auto fn = thunk::Bind(this, &CHFSOClient::OnEndpointStateChange);
  //endpoint.OnStateChanged.Add(fn);
  ++_PlayerCount;

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
const CHFSOEndpoint* CHFSOClient::GetPlayer(uint8_t playerIndex) const 
{
  for (size_t i = 0; i < _PlayerCount; i++)
  {
    auto& ep = __AllPlayers[i];
    if (ep.PlayerData()->PlayerIndex == playerIndex) { 
      return &ep;
    }
  }
  return nullptr;
}


// ------------------------------------------------------------------------------------------------
void CHFSOClient::AddEndpointEvent(CEndpointEvent* event)
{
  __EndpointEvents.Push(event);
}

// ------------------------------------------------------------------------------------------------
void CHFSOClient::Start()
{
  if (_State != EClientState::Init) {
    throw std::runtime_error("Invalid state!");
  }

  for (size_t i = 0; i < _PlayerCount; i++)
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
  HandleEvents();

  _Udp->UpdateStats();

  // Log.Info
  // TODO: Get the logging into place....
  // Send data out!
  // Receive data from network + process messages.
}

// ------------------------------------------------------------------------------------------------
void CHFSOClient::HandleEvents() {

  while (__EndpointEvents.Count()) {

    auto evt = __EndpointEvents.First();

    switch (evt->Type) {

    case EEventType::ClientConnected:
    {
      // NOTE: For now, we only really support a single endpoint, so we can call this good.
      if (!evt->Content.Connected.Endpoint->IsLocalPlayer()) {
        // We can now transition the client to the active state!
        SetState(EClientState::Active);
      }
    }
    break;

    default:
      throw std::runtime_error("Unsupported client event type!");
    }

    delete evt;
    __EndpointEvents.Pop();
  }

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
void CHFSOClient::End()
{
  throw std::runtime_error("finish me!");
}

// ------------------------------------------------------------------------------------------------
uint32_t CHFSOClient::RandInt() {
  auto res = _RNG();
  return res;
}

// =========================================================================================================
struct CSyncMetadataSerializer {

public:

  // ------------------------------------------------------------------------------------------------
  CSyncMetadataSerializer(uint8_t* buffer_, size_t maxSize_, bool forRead)
    : Buffer(buffer_)
    , MaxSize(maxSize_)
    , ReadWritePos(0)
    , _EntryCount(0)
    , DataIndex(0)
  {
    if (forRead) {
      _EntryCount = (uint8_t)Buffer[0];
    }
    ReadWritePos = 1;
  }


  // ------------------------------------------------------------------------------------------------
  // TODO: Enum for data code?  probably?
  void WriteData(uint8_t dataCode, uint8_t* data, uint16_t dataSize)
  {
    size_t end = ReadWritePos + sizeof(uint8_t) + sizeof(uint16_t) + dataSize;
    if (_EntryCount == 0xFF) {
      throw std::runtime_error("too many data entries!");
    }
    if (end > MaxSize) {
      throw std::runtime_error("too much data!");
    }

    Buffer[ReadWritePos] = dataCode;
    ReadWritePos += sizeof(uint8_t);

    // NOTE: This isn't writing two bytes....
    // maybe use EZWriter?
    Buffer[ReadWritePos] = dataSize >> 8;
    Buffer[ReadWritePos + 1] = dataSize;
    ReadWritePos += sizeof(uint16_t);

    memcpy(Buffer + ReadWritePos, data, dataSize);
    ReadWritePos += dataSize;

    ++_EntryCount;
    Buffer[0] = (uint8_t)_EntryCount;
  }

  // ------------------------------------------------------------------------------------------------
  inline size_t DataSize() {
    if (_EntryCount == 0) { return 0; }
    return ReadWritePos;
  }

  // ------------------------------------------------------------------------------------------------
  // Read the next bit of metadata.
  // Interpretation application defined.
  bool ReadNext(uint8_t& code, uint16_t& size, uint8_t*& data) {
    if (DataIndex == _EntryCount) { return false; }

    code = Buffer[ReadWritePos];
    size = (Buffer[ReadWritePos + 1] << 8) | Buffer[ReadWritePos + 2];
    data = Buffer + (ReadWritePos + 3);

    ReadWritePos += 3 + size;

    ++DataIndex;
    return true;
  }

  inline uint8_t EntryCount() { return _EntryCount; }

private:
  uint8_t* Buffer;
  size_t MaxSize;
  size_t ReadWritePos;

  // Total number of entries....
  uint8_t _EntryCount;

  // What data item are we currently reading?
  size_t DataIndex;
};


// ------------------------------------------------------------------------------------------------
int funscrew::HFSO::CHFSOClient::GetSyncMetadata(CHFSOEndpoint* forEndpoint, uint8_t* data, uint16_t& dataSize, uint16_t maxDataSize)
{
  // TOOD: We have to check the sync numbers + acks to decide if we need to send more data or not....
  auto& connData = forEndpoint->ConnectData();
  if (connData.AckedSyncNumber == 0) {
    dataSize = 0;
    return 1;
  }

  CSyncMetadataSerializer mw(data, maxDataSize, false);

  CHFSOEndpoint& local = GetLocalEndpoint();
  auto& name = local.PlayerData()->Name;

  constexpr uint8_t NAME_DATA_CODE = 1;
  mw.WriteData(NAME_DATA_CODE, (uint8_t*)name.data(), name.size());

  dataSize = mw.DataSize();

  return 0;
}

// ------------------------------------------------------------------------------------------------
// Handling of metadata is application defined.  For the reference implementation, we pull out
// the other player's name and make sure that it matches what we expect.
// Your implementation could do all kinds of things; exchange map data, all player positions, whatever you like...
// NOTE: Metadata exchange can happen over many sync request/reply cycles.  It does not have to all fit into one message!
bool funscrew::HFSO::CHFSOClient::HandleSyncMetadata(CHFSOEndpoint* forEndpoint, const CMsg* syncMsg)
{
  auto& rq = syncMsg->Content.SyncRequest;
  if (rq.DataSize == 0) { return true; }

  CSyncMetadataSerializer ms((uint8_t*)rq.SyncData, rq.DataSize, true);

  if (ms.EntryCount() != 1) {
    throw std::runtime_error("There should only be one metadata entry count!");
  }

  uint8_t code;
  uint16_t size;
  uint8_t* data;

  if (ms.ReadNext(code, size, data))
  {
    if (code != 1) {
      throw std::runtime_error("Expected player name data!");
    }
    std::string checkName;
    checkName.append((const char*)data, size);

    auto& epName = forEndpoint->PlayerData()->Name;
    int comp = epName.compare(checkName.data());
    if (comp) {
      // Not the same!
      throw std::runtime_error("endpoint player name does not match metadata name!");
    }

    return true;
  }
  else {
    throw std::runtime_error("There is no data when we expected some!");
  }

  return false;
}

// ------------------------------------------------------------------------------------------------
CHFSOEndpoint& funscrew::HFSO::CHFSOClient::GetLocalEndpoint() {
  for (int i = 0; i < _PlayerCount; i++) {
    auto& ap = __AllPlayers[i];
    if (ap.PlayerData()->IsLocal) {
      return ap;
    }
  }
  throw std::runtime_error("No local player has been registered!");
}

// ------------------------------------------------------------------------------------------------
CSimHFSOClient::CSimHFSOClient(CClientOptions& ops_, IUdpBlaster* udp_, int rngSeed)
  : CHFSOClient(ops_, udp_, rngSeed)
{
}








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