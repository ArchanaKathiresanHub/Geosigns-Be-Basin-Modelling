//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "Scheduler.h"

#include <iostream>
#include <chrono>

std::shared_ptr<Task> Scheduler::popTask(Queue& q)
{
  std::unique_lock<std::mutex> lock(q.mutex);

  while (q.items.empty() && !m_stop)
    q.cv.wait(lock);

  if (q.items.empty())
    return nullptr;

  auto task = q.items.front();
  q.items.pop();

  return task;
}

void Scheduler::run(Queue& q)
{
  while (!m_stop)
  {
    auto task = popTask(q);
    if (task && !task->canceled)
    {
      task->run();

      // task could get canceled while running, in which
      // case we simply skip postprocessing
      if (!task->canceled) 
      {
	    std::unique_lock<std::mutex> lock(m_readyQueueMutex);
	    m_readyQueueWrite.push(task);
      }
    }
  }
}


Scheduler::Scheduler()
  : m_started(false)
  , m_stop(false)
{
}

/**
 * Start processing tasks
 */
void Scheduler::start(size_t numIoThreads, size_t numCpuThreads)
{
  assert(!m_started);

  // init threads
  for (size_t i = 0; i < numIoThreads; ++i)
  {
    m_ioThreads.push_back(
      std::thread(
        &Scheduler::run,
        this,
        std::ref(m_ioQueue)));
  }

  for (size_t i = 0; i < numCpuThreads; ++i)
  {
    m_cpuThreads.push_back(
      std::thread(
        &Scheduler::run,
        this,
        std::ref(m_cpuQueue)));
  }

  m_started = true;
}

/**
 * Stop processing tasks
 */
void Scheduler::stop()
{
  m_stop = true;

  m_ioQueue.cv.notify_all();
  m_cpuQueue.cv.notify_all();

  for (auto& t : m_ioThreads)
    t.join();

  for (auto& t : m_cpuThreads)
    t.join();

  m_started = false;
}

/**
 * Enqueue a task for processing in one of the worker threads
 */
void Scheduler::push(std::shared_ptr<Task> task)
{
  Queue& q = (task->affinity == Task::IOTASK)
    ? m_ioQueue
    : m_cpuQueue;

  std::unique_lock<std::mutex> lock(q.mutex);
  q.items.push(task);
  lock.unlock();

  q.cv.notify_one();
}

/**
 * Calls TaskSource::onTaskCompleted() for all tasks that are ready. This function 
 * needs to be called periodically as part of the main loop, to ensure that all tasks 
 * are properly postprocessed.
 */
bool Scheduler::postProcess()
{
  // Swap the read and write queues. Since the read queue is empty at this 
  // point, it effectively clears the write queue, moving all ready tasks 
  // to the read queue. The tasks are then processed after releasing the mutex,
  // minimizing lock contention.
  {
    std::unique_lock<std::mutex> lock(m_readyQueueMutex);
    m_readyQueueWrite.swap(m_readyQueueRead);
  }

  bool result = !m_readyQueueRead.empty();

  while (!m_readyQueueRead.empty())
  {
    auto task = m_readyQueueRead.front();
    m_readyQueueRead.pop();

    if (task->source)
      task->source->onTaskCompleted(task);
  }

  return result;
}


