#pragma once

#include <unordered_map>

#include "CUdpBlaster.h"

using EndpointPair = std::pair<sockaddr_in, sockaddr_in>;

using namespace funscrew::HFSO;

// NOTE: 
// https://stackoverflow.com/questions/11684008/how-do-you-cast-sockaddr-structure-to-a-sockaddr-in-c-networking-sockets-ubu


// ============================================================================================================
// TODO: Share
struct SockAddrHash {
  std::size_t operator()(const sockaddr_in& addr) const noexcept {
    std::size_t h1 = std::hash<uint32_t>{}(addr.sin_addr.s_addr);
    std::size_t h2 = std::hash<uint16_t>{}(addr.sin_port);

    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};


// ============================================================================================================
// TODO: Share
struct SockAddrEqual {
  bool operator()(const sockaddr_in& a, const sockaddr_in& b) const noexcept {
    return a.sin_addr.s_addr == b.sin_addr.s_addr &&
      a.sin_port == b.sin_port;
  }
};


// ============================================================================================================
struct EndpointPairHash {
  std::size_t operator()(const EndpointPair& pair) const noexcept {
    std::size_t h1 = SockAddrHash{}(pair.first);
    std::size_t h2 = SockAddrHash{}(pair.second);

    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};

// ============================================================================================================
struct EndpointPairEqual {
  bool operator()(const EndpointPair& a, const EndpointPair& b) const noexcept {
    return SockAddrEqual{}(a.first, b.first) &&
      SockAddrEqual{}(a.second, b.second);
  }
};


struct CSimMsg {
public:
    CSimMsg(sockaddr_in& from, sockaddr_in* to, const byte* data, size_t size);
    ~CSimMsg();

    sockaddr_in From;
    sockaddr_in To;

    // When we expect it to arrive.
    uint64_t ReceiveTime;   

    byte* Data = nullptr;
    size_t Size = 0;
};

// ============================================================================================================
// Describes the connection between two endpoints.
// This is part of simulating messages over a network.
struct CLinkOptions { 
    sockaddr_in From;
    sockaddr_in To;

    uint32_t Ping = 0;
    uint32_t Jitter = 0;

    // TODO: Packet loss %, etc.
};

// ============================================================================================================
// Used to deliver simulated packets, etc. at simulated times.
class CSimNetwork {

public:
    CSimNetwork(IClockSource* clock_);

    const CSimMsg* GetNext(sockaddr_in& forAddr);
    void Send(sockaddr_in& from, sockaddr_in* to, byte* data, size_t dataSize);

private:
    IClockSource* _Clock = nullptr;
    std::list<CSimMsg*> _Messages;

    /// <summary>
    /// Get the expected delivery time for the packet.  This takes the link options into account.
    /// A return value of -1 means that the packet will be dropped!
    /// </summary>
    int ComputePing(const CLinkOptions* ops);

    std::unordered_map<EndpointPair, CLinkOptions, EndpointPairHash, EndpointPairEqual> _LinkOptions;

    uint16_t AvgPing = 5;
    uint16_t AvgJitter = 0;
};

// ============================================================================================================
// A simulated UDP client for sending / receiving simulated udp traffic.
// Used in testing scenarios when we want to be able to reliably recreate certain network conditions.
class CSimUdpBlaster : public IUdpBlaster {

public:
    CSimUdpBlaster(uint16_t port_, IClockSource* clock_, CSimNetwork* network_);

protected:
    // Inherited via IUdpBlaster
    int ReceiveImpl(byte* receiveBuffer, sockaddr_in* from) override;
    int SendImpl(byte* sendBuffer, size_t size, sockaddr* to) override;

    CSimNetwork* _Network;
};

