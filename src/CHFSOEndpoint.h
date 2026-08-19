#pragma once

#include <string>
#include "Platform.h"
#include "CEventDriver.h"

// class funscrew::HFSO::CHFSOClient;

namespace funscrew::HFSO
{

  // ============================================================================================================
  struct CPlayer {

    /// <summary>
    /// The name of the player.
    /// </summary>
    std::string Name;

    /// <summary>
    /// Zero based index for each player.  0 == player 1, 1 == player 2 and so on.
    /// Negative numbers can be used for implementation specific reasons.
    /// </summary>
    int PlayerIndex = -1;

    bool IsLocal = false;

    // These are used for remote players.
    // NOTE: If/when we care about ipv6 support (and we should) this is where we will do the updates.
    struct sockaddr_in RemoteAddr;

    //CHFSOEndpoint* Endpoint = nullptr;
  };


  enum class EEndpointState
  {
    Available = 0,
    Connecting,
    Connected,
    Disconnecting,
    Disconnected
  };


  // ============================================================================================================
  struct CStateChangedArgs {
    EEndpointState OldState;
    EEndpointState NewState;
  };

  // ============================================================================================================
  /// <summary>
  /// Represents and endpoint to be used with CHFSO client.  This can be a player, a remote service (ReplayAppliance / LiveSpectate), or anything else that
  /// you may want to plug-in.
  /// </summary>
  /// <remarks>
  /// There is no real plugin support, or plans for pluggable endpoints at this time, despite what the description says.
  /// NOTE: Still in development, this may end up being exclusive to player endpoints...
  /// </remarks>
  class CHFSOEndpoint {

  public:

    
    void Init(CPlayer& data_);
    
    /// <summary>
    /// Clear player data + mark as available.
    /// </summary>
    void Complete();

    /// <summary>
    /// Attempt to connect to the other players.
    /// </summary>
    void BeginConnect();

    inline EEndpointState State() { return _State; }

    inline CPlayer* Data() { return &_Data; }

    CEventDriver<CStateChangedArgs> OnStateChanged;

  protected:
    EEndpointState _State = EEndpointState::Available;

    // Don't write to it!
    CPlayer _Data;


  };

}