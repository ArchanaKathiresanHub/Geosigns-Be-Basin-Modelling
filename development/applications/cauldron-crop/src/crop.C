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

int main(int argc, char ** argv)
{
   if (argc < 3)
   {
      std::cerr << "usage: " << argv[0] << " cauldron-crop input_project output_dir";
      return 1;
   }

   std::string inputProjectFile = argv[1];
   std::string outputDir = argv[2];

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

   if (offsetX != 0 || offsetY != 0)
   {
      throw CropException() 
         << "Non-zero offsets in the project3d file are not supported" ;
   }

   database::Table * gridMapTbl = project->getTable("GridMapIoTbl");
   std::map< std::string, hid_t > outputFiles;
   for (unsigned i = 0; i < gridMapTbl->size(); ++i)
   {
      std::string fileName = getMapFileName(gridMapTbl->getRecord(i));
      int mapSeqNr = getMapSeqNbr(gridMapTbl->getRecord(i));

      std::ostringstream hdfLayerName;
      hdfLayerName << "/Layer=" << std::setfill('0') << std::setw(2) << mapSeqNr;

      // open hdf5 file
      herr_t status;
      hid_t hdfFile = H5Fopen( fileName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

      if (!hdfFile)
      {
         throw CropException() << "Can't open input HDF5 gridmap file '" << fileName << "'" ;
      }

      // do some basic checks
      float layerDX = 0, layerDY = 0;
      int layerNX = 0, layerNY = 0;
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
            H5T_NATIVE_INT, &layerNX);

      if (status != 0)
         throw CropException() << "Dataset 'number in I dimension' not found in"
            " gridmap file '" << fileName << "'";

      if ( Nx != layerNX )
         throw CropException() << "NumberX mismatch between project3d file and gridmap"
            << " '" << fileName << "'";

      status = H5LTread_dataset( hdfFile, "number in J dimension",
            H5T_NATIVE_INT, &layerNY);

      if (status != 0)
         throw CropException() << "Dataset 'number in J dimension' not found in"
            " gridmap file '" << fileName << "'";

      if ( Ny != layerNY )
         throw CropException() << "NumberY mismatch between project3d file and gridmap"
            << " '" << fileName << "'";

      status = H5LTread_dataset( hdfFile, "null value",
            H5T_NATIVE_FLOAT, &layerNull);

      if (status != 0)
         throw CropException() << "Dataset 'null value' not found in"
            " gridmap file '" << fileName << "'";
     
      // read the grid map
      std::vector<float> data( Nx * Ny);
      status = H5LTread_dataset( hdfFile, hdfLayerName.str().c_str(), H5T_NATIVE_FLOAT, &data[0]);

      if (status != 0)
         throw CropException() << "Dataset '" << hdfLayerName.str() << "' not found in"
            " gridmap file '" << fileName << "'";

      // close the hdf5 file
      status = H5Fclose(hdfFile);

      if (status != 0)
         std::clog << "Unabel to close input HDF5 gridmap '" << fileName << "'" << std::endl;


      // compute basic properties
      float layerOriginX = originX + layerDX * x0;
      float layerOriginY = originY + layerDY * y0;
      layerNX = x1 - x0 + 1;
      layerNY = y1 - y0 + 1;


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
         status |= H5LTmake_dataset( file, "number in I dimension", 1, &scalar, H5T_NATIVE_INT, &layerNX);
         status |= H5LTmake_dataset( file, "number in J dimension", 1, &scalar, H5T_NATIVE_INT, &layerNY);
         status |= H5LTmake_dataset( file, "origin in I dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerOriginX);
         status |= H5LTmake_dataset( file, "origin in J dimension", 1, &scalar, H5T_NATIVE_FLOAT, &layerOriginY);
         status |= H5LTmake_dataset( file, "null value", 1, &scalar, H5T_NATIVE_FLOAT, &layerNull);

         if (status != 0)
            throw CropException() << "Could not write a dataset to output gridmap '"
               << outputFileName << "'";
      }

      hid_t outputFile = outputFiles[outputFileName];

      // crop the data
      std::vector<float > cropped( (x1-x0+1) * (y1-y0+1));
      for (unsigned i = x0; i <= x1; ++i)
      {
         for (unsigned j = y0; j <= y1; ++j)
         {
            int newNy = y1 - y0 + 1;
            cropped[j - y0 + (i-x0)* newNy] = data[j + i * Ny ];
         }
      }

      // write the new grid map
      hsize_t croppedDims[2] = { layerNX, layerNY };
      status = H5LTmake_dataset( outputFile, hdfLayerName.str().c_str(), 2, croppedDims, H5T_NATIVE_FLOAT, &cropped[0]); 

      if (status != 0)
         throw CropException() << "Could not write dataset '" 
             << hdfLayerName.str() << "' to output gridmap '"
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
   setNumberX( projectTbl, x1 - x0 + 1 );
   setNumberY( projectTbl, y1 - y0 + 1 );
   setWindowXMin( projectTbl, 0 );
   setWindowYMin( projectTbl, 0 );
   setWindowXMax( projectTbl, x1 - x0 );
   setWindowYMax( projectTbl, y1 - y0 );

   project->saveToFile( outputDir + "/Project.project3d");

   return 0;
}
