#include "anonymizer.h"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "cauldronschema.h"
#include "database.h"
#include "dataschema.h"
#include "FilePath.h"
#include "FolderPath.h"

#include "hdf5.h"
#include "boost/filesystem.hpp"


bool Anonymizer::run( const std::string & projectFolder )
{
   try
   {
      m_projectFolder = projectFolder;

      boost::filesystem::path path(m_projectFolder);
      if( !boost::filesystem::exists(path) || !boost::filesystem::is_directory(path) )
      {
         std::cerr << m_projectFolder << " does not exist" << std::endl;
         return false;
      }

      // Collecting filenames list
      boost::filesystem::directory_iterator it(path);
      // The constructor directory_iterator() with no arguments always constructs an end iterator object,
      // which shall be the only valid iterator for the end condition
      boost::filesystem::directory_iterator endit;
      std::vector< std::string > projectFileList;
      while( it != endit )
      {
         const std::string fileName = it->path().filename().string();
         const std::string extension = it->path().extension().string();
         if ( fileName.find(s_anonymized) != std::string::npos )
         {
            ++it;
            continue;
         }
         else if( extension == ".project3d" || extension == ".project1d" ) projectFileList.push_back( fileName );
         ++it;
      }

      // Anonymizing project files (both 1D and 3D)
      for( const auto & projd : projectFileList )
      {
         m_projectFile = projd;
         ibs::FilePath fPath = ibs::FilePath( m_projectFolder ) << m_projectFile;

         std::cout << "Anonymizing '" << fPath.fullPath().path() << "'..." << std::endl;

         // Creating output folder
         ibs::FolderPath fPathAnonymized( fPath );
         fPathAnonymized.cutLast();
         fPathAnonymized << s_anonymizedFolder;
         fPathAnonymized.create();

         // Opening database
         database::DataSchema * cauldronSchema = database::createCauldronSchema();
         m_db.reset( database::Database::CreateFromFile( fPath.fullPath().path(), *cauldronSchema ) );

         // Anonymizing
         clearNameMappings();
         read();
         renameMapFiles();
         if( m_shiftCoord ) shiftCoordinates();
         write();
      }
   }
   catch( std::exception & e )
   {
      std::cerr << e.what() << std::endl;
      return false;
   }

   return true;
}


void Anonymizer::clearNameMappings()
{
   m_gridMap.clear();
   m_layerNames.clear();
   m_surfaceNames.clear();
   m_lithoTypes.clear();
   m_fluidTypes.clear();
   m_sourceRockTypes.clear();
   m_mapFileNames.clear();
   m_langmuirNames.clear();
   m_wellNames.clear();
   m_reservoirNames.clear();
}


void Anonymizer::read()
{
   database::Table * table = m_db->getTable( "ProjectIoTbl" );
   database::Record * record = table->getRecord( 0 );
   m_coordOriginal[0] = record->getValue<double>( "XCoord" );
   m_coordOriginal[1] = record->getValue<double>( "YCoord" );
   m_coordShift[0] = m_coordOriginal[0] - s_northPoleCoord[0];
   m_coordShift[1] = m_coordOriginal[1] - s_northPoleCoord[1];

   createMapping( "GridMapIoTbl", "MapName", m_gridMap );

   createMapping( "StratIoTbl", "LayerName", m_layerNames );
   createMapping( "StratIoTbl", "SurfaceName", m_surfaceNames );

   createMapping( "LithotypeIoTbl", "Lithotype", m_lithoTypes );
   checkForTabooWords( m_lithoTypes, s_tabooLithoTypes );

   createMapping( "FluidtypeIoTbl", "Fluidtype", m_fluidTypes );

   createMapping( "SourceRockLithoIoTbl", "SourceRockType", m_sourceRockTypes );

   createMapping( "ReservoirIoTbl", "ReservoirName", m_reservoirNames );
   
   createMapping( "GridMapIoTbl", "MapFileName", m_mapFileNames );
   createMapping( "GridMapIoTbl", "HDF5FileName", m_mapFileNames );

   createMapping( "TouchstoneMapIoTbl", "TcfName", m_mapFileNames );

   createMapping( "LangmuirAdsorptionCapacityIsothermSetIoTbl", "LangmuirName", m_langmuirNames );
   createMapping( "LangmuirAdsorptionCapacityTOCFunctionIoTbl", "LangmuirName", m_langmuirNames );
   
   createMapping( "WellLocIoTbl", "WellName", m_wellNames );
}


