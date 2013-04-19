#ifndef __DEVIATED_WELL__MATLAB_WELL_WRITER__H_
#define __DEVIATED_WELL__MATLAB_WELL_WRITER__H_

#include "CauldronWell.h"

#include "WellWriter.h"

/// \brief Class for writing the deviated well data in Matlab format.
class MatlabWellWriter : public WellWriter {

public :

   /// \brief The identifier used in the factory.
   static const std::string identifier;

   /// \brief The default extension for csv format files.
   static const std::string extension;

   /// \brief Constructor.
   MatlabWellWriter ();

   /// \brief Destructor.
   ~MatlabWellWriter ();

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
class MatlabWellWriterAllocator : public WellWriterAllocator {

public :

   /// \brief Allocate a matlab-well-writer object.
   WellWriter* allocate ();

};

#endif // __DEVIATED_WELL__MATLAB_WELL_WRITER__H_
