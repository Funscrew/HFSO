#include "CClient.h"

// #include <stdarg.h>
#include <stdexcept>

#include "errno.h"


using namespace HFPO;




// ------------------------------------------------------------------------------------------------
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
CUdpClient::CUdpClient(uint16_t port_) :
    _Port(port_)
{
    _Socket = CreateSocket(_Port, 3);
}

// ------------------------------------------------------------------------------------------------
int CUdpClient::Send(byte* sendBuffer, int len, struct sockaddr* to) {

    int res = sendto(_Socket, reinterpret_cast<const char*>(sendBuffer), len, 0, to, sizeof to);
    return res;

    return 0;
}

// ------------------------------------------------------------------------------------------------
CUdpClient::~CUdpClient() {
    if (_Socket != INVALIDSOCKET) {
        // Blow it up here!
        closesocket(_Socket);
    }
    _Socket = INVALIDSOCKET;
}