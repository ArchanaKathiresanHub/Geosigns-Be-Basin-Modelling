#include "Interface/FaultFileReaderFactory.h"

using namespace DataAccess;
using namespace Interface;


//------------------------------------------------------------//

FaultFileReaderFactory::FaultFileReaderFactory () {
  s_FaultFileReaderFactoryInstance = 0;
}

//------------------------------------------------------------//

FaultFileReaderFactory::~FaultFileReaderFactory () {
  delete s_FaultFileReaderFactoryInstance;
}

//------------------------------------------------------------//

void FaultFileReaderFactory::registerReader ( const FaultFileReaderID&        ID,
                                              const FaultFileReaderAllocator& newAllocator ) {

  ReaderAllocatorMap::iterator allocatorIter = s_FaultFileReaderFactoryInstance->readerAllocator.find ( ID );

  if ( allocatorIter == s_FaultFileReaderFactoryInstance->readerAllocator.end ()) {
    s_FaultFileReaderFactoryInstance-> readerAllocator [ ID ] = newAllocator;
  }

}

//------------------------------------------------------------//

FaultFileReader* FaultFileReaderFactory::createReader ( const FaultFileReaderID& ID ) const {

  ReaderAllocatorMap::iterator allocatorIter = s_FaultFileReaderFactoryInstance->readerAllocator.find ( ID );

  if ( allocatorIter != s_FaultFileReaderFactoryInstance->readerAllocator.end ()) {
    return (*allocatorIter->second)();
  } else {
    #ifndef _FAULTUNITTEST_
    fprintf ( stderr,
                  "****************    ERROR FaultFileReaderFactory::createReader  ID '%s' was not found   ****************\n",
                  ID.c_str ());
    //PetscPrintf ( PETSC_COMM_WORLD,
    //              "****************    ERROR FaultFileReaderFactory::createReader  ID '%s' was not found   ****************\n",
    //              ID.c_str ());
    #endif

    return 0;
  }

}

//------------------------------------------------------------//

bool FaultFileReaderFactory::readerTypeIsRegistered ( const FaultFileReaderID& ID ) const {

  ReaderAllocatorMap::iterator allocatorIter = s_FaultFileReaderFactoryInstance->readerAllocator.find ( ID );

  return allocatorIter != s_FaultFileReaderFactoryInstance->readerAllocator.end ();
}

//------------------------------------------------------------//

FaultFileReaderFactory* FaultFileReaderFactory::s_FaultFileReaderFactoryInstance = 0;


FaultFileReaderFactory& FaultFileReaderFactory::getInstance () {

  ///
  /// There is no need for any locking here, as we are not in a multithreaded environment.
  ///
  if ( s_FaultFileReaderFactoryInstance == 0 ) {
    s_FaultFileReaderFactoryInstance = new FaultFileReaderFactory;
  }

  return *s_FaultFileReaderFactoryInstance;
}

//------------------------------------------------------------//