void Anonymizer::renameMapFiles()
{
   // Renaming all map files
   unsigned int counter = 0;
   for( auto & mapElem : m_mapFileNames )
   {
      mapElem.second = "MapFile" + intToString(counter) + s_anonymized + "." + ibs::FilePath( mapElem.first ).fileNameExtension();
      ibs::FilePath oldFPath = ibs::FilePath( m_projectFolder );
      oldFPath << mapElem.first;
      if( !oldFPath.exists() )
      {
         std::stringstream errMsg;
         errMsg << "Could not find file '" << mapElem.first << "'";
         throw std::runtime_error( errMsg.str().c_str() );
      }
      const std::string extension = oldFPath.fileNameExtension();
      if( extension != "HDF" && extension != "FLT" )
      {
         std::stringstream errMsg;
         errMsg << "Unhandled file with '" << extension << "' extension";
         throw std::runtime_error( errMsg.str().c_str() );
      }
      ibs::FilePath newFPath = ibs::FilePath( oldFPath.filePath() );
      newFPath << s_anonymizedFolder << mapElem.second;
      oldFPath.copyFile( ibs::Path(newFPath) );
      ++counter;
   }
}


void Anonymizer::clearTables()
{
   clearOutputTables();
   clearTable("ProjectNotesIoTbl");
   clearTable("RelatedProjectsIoTbl");
   clearTable("BPANameMapping");
   clearTable("FilterTimeDepthIoTbl");
}


