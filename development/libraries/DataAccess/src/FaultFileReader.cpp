//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FaultFileReader.h"

#include <fstream>

using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

FaultFileReader::FaultFileReader () {
}

//------------------------------------------------------------//

FaultFileReader::~FaultFileReader () {
}

//------------------------------------------------------------//

ASCIIFaultFileReader::ASCIIFaultFileReader () {
  m_isOpen = false;
}

//------------------------------------------------------------//

ASCIIFaultFileReader::~ASCIIFaultFileReader () {

  if ( m_isOpen ) {
    m_faultFile.close ();
  }

}

//------------------------------------------------------------//

void ASCIIFaultFileReader::open ( const std::string& fileName,
                                        bool&        fileIsOpen ) {

  m_faultFile.open ( fileName.c_str ());
  m_isOpen = m_faultFile.good ();
  fileIsOpen = m_isOpen;

  if ( ! m_isOpen )
  {
     fprintf ( stderr,
               "****************    ERROR ASCIIFaultFileReader::open   fault file, %s, could not be opened   ****************\n", fileName.c_str ());
  }

}

//------------------------------------------------------------//

void ASCIIFaultFileReader::close () {

  if ( m_isOpen ) {
    m_faultFile.close ();
    m_isOpen = false;
  }

}

//------------------------------------------------------------//

bool ASCIIFaultFileReader::isEmpty()
{
    return m_faultFile.peek() == std::ifstream::traits_type::eof();
}
