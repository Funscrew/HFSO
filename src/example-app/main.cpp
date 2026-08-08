#include "main.h"
#include "CClient.h"

#include <stdio.h>
#include "drewCo.Logging.h"

// #include "logger

using namespace HFPO;
using namespace drewCo::Logging;

int main(int argc, char* argv[]) {

    // Init the loggers....
    ILogger* csl = new CConsoleLogger();
    csl->AddLevel(Levels::Info);

    Log.AddLogger(csl);

    Log.Info("Intializing sockets...");

#if WIN32
    WSADATA wsaData;
    int startup = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startup) {
        auto errCode = WSAGetLastError();

        // TODO: drewco logging tools.....
       // printf("Could not initialize winsock because of an error: %d\n", errCode);
        return -1;
    }
#else

#endif


    int usePort = 12345;
    if (argc >= 2) {
        if (!strcmp(argv[1], "target"))
        {
            usePort = 5001;
        }
    }

    Log.Info("Using port: %u\n", usePort);

    CUdpClient c1 = CUdpClient(usePort);


    return 0;
}