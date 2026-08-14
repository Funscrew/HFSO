#include "platform.h"

// ============================================================================================================
class IClockSource {

public:
  virtual int GetTime() = 0;
  virtual int Elapsed() = 0;
};

// ============================================================================================================
class SystemClock : public IClockSource {
private:
  int _LastTime = 0;

public:
  // ----------------------------------------------------------------------------------------------------
  virtual int GetTime() override {
    int res = Platform::GetSystemTime();
    return res;
  }

  // ----------------------------------------------------------------------------------------------------
  int Elapsed() {
    int res = GetTime() - _LastTime;
    return res;
  }
};

// ============================================================================================================
// A clock that we can use to simulate the passage of time, to whatever we want.
class SimClock : public IClockSource {
private:
  int _CurTime = 0;
  int _LastTime = 0;

public:
  virtual int GetTime() override { return _CurTime; };

  virtual int Elapsed() override {
    return _CurTime - _LastTime;
  }

  int AddTime(int amount)
  {
    _LastTime = _CurTime;
    _CurTime += amount; return _CurTime;
  }
};
