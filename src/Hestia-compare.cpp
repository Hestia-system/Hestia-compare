#include "Hestia-compare.h"
#include <cstring>
#include <cmath>
#include <cstdlib>

namespace Compare
{

  const int MAX_EVENTS = 32;

  struct State
  {
    Id id;
    bool oldBool, nowBool;
    int oldInt, nowInt;
    float oldFloat, nowFloat;
    const char *oldStr;
    const char *nowStr;

    // Latches
    bool latch_diff, latch_rise, latch_fall;
    bool latch_diff_int, latch_increasing_int, latch_decreasing_int;
    bool latch_equal_int, latch_greater_int, latch_less_int, latch_outOfRange_int;
    bool latch_diff_float, latch_increasing_float, latch_decreasing_float;
    bool latch_equal_float, latch_greater_float, latch_less_float, latch_outOfRange_float;
    bool latch_diffStr, latch_equal_str;
  };

  static State states[MAX_EVENTS];
  static int num_events = 0;

  // forward declaration for internal helper
  static void resetLatches(State &s);

  State &getState(Id id)
  {
    for (int i = 0; i < num_events; ++i)
    {
      if (states[i].id == id)
        return states[i];
    }
    // Not found, add new if possible
    if (num_events < MAX_EVENTS)
    {
      states[num_events].id = id;
      // Initialize to defaults
      states[num_events].oldBool = false;
      states[num_events].nowBool = false;
      states[num_events].oldInt = 0;
      states[num_events].nowInt = 0;
      states[num_events].oldFloat = 0.0f;
      states[num_events].nowFloat = 0.0f;
      states[num_events].oldStr = "";
      states[num_events].nowStr = "";
      resetLatches(states[num_events]);
      return states[num_events++];
    }
    // Exhausted, return first as dummy (or assert, but for now dummy)
    return states[0];
  }

  void resetLatches(State &s)
  {
    s.latch_diff = false;
    s.latch_rise = false;
    s.latch_fall = false;
    s.latch_diff_int = false;
    s.latch_increasing_int = false;
    s.latch_decreasing_int = false;
    s.latch_equal_int = false;
    s.latch_greater_int = false;
    s.latch_less_int = false;
    s.latch_outOfRange_int = false;
    s.latch_diff_float = false;
    s.latch_increasing_float = false;
    s.latch_decreasing_float = false;
    s.latch_equal_float = false;
    s.latch_greater_float = false;
    s.latch_less_float = false;
    s.latch_outOfRange_float = false;
    s.latch_diffStr = false;
    s.latch_equal_str = false;
  }

  // Read implementations
  bool Read::oldBool() const { return getState(_id).oldBool; }
  bool Read::nowBool() const { return getState(_id).nowBool; }
  int Read::oldInt() const { return getState(_id).oldInt; }
  int Read::nowInt() const { return getState(_id).nowInt; }
  int Read::deltaInt() const { return getState(_id).nowInt - getState(_id).oldInt; }
  int Read::absDeltaInt() const { return std::abs(getState(_id).nowInt - getState(_id).oldInt); }
  float Read::oldFloat() const { return getState(_id).oldFloat; }
  float Read::nowFloat() const { return getState(_id).nowFloat; }
  float Read::deltaFloat() const { return getState(_id).nowFloat - getState(_id).oldFloat; }
  float Read::absDeltaFloat() const { return std::fabs(getState(_id).nowFloat - getState(_id).oldFloat); }
  const char *Read::oldStr() const { return getState(_id).oldStr; }
  const char *Read::nowStr() const { return getState(_id).nowStr; }

  // Event implementations
  void Event::write(bool value)
  {
    State &s = getState(_id);
    s.oldBool = s.nowBool;
    s.nowBool = value;
    resetLatches(s);
  }

  void Event::write(int value)
  {
    State &s = getState(_id);
    s.oldInt = s.nowInt;
    s.nowInt = value;
    resetLatches(s);
  }

  void Event::write(float value)
  {
    State &s = getState(_id);
    s.oldFloat = s.nowFloat;
    s.nowFloat = value;
    resetLatches(s);
  }

  void Event::write(const char *value)
  {
    State &s = getState(_id);
    s.oldStr = s.nowStr;
    s.nowStr = value;
    resetLatches(s);
  }

  bool Event::diff()
  {
    State &s = getState(_id);
    if (!s.latch_diff && (s.nowBool != s.oldBool))
    {
      s.latch_diff = true;
      return true;
    }
    return false;
  }

  bool Event::rise()
  {
    State &s = getState(_id);
    if (!s.latch_rise && (s.oldBool == false && s.nowBool == true))
    {
      s.latch_rise = true;
      return true;
    }
    return false;
  }

