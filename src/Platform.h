#pragma once 

#include <stdexcept>

#if defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <timeapi.h>

using Socket = SOCKET;
constexpr Socket INVALIDSOCKET = INVALID_SOCKET;

namespace Platform {


  // ------------------------------------------------------------------------------------------------------------------
  // TODO: Use this standard clock plz:
  // https://stackoverflow.com/questions/14783468/is-there-a-standard-library-equivalent-of-timegettime
  __forceinline int GetSystemTime() {
    return timeGetTime();
  }

  // ------------------------------------------------------------------------------------------------------------------
  // Any socket iniitialization code.
  __forceinline int InitSockets() {

    WSADATA wsaData;
    int startup = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startup) {
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

using Socket = int;
constexpr Socket INVALIDSOCKET = -1;


namespace Platform {
  // ------------------------------------------------------------------------------------------------------------------
  int GetSystemTime() {
    return timeGetTime();
  }
}

#else 
#error Unsupported Platform
#endif

