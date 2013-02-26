#include "Interface/ProjectHandle.h"
#include "database.h"
#include "cauldronschemafuncs.h"
#include "formattingexception.h"

#include "hdf5.h"
#include "hdf5_hl.h"

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

struct CropException : formattingexception::BaseException<CropException> {};

#define Max(a,b)        (a > b ? a : b)
#define Min(a,b)        (a < b ? a : b)

struct LayerInfo
{
   string name;
   int index;
};

/// Function used to iterate through a HDF5 file to find the name of the DataSet with the given layerIndex
herr_t checkForLayerName (hid_t groupId, const char * layerName, LayerInfo * layerInfo)
{
   if (strncmp (layerName, "Layer=", 6) == 0)
   {
      int layerIndex;
      sscanf (layerName, "Layer=%d", &layerIndex);
      if (layerIndex == layerInfo->index)
      {
	 layerInfo->name = layerName;
	 return 1;
      }
   }
   return 0;
}

int main(int argc, char ** argv)
{
   std::string inputProjectFile;
   std::string outputDir;

   bool oversampled = false;
   bool subsampled = false;

   int oversampled_x = 1, oversampled_y = 1;
   int subsampled_x = 1, subsampled_y = 1;

   int arg;
   for (arg = 1; arg < argc; arg++)
   {
      if (strncmp (argv[arg], "-oversample", Max (4, strlen (argv[arg]))) == 0)
      {
	 if (subsampled)
	 {
	    throw CropException() 
	       << "Only one of -subsample and -oversample is allowed" ;
	 }

         if (arg + 2 >= argc)
         {
	    throw CropException() 
	       << "one or more arguments for -oversample is missing" ;
         }

	 oversampled = true;
         oversampled_x = atoi (argv[++arg]);
         oversampled_y = atoi (argv[++arg]);
      }

      else if (strncmp (argv[arg], "-subsample", Max (2, strlen (argv[arg]))) == 0)
      {
	 throw CropException() 
	    << "-subsample not yet implemented" ;

	 if (oversampled)
	 {
	    throw CropException() 
	       << "Only one of -subsample and -oversample is allowed" ;
	 }

         if (arg + 2 >= argc)
         {
	    throw CropException() 
	       << "one or more arguments for -subsample is missing" ;
         }

	 subsampled = true;
         subsampled_x = atoi (argv[++arg]);
         subsampled_y = atoi (argv[++arg]);
      }
      else if (inputProjectFile.empty())
      {
	 inputProjectFile = argv[arg];
      }

      else if (outputDir.empty())
      {
	 outputDir = argv[arg];
      }
   }


   if (inputProjectFile.empty () || outputDir.empty())
   {
#if 1
      std::cerr << "usage: " << argv[0] << " [-oversample x y] input_project output_dir" << endl;
#else
      std::cerr << "usage: " << argv[0] << " [-oversample x y] [-subsample x y] input_project output_dir" << endl;
#endif
      return 1;
   }

   boost::shared_ptr<DataAccess::Interface::ProjectHandle> project( 
      DataAccess::Interface::OpenCauldronProject( inputProjectFile, "r")
   );

   database::Record * projectTbl = project->getTable("ProjectIoTbl")->getRecord(0);
   int originX = getXCoord( projectTbl );
   int originY = getYCoord( projectTbl );
   int Nx = getNumberX( projectTbl );
   int Ny = getNumberY( projectTbl );
   int offsetX = getOffsetX( projectTbl );
   int offsetY = getOffsetY( projectTbl );
   int dx = getDeltaX( projectTbl );
   int dy = getDeltaY( projectTbl );
   int x0 = getWindowXMin( projectTbl );
   int y0 = getWindowYMin( projectTbl );
   int x1 = getWindowXMax( projectTbl );
   int y1 = getWindowYMax( projectTbl );

   int layerNXout = 0, layerNYout = 0;

   if (offsetX != 0 || offsetY != 0)
   {
      throw CropException() 
         << "Non-zero offsets in the project3d file are not supported" ;
   }

   if (!subsampled)
   {
      layerNXout = (x1 - x0) * oversampled_x + 1;
      layerNYout = (y1 - y0) * oversampled_y + 1;
   }
   else
   {
      if ((x1 -x0) % subsampled_x != 0 || (y1 - y0) % subsampled_y != 0)
      {
	 throw CropException() << "Unable to subsample with" << subsampled_x << ", " << subsampled_y;
      }

      layerNXout = (x1 - x0) / subsampled_x + 1;
      layerNYout = (y1 - y0) * subsampled_y + 1;
   }

   database::Table * gridMapTbl = project->getTable("GridMapIoTbl");
   std::map< std::string, hid_t > outputFiles;
   for (unsigned i = 0; i < gridMapTbl->size(); ++i)
   {
      std::string fileName = getMapFileName(gridMapTbl->getRecord(i));
      int mapSeqNr = getMapSeqNbr(gridMapTbl->getRecord(i));

      std::string hdfLayerName;

      // open hdf5 file
      herr_t status;
      hid_t hdfFile = H5Fopen( fileName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

      if (!hdfFile)
      {
         throw CropException() << "Can't open input HDF5 gridmap file '" << fileName << "'" ;
      }

      // do some basic checks
      float layerDX = 0, layerDY = 0;
      int layerNXin = 0, layerNYin = 0;
      float layerOffsetX = 0, layerOffsetY = 0;
      float layerNull = 0;

      status = H5LTread_dataset( hdfFile, "delta in I dimension",
            H5T_NATIVE_FLOAT, &layerDX);

      if (status != 0)
         throw CropException() << "Dataset 'delta in I dimension' not found in"
            " gridmap file '" << fileName << "'";
 
      if ( dx != layerDX )
         throw CropException() << "DeltaX mismatch between project3d file and gridmap"
            << " '" << fileName << "'";

      status = H5LTread_dataset( hdfFile, "delta in J dimension",
            H5T_NATIVE_FLOAT, &layerDY);

      if (status != 0)
         throw CropException() << "Dataset 'delta in J dimension' not found in"
            " gridmap file '" << fileName << "'";

      if ( dy != layerDY )
         throw CropException() << "DeltaY mismatch between project3d file and gridmap"
            << " '" << fileName << "'";

      status = H5LTread_dataset( hdfFile, "number in I dimension",
            H5T_NATIVE_INT, &layerNXin);

      if (status != 0)
         throw CropException() << "Dataset 'number in I dimension' not found in"
            " gridmap file '" << fileName << "'";

      if ( Nx != layerNXin )
         throw CropException() << "NumberX mismatch between project3d file and gridmap"
            << " '" << fileName << "'";

      status = H5LTread_dataset( hdfFile, "number in J dimension",
            H5T_NATIVE_INT, &layerNYin);

      if (status != 0)
         throw CropException() << "Dataset 'number in J dimension' not found in"
            " gridmap file '" << fileName << "'";

      if ( Ny != layerNYin )
         throw CropException() << "NumberY mismatch between project3d file and gridmap"
            << " '" << fileName << "'";

      status = H5LTread_dataset( hdfFile, "null value",
            H5T_NATIVE_FLOAT, &layerNull);

      if (status != 0)
         throw CropException() << "Dataset 'null value' not found in"
            " gridmap file '" << fileName << "'";
     
      // read the grid map
      std::vector<float> data( Nx * Ny);

      LayerInfo layerInfo;
      layerInfo.index = mapSeqNr;
      layerInfo.name = "";
      
      int ret = H5Giterate (hdfFile, "/", NULL, (H5G_iterate_t) checkForLayerName, &layerInfo);
      if (ret != 1)
      {
         throw CropException() << "Layer " << "'Layer=" << mapSeqNr << "' not found in"
            " gridmap file '" << fileName << "'";
      }

      hdfLayerName = "/" + layerInfo.name;
      status = H5LTread_dataset( hdfFile, hdfLayerName.c_str(), H5T_NATIVE_FLOAT, &data[0]);

      if (status != 0)
         throw CropException() << "Dataset '" << hdfLayerName << "' not found in"
            " gridmap file '" << fileName << "'";

      // close the hdf5 file
      status = H5Fclose(hdfFile);

      if (status != 0)
         std::clog << "Unable to close input HDF5 gridmap '" << fileName << "'" << std::endl;


      // compute basic properties
      float layerOriginX = originX + layerDX * x0;
      float layerOriginY = originY + layerDY * y0;

      // open a new file if one is needed
      std::string outputFileName = outputDir + "/" + fileName;
      if (outputFiles.find( outputFileName ) == outputFiles.end())
      {
         hid_t file = H5Fcreate( outputFileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

         if (!file)
         {
            throw CropException() << "Cannot open output HDF5 Gridmap file '" << outputFileName << "'";
         }

         outputFiles[ outputFileName ] = file;

         hsize_t scalar = 1;
         status = H5LTmake_dataset( file, "delta in I dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerDX);
         status |= H5LTmake_dataset( file, "delta in J dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerDY);
         status |= H5LTmake_dataset( file, "number in I dimension", 1, &scalar, H5T_NATIVE_INT, &layerNXout);
         status |= H5LTmake_dataset( file, "number in J dimension", 1, &scalar, H5T_NATIVE_INT, &layerNYout);
         status |= H5LTmake_dataset( file, "origin in I dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerOriginX);
         status |= H5LTmake_dataset( file, "origin in J dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerOriginY);
         status |= H5LTmake_dataset( file, "null value", 1, &scalar, H5T_NATIVE_FLOAT, &layerNull);

         if (status != 0)
            throw CropException() << "Could not write a dataset to output gridmap '"
               << outputFileName << "'";
      }

      hid_t outputFile = outputFiles[outputFileName];

      // crop and scale the data
      std::vector<float > cropped( layerNXout * layerNYout);
      for (unsigned i = x0; i < x0 + layerNXout; ++i)
      {
         for (unsigned j = y0; j <= layerNYout; ++j)
         {
	    int newNy = layerNYout;
	    int unsampled_Ny = y1 - y0 + 1;
	    
	    int unsampled_i = i / oversampled_x;
	    int unsampled_j = j / oversampled_y;

	    double fraction_i = double (i % oversampled_x) / oversampled_x;
	    double fraction_j = double (j % oversampled_y) / oversampled_y;

	    cropped[j - y0 + (i -x0)* newNy] =
	       (1 - fraction_i) * (1 - fraction_j) * data[unsampled_j     + (unsampled_i    ) * unsampled_Ny ] +
	       (    fraction_i) * (1 - fraction_j) * data[unsampled_j     + (unsampled_i + 1) * unsampled_Ny ] +
	       (1 - fraction_i) * (    fraction_j) * data[unsampled_j + 1 + (unsampled_i    ) * unsampled_Ny ] +
	       (    fraction_i) * (    fraction_j) * data[unsampled_j + 1 + (unsampled_i + 1) * unsampled_Ny ];
	 }
      }

      // write the new grid map
      hsize_t croppedDims[2] = { layerNXout, layerNYout };
      status = H5LTmake_dataset( outputFile, hdfLayerName.c_str(), 2, croppedDims, H5T_NATIVE_FLOAT, &cropped[0]); 

      if (status != 0)
         throw CropException() << "Could not write dataset '" 
             << hdfLayerName << "' to output gridmap '"
             << outputFileName << "'";
   }

   // close output HDF files
   for (std::map<std::string, hid_t>::const_iterator i = outputFiles.begin(); i != outputFiles.end(); ++i)
   {
      herr_t status = H5Fclose( i->second );

      if (status != 0)
         std::clog << "Unable to close output HDF5 gridmap '" << i->first << "'" << std::endl;
   }


   // change project file
   setXCoord( projectTbl, originX + dx * x0 );
   setYCoord( projectTbl, originY  + dy * y0 );
   setNumberX( projectTbl, layerNXout );
   setNumberY( projectTbl, layerNYout );
   setWindowXMin( projectTbl, 0 );
   setWindowYMin( projectTbl, 0 );
   setWindowXMax( projectTbl, layerNXout - 1 );
   setWindowYMax( projectTbl, layerNYout - 1 );

   project->saveToFile( outputDir + "/" + inputProjectFile);

   return 0;
}