void Anonymizer::write()
{
   clearTables();

   writeProjectIoTbl();
   update( "GridMapIoTbl", "MapName", m_gridMap );
   update( "GridMapIoTbl", "MapFileName", m_mapFileNames );
   writeStratIoTbl();

   update( "BoundaryValuesIoTbl", "PropertyValueGrid", m_gridMap );

   update( "SurfaceTempIoTbl", "TemperatureGrid", m_gridMap );
   update( "SurfaceDepthIoTbl", "DepthGrid", m_gridMap );
   update( "MntlHeatFlowIoTbl", "HeatFlowGrid", m_gridMap );
   update( "BasementIoTbl", "TopCrustHeatProdGrid", m_gridMap );
   
   update( "MobLayThicknIoTbl", "LayerName", m_layerNames );
   update( "MobLayThicknIoTbl", "ThicknessGrid", m_gridMap );

   update( "AllochthonLithoIoTbl", "LayerName", m_layerNames );
   update( "AllochthonLithoIoTbl", "Lithotype", m_lithoTypes );
   update( "AllochthonLithoDistribIoTbl", "LayerName", m_layerNames );
   update( "AllochthonLithoDistribIoTbl", "DistributionGrid", m_gridMap );
   update( "AllochthonLithoInterpIoTbl", "LayerName", m_layerNames );

   update( "CrustIoTbl", "ThicknessGrid", m_gridMap );
   update( "ContCrustalThicknessIoTbl", "ThicknessGrid", m_gridMap );
   update( "BasaltThicknessIoTbl", "BasaltThicknessGrid", m_gridMap );
   update( "BasaltThicknessIoTbl", "CrustThicknessMeltOnsetGrid", m_gridMap );

   writeLithotypeIoTbl();
   update( "LitThCondIoTbl", "Lithotype", m_lithoTypes );
   update( "LitHeatCapIoTbl", "Lithotype", m_lithoTypes );

   writeFluidtypeIoTbl();
   update( "FltThCondIoTbl", "Fluidtype", m_fluidTypes );
   update( "FltHeatCapIoTbl", "Fluidtype", m_fluidTypes );
   update( "FltDensityIoTbl", "Fluidtype", m_fluidTypes );
   update( "FltViscoIoTbl", "Fluidtype", m_fluidTypes );
   
   update( "SourceRockLithoIoTbl", "LayerName", m_layerNames );
   update( "SourceRockLithoIoTbl", "SourceRockType", m_sourceRockTypes );
   update( "SourceRockLithoIoTbl", "TocIniGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "S1IniGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "S2IniGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "S3IniGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "HcIniGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "OcIniGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "NcIniGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "CharLengthGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "UpperBiotGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "LowerBiotGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "KerogenStartActGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "KerogenEndActGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "PreAsphaltStartActGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "PreAsphaltEndActGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "NettThickIniGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "NGenexTimeStepsGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "NGenexSlicesGrid", m_gridMap );
   update( "SourceRockLithoIoTbl", "AdsorptionCapacityFunctionName", m_langmuirNames );
   
   update( "CTCIoTbl", "SurfaceName", m_surfaceNames );
   update( "CTCIoTbl", "T0IniGrid", m_gridMap );
   update( "CTCIoTbl", "TRIniGrid", m_gridMap );
   update( "CTCIoTbl", "HCuIniGrid", m_gridMap );
   update( "CTCIoTbl", "HLMuIniGrid", m_gridMap );
   update( "CTCIoTbl", "HBuGrid", m_gridMap );
   update( "CTCIoTbl", "DeltaSLGrid", m_gridMap );
   
   update( "ReservoirIoTbl", "ReservoirName", m_reservoirNames );
   update( "ReservoirIoTbl", "FormationName", m_layerNames );
   update( "ReservoirIoTbl", "DepthOffsetGrid", m_gridMap );
   update( "ReservoirIoTbl", "ThicknessGrid", m_gridMap );
   update( "ReservoirIoTbl", "NetToGrossGrid", m_gridMap );
   update( "ReservoirIoTbl", "Percent1Grid", m_gridMap );
   update( "ReservoirIoTbl", "Percent2Grid", m_gridMap );
   update( "ReservoirIoTbl", "LayerFrequencyGrid", m_gridMap );
   
   update( "DetectedReservoirIoTbl", "DepthOffsetGrid", m_gridMap );
   update( "DetectedReservoirIoTbl", "ThicknessGrid", m_gridMap );
   update( "DetectedReservoirIoTbl", "NetToGrossGrid", m_gridMap );
   update( "DetectedReservoirIoTbl", "Percent1Grid", m_gridMap );
   update( "DetectedReservoirIoTbl", "Percent2Grid", m_gridMap );
   update( "DetectedReservoirIoTbl", "LayerFrequencyGrid", m_gridMap );
   
   update( "PalinspasticIoTbl", "DepthGrid", m_gridMap );
   update( "PalinspasticIoTbl", "FaultcutsMap", m_gridMap );
   update( "PalinspasticIoTbl", "SurfaceName", m_surfaceNames );
   update( "PalinspasticIoTbl", "BottomFormationName", m_layerNames );
   
   update( "FaultcutIoTbl", "SurfaceName", m_gridMap );
   update( "PressureFaultcutIoTbl", "FaultcutsMap", m_gridMap );

   update( "GenexHistoryLocationIoTbl", "FormationName", m_layerNames );

   update( "AdsorptionHistoryIoTbl", "AdsorptionFormationName", m_layerNames );

   update( "DataMiningIoTbl", "FormationName", m_layerNames );
   update( "DataMiningIoTbl", "SurfaceName", m_surfaceNames );
   update( "DataMiningIoTbl", "ReservoirName", m_reservoirNames );

   update( "SourceRockPropIoTbl", "LayerName", m_layerNames );
   update( "SourceRockPropIoTbl", "MapFileName", m_mapFileNames );

   update( "ReservoirPropIoTbl", "ReservoirName", m_reservoirNames );
   update( "ReservoirPropIoTbl", "MapFileName", m_mapFileNames );
   
   update( "TouchstoneMapIoTbl", "TcfName", m_mapFileNames );
   update( "TouchstoneMapIoTbl", "FormationName", m_layerNames );
   update( "TouchstoneMapIoTbl", "SurfaceName", m_surfaceNames );

   update( "TwoWayTimeIoTbl", "TwoWayTimeGrid", m_gridMap );
   update( "TwoWayTimeIoTbl", "SurfaceName", m_surfaceNames );
   
   update( "WellLocIoTbl", "WellName", m_wellNames );
   clearTableField( "WellLocIoTbl", "Datum" );

   clearTableField( "FilterTimeDepthIoTbl", "SurfaceName" );
   clearTableField( "FilterTimeDepthIoTbl", "FormationName" );
   
   // Writing anonymized project to file
   ibs::FilePath fPath = ibs::FilePath( m_projectFolder ) << m_projectFile;
   m_anonymizedProjectFile = "Project" + s_anonymized + "." + fPath.fileNameExtension();
   ibs::FilePath fPathAnonymized( ibs::FilePath( fPath.filePath() ) << s_anonymizedFolder << m_anonymizedProjectFile);
   std::ofstream outStream;
   outStream.open( fPathAnonymized.path(), std::ios::out );
   if( outStream.fail() )
   {
      throw std::runtime_error( "Unable to open anonymized project file" );
   }
   m_db->saveToStream( outStream );
   outStream.close();

   writeMappingtoFile();
}


