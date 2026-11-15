
#include "CUdpBlaster.h"

using namespace funscrew::HFSO;

// ------------------------------------------------------------------------------------------------------------------------
sockaddr_in funscrew::HFSO::CreateAddr(const std::string ip, uint16_t port)
{
  return CreateAddr(ip.data(), port);
}

// ------------------------------------------------------------------------------------------------------------------------
sockaddr_in funscrew::HFSO::CreateAddr(const char *ip, uint16_t port)
{
  sockaddr_in res;
  res.sin_family = AF_INET;
  res.sin_port = htons(port);
  inet_pton(AF_INET, ip, &res.sin_addr.s_addr);

  if (!res.sin_addr.s_addr)
  {
    throw std::runtime_error("Invalid peer address!");
  }

  return res;
}

// ------------------------------------------------------------------------------------------------
// NOTE: This should go off with the Platform.h code, probably.....
SOCKET CreateSocket(uint32_t bind_port, int retries)
{
  SOCKET s;
  sockaddr_in sin{};
  uint16_t port;
  int optval = 1;

  s = socket(AF_INET, SOCK_DGRAM, 0);

  if (s == INVALID_SOCKET)
  {
#ifdef _WIN32
    if (WSAGetLastError() == WSANOTINITIALISED)
    {
      throw std::runtime_error("Winsock is not initialized!");
    }
#endif
    throw std::runtime_error("Failed to create UDP socket!");
  }

  auto o1 = setsockopt(
      s,
      SOL_SOCKET,
      SO_REUSEADDR,
      reinterpret_cast<const char *>(&optval),
      sizeof(optval));

  if (o1 == SOCKET_ERROR)
  {
    closesocket(s);
    throw std::runtime_error("Failed to set SO_REUSEADDR!");
  }

  // non-blocking...
  u_long iMode = 1;
  ioctlsocket(s, FIONBIO, &iMode);

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  for (port = bind_port; port <= bind_port + retries; port++)
  {
    sin.sin_port = htons(port);

    auto bindRes = bind(s, (sockaddr *)&sin, sizeof sin);
    if (bindRes == SOCKET_ERROR)
    {
      // TODO: Some kind of better logging here!
      // printf("err: %d\n", errno);
      // auto e3 = WSAGetLastError();

      throw std::runtime_error("Error creating the socket!");
      // ASSERT(bindRes != SOCKET_ERROR, "Error creating the socket!");
    }
    else
    {
      // Utils::LogIt(CATEGORY_UDP, "Udp bound to port: %d.", port);
      return s;
    }
  }
  closesocket(s);
  return INVALID_SOCKET;
}

// ------------------------------------------------------------------------------------------------
IUdpBlaster::IUdpBlaster(IClockSource *clock_)
    : _Clock(clock_)
{
  auto now = _Clock->GetTime();
  Stats.StartTime = now;
  Stats.LastUpdateTime = now;
}

// ------------------------------------------------------------------------------------------------
void IUdpBlaster::UpdateStats()
{

  auto now = _Clock->GetTime();
  int updateTime = now - Stats.LastUpdateTime;
  if (updateTime > UpdateInterval)
  {
    int totalMs = now - Stats.StartTime;
    float totalTime = (std::max)(1.0f, totalMs / 1000.0f);

    // These are aggregate stats over time.
    // TODO: Use 'LastUpdateTime' to create / extablish a smaller, more recent window for the send/receive stats.
    Stats.KbpsReceived = (float)(Stats.BytesReceived / 1024.0f) / totalTime;
    Stats.KbpsSent = (float)(Stats.BytesSent / 1024.0f) / totalTime;

    Stats.LastUpdateTime = now;
  }
}

// ------------------------------------------------------------------------------------------------
CUdpBlaster::CUdpBlaster(uint16_t port_, IClockSource *clock_) : IUdpBlaster(clock_), _Port(port_)
{
  _Socket = CreateSocket(_Port, 3);
  _Addr = CreateAddr("127.0.0.1", _Port);
}

// ------------------------------------------------------------------------------------------------
int IUdpBlaster::Send(uint8_t *sendBuffer, size_t size, struct sockaddr *to)
{
  Stats.PacketsSent++;
  Stats.BytesSent += size;
  Stats.LastSendTime = _Clock->GetTime();

  int res = SendImpl(sendBuffer, size, to);

  // Get the current time + update stats.

  return res;
}

// ------------------------------------------------------------------------------------------------
int IUdpBlaster::Receive(uint8_t *buffer, sockaddr_in *from)
{

  int res = ReceiveImpl(buffer, from);
  return res;
}

// ------------------------------------------------------------------------------------------------
int CUdpBlaster::ReceiveImpl(uint8_t *receiveBuffer, sockaddr_in *from)
{

  // GGPO style:
  uint8_t recv_buf[MAX_PACKET_SIZE];
  sockaddr_in recv_addr;
  socklen_t recv_addr_len;
  int size = 0;

  // NOTE: This is all kind of windows specific stuff......
  for (;;)
  {

    recv_addr_len = sizeof(recv_addr);
    size = recvfrom(_Socket, (char *)recv_buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);

    // TODO: handle len == 0... indicates a disconnect.
    if (size == -1)
    {
      int error = WSAGetLastError();
      if (error != WSAEWOULDBLOCK)
      {
        // Utils::LogIt(CATEGORY_UDP, "recvfrom WSAGetLastError returned %d (%x).", error, error);
      }
      break;
    }
    else if (size > 0)
    {
      // NOTE: We can probably avoid this copy....
      *from = recv_addr;

      // NOTE: This kind of logic makes me think more and more that the stats should be in UDPBlaster.
      Stats.PacketsReceived += 1;
      Stats.BytesReceived += size;
      Stats.LastReceiveTime = _Clock->GetTime();
    }
  }

  return size;
}

// ------------------------------------------------------------------------------------------------
int CUdpBlaster::SendImpl(uint8_t *sendBuffer, size_t size, struct sockaddr *to)
{
  int res = sendto(_Socket, reinterpret_cast<const char *>(sendBuffer), size, 0, to, sizeof to);

  if (res == SOCKET_ERROR)
  {
    DWORD err = WSAGetLastError();

    throw std::runtime_error("Unknown error in send!");

    // Utils::LogIt(CATEGORY_UDP, "unknown error in sendto (erro: %d  wsaerr: %d).", res, err);
    // ASSERT(FALSE && "Unknown error in sendto");
  }

  // Other logging can happen here too....
  // char dst_ip[1024];
  // Utils::LogIt(CATEGORY_UDP, "sent %d to %s:%d", len, inet_ntop(AF_INET, (void*)&to->sin_addr, dst_ip, ARRAY_SIZE(dst_ip)), ntohs(to->sin_port), res);

  return res;
}

// ------------------------------------------------------------------------------------------------
CUdpBlaster::~CUdpBlaster()
{
  if (_Socket != INVALIDSOCKET)
  {
    // Blow it up here!
    closesocket(_Socket);
  }
  _Socket = INVALIDSOCKET;
}

namespace funscrew::HFSO
{
  // --------------------------------------------------------------------------------
  // TODO: Share
  bool AreSame(const sockaddr_in &l, const sockaddr_in &r)
  {
    return l.sin_addr.s_addr == r.sin_addr.s_addr &&
           l.sin_port == r.sin_port;
  }
}