#include "NetworkTests.h"
#include "CSimUdpBlaster.h"

#include "drewCo.Logging.hpp"

using namespace drewCo::Logging;


namespace HFSO::Tests
{
    // NOTE: This must be run before ALL tests.
    void InitTests() { 
        //int initOK = Platform::InitSockets();
        //if (!initOK) { throw std::runtime_error("Could not initialize sockets!"); }
    }

    // ----------------------------------------------------------------------------------------------------------------
    void CanConnectClients() { 
        // Log.Info("I am doing a test!");

        // OPTIONS:
        uint16_t localPort = 5000;
        uint16_t remotePort = 5001;

        auto clock = new CSimClock();
        auto blaster = new CSimUdpBlaster(localPort, clock);


    }

    // ----------------------------------------------------------------------------------------------------------------

    // ----------------------------------------------------------------------------------------------------------------

}