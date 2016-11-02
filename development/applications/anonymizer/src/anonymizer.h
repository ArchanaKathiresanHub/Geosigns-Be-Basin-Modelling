//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef _PROJECT__ANONYMIZER_H_
#define _PROJECT__ANONYMIZER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "pugixml.hpp"

namespace database
{
   class Database;
}

class Anonymizer
{
public:
   // North Pole coordinates and wktGeometryOutline
   const double s_northPoleCoord[2] = { 535520.5235, 3663658.586 };
   const std::string s_wktGeometryOutline = "<WktGeometryOutline><CoordinateOperationName></CoordinateOperationName><SrsName>urn:ogc:def:crs:EPSG::32614</SrsName><Wkt>POLYGON  (( 535118.22781 3663256.20505, 552016.31291 3663256.25989, 552016.29953 3680154.41266, 535118.19884 3680154.36561, 535118.22781 3663256.20505))</Wkt></WktGeometryOutline>";

   // Undefined fault value for .FLT files
   const double s_undefFault = 999.999;

   // Name of the folder containing the anonymized project
   const std::string s_anonymizedFolder = "Anonymized";

   // Predefined output strings
   const std::string s_anonymized = ".anonymized";
   const std::string s_mapping = "namesMapping.txt";

   // Banned word to not be changed ny the anonymizer
   const std::vector<std::string> s_tabooLithoTypes = { "Crust", "Litho. Mantle" };



   // Default constructor
   Anonymizer() : m_shiftCoord(true) {}
   
   // Default destructor
   ~Anonymizer()
   {
      // Intentionally unimplemented
   }

   // \brief Main function fo anonymize all project files in the provided input folder
   // \param projectFolder: folder containing project files
   bool run( const std::string & projectFolder );
   
   // Boolean flag for shifting coordinates
   bool m_shiftCoord;

   // \brief Set flag for shifting coordinates
   inline void setShiftCoordinatesFlag(const bool shift) { m_shiftCoord = shift; }

   // \brief Get original project file name
   inline const std::string & getOriginalFileName() const { return m_projectFile; }

   // \brief Get anonymized project file name
   inline const std::string & getAnonymizedFileName() const { return m_anonymizedProjectFile; }

   // \brief Get anonymized project file name
   inline const std::string & getNamesMappingFileName() const { return m_namesMappingFileName; }

   // \brief Get anonymized project file name
   inline const std::map< std::string, std::string > &
   getMapFileNames() const { return m_mapFileNames; }

#ifndef _WIN32
   // \brief remove attributes from 2D output file (needs h5dump so it cannot run on Windows)
   void removeAttributesFrom2DOutputFile( const std::string & fileName ) const;
#endif

private:
   // \brief Read the tables of the project file and creates original-to-anonymized names mapping
   void read();
   
   // \brief Anonymize the whole project file and writes it to a new file
   void write();
   
   // \brief Empty mappings original-to-anonymized
   void clearNameMappings();
   
   // \brief Rename all map files
   void renameMapFiles();
   
   // \brief Creates names mapping for a specific table and field
   // \param tableName: name of the table
   // \param fieldName: name of the field to be anonymized
   // \param mapping  : original-to-anonymized mapping
   void createMapping( const std::string & tableName,
                       const std::string & fieldName,
                       std::map< std::string, std::string > & mapping );
   
   // \brief Names matching the provided taboo list won't be anonymized
   // \param mapping   : original-to-anonymized mapping
   // \param tabooList : list of names that cannot be changed
   void checkForTabooWords( std::map< std::string, std::string > & mapping,
                            const std::vector<std::string> & tabooList );
   
   // \brief Anonymize names in the provided filed of a table according to the provided mapping
   // \param tableName: name of the table
   // \param fieldName: name of the field to be anonymized
   // \param mapping  : original-to-anonymized mapping
   void update( const std::string & tableName,
                const std::string & fieldName,
                const std::map< std::string, std::string > & mapping );

   // \brief Clear all useless tables in the project file
   void clearTables();
   
   // \brief Clear the provided table
   // \param tableName: name of the table
   void clearTable( const std::string & tableName );
   
   // \brief Clear the provided field of a table
   // \param tableName: name of the table
   // \param fieldName: name of the field to be cleared
   void clearTableField( const std::string & tableName,
                         const std::string & fieldName );
   
   // \brief Clear results from output tables
   void clearOutputTables();
   
   // \brief Write all original-to-anonymized names mapping to file
   void writeMappingtoFile();
   
   // \brief Utility function to write an integer always with 3 digits
   std::string intToString( const int i );
   
   // \brief Writes generic coordinates (North Pole) to the ProjectIoTbl
   void writeProjectIoTbl();
   
   // \brief Renames fields in StratIoTbl
   void writeStratIoTbl();
   
   // \brief Renames fields in LithotypeIoTbl
   void writeLithotypeIoTbl();
   
   // \brief Renames fields in FluidtypeIoTbl
   void writeFluidtypeIoTbl();
   
   // \brief Shift coordinates
   void shiftCoordinates();
   
   // \brief Shift coordinates
   void shift( const std::string & tableName,
               const std::string & fieldNameX,
               const std::string & fieldNameY );
   
   // \brief Shift coordinates in fault files
   void shiftFaultCoordinates( const std::string & oldFileName,
                               const std::string & newFileName,
                               const bool isZycor = false ) const;
   
   // \brief Shift coordinates in HDF file
   void shiftHDFCoordinates( const std::string & fileName ) const;
   
   // \brief Anonymize touchstone file
   void processTouchstonFile() const;
   
   // \brief 
   void updateXMLField( pugi::xml_node node,
                        const char* field,
                        const char* value,
                        bool & status ) const;
   
   // Project folder
   std::string m_projectFolder;
   
   // Project file name
   std::string m_projectFile;
   
   // Anonymized project file name
   std::string m_anonymizedProjectFile;

   // File name of the names mapping (original-to-anonymized)
   std::string m_namesMappingFileName;
   
   // X-Y original coordinates
   double m_coordOriginal[2];
   
   // X-Y coordinates shift (shift = original - anonymus)
   double m_coordShift[2];

   // Databse created from project file
   std::shared_ptr<database::Database> m_db;
   
   // Gridmaps names mapping, from GridMapIoTbl::MapName
   std::map< std::string, std::string > m_gridMap;
   
   // Layers names mapping, from StratIoTbl::LayerName
   std::map< std::string, std::string > m_layerNames;
   
   // surfaces names mapping, from StratIoTbl::SurfaceName
   std::map< std::string, std::string > m_surfaceNames;
   
   // Lytho types names mapping, from LithotypeIoTbl::Lithotype
   std::map< std::string, std::string > m_lithoTypes;
   
   // Fluid types names mapping, from FluidtypeIoTbl::Fluidtype
   std::map< std::string, std::string > m_fluidTypes;
   
   // Gridmaps file names mapping
   std::map< std::string, std::string > m_mapFileNames;
   
   // Source rock types names mapping, from SourceRockLithoIoTbl::SourceRockType
   std::map< std::string, std::string > m_sourceRockTypes;
   
   // Source rock types names mapping, from LangmuirAdsorptionCapacityIsothermSetIoTbl::LangmuirName
   // and LangmuirAdsorptionCapacityTOCFunctionIoTbl::LangmuirName
   std::map< std::string, std::string > m_langmuirNames;
   
   // Source rock types names mapping, from ReservoirIoTbl::ReservoirName
   std::map< std::string, std::string > m_reservoirNames;
   
   // Source rock types names mapping, from WellLocIoTbl::WellName
   std::map< std::string, std::string > m_wellNames;
};

#endif
