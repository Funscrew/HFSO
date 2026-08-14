
#if defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <timeapi.h>

using Socket = SOCKET;
constexpr Socket INVALIDSOCKET = INVALID_SOCKET;


namespace Platform {
  // ------------------------------------------------------------------------------------------------------------------
  int GetSystemTime() {
    return timeGetTime();
  }
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

