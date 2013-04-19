#ifndef __DEVIATED_WELL__WELL_WRITER__H_
#define __DEVIATED_WELL__WELL_WRITER__H_


#include "CauldronWell.h"
#include "ElementPosition.h"
#include "DataMiner.h"

#include "GlobalDefs.h"


class WellWriter {

public :

   WellWriter ( const std::string extension = "" );

   virtual ~WellWriter ();

   /// \brief Write the result in required format.
   ///
   /// All distances in results are in metres.
   virtual void write ( const std::string&                                 fileName,
                        const DataAccess::Mining::ElementPositionSequence& elements,
                        const DataAccess::Mining::CauldronWell&            well,
                        const DataAccess::Mining::DataMiner::ResultValues& results,
                        const DataAccess::Mining::DataMiner::PropertySet&  properties,
                        const DistanceUnit                                 outputDistanceUnit ) = 0;

   /// \brief Get the file-extension for the particular format.
   const std::string& getExtension () const;

private :

   std::string m_extension;

};

/// \brief Base-class of functor classes for allocating the required well-writer object.
class WellWriterAllocator {

public :

   /// \brief Destructor.
   virtual ~WellWriterAllocator () {}

   /// \brief Allocate the correct well-writer object.
   virtual WellWriter* allocate () = 0;

};

inline const std::string& WellWriter::getExtension () const {
   return m_extension;
}

#endif // __DEVIATED_WELL__WELL_WRITER__H_
