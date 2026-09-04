#pragma once

#include "CClient.h"
#include <list>

using namespace funscrew::HFSO;

//struct CAddr {
//    uint64_t Host;          // Serialized verison of an IPv4/v6 address.  Can also be a simple number for convenience.
//    uint16_t Port;          // Whatever the port might be.
//};

struct CMsg {
public:
    CMsg(sockaddr& from, sockaddr& to, const byte* data, size_t size);
    ~CMsg();

    sockaddr_in From;
    sockaddr_in To;

    uint64_t DeliverTime;   // When we expect it to arrive.

private:

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

    const CMsg* GetNext();
    void Send(sockaddr_in& from, sockaddr_in& to, byte* data, size_t dataSize);

private:
    IClockSource* _Clock = nullptr;
    std::list<CMsg*> _Messages;

    // Get the link options between the two addresses.
    // Not that from/to do not matter as they will describe the same link as long as
    // they are the same permutation of addresses.
    CLinkOptions& GetLinkOptions(const sockaddr_in& from, const sockaddr_in& to);

    std::list<CLinkOptions> _LinkOptions;

};

// ============================================================================================================
// A simulated UDP client for sending / receiving simulated udp traffic.
// Used in testing scenarios when we want to be able to reliably recreate certain network conditions.
class CSimUdpBlaster : public IUdpBlaster {

public:
    CSimUdpBlaster(uint16_t port_, IClockSource* clock_);

protected:
    // Inherited via IUdpBlaster
    int ReceiveImpl(byte* receiveBuffer, sockaddr_in* from) override;
    int SendImpl(byte* sendBuffer, size_t size, sockaddr* to) override;

    CSimNetwork _Network;
};
