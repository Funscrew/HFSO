#include "CSimUdpBlaster.h"

#include "drewCo.Logging.hpp"
#include <climits>

extern drewCo::Logging::CLogMgr Log;

// --------------------------------------------------------------------------------
CMsg::CMsg(sockaddr_in& from, sockaddr_in& to, const byte* data, size_t size) {
  Data = (byte*)malloc(size);
  if (!Data) {
    throw std::runtime_error("Could not allocate data for CMsg");
  }
  memcpy(Data, data, size);
}

// --------------------------------------------------------------------------------
CMsg::~CMsg() {
  if (Data) {
    free(Data);
  }
}

// --------------------------------------------------------------------------------
CSimUdpBlaster::CSimUdpBlaster(uint16_t port_, IClockSource* clock_)
  : IUdpBlaster(clock_)
  , _Network(clock_)
{
  _Addr = CreateAddr("127.0.0.1", port_);
}

// --------------------------------------------------------------------------------
int CSimUdpBlaster::ReceiveImpl(byte* receiveBuffer, sockaddr_in* from)
{
  const CMsg* next = _Network.GetNext(_Addr);
  if (next)
  {
    memcpy(receiveBuffer, next->Data, next->Size);
    *from = next->From;
    return next->Size;
  }
  return 0;
}

// --------------------------------------------------------------------------------
int CSimUdpBlaster::SendImpl(byte* sendBuffer, size_t size, sockaddr* to)
{
  throw std::runtime_error("not implemented!");
}

// --------------------------------------------------------------------------------
CSimNetwork::CSimNetwork(IClockSource* clock_)
  : _Clock(clock_)
{
}

// --------------------------------------------------------------------------------
const CMsg* CSimNetwork::GetNext(sockaddr_in& forAddr) {
  // throw std::runtime_error("please finish me!");

  // This is us pulling data from the message queue....
  int minTime = INT_MAX;
  CMsg* res = nullptr;

  int curTime = _Clock->GetTime(); // udp.Clock.CurTime;

  // Grab all messages in the queue up to the current time.
  // only include those messages that have the matching port...
  // SimUdpMessage? toRemove = null;
  int index = -1;
  size_t len = _Messages.size();

  std::list<CMsg*>::iterator toRemove;
  for (auto it = _Messages.begin(); it != _Messages.end(); it++)
  {
    CMsg* next = *it;
    if (next->ReceiveTime <= curTime &&
      next->ReceiveTime <= minTime &&
      AreSame(next->To, forAddr))
      {
        toRemove = it;
        res = next;
        minTime = res->ReceiveTime;
      }
  }
  if (res) { 
    _Messages.erase(toRemove);
  }

  return res;


}

// --------------------------------------------------------------------------------
void CSimNetwork::Send(sockaddr_in& from, sockaddr_in& to, byte* data, size_t dataSize)
{
  auto curTime = _Clock->GetTime();

  int deliveryTime = GetDeliveryTime(from, to);
  if (deliveryTime != -1) {
    auto msg = new CMsg(from, to, data, dataSize);
    _Messages.push_back(msg);
  }
  else {
    Log.Info("Simulated dropped packet!");
  }
}

// --------------------------------------------------------------------------------
// TODO: Share
bool AreSame(const sockaddr_in& l, const sockaddr_in& r)
{
  return l.sin_addr.S_un.S_addr == r.sin_addr.S_un.S_addr &&
    l.sin_port == r.sin_port;
}

// --------------------------------------------------------------------------------
// NOTE Could be faster if I could figure out direct pointer / reference return.
int CSimNetwork::GetDeliveryTime(const sockaddr_in& from, const sockaddr_in& to)
{
  for (auto it = _LinkOptions.begin(); it != _LinkOptions.end(); it++) {
    if ((AreSame(it->From, from) && AreSame(it->To, to)) ||
      (AreSame(it->To, from) && AreSame(it->From, to)))
    {
      // TODO: Take jitter into account.
      // TODO: Some kind of mechanism to simulate jitter spikes... probably a manual update to the link options....
      if (it->Jitter != 0) { throw new std::runtime_error("nonzero jitter is not supported!"); }

      // TODO: Some way to simulate packet loss.  NOTE: I also want a way to simulate packet loss in the real clients so that we can experiment in real life!

      // OK: 
      auto now = _Clock->GetTime();
      int res = now + it->Ping;
      return res;
    }
  }

  throw std::runtime_error("Could not find the link options for the given addresses!");

}