#ifndef _SERIALMAPWRITER_H_
#define _SERIALMAPWRITER_H_

#include <string>
using std::string;

#include "hdf5.h"



#include "Interface/Interface.h"
#include "Interface/MapWriter.h"

namespace DataAccess
{
   namespace Interface
   {
      /// This class is used for writing GridMaps to file
      class SerialMapWriter: public MapWriter
      {
	 public:

		SerialMapWriter(void);
	    
		~SerialMapWriter ();

	    /// open a file
	    bool open (const string & fileName, bool append = false);

	    /// close a file
	    bool close ();

	    /// return the filename without the leading directories
	    const string & getFileName (void);

	    /// write a 2D gridmap to a file
	    bool writeMapToHDF (GridMap * gridMap, float time, double depoAge,
                                const string & propertyGrid, const string & surfaceName, const bool saveAsPrimary = false);

	    /// write a 2D gridmap to a file
	    bool writeMapToHDF (GridMap * gridMap, float time, double depoAge,
                                const string & propertyGrid);

	    bool writeMapData (const string & dataSetName, const GridMap * gridMap);

	    /// write a 3D gridmap to a file, not yet implemented
	    bool writeVolumeToHDF (GridMap * gridMap, const string & propertyName, const string & layerName);

         bool writePrimaryVolumeToHDF (GridMap * gridMap, const string & propertyName, double time, const string & layerName, const bool useGroupName = true);

	    //bool write2DDataSet (const string & dataSetName, float *data, int *start, int *count, int *size);

	    bool Write1DDataSet (const long size, const string & dataSetName, const hid_t dataType, const void *data);

	    bool writeAttribute (const string & dataSetName, const char *attributeName,
		  const hid_t attributeType, const hsize_t dims, void *data);

	    /// save a grid description to file
	    bool saveDescription (const Grid * grid);
	 protected:

	 private:
	    static const char *VERSION_DATASET_NAME;
	    static const char *NULL_VALUE_NAME;
	    static const char *NR_I_DATASET_NAME;
	    static const char *NR_J_DATASET_NAME;
	    static const char *ORIGIN_I_DATASET_NAME;
	    static const char *ORIGIN_J_DATASET_NAME;
	    static const char *DELTA_I_DATASET_NAME;
	    static const char *DELTA_J_DATASET_NAME;
	    static const char *PROJECT_DATASET_NAME;
	    static const char *PROJECT_COMMENT_DATASET_NAME;
	    static const char *PROJECT_MISC_COMMENT_DATASET_NAME;
	    static const char *PROJECT_PILOT_FILE_DATASET_NAME;
	    static const char *STRATTOP_AGE_DATASET_NAME;
	    static const char *LAYER_DATASET_PREFIX;
	    static const char *MAX_K_DATASET_NAME;

	    static const char *GRID_NAME_ATTRIBUTE;
	    static const char *PROPERTY_NAME_ATTRIBUTE;
	    static const char *STRATTOP_AGE_ATTRIBUTE;
	    static const char *STRATTOP_NAME_ATTRIBUTE;
	    static const char *STRATTOP_PROPERTY_ATTRIBUTE;

	    string m_fileName;

	    hid_t m_fileHandle;
      };
   }
}

#endif                          /* _MAPWRITER_H_ */
