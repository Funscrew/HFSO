#include "CSimUdpBlaster.h"

// --------------------------------------------------------------------------------
CMsg::CMsg(const byte* data, size_t size) {
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
}



// --------------------------------------------------------------------------------
int CSimUdpBlaster::ReceiveImpl(byte* receiveBuffer, sockaddr_in* from)
{
    throw std::runtime_error("not implemented!");
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
void CSimNetwork::Send(sockaddr_in& from, sockaddr_in& to, byte* data, size_t dataSize)
{
    auto curTime = _Clock->GetTime();

    CLinkOptions& ops = GetLinkOptions(from, to);

    auto msg = new CMsg(from, to , data, dataSize);
        
    _Messages.push_back(msg);
}

// --------------------------------------------------------------------------------
CLinkOptions& CSimNetwork::GetLinkOptions(const sockaddr_in& from, const sockaddr_in& to)
{
}