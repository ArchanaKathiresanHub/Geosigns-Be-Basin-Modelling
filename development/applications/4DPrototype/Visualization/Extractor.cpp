#include "Extractor.h"
#include "SkinExtractor.h"

#include <Inventor/SoDB.h>
#include <Inventor/threads/SbThreadAutoLock.h>

#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mesh/MiSurfaceMeshUnstructured.h>

#include <iostream>

void* Extractor::threadFunc(void* userData)
{
  SoDB::threadInit();

  Extractor* extractor = (Extractor*)userData;

  while(true)
  {
    SbThreadAutoLock autoLock(extractor->m_mutex);

    while(extractor->mustWait())
      extractor->m_condition.wait();

    if(extractor->m_stop)
      break;

    Extractor::WorkItem item = extractor->m_items.front();
    extractor->m_items.pop_front();

    // process item
    const MiSurfaceMeshUnstructured* surfaceMesh = SkinExtractor::doSkinExtraction(*item.bpaMesh, 0);

    SoDB::writelock();
    item.parentMesh->setMesh(surfaceMesh);
    SoDB::writeunlock();
  }

  SoDB::threadFinish();

  return 0;
}

bool Extractor::mustWait() const
{
  return !m_stop && m_items.empty();
}

Extractor::Extractor()
  : m_condition(&m_mutex)
  , m_stop(false)
{
  for(int i=0; i < nthreads; ++i)
    m_threads[i] = 0;
}

void Extractor::start()
{
  for(int i=0; i < nthreads; ++i)
    m_threads[i] = SbThread::create(threadFunc, this);
}

void Extractor::stop()
{
  SbThreadAutoLock autoLock(m_mutex);
  
  m_stop = true;
  m_condition.signalAll();

  // wait for threads to finish
  // ...
}

void Extractor::put(const Extractor::WorkItem& item)
{
  SbThreadAutoLock autoLock(m_mutex);

  m_items.push_back(item);
  m_condition.signalOne();
}
