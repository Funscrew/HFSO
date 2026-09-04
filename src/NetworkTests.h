// These tests are here to help develop the clients by using simulated UPD/TCP components.
#pragma once

#include "drewCo.Logging.hpp"
#include "CClient.h"

using namespace funscrew::HFSO;

extern drewCo::Logging::CLogMgr Log;




namespace HFSO::Tests
{
    void InitTests();
    void CanConnectClients();
}