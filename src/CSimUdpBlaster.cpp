#include "CSimUdpBlaster.h"

#include "drewCo.Logging.hpp"
#include <climits>

extern drewCo::Logging::CLogMgr Log;

// --------------------------------------------------------------------------------
CSimMsg::CSimMsg(sockaddr_in& from, sockaddr_in* to, const byte* data, size_t size)
  :From(from)
  , To(*to)
  , Size(size)
{
  Data = (byte*)malloc(Size);
  if (!Data) {
    throw std::runtime_error("Could not allocate data for CMsg");
  }
  memcpy(Data, data, Size);
}

// --------------------------------------------------------------------------------
CSimMsg::~CSimMsg() {
  if (Data) {
    free(Data);
  }
}

// --------------------------------------------------------------------------------
CSimUdpBlaster::CSimUdpBlaster(uint16_t port_, IClockSource* clock_, CSimNetwork* network_)
  : IUdpBlaster(clock_)
  , _Network(network_)
{
  _Addr = CreateAddr("127.0.0.1", port_);
}

// --------------------------------------------------------------------------------
int CSimUdpBlaster::ReceiveImpl(byte* receiveBuffer, sockaddr_in* from)
{
  const CSimMsg* next = _Network->GetNext(_Addr);
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
  _Network->Send(_Addr, reinterpret_cast<sockaddr_in*>(to), sendBuffer, size);
  return size;

  // CSimMsg toSend(_Addr, (sockaddr_in*)to, sendBuffer, size);

  //auto curTime = _Clock->GetTime();

  //int deliveryTime = GetDeliveryTime(from, to);
  //if (deliveryTime != -1) {
  //  auto msg = new CSimMsg(from, to, data, dataSize);
  //  _Messages.push_back(msg);
  //}
  //else {
  //  Log.Info("Simulated dropped packet!");
  //}


  //throw std::runtime_error("not implemented!");
}

// --------------------------------------------------------------------------------
CSimNetwork::CSimNetwork(IClockSource* clock_)
  : _Clock(clock_)
{ }

// --------------------------------------------------------------------------------
const CSimMsg* CSimNetwork::GetNext(sockaddr_in& forAddr) {
  // throw std::runtime_error("please finish me!");

  // This is us pulling data from the message queue....
  int minTime = INT_MAX;
  CSimMsg* res = nullptr;

  int curTime = _Clock->GetTime(); // udp.Clock.CurTime;

  if (curTime >= 5) { 
    int x = 10;
  }

  // Grab all messages in the queue up to the current time.
  // only include those messages that have the matching port...
  // SimUdpMessage? toRemove = null;
  int index = -1;
  size_t len = _Messages.size();

  std::list<CSimMsg*>::iterator toRemove;
  for (auto it = _Messages.begin(); it != _Messages.end(); it++)
  {
    CSimMsg* next = *it;

    bool same = AreSame(next->To, forAddr);
    if (same) {
      int x = 10;
    }

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
void CSimNetwork::Send(sockaddr_in& from, sockaddr_in* to, byte* data, size_t dataSize)
{
  // TEMP:
  if (AreSame(from, *to)) {
    throw std::runtime_error("loopback detected!");
    // printf("loopback detected!\n");
  }

  auto curTime = _Clock->GetTime();

  // TODO: This is where we will simulate packet loss...
  // bool hasLinkOptions = 
  CLinkOptions* ops = nullptr;
  auto it = _LinkOptions.find({ from, *to });
  if (it != _LinkOptions.end())
  {
    ops = &it->second;
  }

  int deliveryTime = curTime + ComputePing(ops);
  if (deliveryTime != -1) {
    auto msg = new CSimMsg(from, to, data, dataSize);
    msg->ReceiveTime = deliveryTime;
    //msg->Data = data;
    //msg->Size = dataSize;

    _Messages.push_back(msg);
  }
  else {
    Log.Info("Simulated dropped packet!");
  }
}


// --------------------------------------------------------------------------------
// NOTE Could be faster if I could figure out direct pointer / reference return.
// Or acutally, a hash of the addresses + map.....
int CSimNetwork::ComputePing(const CLinkOptions* ops)
{
  int usePing = AvgPing;
  int useJitter = AvgJitter;

  if (ops) {
    throw std::runtime_error("not implemented!");
  }

  return usePing;

  // TODO: Use the jitter when computing....

  //for (auto it = _LinkOptions.begin(); it != _LinkOptions.end(); it++) {
  //  if ((AreSame(it->From, from) && AreSame(it->To, to)) ||
  //    (AreSame(it->To, from) && AreSame(it->From, to)))
  //  {
  //    // TODO: Take jitter into account.
  //    // TODO: Some kind of mechanism to simulate jitter spikes... probably a manual update to the link options....
  //    if (it->Jitter != 0) { throw new std::runtime_error("nonzero jitter is not supported!"); }

  //    // TODO: Some way to simulate packet loss.  NOTE: I also want a way to simulate packet loss in the real clients so that we can experiment in real life!

  //    // OK: 
  //    auto now = _Clock->GetTime();
  //    int res = now + it->Ping;
  //    return res;
  //  }
  //}

 //  throw std::runtime_error("Could not find the link options for the given addresses!");

}