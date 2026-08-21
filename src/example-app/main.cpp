#include "main.h"
#include "CClient.h"

#include <stdio.h>
#include "drewCo.Logging.hpp"
#include "drewCo.CLI.hpp"

#define DEL(x) { if(x) { delete(x); } }

using namespace funscrew::HFSO;
using namespace drewCo::Logging;
using namespace drewCo::CLI;

const size_t MAX_HOST = 128;

// ========================================================================================================================
class RunCommand : public ICommand {
public:
  virtual CommandValidationResult Validate() const override;
  virtual CCommandDef Configure() const override;

  // Parameters with defaults:
  uint16_t LocalPort = 5000;
  // uint16_t RemotePort = 5001;
  std::vector<std::string> RemotePlayers;
};

int RunClient(ICommand* cmd);
ICommand* FromToml(const toml::table& from);
void ParseAddress(const char* addr, char* host, UINT16* port);


// ------------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

  // Init the loggers....
  ILogger* csl = new CConsoleLogger();
  csl->AddLevel(Levels::Info);
  Log.AddLogger(csl);

  Log.Info("Intializing sockets...");

  int initOK = Platform::InitSockets();
  if (initOK) { return initOK; }

  RunCommand c;
  CParser parser;
  parser.Register(c, &FromToml, &RunClient);

  int res = parser.ExecuteCommandLine(argc, argv);
  return res;


  return 0;
}



// ------------------------------------------------------------------------------------------------------------------------
int RunClient(ICommand* cmd) {
  Log.Info("Running it!");

  auto useCmd = (RunCommand*)cmd;

  // Init components.  This is where someone could use test / fake stuff for testing.
  auto clock = new CSystemClock();
  auto blaster = new CUdpBlaster(useCmd->LocalPort, clock);

  // Open local port + add remote player....
  const int MAX_PLAYERS = 2;
  CClientOptions ops;
  ops.MaxPlayers = MAX_PLAYERS;
  CSimHFSOClient client(ops, blaster);


  // Crete + add a local player.
  {
    CPlayer p;
    CreateLocalPlayer("Bob", 0, &p);
    client.AddPlayer(p);
    Log.Info("A player was added!");
  }

  // Let's make a remote player now...
  {
    char host[MAX_HOST];
    uint16_t port;
    ParseAddress(useCmd->RemotePlayers[0].data(), (char*)(&host), &port);

    CPlayer r;
    CreateRemotePlayer("Dave", 1, host, port, &r);
    client.AddPlayer(r);

    Log.Info("A remote player was added!");
  }

  // Now we begin!
  client.Begin();


  // TODO: Game Loop:





  // client.

  // sockaddr_in    _peer_addr;
//  _peer_addr.sin_family = AF_INET;
//_peer_addr.sin_port = htons(port);
//inet_pton(AF_INET, ip, &_peer_addr.sin_addr.s_addr);

//if (!_peer_addr.sin_addr.s_addr) {
//  throw std::runtime_error("Invalid peer address!");
//}

//do {
//  _magic_number = (uint16)rand();
//} while (_magic_number == 0);
//poll.RegisterLoopSink(this);





  uint8_t count = client.GetPlayerCount();
  Log.Info("the listed player count is: %d", count);

  // DO the loop.
  // For example purposes, we will attempt to connect to our remote players....

  // Cleanup the base components.
  DEL(blaster);
  DEL(clock);

  return 0;
}


#include <chrono>
#include <cstdint>
#include <thread>

// ------------------------------------------------------------------------------------------------------------------------
int Run()
{
  using Clock = std::chrono::steady_clock;
  using duration = Clock::duration;
  using Rep = duration::rep;

  constexpr int64_t FPS_SCALE = 100;
  constexpr int64_t targetFPS = 6000; // 6000 FPS

  const auto sleepInterval = std::chrono::milliseconds(1);
  const auto busyWaitThreshold = std::chrono::milliseconds(1);

  // Clock ticks per frame are:
  //
  //     Duration::period::den * FPS_SCALE
  //     --------------------------------
  //     Duration::period::num * targetFPS
  //
  // --> ticks per second / (FPS / FPS_SCALE)
  constexpr int64_t tpfNumerator = duration::period::den * FPS_SCALE;
  constexpr int64_t tpfDenominator = duration::period::num * targetFPS;

  constexpr Rep wholeTicksPerFrame = static_cast<Rep>(tpfNumerator / tpfDenominator);
  constexpr int64_t remainderTpf = tpfNumerator % tpfDenominator;

  auto targetTime = Clock::now();

  // Leftover fractional frame time that is carried over.
  int64_t leftoverTicks = 0;

  bool running = true;

  while (running)
  {
    // ------------------------------------------------------------
    // Advance the target by exactly one rational frame interval.
    // ------------------------------------------------------------
    targetTime += duration(wholeTicksPerFrame);
    leftoverTicks += remainderTpf;

    if (leftoverTicks >= tpfDenominator)
    {
      const auto extraTicks = leftoverTicks / tpfDenominator;

      targetTime += duration(static_cast<Rep>(extraTicks));
      leftoverTicks %= tpfDenominator;
    }

    // ------------------------------------------------------------
    // Frame work.
    // ------------------------------------------------------------

    // ProcessInput();
    // Update();
    // Render();

    // ------------------------------------------------------------
    // Use remaining frame time for background work.
    // ------------------------------------------------------------

    for (;;)
    {
      const auto now = Clock::now();

      if (now >= targetTime)
      {
        break;
      }

      const auto remaining = targetTime - now;

      if (remaining <= busyWaitThreshold)
      {
        while (Clock::now() < targetTime)
        {
          // Busy wait.
        }

        break;
      }

      // Useful background work.
      //
      // Idle();
      // PumpNetwork();
      // ProcessStreaming();
      // ProcessBackgroundJobs();

      const auto availableSleepTime =
        remaining - busyWaitThreshold;

      if (availableSleepTime >= sleepInterval)
      {
        std::this_thread::sleep_for(sleepInterval);
      }
      else
      {
        std::this_thread::sleep_for(availableSleepTime);
      }
    }
  }

  return 0;
}


