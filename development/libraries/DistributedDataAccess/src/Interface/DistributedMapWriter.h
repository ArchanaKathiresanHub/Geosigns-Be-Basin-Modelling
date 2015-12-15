#ifndef _DISTRIBUTEDMAPWRITER_H_
#define _DISTRIBUTEDMAPWRITER_H_

#include "petscvector_readwrite.h"
#include "PetscVectors.h"

#include "Interface/Interface.h"
#include "Interface/MapWriter.h"

#include <string>

class H5_Write_File;

namespace DataAccess
{
   namespace Interface
   {
      /// This class is used for writing GridMaps to file
      class DistributedMapWriter: public MapWriter
      {
	 public:
	    DistributedMapWriter (void);
	    /// open a file
	    bool open (const std::string & fileName, bool append = false);
	    /// close a file
	    bool close ();
	    ~DistributedMapWriter ();

	    /// return the filename without the leading directories
	    const std::string & getFileName (void);

	    /// write a 2D gridmap to a file
	    bool writeMapToHDF (GridMap * gridMap, float time, double depoAge,
                                const std::string & propertyGrid, const std::string& surfaceName, const bool saveAsPrimary = false);
	    
		//bool writeMapToHDF (GridMap * gridMap, float time, double depoAge,
        //                        const std::string & propertyGrid, const std::string& surfaceName );

	    /// write a 2D gridmap to a file
	    bool writeMapToHDF (DM & da, Vec & vec, float time, double depoAge,
                                const std::string & propertyGrid, const std::string& surfaceName, const bool saveAsPrimary = false );

	    /// write a 3D gridmap to a file; generic implementation (V.R.Ambati, 13/07/2011).
	    bool writeVolumeToHDF (GridMap * gridMap, const std::string & propertyName, const std::string & layerName);
	    
		/// write a 3D gridmap to a file; originally implemented.
	    bool writeVolumeToHDF (DM & da, Vec & vec, const std::string & propertyName, const std::string & layerName);

	    /// write a 3D gridmap to a primary properties file; generic implementation 
         bool writePrimaryVolumeToHDF (GridMap * gridMap, const std::string & propertyName, double time, const std::string & layerName, const bool useGroupName = true ); 
	    
		/// write a 3D gridmap to a primary properties file; originally implemented.
         bool writePrimaryVolumeToHDF (DM & da, Vec & vec, const std::string & propertyName, double time, const std::string & layerName, const bool useGroupName = true);

	    bool write2DDataSet (const std::string & dataSetName, float *data, int *start, int *count, int *size);

	    bool Write1DDataSet (const long size, const std::string & dataSetName, const hid_t dataType, const void *data);

	    bool writeAttribute (const std::string & dataSetName, const char *attributeName,
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

	    H5_Write_File *m_outFile;

	    std::string m_fileName;

	    typedef PetscVector_ReadWrite<float> *WriterType;
	    WriterType m_writer;
      };
   }
}

#endif                          /* _DISTRIBUTEDMAPWRITER_H_ */
