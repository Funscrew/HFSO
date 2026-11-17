#pragma once
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdexcept>

#include <chrono>

#if defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <timeapi.h>

using Socket = SOCKET;
constexpr Socket INVALIDSOCKET = INVALID_SOCKET;

namespace Platform
{

  // ------------------------------------------------------------------------------------------------------------------
  // TODO: Use this standard clock plz:
  // https://stackoverflow.com/questions/14783468/is-there-a-standard-library-equivalent-of-timegettime
  __forceinline int GetSystemTime()
  {
    return timeGetTime();
  }

  // ------------------------------------------------------------------------------------------------------------------
  // Any socket iniitialization code.
  __forceinline int InitSockets()
  {

    WSADATA wsaData;
    int startup = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startup)
    {
      auto errCode = WSAGetLastError();

      // TODO: drewco logging tools.....
      // printf("Could not initialize winsock because of an error: %d\n", errCode);
      return -1;
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------------------------------
  // Code to shutdown sockets (if any)
  __forceinline void ShutdownSockets() { throw std::runtime_error("not implemented!"); }

}

#elif defined(__linux__)

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cerrno>

// ALIASES re: sockets
using SOCKET = int;
using DWORD = uint32_t;

constexpr SOCKET INVALIDSOCKET = -1;
constexpr int SOCKET_ERROR = -1;
constexpr int WSAEWOULDBLOCK = EWOULDBLOCK;
constexpr int SO_DONTLINGER = SO_LINGER;
constexpr SOCKET INVALID_SOCKET = INVALIDSOCKET;

// ------------------------------------------------------------------------------------------------------------------
inline int closesocket(int socket)
{
  return close(socket);
}

// ------------------------------------------------------------------------------------------------------------------
inline int ioctlsocket(SOCKET socket, long command, unsigned long *argument)
{
  return ioctl(socket, command, argument);
}

// ------------------------------------------------------------------------------------------------------------------
inline int WSAGetLastError()
{
  return errno;
}

namespace Platform
{
  // ------------------------------------------------------------------------------------------------------------------
  inline int GetSystemTime()
  {
    return 0;
  }

  // ------------------------------------------------------------------------------------------------------------------
  // NOTE: Linux doesn't require socket initialization like windows does.
  inline int InitSockets()
  {
    
    return 0;
  }
}

#else
#error Unsupported Platform
#endif

#endif // #define PLATFORM_H
