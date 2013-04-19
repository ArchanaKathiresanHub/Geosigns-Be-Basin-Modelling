#include "Interface/ReconModelFaultFileReader.h"

using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

DataAccess::Interface::FaultFileReader* DataAccess::Interface::allocateReconModelFaultFileReader () {
  return new ReconModelFaultFileReader;
}

//------------------------------------------------------------//


ReconModelFaultFileReader::ReconModelFaultFileReader () {

}

//------------------------------------------------------------//


void ReconModelFaultFileReader::preParseFaults () {

  if ( ! isOpen ) {
    #ifndef _FAULTUNITTEST_
    fprintf ( stderr,
                  "****************    ERROR ReconModelFaultFileReader::readFault   fault file is not open   ****************\n");
//    PetscPrintf ( PETSC_COMM_WORLD,
//                  "****************    ERROR ReconModelFaultFileReader::readFault   fault file is not open   ****************\n");
    #endif
    return;
  }

}

//------------------------------------------------------------//
