#ifndef _DEVIATED_WELL__STREAM_WELL_WRITER__H_
#define _DEVIATED_WELL__STREAM_WELL_WRITER__H_

#include <ostream>

#include "CauldronWell.h"

#include "WellWriter.h"

/// \brief Class for writing the deviated well data in csv format to stl stream.
class StreamWellWriter : public WellWriter {

public :

   /// \brief Constructor.
   StreamWellWriter ( const std::string& extension );

protected :

   /// \brief Perform writing of results to stream.
   void doWrite ( std::ostream&                                      out,
                  const DataAccess::Mining::ElementPositionSequence& elements,
                  const DataAccess::Mining::CauldronWell&            well,
                  const DataAccess::Mining::DataMiner::ResultValues& results,
                  const DataAccess::Mining::DataMiner::PropertySet&  properties,
                  const DistanceUnit                                 outputDistanceUnit );

};

#endif // _DEVIATED_WELL__STREAM_WELL_WRITER__H_
