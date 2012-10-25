#ifndef _DEVIATED_WELL__STD_OUT_WELL_WRITER__H_
#define _DEVIATED_WELL__STD_OUT_WELL_WRITER__H_

#include "CauldronWell.h"

#include "StreamWellWriter.h"

/// \brief Class for writing the deviated well data in csv format to std-out.
class StdOutWellWriter : public StreamWellWriter {

public :

   /// \brief The identifier used in the factory.
   static const std::string identifier;

   /// \brief The default extension for csv format files.
   static const std::string extension;

   /// \brief Constructor.
   StdOutWellWriter ();

   /// \brief Destructor.
   ~StdOutWellWriter ();

   /// \brief Write the result in csv format.
   ///
   /// All distances in results are in metres.
   void write ( const std::string&                                 name,
                const DataAccess::Mining::ElementPositionSequence& elements,
                const DataAccess::Mining::CauldronWell&            well,
                const DataAccess::Mining::DataMiner::ResultValues& results,
                const DataAccess::Mining::DataMiner::PropertySet&  properties,
                const DistanceUnit                                 outputDistanceUnit );


};

/// \brief Allocator class for csv-well-writer classes.
class StdOutWellWriterAllocator : public WellWriterAllocator {

public :

   /// \brief Allocate a matlab-well-writer object.
   WellWriter* allocate ();

};


#endif // _DEVIATED_WELL__STD_OUT_WELL_WRITER__H_
