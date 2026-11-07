#include "CClient.h"

#include <stdexcept>
#include "errno.h"
#include <cassert>

#include "CThunk.h"

using namespace funscrew::HFSO;

const int MIN_PLAYERS = 2;

// ------------------------------------------------------------------------------------------------
// NOTE: This should go off with the Platform.h code, probably.....
SOCKET CreateSocket(uint32_t bind_port, int retries)
{
  SOCKET s;
  sockaddr_in sin;
  uint16_t port;
  int optval = 1;

  s = socket(AF_INET, SOCK_DGRAM, 0);
  auto o1 = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);

  auto e1 = WSAGetLastError();   //NOTE: This might be 'errno' on linux!
  if (e1 == 10093) {
    throw std::runtime_error("Winsock is not initialized!");
  }

  auto o2 = setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char*)&optval, sizeof optval);

  // non-blocking...
  u_long iMode = 1;
  ioctlsocket(s, FIONBIO, &iMode);

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  for (port = bind_port; port <= bind_port + retries; port++) {
    sin.sin_port = htons(port);

    auto bindRes = bind(s, (sockaddr*)&sin, sizeof sin);
    if (bindRes == SOCKET_ERROR) {
      // TODO: Some kind of better logging here!
      // printf("err: %d\n", errno);
      // auto e3 = WSAGetLastError();

      throw std::runtime_error("Error creating the socket!");
      // ASSERT(bindRes != SOCKET_ERROR, "Error creating the socket!");
    }
    else {
      // Utils::LogIt(CATEGORY_UDP, "Udp bound to port: %d.", port);
      return s;
    }
  }
  closesocket(s);
  return INVALID_SOCKET;
}

// ------------------------------------------------------------------------------------------------
IUdpBlaster::IUdpBlaster(IClockSource* clock_)
  : _Clock(clock_)
{
}

// ------------------------------------------------------------------------------------------------
int IUdpBlaster::Send(byte* sendBuffer, size_t size, struct sockaddr* to)
{

  int res = SendImpl(sendBuffer, size, to);

  // Get the current time + update stats.

  return res;
}



// ------------------------------------------------------------------------------------------------
CUdpBlaster::CUdpBlaster(uint16_t port_, IClockSource* clock_) :
  IUdpBlaster(clock_)
  , _Port(port_)
{
  _Socket = CreateSocket(_Port, 3);
  _Addr = CreateAddr("127.0.0.1", _Port);
}

// ------------------------------------------------------------------------------------------------
int CUdpBlaster::ReceiveImpl(byte* receiveBuffer, sockaddr_in* from) {
  // throw std::runtime_error("not implemented!");


  // GGPO style:
  uint8_t          recv_buf[MAX_PACKET_SIZE];
  sockaddr_in    recv_addr;
  int            recv_addr_len;
  int len = 0;

  // NOTE: This is all kind of windows specific stuff......
  for (;;) {

    recv_addr_len = sizeof(recv_addr);
    len = recvfrom(_Socket, (char*)recv_buf, MAX_PACKET_SIZE, 0, (struct sockaddr*)&recv_addr, &recv_addr_len);

    // TODO: handle len == 0... indicates a disconnect.
    if (len == -1) {
      int error = WSAGetLastError();
      if (error != WSAEWOULDBLOCK) {
        // Utils::LogIt(CATEGORY_UDP, "recvfrom WSAGetLastError returned %d (%x).", error, error);
      }
      break;
    }
    else if (len > 0) {
      // NOTE: We can probably avoid this copy....
      *from = recv_addr;

      // NOTE: For our implementation, the client is responsible for interpreting the data...

      // char src_ip[1024];
      // Utils::LogIt(CATEGORY_UDP, "recvfrom returned (len:%d  from:%s:%d).", len, inet_ntop(AF_INET, (void*)&recv_addr.sin_addr, src_ip, ARRAY_SIZE(src_ip)), ntohs(recv_addr.sin_port));
      // CUdpMsg* msg = (CUdpMsg*)ReadBuffer;

      // This is where we send off the messages to all endpoints that are listening...
      //_callbacks->OnMsg(recv_addr, msg, len);
      //++pollCount;
    }
  }

  return len;
}

// ------------------------------------------------------------------------------------------------
int CUdpBlaster::SendImpl(byte* sendBuffer, size_t size, struct sockaddr* to) {
  int res = sendto(_Socket, reinterpret_cast<const char*>(sendBuffer), size, 0, to, sizeof to);
  return res;
}

// ------------------------------------------------------------------------------------------------
CUdpBlaster::~CUdpBlaster() {
  if (_Socket != INVALIDSOCKET) {
    // Blow it up here!
    closesocket(_Socket);
  }
  _Socket = INVALIDSOCKET;
}


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
    if (p.State() != EEndpointState::Available)
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
  endpoint.Init(player, this);

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
    int received = _Udp->Receive(ReadBuffer, &from);
    if (received > 0) {

      CUdpMsg* msg = (CUdpMsg*)ReadBuffer;

      for (size_t i = 0; i < MAX_MAX_PLAYERS; i++)
      {
        __AllPlayers[i].OnMessage(msg, from);
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
CSimHFSOClient::CSimHFSOClient(CClientOptions& ops_, IUdpBlaster* udp_, int rngSeed)
  : CHFSOClient(ops_, udp_, rngSeed)
{ }








// ------------------------------------------------------------------------------------------------------------------------
sockaddr_in funscrew::HFSO::CreateAddr(const std::string ip, uint16_t port)
{
  return CreateAddr(ip.data(), port);
}

// ------------------------------------------------------------------------------------------------------------------------
sockaddr_in funscrew::HFSO::CreateAddr(const char* ip, uint16_t port)
{
  sockaddr_in res;
  res.sin_family = AF_INET;
  res.sin_port = htons(port);
  inet_pton(AF_INET, ip, &res.sin_addr.s_addr);

  if (!res.sin_addr.s_addr) {
    throw std::runtime_error("Invalid peer address!");
  }

  return res;
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