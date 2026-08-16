#include "CClient.h"

#include <stdexcept>
#include "errno.h"

using namespace funscrew::HFSO;
const int MIN_PLAYERS = 2;

// ------------------------------------------------------------------------------------------------
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
}

// ------------------------------------------------------------------------------------------------
int CUdpBlaster::ReceiveImpl(byte* receiveBuffer, struct sockaddr_in& from) {
  throw std::runtime_error("not implemented!");
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
CHFSOClient::CHFSOClient(uint8_t maxPlayers_)
  : MaxPlayers(maxPlayers_)
{
  if (MaxPlayers < MIN_PLAYERS) { 
    throw new std::runtime_error("You must have a minimum of two players!");
  }
  if (MaxPlayers > 2) { 
    throw new std::runtime_error("This client can only support a max of two players!");
  }
}


// ------------------------------------------------------------------------------------------------
CHFSOClient::~CHFSOClient() {}


// ------------------------------------------------------------------------------------------------
void CHFSOClient::Init(IUdpBlaster* udp_)
{
  if (IsInitialized) { throw std::runtime_error("Client has already been initialized!"); }

  _Udp = udp_;
  IsInitialized = true;
}

// ------------------------------------------------------------------------------------------------
void CHFSOClient::CheckInit() {
  if (!IsInitialized) {
    throw std::runtime_error("Client is not initialized!");
  }
}


// ------------------------------------------------------------------------------------------------
void CHFSOClient::AddPlayer(CPlayer& player)
{
  size_t len = _AllPlayers.size();
  if (len >= MaxPlayers) {
    throw std::runtime_error("Max players exceeded!");
  }

  // TODO: Find a way to validate the addresses depending on the player type....
  //if (!player.IsLocal && player.RemoteAddr) {
  //  throw std::runtime_error("Remote players must have an assigned address!");
  //}
  //if (player.IsLocal && player.RemoteAddr) { 
  //  throw std::runtime_error("Local players must not have an assigned address!");
  //}

  for (size_t i = 0; i < len; i++)
  {
    auto& p = _AllPlayers[i];
    if (player.PlayerIndex == p.PlayerIndex) {
      throw std::runtime_error("Player index is already in use!");
    }
  }

  _AllPlayers.push_back(player);
}



// ------------------------------------------------------------------------------------------------
CSimHFSOClient::CSimHFSOClient(uint8_t maxPlayers_)
  : CHFSOClient(maxPlayers_)
  {  }









// ------------------------------------------------------------------------------------------------------------------------
sockaddr_in funscrew::HFSO::CreateAddr(const char* ip, uint16_t port)
{
  sockaddr_in    res;
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
  __target->Name = name,
    __target->PlayerIndex = playerIndex;
  // __target->Magic = GetMagic();
}

// ------------------------------------------------------------------------------------------------------------------------
void funscrew::HFSO::CreateRemotePlayer(const char* name, uint16_t playerIndex, const char* ip, uint16_t port, CPlayer* __target)
{
  __target->PlayerIndex = playerIndex;
  __target->IsLocal = false,
    __target->Name = name,
    __target->RemoteAddr = CreateAddr(ip, port);
}