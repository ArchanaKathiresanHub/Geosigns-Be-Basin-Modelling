#include "WellWriterFactory.h"

#include "CSVWellWriter.h"
#include "MatlabWellWriter.h"
#include "StdOutWellWriter.h"


WellWriterFactory* WellWriterFactory::s_instance = 0;

WellWriterFactory::WellWriterFactory () {

   m_defaultWriterIdentifier = CSVWellWriter::identifier;

   m_allocators [ CSVWellWriter::identifier ] = new CSVWellWriterAllocator;
   m_allocators [ MatlabWellWriter::identifier ] = new MatlabWellWriterAllocator;
   m_allocators [ MatlabWellWriter::extension ] = new MatlabWellWriterAllocator;
   m_allocators [ StdOutWellWriter::identifier ] = new StdOutWellWriterAllocator;

}

WellWriterFactory::~WellWriterFactory () {

   WellWriterAllocatorMap::iterator allocator;

   for ( allocator = m_allocators.begin (); allocator != m_allocators.end (); ++allocator ) {
      delete allocator->second;
   }


}

void WellWriterFactory::finalise () {

   if ( s_instance != 0 ) {
      delete s_instance;
      s_instance = 0;
   }

}


/// \brief Allocate a well-writer class
WellWriter* WellWriterFactory::allocate ( const std::string& type ) {

   WellWriterAllocatorMap::iterator allocator = m_allocators.find ( type );

   if ( allocator != m_allocators.end ()) {
      return allocator->second->allocate ();
   } else {
      return 0;
   }

}

bool WellWriterFactory::outputFormatIsDefined ( const std::string& type ) const {

   WellWriterAllocatorMap::const_iterator allocator = m_allocators.find ( type );

   return allocator != m_allocators.end ();

}
