// These tests are here to help develop the clients by using simulated UPD/TCP components.
#pragma once

#include <functional>

#include "drewCo.Logging.hpp"
#include "CClient.h"

class CSimClock;
class CSimUdpBlaster;

using namespace funscrew::HFSO;

extern drewCo::Logging::CLogMgr Log;

class CSimNetwork;

#define DEL(x) if(x) { delete(x); x = nullptr; }

namespace HFSO::Tests
{

  // ====================================================================================================================================
  struct CSetupOptions {
    CPlayer P1;        // NOTE: This is the local player for client 1, and remote player for client 2
    CPlayer P2;

    /// <summary>
    /// Should all clients use the same clock?
    /// Different clocks are used in test cases where we want to show that we can still sync if one or more clients are lagging behind.
    /// </summary>
    bool UseSameClock = true;

    size_t InputSize;
    uint8_t PlayerCount = 2;
  };

  // ====================================================================================================================================
  struct CTestContext {
    ~CTestContext();

    CSimNetwork* Network = nullptr;

    // NOTE: Eventually these will become arrays when we try testing with more than two players.
    // bool UseSameClock = false;
    CSimClock* Clock1 = nullptr;
    CSimUdpBlaster* Blaster1 = nullptr;

    // CSimClock* Clock2 = nullptr;
    CSimUdpBlaster* Blaster2 = nullptr;

    CSimHFSOClient* Client1 = nullptr;
    CSimHFSOClient* Client2 = nullptr;

    CSimHFSOClient* AllClients[2];
    uint32_t ClientCount = 2;

    int LastFrame = -1;

    size_t InputDataSize;
    size_t TotalInputDataSize = 0;
    byte* InputData = nullptr;


    // --------------------------------------------------------------------------------------------------------------------------
    /// <param name="setInputs">Callback for each frame and player so their inputs for the frame can be set.  Args are: input data, player index, currentTime</param>
    void RunGame(int totalTime, std::function<void(byte*, int, int)>* setInputs);
    void RunGame(int totalTime) { RunGame(totalTime, nullptr); }
    
    void RunStep(int interval, std::function<void(byte*, int, int)>* setInputs);

    bool RunFrame(CHFSOClient* client);
  };

  void InitTests();
  void SetupTestContext(CTestContext& context, const CSetupOptions& setupOps);
  void SetupDefaultTestContext(CTestContext& context);

  void CanConnectClients();
}