#ifndef _MAPWRITER_H_
#define _MAPWRITER_H_

#include <string>
#include "hdf5.h"

// V.R. Ambat (21/07/2011): Initial implementation

#include "Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      /// An abstract MapWriter class. This class is used for writing GridMaps to file
      class MapWriter
      {
      public:
         /// Destructor
         virtual ~MapWriter() {}
         
         /// open a file
         virtual bool open( const std::string & fileName, bool append = false ) = 0;
         
         /// close a file
         virtual bool close() = 0;
         
         /// return the filename without the leading directories
         virtual const std::string & getFileName() = 0;
         
         /// enable output in chunks
         virtual void setChunking() = 0;
         
         /// write a 2D gridmap to a file
         virtual bool writeMapToHDF( GridMap      * gridMap
                                     , float          time
                                     , double         depoAge
                                     , const std::string & propertyGrid
                                     , const std::string & surfaceName
                                     , const bool     saveAsPrimary = false 
                                     ) = 0;
         
         /// Write input map to a file. In this case we do not need layer name, property name and other stuff
         virtual bool writeInputMap( GridMap * gridMap, int mapSeqNumber ) = 0;
         
         /// write a 3D gridmap to a file, not yet implemented
         virtual bool writeVolumeToHDF( GridMap * gridMap, const std::string & propertyName, const std::string & layerName, const bool isPrimary = false ) = 0;
         
         //virtual bool write2DDataSet (const std::string & dataSetName, float *data, int *start, int *count, int *size) = 0;
         
         virtual bool Write1DDataSet( const long size, const std::string & dataSetName, const hid_t dataType, const void *data ) = 0;
         
         virtual bool writeAttribute( const std::string & dataSetName
                                      , const char   * attributeName
                                      , const hid_t    attributeType
                                      , const hsize_t  dims
                                      , void * data 
                                      ) = 0;
         
         /// save a grid description to file
         virtual bool saveDescription (const Grid * grid) = 0;
         
      protected:
         MapWriter() {}
         
      private:
      };
   }
}

#endif  /* _MAPWRITER_H_ */
