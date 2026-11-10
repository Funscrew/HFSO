#ifndef  CUDPBlaster_H
#define CUDPBlaster_H

#include "IClockSource.hpp"

constexpr size_t MAX_PACKET_SIZE = 4000;

namespace funscrew::HFSO
{

  // ============================================================================================================
  // Represents a UDP connection.
  // Such connections can be real, or simulated.
  class IUdpBlaster {

  public:

    IUdpBlaster(IClockSource* clock_);

    int Send(byte* sendBuffer, size_t size, struct sockaddr* to);
    int Receive(byte* buffer, sockaddr_in* from);

    // NOTE: I don't think that the blaster needs to have the clock.  Should be exposed by the client, probably....
    inline IClockSource* Clock() const { return _Clock; }

  protected:

    // TODO: Move to the endpoint.
    IClockSource* _Clock = nullptr;

    /// <summary>
    /// The address that we are sending data to....  Shouldn't this be 
    /// </summary>
    sockaddr_in _Addr;

    // These need to be the 'internal' versions of send + receive.
    virtual int ReceiveImpl(byte* receiveBuffer, sockaddr_in* from) = 0;
    virtual int SendImpl(byte* sendBuffer, size_t size, struct sockaddr* to) = 0;
  };

  // ============================================================================================================
  class CUdpBlaster : public IUdpBlaster {

  public:
    CUdpBlaster(uint16_t port_, IClockSource* clock_);
    ~CUdpBlaster();

  protected:
    virtual int ReceiveImpl(byte* receiveBuffer, sockaddr_in* from) override;
    virtual int SendImpl(byte* sendBuffer, size_t size, struct sockaddr* to) override;

  private:
    uint16_t _Port = 0;
    Socket _Socket = 0;
  };



  // Free functions....
  sockaddr_in CreateAddr(const std::string host, uint16_t port);
  sockaddr_in CreateAddr(const char* ip, uint16_t port);

}
#endif // ! CUDPBlaster_H
