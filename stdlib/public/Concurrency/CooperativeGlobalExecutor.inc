///===--- CooperativeGlobalExecutor.inc ---------------------*- C++ -*--===///
///
/// This source file is part of the Swift.org open source project
///
/// Copyright (c) 2014 - 2020 Apple Inc. and the Swift project authors
/// Licensed under Apache License v2.0 with Runtime Library Exception
///
/// See https:///swift.org/LICENSE.txt for license information
/// See https:///swift.org/CONTRIBUTORS.txt for the list of Swift project authors
///
///===------------------------------------------------------------------===///
///
/// The implementation of the cooperative global executor.
///
/// This file is included into GlobalExecutor.cpp only when
/// the cooperative global executor is enabled.  It is expected to
/// declare the following functions:
///   swift_task_enqueueGlobalImpl
///   swift_task_enqueueGlobalWithDelayImpl
///   swift_task_enqueueMainExecutorImpl
/// as well as any cooperative-executor-specific functions in the runtime.
///
///===------------------------------------------------------------------===///

#include <chrono>
#include <thread>
#include "swift/Basic/ListMerger.h"

namespace {

struct JobQueueTraits {
  static Job *&storage(Job *cur) {
    return reinterpret_cast<Job*&>(cur->SchedulerPrivate[0]);
  }

  static Job *getNext(Job *job) {
    return storage(job);
  }
  static void setNext(Job *job, Job *next) {
    storage(job) = next;
  }
  static int compare(Job *lhs, Job *rhs) {
    return descendingPriorityOrder(lhs->getPriority(), rhs->getPriority());
  }
};
using JobQueueMerger = ListMerger<Job*, JobQueueTraits>;

using JobDeadline = std::chrono::time_point<std::chrono::steady_clock>;

template <bool = (sizeof(JobDeadline) <= sizeof(void*) &&
                  alignof(JobDeadline) <= alignof(void*))>
struct JobDeadlineStorage;

/// Specialization for when JobDeadline fits in SchedulerPrivate.
template <>
struct JobDeadlineStorage<true> {
  static JobDeadline &storage(Job *job) {
    return reinterpret_cast<JobDeadline&>(job->SchedulerPrivate[1]);
  }
  static JobDeadline get(Job *job) {
    return storage(job);
  }
  static void set(Job *job, JobDeadline deadline) {
    new(static_cast<void*>(&storage(job))) JobDeadline(deadline);
  }
  static void destroy(Job *job) {
    storage(job).~JobDeadline();
  }
};

/// Specialization for when JobDeadline doesn't fit in SchedulerPrivate.
template <>
struct JobDeadlineStorage<false> {
  static JobDeadline *&storage(Job *job) {
    return reinterpret_cast<JobDeadline*&>(job->SchedulerPrivate[1]);
  }
  static JobDeadline get(Job *job) {
    return *storage(job);
  }
  static void set(Job *job, JobDeadline deadline) {
    storage(job) = new JobDeadline(deadline);
  }
  static void destroy(Job *job) {
    delete storage(job);
  }
};

} // end anonymous namespace

static Job *JobQueue = nullptr;
static Job *DelayedJobQueue = nullptr;

/// Insert a job into the cooperative global queue.
SWIFT_CC(swift)
static void swift_task_enqueueGlobalImpl(Job *job) {
  assert(job && "no job provided");

  JobQueueMerger merger(JobQueue);
  merger.insert(job);
  JobQueue = merger.release();
}

/// Enqueues a task on the main executor.
SWIFT_CC(swift)
static void swift_task_enqueueMainExecutorImpl(Job *job) {
  // The cooperative executor does not distinguish between the main
  // queue and the global queue.
  swift_task_enqueueGlobalImpl(job);
}

/// Insert a job into the cooperative global queue with a delay.
SWIFT_CC(swift)
static void swift_task_enqueueGlobalWithDelayImpl(JobDelay delay,
                                                  Job *newJob) {
  assert(newJob && "no job provided");

  auto deadline = std::chrono::steady_clock::now()
                + std::chrono::duration_cast<JobDeadline::duration>(
                    std::chrono::nanoseconds(delay));
  JobDeadlineStorage<>::set(newJob, deadline);

  Job **position = &DelayedJobQueue;
  while (auto cur = *position) {
    // If we find a job with a later deadline, insert here.
    // Note that we maintain FIFO order.
    if (deadline < JobDeadlineStorage<>::get(cur)) {
      JobQueueTraits::setNext(newJob, cur);
      *position = newJob;
      return;
    }

    // Otherwise, keep advancing through the queue.
    position = &JobQueueTraits::storage(cur);
  }
  JobQueueTraits::setNext(newJob, nullptr);
  *position = newJob;
}

SWIFT_CC(swift)
static void swift_task_enqueueGlobalWithDeadlineImpl(long long sec,
                                                     long long nsec,
                                                     long long tsec,
                                                     long long tnsec,
                                                     int clock, Job *job) {
  assert(job && "no job provided");
  // TODO: implementation
}

/// Recognize jobs in the delayed-jobs queue that are ready to execute
/// and move them to the primary queue.
static void recognizeReadyDelayedJobs() {
  // Process all the delayed jobs.
  auto nextDelayedJob = DelayedJobQueue;
  if (!nextDelayedJob) return;

  auto now = std::chrono::steady_clock::now();
  JobQueueMerger readyJobs(JobQueue);

  // Pull jobs off of the delayed-jobs queue whose deadline has been
  // reached, and add them to the ready queue.
  while (nextDelayedJob &&
         JobDeadlineStorage<>::get(nextDelayedJob) <= now) {
    // Destroy the storage of the deadline in the job.
    JobDeadlineStorage<>::destroy(nextDelayedJob);

    auto next = JobQueueTraits::getNext(nextDelayedJob);
    readyJobs.insert(nextDelayedJob);
    nextDelayedJob = next;
  }

  JobQueue = readyJobs.release();
  DelayedJobQueue = nextDelayedJob;
}

/// Claim the next job from the cooperative global queue.
static Job *claimNextFromCooperativeGlobalQueue() {
  while (true) {
    // Move any delayed jobs that are now ready into the primary queue.
    recognizeReadyDelayedJobs();

    // If there's a job in the primary queue, run it.
    if (auto job = JobQueue) {
      JobQueue = JobQueueTraits::getNext(job);
      return job;
    }

    // If there are only delayed jobs left, sleep until the next deadline.
    // TODO: should the donator have some say in this?
    if (auto delayedJob = DelayedJobQueue) {
      auto deadline = JobDeadlineStorage<>::get(delayedJob);
      std::this_thread::sleep_until(deadline);
      continue;
    }

    return nullptr;
  }
}

void swift::
swift_task_donateThreadToGlobalExecutorUntil(bool (*condition)(void *),
                                             void *conditionContext) {
  while (!condition(conditionContext)) {
    auto job = claimNextFromCooperativeGlobalQueue();
    if (!job) return;
    swift_job_run(job, ExecutorRef::generic());
  }
}