// ------------------------------------------------------------------------------------------------------------------------
CommandValidationResult RunCommand::Validate() const {
  CommandValidationResult res;

  return res;
}

// ------------------------------------------------------------------------------------------------------------------------
CCommandDef RunCommand::Configure() const {

  // Init CLI stuff:
  CCommandDef cd;
  cd.Name = "Options";
  cd.Alias = "run";
  cd.HelpText = "Run the application";

  CCommandOption local;
  local.Name = "LocalPort";
  local.Aliases.push_back("--local-port");
  local.HelpText = "Local Port";
  local.DataTypeValue = DataType::UInt16;
  local.DefaultValue = "5000";

  cd.Options.push_back(local);

  CCommandOption remote;
  remote.Name = "RemotePlayers";
  remote.Aliases.push_back("--remotes");
  remote.HelpText = "Remote Players";
  remote.DataTypeValue = DataType::StringArray;
  remote.IsRequired = true;
  // local.DefaultValue = "5001";

  cd.Options.push_back(remote);

  return cd;
}

// --------------------------------------------------------------------------------------------------------------------------
// SHARE:
void SplitString(const std::string& input, char delimiter, std::vector<std::string>& output)
{
  output.clear();

  std::string::size_type start = 0;

  while (true)
  {
    const std::string::size_type end = input.find(delimiter, start);

    if (end == std::string::npos)
    {
      output.emplace_back(input, start);
      break;
    }

    output.emplace_back(input, start, end - start);
    start = end + 1;
  }
}

// --------------------------------------------------------------------------------------------------------------------------
// SHARE:
void Trim(std::string& str)
{
  constexpr char whitespace[] = " \t\n\r\f\v";

  const size_t first = str.find_first_not_of(whitespace);

  if (first == std::string::npos)
  {
    str.clear();
    return;
  }

  const size_t last = str.find_last_not_of(whitespace);

  str.erase(last + 1);
  str.erase(0, first);
}

// --------------------------------------------------------------------------------------------------------------------------
void GetStringArray(const toml::v3::node& node, std::vector<std::string>& target)
{
  if (!node.is_array()) {
    throw std::runtime_error("node is not nan array!");
  }
  auto arr = node.as_array();
  size_t len = arr->size();
  for (size_t i = 0; i < len; i++)
  {
    auto next = arr->get(i);
    if (!next->is_string()) {
      throw std::runtime_error("array item is not a string!");
    }
    target.push_back(next->as_string()->get());
  }
}

// ------------------------------------------------------------------------------------------------------------------------
ICommand* FromToml(const toml::table& from) {
  auto res = new RunCommand();

  // We can use this as an example of how to generate code for our hydrated types...
  auto lpMatch = from.find("LocalPort");
  if (lpMatch != from.end()) {
    auto x = lpMatch->second.as_integer()->get();
    res->LocalPort = x;
  }

  auto rpMatch = from.find("RemotePlayers");
  if (rpMatch != from.end()) {
    GetStringArray(rpMatch->second, res->RemotePlayers);
  }
  else {
    // This is where some kind of fallback would come into play....
  }

  return res;
}






// ----------------------------------------------------------------------------------------------------------------------------------------------
// Parse the address of the form [host]:port into host (ip) and port.
void ParseAddress(const char* addr, char* host, UINT16* port)
{
  memset(host, 0, MAX_HOST);

  int pos = -1;
  size_t len = strnlen_s(addr, MAX_HOST);
  for (UINT i = 0; i < len; i++)
  {
    if (addr[i] == ':') { pos = i; break; }
  }
  if (pos == -1) {
    memcpy(host, addr, len);
  }
  else {
    if (pos > 0) {
      memcpy(host, addr, pos);
    }
    char portPart[MAX_HOST];
    strncpy_s(portPart, len - (pos + 1) + 1, addr + pos + 1, MAX_HOST);

    UINT16 usePort = std::atoi(portPart);
    if (usePort != 0) {
      *port = usePort;
    }

  }
}