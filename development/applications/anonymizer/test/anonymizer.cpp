#include "anonymizer.h"
#include <gtest/gtest.h>
#include <cmath>
#include <map>
#include <fstream>
#include "hdf5.h"
#include "FilePath.h"
#include "FolderPath.h"

namespace supportFunc
{
   // Support function to execute system commands
   std::string exec(const char* cmd)
   {
      char buffer[128];
      std::string result = "";
#ifdef WIN32
      std::shared_ptr<FILE> pipe(_popen(cmd, "r"), _pclose);
#else
      std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
#endif
      if (!pipe) throw std::runtime_error("popen() failed!");
      while (!feof(pipe.get()))
      {
         if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
      }
      return result;
   }

   // Support function to compare text files using system commands
   bool compareTextFiles(const std::string & leftFileName, const std::string & rightFileName )
   {
      bool sameFile = true;
      
      // read to string left file
      std::ifstream ilfs( leftFileName.c_str() );
      std::string leftContent( ( std::istreambuf_iterator<char>( ilfs ) ), ( std::istreambuf_iterator<char>() ) );
      
      // read to string right file
      std::ifstream irfs( rightFileName.c_str() );
      std::string rightContent( ( std::istreambuf_iterator<char>( irfs ) ), ( std::istreambuf_iterator<char>() ) );
      
      size_t ll = 1; // left line number
      size_t rl = 1; // right line number

      // compare contents
      for ( size_t i = 0, j = 0; i < leftContent.size(); ++i, ++j )
      {
         // skip white spaces/eol in left and right
         for ( ; i < leftContent.size()  && ( ::isspace( leftContent[i] )  || leftContent[i]  == '\n' || leftContent[i]  == '\r' ); ++i )
         {
            if ( leftContent[i] == '\n' ) { ++ll; }
         }

         for ( ; j < rightContent.size() && ( ::isspace( rightContent[j] ) || rightContent[j] == '\n' || rightContent[j] == '\r' ); ++j )
         {
            if ( rightContent[j] == '\n' ) { ++rl; }
         }

         if ( leftContent.size() == i && j == rightContent.size() ) { break; } // eof - comparison is completed

         if ( leftContent.size()  == i && j < rightContent.size() ||
              rightContent.size() == j && i < leftContent.size()  || // files are different in length
              leftContent[i] != rightContent[j]                      // files are different in contents
            )
         { 
            std::cerr << "Files: "  << leftFileName << " and " << rightFileName << " are different in lines: " << ll << "<->" << rl << "\n";
            while( i < leftContent.size() && leftContent[i] != '\n' ) { std::cerr << leftContent[i++]; }
            std::cerr << "\n";
            while( j < rightContent.size() && rightContent[j] != '\n' ) { std::cerr << rightContent[j++]; }
            std::cerr << "\n";

            sameFile = false;
            break;
         } 
      }

      return sameFile;
   }


   // Support function to read updated fields from HDF files
   void readOriginFromHDF(const std::string & fileName,
                          double & originI,
                          double & originJ)
   {
      // Open file
      hid_t fileId = H5Fopen(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);

      // Open an existing dataset
      hid_t datasetId = H5Dopen2(fileId, "/origin in I dimension", H5P_DEFAULT);
      // Read current value
      herr_t status = H5Dread(datasetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &originI);
      // Close current dataset
      status = H5Dclose( datasetId );

      // Open an existing dataset
      datasetId = H5Dopen2(fileId, "/origin in J dimension", H5P_DEFAULT);
      // Read current value
      status = H5Dread(datasetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &originJ);
      // Close current dataset
      status = H5Dclose( datasetId );

      H5Fclose(fileId);
   }


