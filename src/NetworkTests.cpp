#include "NetworkTests.h"
#include "CSimUdpBlaster.h"

#include "drewCo.Logging.hpp"

using namespace drewCo::Logging;


const int RNG_SEED_1 = 12345;
const int RNG_SEED_2 = 67890;


namespace HFSO::Tests
{
  template <typename T>
  void AssertEqual(T l, T r) { 
    if (l != r)  {
      throw std::exception("Values are not equal!");
    }
  }

  // ----------------------------------------------------------------------------------------------------------------
  // NOTE: This must be run before ALL tests.
  void InitTests() {
    //int initOK = Platform::InitSockets();
    //if (!initOK) { throw std::runtime_error("Could not initialize sockets!"); }
  }


  // ----------------------------------------------------------------------------------------------------------------
  void CanConnectClients() {

    CTestContext context;
    SetupDefaultTestContext(context);

    // run a loop....
    context.RunGame(100);

    // Now we have to make sure that the clients are actually connected.....
    auto c1 = context.Client1;
    EClientState s1 = c1->State();
    AssertEqual(s1, EClientState::Active);

    //if (s1 != EClientState::Active) {
    //  throw std::exception("Client one state should be active!");
    //}

    auto c2 = context.Client2;



    int x = 10;

  }

  // ----------------------------------------------------------------------------------------------------------------
  void SetupDefaultTestContext(CTestContext& context)
  {
    // Add local + remote player....
    CPlayer local;
    local.IsLocal = true;
    local.Name = "Screwie";
    local.PlayerIndex = 0;
    local.Host = "127.0.0.1";
    local.Port = 5000;

    CPlayer remote;
    remote.IsLocal = false;
    remote.Name = "Dave";
    remote.PlayerIndex = 1;
    remote.Host = "127.0.0.1";
    remote.Port = 5001;

    CSetupOptions ops;
    ops.P1 = local;
    ops.P2 = remote;
    ops.InputSize = 5;

    SetupTestContext(context, ops);
  }

  // ----------------------------------------------------------------------------------------------------------------
  void SetupTestContext(CTestContext& context, const CSetupOptions& setupOps)
  {
    CClientOptions ops;
    ops.MaxPlayers = setupOps.PlayerCount;
    ops.InputDataSize = 5;        // This is default input size for FBNEO.

    auto clock = new CSimClock();
    context.Network = new CSimNetwork(clock);

    // context.UseSameClock = setupOps.UseSameClock;
    context.Clock1 = clock;
    context.Blaster1 = new CSimUdpBlaster(setupOps.P1.Port, context.Clock1, context.Network);
    context.Client1 = new CSimHFSOClient(ops, context.Blaster1, RNG_SEED_1);

    // TODO: A setup option to use different clocks:
    // context.Clock2 = context.UseSameClock ? context.Clock1 : new CSimClock();
    context.Blaster2 = new CSimUdpBlaster(setupOps.P2.Port, clock, context.Network);
    context.Client2 = new CSimHFSOClient(ops, context.Blaster2, RNG_SEED_2);

    context.AllClients[0] = context.Client1;
    context.AllClients[1] = context.Client2;

    // Now add the players:
    {
      CPlayer local = setupOps.P1;
      local.IsLocal = true;

      CPlayer remote = setupOps.P2;
      remote.IsLocal = false;

      context.Client1->AddPlayer(local);
      context.Client1->AddPlayer(remote);
      context.Client1->Start();
    }

    {
      CPlayer local = setupOps.P2;
      local.IsLocal = true;

      CPlayer remote = setupOps.P1;
      remote.IsLocal = false;
      context.Client2->AddPlayer(local);
      context.Client2->AddPlayer(remote);

      // NOTE: We could add some kind of start delay here too?
      context.Client2->Start();
    }


  }


  // ----------------------------------------------------------------------------------------------------------------
  const int TIME_INTERVAL = 1;
  void  CTestContext::RunGame(int totalTime, std::function<void(byte*, int, int)>* setInputs)
  {
    for (int curTime = 0; curTime < totalTime; curTime++)
    {
      RunStep(TIME_INTERVAL, setInputs);
    }
  }

  // ----------------------------------------------------------------------------------------------------------------
  const int FRAME_INTERVAL = 16;
  void CTestContext::RunStep(int interval, std::function<void(byte*, int, int)>* setInputs)
  {
    // NOTE: We have to do something about when there are different clocks.....
    Clock1->AddTime(interval);
    int curTime = Clock1->GetTime();

    //if (ReplayAppliance != null)
    //{
    //  ReplayAppliance.Update();
    //}

    if (curTime % FRAME_INTERVAL == 0)
    {
      // This is where a normal game frame would be processed.
      // TODO: I want to change the inputs per frame.  Data doesn't matter, just that it can be exchanged.
      // int len = ;  // HINT: (Client1 + Client2).Count;
      for (int clientIndex = 0; clientIndex < ClientCount; clientIndex++)
      {
        auto client = AllClients[clientIndex];

        if (setInputs)
        {
          (*setInputs)(nullptr, 1, 2); //  (InputBuffers[clientIndex], client.GetLocalPlayer().PlayerIndex, curTime);
        }

        RunFrame(client); //, clientIndex, TotalInputDataSize);

        ++LastFrame;
      }

    }
    else
    {
      // TODO: A proper idle() function.....    (see Program.cs for example)
      // This is where we would send out the player inputs and so on....
      for (int j = 0; j < ClientCount; j++)
      {
        auto c = AllClients[j];
        c->Idle();
      }
    }
  }


  // ------------------------------------------------------------------------------------------------------
  bool CTestContext::RunFrame(CHFSOClient* client)
  {
    if (!client->IsSynchronizing())
    {
      bool syncOK = client->SyncInput(); ///testInput, INPUT_SIZE, GGPOConsts.MAX_PLAYERS);

      // Tell the client that we have moved ahead one frame.
      if (syncOK)
      {
        client->IncrementFrame();
      }
      return syncOK;
    }

    return false;
  }

  //// ------------------------------------------------------------------------------------------------------
  //public static bool RunFrame(GGPOClient c, byte[] testInput)
  //{
  //  if (!c._synchronizing)
  //  {
  //    bool syncOK = c.SyncInput(testInput, INPUT_SIZE, GGPOConsts.MAX_PLAYERS);

  //    // Tell the client that we have moved ahead one frame.
  //    if (syncOK)
  //    {
  //      c.IncrementFrame();
  //    }
  //    return syncOK;
  //  }
  //  return false;
  //}



  // ----------------------------------------------------------------------------------------------------------------
  CTestContext::~CTestContext() {

    DEL(Clock1);
    //if (UseSameClock) {
    //  Clock2 = nullptr;
    //}

    DEL(Network);
    DEL(Blaster1);
    // DEL(Clock2);
    DEL(Blaster2);
    DEL(Client1);
    DEL(Client2);
  }


}

//{

//  // The total number of 'frames' that we want to simulate in this case.
//  //  const int MAX_FRAMES = 50;
//  for (int curTime = 0; curTime < totalTime; curTime++)
//  {
//    RunStep(TIME_INTERVAL);
//  }

//}