void Anonymizer::createMapping( const std::string & tableName,
                                const std::string & fieldName,
                                std::map< std::string, std::string > & mapping )
{
   database::Table * table = m_db->getTable( tableName );

   int counter = 0;
   database::Table::iterator itEnd = table->end();
   for( database::Table::iterator it = table->begin(); it != itEnd; ++it )
   {
      database::Record * record = *it;
      const std::string & entry = record->getValue<std::string>(fieldName);
      if( !entry.empty() )
      {
         std::map< std::string, std::string >::iterator mapIt = mapping.find( entry );
         if( mapIt == mapping.end() )
         {
            mapping[ entry ] = fieldName + intToString(counter);
            ++counter;
         }
      }
   }
}


void Anonymizer::checkForTabooWords( std::map< std::string, std::string > & mapping,
                                     const std::vector<std::string> & tabooList )
{
   for( auto & mapElem : mapping )
   {
      if( std::find( tabooList.begin(), tabooList.end(), mapElem.first ) != tabooList.end() )
      {
         mapElem.second = mapElem.first;
      }
   }
}


void Anonymizer::update( const std::string & tableName,
                         const std::string & fieldName,
                         const std::map< std::string, std::string > & mapping )
{
   database::Table * table = m_db->getTable( tableName );
   assert( table != nullptr );

   int counter = 0;
   database::Table::iterator itEnd = table->end();
   for( database::Table::iterator it = table->begin(); it != itEnd; ++it, ++counter )
   {
      assert( (*it)->getIndex(fieldName) >= 0 );
      const std::string & entry = (*it)->getValue<std::string>(fieldName);
      if( entry.empty() ) continue;
      std::map< std::string, std::string >::const_iterator mapIt = mapping.find( entry );
      // If the entry is in the suggested mapping it will be used
      // otherwise, if the original entry was not empty, the value will be
      // replaced incrementally according to the provided prefix
      if( mapIt != mapping.end() )
      {
         (*it)->setValue<std::string>( fieldName, mapIt->second );
      }
      else
      {
         (*it)->setValue<std::string>( fieldName, fieldName + intToString(counter) );
      }
   }
}


void Anonymizer::clearTable( const std::string & tableName )
{
   database::Table * table = m_db->getTable( tableName );
   table->clear();
}


void Anonymizer::clearTableField( const std::string & tableName,
                                  const std::string & fieldName )
{
   database::Table * table = m_db->getTable( tableName );
   database::Table::iterator itEnd = table->end();
   for( database::Table::iterator it = table->begin(); it != itEnd; ++it )
   {
      (*it)->setValue<std::string>( fieldName, "" );
   }
}


std::string Anonymizer::intToString( const int i )
{
   std::stringstream ss;
   ss << std::setw(3) << std::setfill('0') << i;
   return ss.str();
}


void Anonymizer::writeProjectIoTbl()
{
   database::Table * table = m_db->getTable( "ProjectIoTbl" );
   database::Record * record = table->getRecord( 0 );

   record->setValue<std::string>( "ProjectName", "AnonymizedProject" );
   record->setValue<std::string>( "Description", "" );
   if( m_shiftCoord )
   {
      record->setValue<double>( "XCoord", s_northPoleCoord[0] );
      record->setValue<double>( "YCoord", s_northPoleCoord[1] );
      record->setValue<std::string>( "WktGeometryOutline", s_wktGeometryOutline );
   }
}


