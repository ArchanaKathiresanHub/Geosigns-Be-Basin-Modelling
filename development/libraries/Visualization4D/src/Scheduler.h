//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <atomic>
#include <thread>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

struct TaskSource;

struct Task
{
  enum Affinity
  {
    IOTASK,
    CPUTASK
  } affinity = CPUTASK;

  int type = 0; //source-specific type identifier

  TaskSource* source = nullptr;

  std::atomic<bool> error = false;
  std::atomic<bool> canceled = false;
  std::atomic<bool> finished = false;

  virtual ~Task() {}

  virtual void run() = 0;
};

struct TaskSource
{
  virtual void onTaskCompleted(std::shared_ptr<Task> task) = 0;
};

class Scheduler
{
  struct Queue
  {
    std::queue<std::shared_ptr<Task>> items;
    std::condition_variable cv;
    std::mutex mutex;
  };

  Queue m_ioQueue;
  Queue m_cpuQueue;
  
  std::queue<std::shared_ptr<Task>> m_readyQueueWrite;
  std::queue<std::shared_ptr<Task>> m_readyQueueRead;
  std::mutex m_readyQueueMutex;

  std::vector<std::thread> m_ioThreads;
  std::vector<std::thread> m_cpuThreads;

  bool m_started;
  bool m_stop;

  std::shared_ptr<Task> getTask(Queue& q);

  void run(Queue& q);

public:

  Scheduler();

  /**
   * Start processing tasks
   */
  void start();

  /**
   * Stop processing tasks
   */
  void stop();

  /**
   * Enqueue a task for processing in one of the worker threads
   */
  void put(std::shared_ptr<Task> task);

  /**
   * Calls postRun() on all tasks that are ready. This function needs to be called 
   * periodically as part of the main loop, to ensure that all tasks are properly
   * postprocessed. Returns true if any tasks are processed, false otherwise.
   */
  bool postProcess();
};

#endif
