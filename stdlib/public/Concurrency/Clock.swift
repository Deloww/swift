//===----------------------------------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2021 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
import Swift

/// A mechanism in which to measure time, and delay work until a given point 
/// in time.
///
/// Types that conform to the `Clock` protocol define a concept of "now" which 
/// is the specific instant in time that property is accessed. Any pair of calls
/// to the `now` property may have a minimum duration between them - this
/// minimum resolution is exposed by the `minimumResolution` property to inform
/// any user of the type the expected granularity of accuracy. 
///
/// One of the primary uses for clocks is to schedule task sleeping. This method
/// resumes the calling task after a given deadline has been met or passed with
/// a given tolerance value. The tolerance is expected as a leeway around the 
/// deadline. The clock may reschedule tasks within the tolerance to ensure 
/// efficient execution of resumptions by reducing potential operating system
/// wake-ups. If no tolerance is specified (i.e. nil is passed in) the sleep
/// function is expected to schedule with a default tolerance strategy. 
///
/// For more information about specific clocks see `ContinuousClock` and 
/// `SuspendingClock`.
@available(SwiftStdlib 5.7, *)
public protocol Clock: Sendable {
  associatedtype Duration
  associatedtype Instant: InstantProtocol where Instant.Duration == Duration

  var now: Instant { get }
  var minimumResolution: Instant.Duration { get }

  func sleep(until deadline: Instant, tolerance: Instant.Duration?) async throws
}


@available(SwiftStdlib 5.7, *)
extension Clock {
  /// Measure the elapsed time to execute a closure.
  ///
  ///       let clock = ContinuousClock()
  ///       let elapsed = clock.measure {
  ///          someWork()
  ///       }
  @available(SwiftStdlib 5.7, *)
  public func measure(_ work: () throws -> Void) rethrows -> Instant.Duration {
    let start = now
    try work()
    let end = now
    return start.duration(to: end)
  }

  /// Measure the elapsed time to execute an asynchronous closure.
  ///
  ///       let clock = ContinuousClock()
  ///       let elapsed = await clock.measure {
  ///          await someWork()
  ///       }
  @available(SwiftStdlib 5.7, *)
  public func measure(
    _ work: () async throws -> Void
  ) async rethrows -> Instant.Duration {
    let start = now
    try await work()
    let end = now
    return start.duration(to: end)
  }
}

enum _ClockID: Int32 {
  case continuous = 1
  case suspending = 2
}

@available(SwiftStdlib 5.7, *)
@_silgen_name("swift_get_time")
internal func _getTime(
  seconds: UnsafeMutablePointer<Int64>,
  nanoseconds: UnsafeMutablePointer<Int64>,
  clock: CInt)

@available(SwiftStdlib 5.7, *)
@_silgen_name("swift_get_clock_res")
internal func _getClockRes(
  seconds: UnsafeMutablePointer<Int64>,
  nanoseconds: UnsafeMutablePointer<Int64>,
  clock: CInt)