void Anonymizer::writeStratIoTbl()
{
   update( "StratIoTbl", "LayerName", m_layerNames );
   update( "StratIoTbl", "SurfaceName", m_surfaceNames );
   update( "StratIoTbl", "DepthGrid", m_gridMap );
   update( "StratIoTbl", "ThicknessGrid", m_gridMap );
   update( "StratIoTbl", "Percent1Grid", m_gridMap );
   update( "StratIoTbl", "Percent2Grid", m_gridMap );
   update( "StratIoTbl", "SourceRockType1", m_sourceRockTypes );
   update( "StratIoTbl", "SourceRockType2", m_sourceRockTypes );
   update( "StratIoTbl", "SourceRockMixingHIGrid", m_gridMap );

   database::Table * table = m_db->getTable( "StratIoTbl" );

   int counter = 0;
   database::Table::iterator itEnd = table->end();
   for( database::Table::iterator it = table->begin(); it != itEnd; ++it, ++counter )
   {
      database::Record * record = *it;

      // Rename Litho types
      const std::string & litho1 = record->getValue<std::string>( "Lithotype1" );
      if( !litho1.empty() ) record->setValue<std::string>( "Lithotype1", m_lithoTypes[ litho1 ] );
      const std::string & litho2 = record->getValue<std::string>( "Lithotype2" );
      if( !litho2.empty() ) record->setValue<std::string>( "Lithotype2", m_lithoTypes[ litho2 ] );
      const std::string & litho3 = record->getValue<std::string>( "Lithotype3" );
      if( !litho3.empty() ) record->setValue<std::string>( "Lithotype3", m_lithoTypes[ litho3 ] );

      // Rename Fluidtype
      const std::string & fluid = record->getValue<std::string>( "Fluidtype" );
      if( !fluid.empty() ) record->setValue<std::string>( "Fluidtype", m_fluidTypes[ fluid ] );
   }
}


void Anonymizer::writeLithotypeIoTbl()
{
   update( "LithotypeIoTbl", "Lithotype", m_lithoTypes );
   clearTableField( "LithotypeIoTbl", "Description" );
   clearTableField( "LithotypeIoTbl", "DefinedBy" );
   clearTableField( "LithotypeIoTbl", "DefinitionDate" );
   clearTableField( "LithotypeIoTbl", "LastChangedBy" );
   clearTableField( "LithotypeIoTbl", "LastChangedDate" );
}


void Anonymizer::writeFluidtypeIoTbl()
{
   update( "FluidtypeIoTbl", "Fluidtype", m_fluidTypes );
   clearTableField( "FluidtypeIoTbl", "Description" );
   clearTableField( "FluidtypeIoTbl", "DefinedBy" );
   clearTableField( "FluidtypeIoTbl", "DefinitionDate" );
   clearTableField( "FluidtypeIoTbl", "LastChangedBy" );
   clearTableField( "FluidtypeIoTbl", "LastChangedDate" );
}


void Anonymizer::clearOutputTables()
{
   clearTable("TimeIoTbl");
   clearTable("3DTimeIoTbl");
   clearTable("1DTimeIoTbl");
   clearTable( "TrapIoTbl" );
   clearTable( "TrapperIoTbl" );
   clearTable( "MigrationIoTbl" );
   clearTableField( "SnapshotIoTbl", "SnapshotFileName" );
}


void Anonymizer::writeMappingtoFile()
{
   ibs::FilePath fPath = ibs::FilePath( m_projectFolder );
   m_namesMappingFileName = ibs::FilePath(m_projectFile).fileNameNoExtension() + "." + s_mapping;
   fPath << s_anonymizedFolder << m_namesMappingFileName;
   std::ofstream file;
   file.open ( fPath.fullPath().cpath() );

   file << "\n# Original coordinates (X,Y)\n";
   file << std::setprecision(16) << m_coordOriginal[0] << " " << m_coordOriginal[1] << "\n";
   file << "\n# Coordinates shift (X,Y)\n";
   file << std::setprecision(16) << m_coordShift[0] << " " << m_coordShift[1] << "\n";

   file << "\n# GridMapIoTbl::MapName\n";
   for( const auto & i : m_gridMap )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file << "\n# StratIoTbl::LayerName\n";
   for( const auto & i : m_layerNames )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file << "\n# StratIoTbl::SurfaceName\n";
   for( const auto & i : m_surfaceNames )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file << "\n# LithotypeIoTbl::Lithotype\n";
   for( const auto & i : m_lithoTypes )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file << "\n# FluidtypeIoTbl::Fluidtype\n";
   for( const auto & i : m_fluidTypes )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file << "\n# File names\n";
   for( const auto & i : m_mapFileNames )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file << "\n# SourceRockLithoIoTbl::SourceRockType\n";
   for( const auto & i : m_sourceRockTypes )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file << "\n# LangmuirAdsorptionCapacityIsothermSetIoTbl::LangmuirName and LangmuirAdsorptionCapacityTOCFunctionIoTbl::LangmuirName\n";
   for( const auto & i : m_langmuirNames )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file << "\n# ReservoirIoTbl::ReservoirName\n";
   for( const auto & i : m_reservoirNames )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file << "\n# WellLocIoTbl::WellName\n";
   for( const auto & i : m_wellNames )
   {
      file << "\"" << i.first << "\" \"" << i.second << "\"\n";
   }

   file.close();
}

