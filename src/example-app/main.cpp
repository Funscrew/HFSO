#include "main.h"
#include "CClient.h"

#include <stdio.h>

using namespace HFPO;


int main(int argc, char* argv[]) {

    //printf("yo\n");
    //printf("args are: %u\n", argc);
    //for (size_t i = 0; i < argc; i++)
    //{
    //    printf("arg: %u is: '%s'\n", i + 1, argv[i]);
    //}

#if WIN32
    WSADATA wsaData;
    int startup = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startup) {
        auto errCode = WSAGetLastError();

        // TODO: drewco logging tools.....
        printf("Could not initialize winsock because of an error: %d\n", errCode);
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

    printf("Using port: %u\n", usePort);

    CUdpClient c1 = CUdpClient(usePort);


    return 0;
}