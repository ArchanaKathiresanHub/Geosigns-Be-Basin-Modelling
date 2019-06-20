#include "StdOutWellWriter.h"

#include <iostream>
#include <fstream>
#include <iomanip>

#include "GridMap.h"
#include "Formation.h"
#include "LithoType.h"
#include "Property.h"

#include "GlobalDefs.h"

using namespace DataAccess;
using namespace Mining;

const std::string StdOutWellWriter::identifier = "cout";
const std::string StdOutWellWriter::extension = "";


StdOutWellWriter::StdOutWellWriter () : StreamWellWriter ( extension ) {
}

StdOutWellWriter::~StdOutWellWriter () {
} 

void StdOutWellWriter::write ( const std::string&                                 name,
                               const DataAccess::Mining::ElementPositionSequence& elements,
                               const DataAccess::Mining::CauldronWell&            well,
                               const DataAccess::Mining::DataMiner::ResultValues& results,
                               const DataAccess::Mining::DataMiner::PropertySet&  properties,
                               const DistanceUnit                                 outputDistanceUnit ) {

   doWrite ( std::cout, elements, well, results, properties, outputDistanceUnit );
}



WellWriter* StdOutWellWriterAllocator::allocate () {
   return new StdOutWellWriter;
}