   // Support function to compare HDF files
   bool compareHDFFiles(const std::string & leftFileName,
                        const std::string & rightFileName)
   {
      // On Windows and Linux without h5diff command we can only check the two fields
      // (originI and originJ we actually changed), otherwise we'll use h5diff
      bool sameFile = false;
#ifdef WIN32
      double leftOriginI = 0.;
      double leftOriginJ = 0.;
      readOriginFromHDF(leftFileName, leftOriginI, leftOriginJ);
      double rightOriginI = 0.;
      double rightOriginJ = 0.;
      readOriginFromHDF(rightFileName, rightOriginI, rightOriginJ);
      sameFile = (std::abs(leftOriginI - rightOriginI) <= 1e-16);
      sameFile &= (std::abs(leftOriginJ - rightOriginJ) <= 1e-16);
#else
      // Check for h5diff
      std::string cmd = "command -v h5diff";
      const std::string hasH5diff = exec(cmd.c_str());
      if (!hasH5diff.empty())
      {
         std::string diffCommand = "h5diff " + leftFileName + " " + rightFileName;
         const std::string result = exec(diffCommand.c_str());
         sameFile = result.empty();
      }
      else
      {
         double leftOriginI = 0.;
         double leftOriginJ = 0.;
         readOriginFromHDF(leftFileName, leftOriginI, leftOriginJ);
         double rightOriginI = 0.;
         double rightOriginJ = 0.;
         readOriginFromHDF(rightFileName, rightOriginI, rightOriginJ);
         sameFile = (std::abs(leftOriginI - rightOriginI) <= 1e-16);
         sameFile &= (std::abs(leftOriginJ - rightOriginJ) <= 1e-16);
      }
#endif
      return sameFile;
   }

   void checkProjectFile(const Anonymizer & anonymizer,
                         const std::string & folder)
   {
      ibs::FilePath reference(folder);
      reference << "anonymized2compare" << anonymizer.getAnonymizedFileName();
      ibs::FilePath currentTest(folder);
      currentTest << anonymizer.s_anonymizedFolder << anonymizer.getAnonymizedFileName();
      ASSERT_TRUE(supportFunc::compareTextFiles(reference.fullPath().path(), currentTest.fullPath().path()));
   }

   void checkMapping(const Anonymizer & anonymizer,
                     const std::string & folder)
   {
      ibs::FilePath reference(folder);
      reference << "anonymized2compare" << anonymizer.getNamesMappingFileName();
      ibs::FilePath currentTest(folder);
      currentTest << anonymizer.s_anonymizedFolder << anonymizer.getNamesMappingFileName();
      ASSERT_TRUE(supportFunc::compareTextFiles(reference.fullPath().path(), currentTest.fullPath().path()));
   }

   void checkMapFiles(const Anonymizer & anonymizer,
                      const std::string & folder)
   {
      const std::map< std::string, std::string > & mapFiles = anonymizer.getMapFileNames();
      for (const auto & mapElem : mapFiles)
      {
         ibs::FilePath reference(folder);
         reference << "anonymized2compare" << mapElem.second;
         ibs::FilePath currentTest(folder);
         currentTest << anonymizer.s_anonymizedFolder << mapElem.second;
         if (reference.fileNameExtension() == "HDF")
         {
            ASSERT_TRUE(supportFunc::compareHDFFiles(reference.fullPath().path(), currentTest.fullPath().path()));
         }
         else
         {
            ASSERT_TRUE(supportFunc::compareTextFiles(reference.fullPath().path(), currentTest.fullPath().path()));
         }
      }
   }
}


TEST( AnonymizerTest, test_cbm00 )
{
   Anonymizer anonymizer;

   ibs::FolderPath outputFolder("test_cbm00");
   outputFolder << anonymizer.s_anonymizedFolder;
   outputFolder.clean();
   
   try{
     bool status = anonymizer.run( "test_cbm00" );
     EXPECT_TRUE(status);
   }
   catch(...){
     FAIL() << "Unexpected exception" << std::endl;
   }

   supportFunc::checkProjectFile(anonymizer, "test_cbm00");
   supportFunc::checkMapping(anonymizer, "test_cbm00");
   supportFunc::checkMapFiles(anonymizer, "test_cbm00");
}
