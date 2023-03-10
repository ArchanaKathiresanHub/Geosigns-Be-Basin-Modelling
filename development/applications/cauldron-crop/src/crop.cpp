#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "database.h"
#include "cauldronschemafuncs.h"
#include "FormattingException.h"
#include "FilePath.h"

#include "hdf5.h"
#include "hdf5_hl.h"

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <memory>
#include <assert.h>

struct CropException : formattingexception::BaseException<CropException> {};

#define Max(a,b)        (a > b ? a : b)
#define Min(a,b)        (a < b ? a : b)

struct LayerInfo
{
   std::string name;
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
      std::cerr << "usage: " << argv[0] << " [-oversample x y] input_project output_dir" << std::endl;
#else
      std::cerr << "usage: " << argv[0] << " [-oversample x y] [-subsample x y] input_project output_dir" << std::endl;
#endif
      return 1;
   }

   DataAccess::Interface::ObjectFactory* factory = new DataAccess::Interface::ObjectFactory();
   std::unique_ptr<DataAccess::Interface::ProjectHandle> project(
      DataAccess::Interface::OpenCauldronProject( inputProjectFile, factory)
   );

   database::Record * projectTbl = project->getTable("ProjectIoTbl")->getRecord(0);
   int originX = getXCoord( projectTbl );
   int originY = getYCoord( projectTbl );
   int Nx = getNumberX( projectTbl );
   int Ny = getNumberY( projectTbl );
   int offsetX = getOffsetX( projectTbl );
   int offsetY = getOffsetY( projectTbl );
   double dx = getDeltaX( projectTbl );
   double dy = getDeltaY( projectTbl );
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
      float layerDXin = 0, layerDYin = 0;
      float layerDXout = 0, layerDYout = 0;
      int layerNXin = 0, layerNYin = 0;
      float layerOffsetX = 0, layerOffsetY = 0;
      float layerNull = 0;

      status = H5LTread_dataset( hdfFile, "delta in I dimension",
            H5T_NATIVE_FLOAT, &layerDXin);

      if (status != 0)
         throw CropException() << "Dataset 'delta in I dimension' not found in"
            " gridmap file '" << fileName << "'";

      if ( dx != layerDXin )
         throw CropException() << "DeltaX mismatch between project3d file and gridmap"
            << " '" << fileName << "'";

      status = H5LTread_dataset( hdfFile, "delta in J dimension",
            H5T_NATIVE_FLOAT, &layerDYin);

      if (status != 0)
         throw CropException() << "Dataset 'delta in J dimension' not found in"
            " gridmap file '" << fileName << "'";

      if ( dy != layerDYin )
         throw CropException() << "DeltaY mismatch between project3d file and gridmap"
            << " '" << fileName << "'";

      layerDXout = layerDXin / oversampled_x;
      layerDYout = layerDYin / oversampled_y;

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
      float layerOriginX = originX + layerDXin * x0;
      float layerOriginY = originY + layerDYin * y0;

      // open a new file if one is needed
      ibs::FilePath outputFileName( outputDir );
      outputFileName << fileName;
      if (outputFiles.find( outputFileName.path() ) == outputFiles.end())
      {
         hid_t file = H5Fcreate( outputFileName.cpath(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

         if (!file)
         {
            throw CropException() << "Cannot open output HDF5 Gridmap file '" << outputFileName.path() << "'";
         }

         outputFiles[ outputFileName.path() ] = file;

         hsize_t scalar = 1;
         status = H5LTmake_dataset( file, "delta in I dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerDXout);
         status |= H5LTmake_dataset( file, "delta in J dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerDYout);
         status |= H5LTmake_dataset( file, "number in I dimension", 1, &scalar, H5T_NATIVE_INT, &layerNXout);
         status |= H5LTmake_dataset( file, "number in J dimension", 1, &scalar, H5T_NATIVE_INT, &layerNYout);
         status |= H5LTmake_dataset( file, "origin in I dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerOriginX);
         status |= H5LTmake_dataset( file, "origin in J dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerOriginY);
         status |= H5LTmake_dataset( file, "null value", 1, &scalar, H5T_NATIVE_FLOAT, &layerNull);

         if (status != 0)
            throw CropException() << "Could not write a dataset to output gridmap '"
               << outputFileName.path() << "'";
      }

      hid_t outputFile = outputFiles[outputFileName.path() ];

      // crop and scale the data
      std::vector<float > cropped( layerNXout * layerNYout);
      for (unsigned i = x0; i < x0 + layerNXout; ++i)
      {
         for (unsigned j = y0; j < y0 + layerNYout; ++j)
         {
      int newNy = layerNYout;
      int unsampled_Ny = y1 - y0 + 1;

			int unsampled_i = i / oversampled_x;
			int unsampled_j = j / oversampled_y;

			double fraction_i = double (i % oversampled_x) / oversampled_x;
			double fraction_j = double (j % oversampled_y) / oversampled_y;

			double dataloc[2][2];

			// Should check data index is not out of bounds.
			// Now checking for NaN below.

			double indices[2][2];
			indices[0][0] = unsampled_j     + (unsampled_i    ) * unsampled_Ny;
			indices[1][0] = unsampled_j     + (unsampled_i + 1) * unsampled_Ny;
			indices[0][1] = unsampled_j + 1 + (unsampled_i    ) * unsampled_Ny;
			indices[1][1] = unsampled_j + 1 + (unsampled_i + 1) * unsampled_Ny;

			if (indices[0][0] < Nx * Ny)
			{
				 dataloc[0][0] = data[indices[0][0]];
			}
			else
			{
				 assert (fraction_i == 1 || fraction_j == 1);
				 dataloc[0][0] = 0;
			}
			if (indices[1][0] < Nx * Ny)
			{
				 dataloc[1][0] = data[indices[1][0]];
			}
			else
			{
				 assert (fraction_i == 0 || fraction_j == 1);
				 dataloc[1][0] = 0;
			}
			if (indices[0][1] < Nx * Ny)
			{
				 dataloc[0][1] = data[indices[0][1]];
			}
			else
			{
				 assert (fraction_i == 1 || fraction_j == 0);
				 dataloc[0][1] = 0;
			}
			if (indices[1][1] < Nx * Ny)
			{
				 dataloc[1][1] = data[indices[1][1]];
			}
			else
			{
				 assert (fraction_i == 0 || fraction_j == 0);
				 dataloc[1][1] = 0;
			}
			double croppedloc;

			if (  (dataloc[0][0] == 99999 && fraction_i != 1 && fraction_j != 1) ||
			(dataloc[1][0] == 99999 && fraction_i != 0 && fraction_j != 1) ||
			(dataloc[0][1] == 99999 && fraction_i != 1 && fraction_j != 0) ||
			(dataloc[1][1] == 99999 && fraction_i != 0 && fraction_j != 0))
			{
				 croppedloc = 99999;
			}
			else
			{
				 croppedloc =
			(1 - fraction_i) * (1 - fraction_j) * dataloc[0][0] +
			(    fraction_i) * (1 - fraction_j) * dataloc[1][0] +
			(1 - fraction_i) * (    fraction_j) * dataloc[0][1] +
			(    fraction_i) * (    fraction_j) * dataloc[1][1];
			}

			assert (!std::isnan(croppedloc));

			cropped[j - y0 + (i -x0)* newNy] = croppedloc;
	 }
			}

      // write the new grid map
      hsize_t croppedDims[2] = { layerNXout, layerNYout };
      status = H5LTmake_dataset( outputFile, hdfLayerName.c_str(), 2, croppedDims, H5T_NATIVE_FLOAT, &cropped[0]);

      if (status != 0)
         throw CropException() << "Could not write dataset '"
             << hdfLayerName << "' to output gridmap '"
             << outputFileName.path() << "'";
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
   setDeltaX( projectTbl, dx / oversampled_x  );
   setDeltaY( projectTbl, dy / oversampled_y );
   setWindowXMin( projectTbl, 0 );
   setWindowYMin( projectTbl, 0 );
   setWindowXMax( projectTbl, layerNXout - 1 );
   setWindowYMax( projectTbl, layerNYout - 1 );

   ibs::FilePath outProjFile( outputDir );
   outProjFile << inputProjectFile;
   project->saveToFile( outProjFile.path() );
   delete factory;

   return 0;
}
