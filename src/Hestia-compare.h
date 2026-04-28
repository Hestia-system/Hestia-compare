// ============================================================================
// Hestia-Compare.h
// ============================================================================
//
// OSS_DOC (English)
// ---------
// Hestia Compare provides a small, deterministic, cycle-synchronous event engine
// for "old/new" comparisons in embedded systems.
//
// Concept
// -------
// Compare is designed for firmware structured around a deterministic loop()
// with clearly separated phases:
//
//   (1) Acquisition phase:  event(id).write(value)
//   (2) Decision phase:     event(id).diff()/rise()/equal(...)/increasing(...)
//
// Compare stores two samples per Id:
//   - old : previous written value
//   - now : last written value
// Numeric diff(delta), increasing(delta), and decreasing(delta) also store
// their own reference values, initialized by the first write() and refreshed
// only when the corresponding detector returns true.
//
// Event methods are "latched per write":
//   - Each event can return true at most once after a write().
//   - The latch is reset by the next write() for the same Id.
//   - Different detectors are independent (diff/rise/fall/... do not interfere).
//
// Compare does NOT:
//   - trigger callbacks
//   - enqueue events
//   - schedule tasks
//   - write values implicitly
//
// It is meant to compose cleanly with HestiaTempo and FSM logic.
//
// Notes on >= / <=
// ---------------
// Inclusive operators (>=, <=) are intentionally not provided as event primitives.
// They are ambiguous in event semantics. If needed, use composition:
//   equal(target,tol) || greater(target,tol)
// or use read().now() for state checks.
//
// ============================================================================

