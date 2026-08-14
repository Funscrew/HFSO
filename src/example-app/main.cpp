#include "main.h"
#include "CClient.h"

#include <stdio.h>
#include "drewCo.Logging.hpp"
#include "drewCo.CLI.hpp"


using namespace HFPO;
using namespace drewCo::Logging;
using namespace drewCo::CLI;

class RunCommand : public ICommand {
public:
  virtual CommandValidationResult Validate() const override;
  virtual CCommandDef Configure() const override;

  // Parameters with defaults:
  uint16_t LocalPort = 5000;
  uint16_t RemotePort = 5001;
};

int RunClient(ICommand* cmd);
ICommand* FromToml(const toml::table& from);


// ------------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

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
  throw std::runtime_error("not supported!");
#endif

  // Init the loggers....
  ILogger* csl = new CConsoleLogger();
  csl->AddLevel(Levels::Info);
  Log.AddLogger(csl);


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

  // Log.Info("port is: %u", useCmd->LocalPort);

  //CHFPOClient client;
  //client.


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
  remote.Name = "RemotePort";
  remote.Aliases.push_back("--remote-port");
  remote.HelpText = "remote Port";
  remote.DataTypeValue = DataType::UInt16;
  local.DefaultValue = "5001";

  cd.Options.push_back(remote);

  return cd;
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

  auto rpMatch = from.find("RemotePort");
  if (rpMatch != from.end()) {
    auto x = rpMatch->second.as_integer()->get();
    res->RemotePort = x;
  }

  return res;
}