  bool Event::fall()
  {
    State &s = getState(_id);
    if (!s.latch_fall && (s.oldBool == true && s.nowBool == false))
    {
      s.latch_fall = true;
      return true;
    }
    return false;
  }

  bool Event::diff(int delta)
  {
    State &s = getState(_id);
    if (!s.latch_diff_int && (std::abs(s.nowInt - s.oldInt) > delta))
    {
      s.latch_diff_int = true;
      return true;
    }
    return false;
  }

  bool Event::increasing(int delta)
  {
    State &s = getState(_id);
    if (!s.latch_increasing_int && (s.nowInt > s.oldInt + delta))
    {
      s.latch_increasing_int = true;
      return true;
    }
    return false;
  }

  bool Event::decreasing(int delta)
  {
    State &s = getState(_id);
    if (!s.latch_decreasing_int && (s.nowInt < s.oldInt - delta))
    {
      s.latch_decreasing_int = true;
      return true;
    }
    return false;
  }

  bool Event::equal(int target, int tol)
  {
    State &s = getState(_id);
    bool old_in = (s.oldInt >= target - tol && s.oldInt <= target + tol);
    bool now_in = (s.nowInt >= target - tol && s.nowInt <= target + tol);
    if (!s.latch_equal_int && !old_in && now_in)
    {
      s.latch_equal_int = true;
      return true;
    }
    return false;
  }

  bool Event::greater(int target, int tol)
  {
    State &s = getState(_id);
    if (!s.latch_greater_int && s.oldInt <= target + tol && s.nowInt > target + tol)
    {
      s.latch_greater_int = true;
      return true;
    }
    return false;
  }

  bool Event::less(int target, int tol)
  {
    State &s = getState(_id);
    if (!s.latch_less_int && s.oldInt >= target - tol && s.nowInt < target - tol)
    {
      s.latch_less_int = true;
      return true;
    }
    return false;
  }

  bool Event::outOfRange(int min, int max)
  {
    State &s = getState(_id);
    if (!s.latch_outOfRange_int && (s.nowInt < min || s.nowInt > max))
    {
      s.latch_outOfRange_int = true;
      return true;
    }
    return false;
  }

  bool Event::diff(float delta)
  {
    State &s = getState(_id);
    if (!s.latch_diff_float && (std::fabs(s.nowFloat - s.oldFloat) > delta))
    {
      s.latch_diff_float = true;
      return true;
    }
    return false;
  }

  bool Event::increasing(float delta)
  {
    State &s = getState(_id);
    if (!s.latch_increasing_float && (s.nowFloat > s.oldFloat + delta))
    {
      s.latch_increasing_float = true;
      return true;
    }
    return false;
  }

  bool Event::decreasing(float delta)
  {
    State &s = getState(_id);
    if (!s.latch_decreasing_float && (s.nowFloat < s.oldFloat - delta))
    {
      s.latch_decreasing_float = true;
      return true;
    }
    return false;
  }

  bool Event::equal(float target, float tol)
  {
    State &s = getState(_id);
    bool old_in = (s.oldFloat >= target - tol && s.oldFloat <= target + tol);
    bool now_in = (s.nowFloat >= target - tol && s.nowFloat <= target + tol);
    if (!s.latch_equal_float && !old_in && now_in)
    {
      s.latch_equal_float = true;
      return true;
    }
    return false;
  }

  bool Event::greater(float target, float tol)
  {
    State &s = getState(_id);
    if (!s.latch_greater_float && s.oldFloat <= target + tol && s.nowFloat > target + tol)
    {
      s.latch_greater_float = true;
      return true;
    }
    return false;
  }

  bool Event::less(float target, float tol)
  {
    State &s = getState(_id);
    if (!s.latch_less_float && s.oldFloat >= target - tol && s.nowFloat < target - tol)
    {
      s.latch_less_float = true;
      return true;
    }
    return false;
  }

  bool Event::outOfRange(float min, float max)
  {
    State &s = getState(_id);
    if (!s.latch_outOfRange_float && (s.nowFloat < min || s.nowFloat > max))
    {
      s.latch_outOfRange_float = true;
      return true;
    }
    return false;
  }

  bool Event::diffStr()
  {
    State &s = getState(_id);
    if (!s.latch_diffStr && (std::strcmp(s.nowStr, s.oldStr) != 0))
    {
      s.latch_diffStr = true;
      return true;
    }
    return false;
  }

  bool Event::equal(const char *value)
  {
    State &s = getState(_id);
    if (!s.latch_equal_str && (std::strcmp(s.oldStr, value) != 0 && std::strcmp(s.nowStr, value) == 0))
    {
      s.latch_equal_str = true;
      return true;
    }
    return false;
  }

  Event &event(Id id)
  {
    // single reusable Event object; id is updated on each call
    static Event e(0);
    e._id = id; // event() is friend of Event, can modify
    return e;
  }

} // namespace Compare