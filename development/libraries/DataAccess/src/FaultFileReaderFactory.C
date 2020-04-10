//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FaultFileReaderFactory.h"

#include <fstream>

using namespace DataAccess;
using namespace Interface;

FaultFileReaderFactory* FaultFileReaderFactory::s_FaultFileReaderFactoryInstance = nullptr;

FaultFileReaderFactory::FaultFileReaderFactory ()
{
}

FaultFileReaderFactory& FaultFileReaderFactory::getInstance ()
{
  /// There is no need for any locking here, as we are not in a multithreaded environment.
  if ( s_FaultFileReaderFactoryInstance == 0 ) {
    s_FaultFileReaderFactoryInstance = new FaultFileReaderFactory;
  }

  return *s_FaultFileReaderFactoryInstance;
}

FaultFileReaderFactory::~FaultFileReaderFactory ()
{
  delete s_FaultFileReaderFactoryInstance;
}

FaultFileReader* FaultFileReaderFactory::createReader ( const std::string& ID ) const
{
  if      (ID == IBSFaultFileReaderID)      return new IBSFaultFileReader();
  else if (ID == LandmarkFaultFileReaderID) return new LandmarkFaultFileReader();
  else if (ID == ZyCorFaultFileReaderID)    return new ZyCorFaultFileReader();
  else if (ID == CharismaFaultFileReaderID) return new CharismaFaultFileReader();
  else
  {
    fprintf ( stderr,
              "****************    ERROR FaultFileReaderFactory::createReader  ID '%s' was not found   ****************\n",
              ID.c_str ());
    return nullptr;
  }
}
