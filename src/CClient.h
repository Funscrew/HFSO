#pragma once

#include <stdint.h>

// class HFPO::CEndpoint;
// class HFPO::CUdp;

//typedef char byte;

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

using Socket = SOCKET;
constexpr Socket INVALIDSOCKET = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using Socket = int;
constexpr Socket INVALIDSOCKET = -1;
#endif

constexpr int MAX_PACKET_SIZE = 4000;

namespace HFPO
{
    // Represents a UDP connection.
    // Such connections can be real, or simulated.
    class CUdpBlaster {

        virtual int Receive(byte* receiveBuffer) = 0;

        virtual int Send(byte* sendBuffer, int size, struct sockaddr* to) = 0;

    };

    // ============================================================================================================
    class CUdpClient : CUdpBlaster {

        public:
            CUdpClient(uint16_t port_);
            ~CUdpClient();

        virtual int Send(byte* sendBuffer, int size, struct sockaddr* to) override;

        private:
        uint16_t _Port = 0;
        Socket _Socket = 0;
    };

    // ============================================================================================================
    // A simulated UDP client for sending / receiving simulated udp traffic.
    // Used in testing scenarios when we want to be able to reliably recreate certain network conditions.
    class CSimUdpClient : CUdpBlaster {

    };

    // ============================================================================================================
    // HFPOClient is responsible for managing local and remote connections to other players and services.
    class CHFPOClient
    {
    };
}