#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace Compare {

  // ==========================================================================
  // Types
  // ==========================================================================

  /// @brief Stable identifier type (same spirit as HestiaTempo Id).
  /// Use a compile-time hash or user literal in your project if available.
  using Id = uint32_t;

  // ==========================================================================
  // Read View (Introspection)
  // ==========================================================================

  /// @brief Read-only access to the (old, now) snapshot and derived deltas.
  ///
  /// OSS_DOC:
  /// - This view has NO side effects.
  /// - Values remain stable until the next write() for the same Id.
  /// - delta()/absDelta() are meaningful only for numeric types.
  class Read {
  public:
    // ---- bool ----
    bool oldBool() const;
    bool nowBool() const;

    // ---- int ----
    int  oldInt() const;
    int  nowInt() const;
    int  deltaInt() const;     // now - old
    int  absDeltaInt() const;  // abs(now - old)

    // ---- float ----
    float oldFloat() const;
    float nowFloat() const;
    float deltaFloat() const;     // now - old
    float absDeltaFloat() const;  // fabs(now - old)

    // ---- string ----
    const char* oldStr() const;
    const char* nowStr() const;

  private:
    friend class Event;
    explicit Read(Id id) : _id(id) {}
    Id _id;
  };

  // ==========================================================================
  // Event (Stateful, Latched-per-write)
  // ==========================================================================

  /// @brief Stateful comparison engine bound to an Id (singleton per Id).
  ///
  /// OSS_DOC:
  /// - Call write(...) during the acquisition phase.
  /// - Call diff/rise/fall/increasing/equal/... during the decision phase.
  /// - Each detector is latched: once it returns true for the current (old, now),
  ///   it will return false until the next write() (same detector, same Id).
  ///
  /// Safety:
  /// - Not thread-safe. Intended for single-threaded Arduino loop().
  /// - Do not call write() while consuming events in the same phase.
  class Event {
  public:
    // ------------------------------------------------------------------------
    // Write (Acquisition)
    // ------------------------------------------------------------------------

    /// @brief Write a boolean sample (updates old/now; resets event latches).
    void write(bool value);

    /// @brief Write an integer sample (updates old/now; resets event latches).
    void write(int value);

    /// @brief Write a float sample (updates old/now; resets event latches).
    void write(float value);

    /// @brief Write a C-string sample (pointer/value semantics defined by impl).
    ///
    /// OSS_DOC:
    /// - The implementation may store a copy or store the pointer.
    /// - If you pass a transient pointer, you must guarantee lifetime if the
    ///   implementation stores the pointer. Prefer stable buffers or string
    ///   literals unless your implementation explicitly copies.
    void write(const char* value);

    // ------------------------------------------------------------------------
    // Bool Events (latched)
    // ------------------------------------------------------------------------

    /// @brief Event: old != now
    bool diff();

    /// @brief Event: false -> true
    bool rise();

    /// @brief Event: true -> false
    bool fall();

    // ------------------------------------------------------------------------
    // Numeric Events — Variation (reference = last detector trigger) (latched)
    // ------------------------------------------------------------------------

    /// @brief Event (int): |now - reference| > delta.
    ///
    /// The reference is initialized on the first write() and updated only when
    /// this detector returns true.
    bool diff(int delta);

    /// @brief Event (int): |now - old| > delta.
    bool sampleDiff(int delta);

    /// @brief Event (int): now > reference + delta.
    ///
    /// The reference is initialized on the first write() and updated only when
    /// this detector returns true.
    bool increasing(int delta);

    /// @brief Event (int): now > old + delta.
    bool sampleIncreasing(int delta);

    /// @brief Event (int): now < reference - delta.
    ///
    /// The reference is initialized on the first write() and updated only when
    /// this detector returns true.
    bool decreasing(int delta);

    /// @brief Event (int): now < old - delta.
    bool sampleDecreasing(int delta);

    /// @brief Event (float): |now - reference| > delta.
    ///
    /// The reference is initialized on the first write() and updated only when
    /// this detector returns true.
    bool diff(float delta);

    /// @brief Event (float): |now - old| > delta.
    bool sampleDiff(float delta);

    /// @brief Event (float): now > reference + delta.
    ///
    /// The reference is initialized on the first write() and updated only when
    /// this detector returns true.
    bool increasing(float delta);

    /// @brief Event (float): now > old + delta.
    bool sampleIncreasing(float delta);

    /// @brief Event (float): now < reference - delta.
    ///
    /// The reference is initialized on the first write() and updated only when
    /// this detector returns true.
    bool decreasing(float delta);

    /// @brief Event (float): now < old - delta.
    bool sampleDecreasing(float delta);

    // ------------------------------------------------------------------------
    // Numeric Events — Target Crossing (reference = target) (latched)
    // ------------------------------------------------------------------------

    /// @brief Event (int): enters target band: old خارج band, now داخل band.
    /// Band is [target - tol, target + tol].
    bool equal(int target, int tol = 0);

    /// @brief Event (int): crosses upward beyond (target + tol).
    /// Condition: old <= target + tol  AND  now > target + tol
    bool greater(int target, int tol = 0);

    /// @brief Event (int): crosses downward below (target - tol).
    /// Condition: old >= target - tol  AND  now < target - tol
    bool less(int target, int tol = 0);

    /// @brief Event (float): enters target band [target - tol, target + tol].
    bool equal(float target, float tol = 0.0f);

    /// @brief Event (float): crosses upward beyond (target + tol).
    bool greater(float target, float tol = 0.0f);

    /// @brief Event (float): crosses downward below (target - tol).
    bool less(float target, float tol = 0.0f);

    // ------------------------------------------------------------------------
    // Numeric Events — Absolute Range (latched)
    // ------------------------------------------------------------------------

    /// @brief Event (int): now < min || now > max
    bool outOfRange(int min, int max);

    /// @brief Event (float): now < min || now > max
    bool outOfRange(float min, float max);

    // ------------------------------------------------------------------------
    // String Events (latched)
    // ------------------------------------------------------------------------

    /// @brief Event: oldStr != nowStr (string compare).
    bool diffStr();

    /// @brief Event: now becomes equal to value (old != value && now == value).
    bool equal(const char* value);

    // ------------------------------------------------------------------------
    // Read-only Introspection
    // ------------------------------------------------------------------------

    /// @brief Access the stable (old, now) snapshot and derived deltas.
    Read read() const { return Read(_id); }

  private:
    friend Event& event(Id id);
    explicit Event(Id id) : _id(id) {}
    Id _id;
  };

  // ==========================================================================
  // Factory
  // ==========================================================================

  /// @brief Get the singleton Event instance for a given Id.
  ///
  /// OSS_DOC:
  /// - The implementation stores a small fixed number of slots.
  /// - If slots are exhausted, behavior is defined by implementation (typically
  ///   returns a valid object but with operations as no-ops, or asserts in debug).
  Event& event(Id id);

  // ==========================================================================
  // Optional: Stateless Operators (pure comparisons)
  // ==========================================================================
  //
  // If you later want purely stateless comparisons, keep them separate from
  // event() to avoid semantic ambiguity. Example:
  //
  // namespace op {
  //   bool ge(int a, int b);     // state check (>=)
  //   bool le(int a, int b);     // state check (<=)
  // }
  //
  // Not implemented here by design.
  //

} // namespace Compare
