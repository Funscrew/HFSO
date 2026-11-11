#pragma once
#ifndef EZQ_HHP
#define EZQ_HHP

#include <assert.h>

// ==============================================================================================================
/// <summary>
/// This is a simple FIFO queue.
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="Capacity"></typeparam>
/// <remarks>Ported from some C# code in drewCo.Tools</remarks>
template<typename T, size_t Capacity>
class EZQ
{
private:
  T _Items[Capacity];
  size_t Start = 0;
  size_t End = 0;
  size_t _Count = 0;

public:
  static_assert(Capacity > 0);

  bool IsEmpty() const { return _Count == 0; }
  bool IsFull() const { return _Count == Capacity; }
  size_t Count() const { return _Count; }

  void Push(const T& item)
  {
    assert(!IsFull());

    _Items[End] = item;
    End = (End + 1) % Capacity;
    _Count++;
  }

  void Pop()
  {
    assert(!IsEmpty());

    Start = (Start + 1) % Capacity;
    _Count--;
  }

  T& First()
  {
    assert(!IsEmpty());
    return _Items[Start];
  }

  T& Last()
  {
    assert(!IsEmpty());
    return _Items[(End + Capacity - 1) % Capacity];
  }

  // -------------------------------------------------------------------------------------------------------------
  bool Empty() { 
    return _Count == 0;
  }

  // -------------------------------------------------------------------------------------------------------------
  void Clear() {
    _Count = 0;
    Start = 0;
    End = 0;
  }
};

#endif // !EZQ_HHP
