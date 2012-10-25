#ifndef _DEVIATED_WELL__WELL_WRITER_FACTORY__H_
#define _DEVIATED_WELL__WELL_WRITER_FACTORY__H_

#include <string>
#include <map>

#include "WellWriter.h"

class WellWriterFactory {

   typedef std::map<std::string, WellWriterAllocator*> WellWriterAllocatorMap;

public :

   WellWriterFactory ();

   ~WellWriterFactory ();

   /// Get the instance of the allocator-factory.
   static WellWriterFactory& getInstance ();

   /// \brief Delete the factory.
   static void finalise ();


   /// \brief Allocate a well-writer class.
   ///
   /// It is left to the user to delete the well-writer when finished.
   WellWriter* allocate ( const std::string& type );

   /// \brief Get the name of the default well-writer.
   const std::string& getDefaultWriterIdentifier () const;

   /// \brief Determine if the output format has been defined.
   ///
   /// I.e. There is an ellocator for the type specified.
   bool outputFormatIsDefined ( const std::string& type ) const;

private :

   static WellWriterFactory* s_instance;

   WellWriterAllocatorMap m_allocators;
   std::string            m_defaultWriterIdentifier;

};

inline WellWriterFactory& WellWriterFactory::getInstance () {

   if ( s_instance == 0 ) {
      s_instance = new WellWriterFactory;
   }

   return *s_instance;
}

inline const std::string& WellWriterFactory::getDefaultWriterIdentifier () const {
   return m_defaultWriterIdentifier;
}


#endif // _DEVIATED_WELL__WELL_WRITER_FACTORY__H_
