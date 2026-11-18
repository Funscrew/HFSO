#pragma once

#include <vector>

//namespace funscrew::HFSO
//{
//
//  // ============================================================================================================
//  // Used to signal events.
//  template <typename TArgs>
//
//  class CEventDriver
//  {
//  public:
//    using Handler = void (*)(void* sender, TArgs& args);
//
//  private:
//
//    std::vector<Handler> AllHandlers;
//
//  public:
//
//    // ----------------------------------------------------------------------------------------------------------------
//    // NOTE: We could set this function up to embed thunks if we wanted....
//    void Add(void handler(void* sender, TArgs& args))
//    {
//      // Search for space!
//      size_t len = AllHandlers.size();
//      for (size_t i = 0; i < len; i++)
//      {
//        if (!AllHandlers[i]) { 
//          AllHandlers[i] = handler;
//          return;
//        }
//      }
//
//      // Add a new one!
//      AllHandlers.push_back(handler);
//    }
//
//    // ----------------------------------------------------------------------------------------------------------------
//    void Remove(void handler(void* sender, TArgs& args))
//    {
//      size_t len = AllHandlers.size();
//      for (size_t i = 0; i < len; i++)
//      {
//        if (AllHandlers[i] == handler) {
//          AllHandlers[i] = nullptr;
//        }
//      }
//
//      // NOTE: We could do some extra book keeping to reduce the size of the vectors, but
//      // this isn't really made for that kind of thing.  This is sort of an easy-ish way to
//      // hook up some callbacks that will probably be permanent for the lifetime of the application.
//    }
//
//    // ----------------------------------------------------------------------------------------------------------------
//    void Trigger(void* sender, TArgs& args) {
//      size_t len = AllHandlers.size();
//      for (size_t i = 0; i < len; i++)
//      {
//        AllHandlers[i](sender, args);
//      }
//    }
//
//  };
//
//
//
//
//}


#ifndef CEVENT_H
#define CEVENT_H

#include <functional>
#include <vector>
#include <utility>

template<typename... TArgs>
class CEvent
{
public:
  using Handler = std::function<void(TArgs...)>;

private:
  std::vector<Handler> _Handlers;

public:
  template<typename TObject>
  void Add(TObject* instance, void (TObject::* method)(TArgs...))
  {
    _handlers.emplace_back(
      [instance, method](TArgs... args) {
        (instance->*method)(args...);
      }
    );
  }
  void Add(Handler handler)
  {
    _Handlers.push_back(std::move(handler));
  }

  void Trigger(TArgs... args)
  {
    for (auto& handler : _Handlers)
    {
      handler(args...);
    }
  }

  void Clear()
  {
    _Handlers.clear();
  }

  size_t Count() const
  {
    return _Handlers.size();
  }
};

#endif