void Anonymizer::shiftCoordinates()
{
   shift( "WellLocIoTbl", "XCoord", "YCoord" );
   shift( "TouchstoneWellIoTbl", "XCoord", "YCoord" );
   shift( "DataMiningIoTbl", "XCoord", "YCoord" );
   shift( "GenexHistoryLocationIoTbl", "XPlanePosition", "YPlanePosition" );
   shift( "AdsorptionHistoryIoTbl", "XPlanePosition", "YPlanePosition" );

   // Shifting coordinates in HDF and fault files
   for( const auto & mapElem : m_mapFileNames )
   {
      ibs::FilePath fPath = ibs::FilePath( m_projectFolder );
      fPath << s_anonymizedFolder << mapElem.second;
      const std::string extension = fPath.fileNameExtension();
      assert( fPath.exists() );
      if( extension == "HDF" )
      {
         shiftHDFCoordinates( fPath.fullPath().path() );
      }
      else if( extension == "FLT" )
      {
         ibs::FilePath oldfPath = ibs::FilePath( m_projectFolder );
         oldfPath << mapElem.first;
         assert( oldfPath.exists() );
         shiftFaultCoordinates( oldfPath.fullPath().path(),
                                fPath.fullPath().path() );
      }
      else
      {
         std::stringstream errMsg;
         errMsg << "Unhandled file with '" << extension << "' extension";
         throw std::runtime_error( errMsg.str().c_str() );
      }
   }
}

void Anonymizer::shift( const std::string & tableName,
                        const std::string & fieldNameX,
                        const std::string & fieldNameY )
{
   database::Table * table = m_db->getTable( tableName );
   database::Table::iterator itEnd = table->end();
   for( database::Table::iterator it = table->begin(); it != itEnd; ++it )
   {
      (*it)->setValue<double>( fieldNameX, (*it)->getValue<double>(fieldNameX) - m_coordShift[0] );
      (*it)->setValue<double>( fieldNameY, (*it)->getValue<double>(fieldNameY) - m_coordShift[1] );
   }
}


void Anonymizer::shiftFaultCoordinates( const std::string & oldFileName,
                                        const std::string & newFileName ) const
{
   std::fstream oldFile( oldFileName, std::fstream::in );
   std::fstream newFile( newFileName, std::fstream::out );

   double x = 0.0;
   double y = 0.0;
   std::string line;
   while( std::getline(oldFile, line) )
   {
      std::istringstream iss(line);
      iss >> x >> y;
      if( x != s_undefFault && y != s_undefFault )
      {
         x -= m_coordShift[0];
         y -= m_coordShift[1];
      }
      std::ostringstream oss;
      oss << std::setprecision(12) << x << " " << y << std::endl;
      newFile << oss.str();
   }

   oldFile.close();
   newFile.close();
}


void Anonymizer::shiftHDFCoordinates( const std::string & fileName ) const
{
   // Open file
   hid_t fileId = H5Fopen( fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT );
   if( fileId < 0 )
   {
      std::stringstream errMsg;
      errMsg << "Unable to open '" << fileName << "'";
      throw std::runtime_error( errMsg.str().c_str() );
   }
   
   double value;
   herr_t status;
   
   // Open an existing dataset
   hid_t datasetId = H5Dopen2( fileId, "/origin in I dimension", H5P_DEFAULT);
   // Read current value
   status = H5Dread( datasetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);
   // Write shifted value
   value -= m_coordShift[0];
   status = H5Dwrite( datasetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);
   // Close dataset
   status = H5Dclose( datasetId );

   // Open an existing dataset
   datasetId = H5Dopen2( fileId, "/origin in J dimension", H5P_DEFAULT);
   // Read current value
   status = H5Dread( datasetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);
   // Write shifted value
   value -= m_coordShift[1];
   status = H5Dwrite( datasetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);
   // Close dataset
   status = H5Dclose( datasetId );

   H5Fclose ( fileId );
}