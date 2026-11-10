
#include "CUdpBlaster.h"

using namespace funscrew::HFSO;



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
      // CMsg* msg = (CMsg*)ReadBuffer;

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

  if (res == SOCKET_ERROR) {
    DWORD err = WSAGetLastError();

    throw std::runtime_error("Unknown error in send!");

    // Utils::LogIt(CATEGORY_UDP, "unknown error in sendto (erro: %d  wsaerr: %d).", res, err);
    // ASSERT(FALSE && "Unknown error in sendto");
  }

  // Other logging can happen here too....
  //char dst_ip[1024];
  //Utils::LogIt(CATEGORY_UDP, "sent %d to %s:%d", len, inet_ntop(AF_INET, (void*)&to->sin_addr, dst_ip, ARRAY_SIZE(dst_ip)), ntohs(to->sin_port), res);

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

