//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef EXTRACTOR_H_INCLUDED
#define EXTRACTOR_H_INCLUDED

#include <Inventor/threads/SbThreadMutex.h>
#include <Inventor/threads/SbThreadSignal.h>

#include <list>

class BpaMesh;
class MoMesh;

class Extractor
{
public:

  struct WorkItem
  {
    const BpaMesh* bpaMesh;
    MoMesh* parentMesh;
  };

private:

  static const int nthreads = 4;

  SbThreadMutex  m_mutex;
  SbThreadSignal m_condition;
  SbThread*      m_threads[nthreads];

  std::list<WorkItem> m_items;
  bool m_stop;

  static void* threadFunc(void* userData);

  bool mustWait() const;

public:

  Extractor();

  void start();

  void stop();

  void put(const WorkItem& item);
};

#endif
