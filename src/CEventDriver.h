#pragma once

#ifndef CEVENT_H
#define CEVENT_H

#include <functional>
#include <vector>

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