#ifndef  CUDPBlaster_H
#define CUDPBlaster_H

#include "IClockSource.hpp"

constexpr size_t MAX_PACKET_SIZE = 4000;

namespace funscrew::HFSO
{

  // ============================================================================================================
  struct CNetworkStats {
    int     LastSendTime = 0;
    int     LastReceiveTime = 0;

    int      PacketsSent = 0;
    int      PacketsReceived = 0;

    int      BytesSent = 0;
    int      BytesReceived = 0;

    float    KbpsReceived = 0;
    float    KbpsSent = 0.0f;

    // NEW: how long has client been active for?
    int       Uptime = 0.0;

    int StartTime = -1;
    int LastUpdateTime = -1;
  };

  // ============================================================================================================
  // Represents a UDP connection.
  // Such connections can be real, or simulated.
  class IUdpBlaster {

  public:

    IUdpBlaster(IClockSource* clock_);

    int Send(uint8_t* sendBuffer, size_t size, struct sockaddr* to);
    int Receive(uint8_t* buffer, sockaddr_in* from);

    // NOTE: I don't think that the blaster needs to have the clock.  Should be exposed by the client, probably....
    // Maybe the sim blaster?
    inline IClockSource* Clock() const { return _Clock; }

    // TEMP:
    inline sockaddr_in Addr() { return _Addr; }

    void UpdateStats();

  protected:

    // OPTIONS:
    int UpdateInterval = 1000;

    // TODO: Move to the endpoint.
    IClockSource* _Clock = nullptr;

    /// <summary>
    /// The address that we are sending data to....  Shouldn't this be 
    /// </summary>
    sockaddr_in _Addr;

    CNetworkStats Stats;

    // These need to be the 'internal' versions of send + receive.
    virtual int ReceiveImpl(uint8_t* receiveBuffer, sockaddr_in* from) = 0;
    virtual int SendImpl(uint8_t* sendBuffer, size_t size, struct sockaddr* to) = 0;
  };

  // ============================================================================================================
  class CUdpBlaster : public IUdpBlaster {

  public:
    CUdpBlaster(uint16_t port_, IClockSource* clock_);
    ~CUdpBlaster();

  protected:
    virtual int ReceiveImpl(uint8_t* receiveBuffer, sockaddr_in* from) override;
    virtual int SendImpl(uint8_t* sendBuffer, size_t size, struct sockaddr* to) override;

  private:
    uint16_t _Port = 0;
    SOCKET  _Socket = 0;
  };



  // Free functions....
  sockaddr_in CreateAddr(const std::string host, uint16_t port);
  sockaddr_in CreateAddr(const char* ip, uint16_t port);
  bool AreSame(const sockaddr_in& l, const sockaddr_in& r);

}
#endif // ! CUDPBlaster_H
