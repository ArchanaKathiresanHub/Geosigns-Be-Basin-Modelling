#include "Interface/FaultFileReader.h"

using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

FaultFileReader::FaultFileReader () {
}

//------------------------------------------------------------//

FaultFileReader::~FaultFileReader () {

  FaultDataSet::iterator faultIter;

  for ( faultIter = faultData.begin (); faultIter != faultData.end (); ++faultIter ) {
    faultIter->faultLine.clear ();
  }

  faultData.clear ();
}

//------------------------------------------------------------//


const std::string& FaultFileReader::faultName ( const FaultDataSetIterator& Iter ) const {
  return Iter->faultName;
}

//------------------------------------------------------------//

const PointSequence&  FaultFileReader::faultLine ( const FaultDataSetIterator& Iter ) const {
  return Iter->faultLine;
}

//------------------------------------------------------------//


FaultFileReader::FaultDataSetIterator FaultFileReader::begin () const {
  return faultData.begin ();
}

//------------------------------------------------------------//

FaultFileReader::FaultDataSetIterator FaultFileReader::end () const {
  return faultData.end ();
}

//------------------------------------------------------------//

void FaultFileReader::addFault ( const std::string&   newFaultName,
                                 const PointSequence& newFaultLine ) {

  FaultDataItem newItem;

  newItem.faultName = newFaultName;
  newItem.faultLine = newFaultLine;

  faultData.push_back ( newItem );

}

//------------------------------------------------------------//

ASCIIFaultFileReader::ASCIIFaultFileReader () {
  isOpen = false;
}

//------------------------------------------------------------//

ASCIIFaultFileReader::~ASCIIFaultFileReader () {

  if ( isOpen ) {
    faultFile.close ();
  }

}

//------------------------------------------------------------//

void ASCIIFaultFileReader::open ( const std::string& fileName,
                                        bool&        fileIsOpen ) {

  faultFile.open ( fileName.c_str ());
  isOpen = faultFile.good ();
  fileIsOpen = isOpen;

  if ( ! isOpen ) {
    #ifndef _FAULTUNITTEST_
     fprintf ( stderr,
                  "****************    ERROR ASCIIFaultFileReader::open   fault file, %s, could not be opened   ****************\n", fileName.c_str ());
     //PetscPrintf ( PETSC_COMM_WORLD,
     //             "****************    ERROR ASCIIFaultFileReader::open   fault file, %s, could not be opened   ****************\n", fileName.c_str ());
    #endif
  }

}

//------------------------------------------------------------//

void ASCIIFaultFileReader::close () {

  if ( isOpen ) {
    faultFile.close ();
    isOpen = false;
  }

}

//------------------------------------------------------------//
