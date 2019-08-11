//
// Copyright (C) 2012-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file cmbAPI.h
/// @brief This file keeps API declaration for creating and manipulating Cauldron data model

#ifndef CMB_API
#define CMB_API

//CMB API
#include "ErrorHandler.h"
#include "LithologyManager.h"
#include "PropertyManager.h"
#include "SnapshotManager.h"
#include "SourceRockManager.h"
#include "StratigraphyManager.h"
#include "FluidManager.h"
#include "MapsManager.h"
#include "ReservoirManager.h"
#include "UndefinedValues.h"
#include "BiodegradeManager.h"
#include "BottomBoundaryManager.h"
#include "CtcManager.h"
#include "FracturePressureManager.h"
#include "ProjectDataManager.h"
#include "TopBoundaryManager.h"

// TableIO
#include "datatype.h"

// DataAccess
#include "ProjectData.h"

// STL
#include <memory>
#include <set>

/// @mainpage Cauldron APIs
/// @tableofcontents
/// This documentation describes set of APIs which allows 3d party applications interact with Cauldron.
///
/// The following APIs are available now:
/// -# @subpage CMB_API_Descr
/// -# @subpage CASA_API_Descr
/// -# @subpage ErrorHandlerPage

/// @page ErrorHandlerPage Error handling in CMB and CASA APIs
/// Because of using Cauldron APIs in multi programming language environment, the error handling in the API is 
/// implemented in the following way. 
///
/// Each object which supposed to be used by API user inherited
/// from @link ErrorHandler ErrorHandler @endlink base class which allows to keep error message and error code. If during 
/// any API call error is happened, API object set up an error message inside itself and return an
/// error code. User can analyze an error code and ask object for more detailed explanation by
/// asking for error message. This C-like error handling allows to implement thread safe error
/// handling and operate by simple objects like integer error code and char* string to simplify
/// C++ - C# interface usage.

/// @page CMB_API_Descr Cauldron Model Building API
/// Cauldron Model Building (CMB) API provides a set of classes for creating and modifying of the Cauldron model
/// in memory. It is built as a layer over DataAccess library. It allows to change and verify the cauldron data model 
/// parameters.
///
/// This manual is divided in the following sections:
/// -# \subpage ClassHierachyPage
/// -# \subpage StratigraphyManagerPage
/// -# \subpage LithologyManagerPage
/// -# \subpage SourceRockManagerPage
/// -# \subpage FluidManagerPage
/// -# \subpage MapsManagerPage
///
/// @page ClassHierachyPage Cauldron Model hierarchy description.
/// The top level class is the mbapi::Model . It includes and provides access to the following set of classes:
///   -# mbapi::StratigraphyManager - for manipulating by layers and surfaces
///   -# mbapi::FluidManager - for manipulating fluids
///   -# mbapi::SourceRockManager - for manipulating source rocks
///   -# mbapi::LithologyManager for manipulating lithologies
///   -# mbapi::MapsManager for manipulating 2D input maps

/// @brief Namespace which keeps API to manipulate Cauldron model
namespace mbapi {

   /// @class Model cmbAPI.h "cmbAPI.h"
   /// @brief Class Model keeps the Cauldron data model insides and provides a set of interfaces to work with the model
   class Model : public ErrorHandler
   {
   public:
      /// @{
      /// Types definitions
      /// @}
 
      /// @{
      /// Global constants definition
      static const char * s_ResultsFolderSuffix; ///< defines Cauldron results folder name suffix
      /// @}

      /// @{
      /// Static methods
      /// @brief Generates random string with the given leng
      /// @return string which consists of random low/uppercase characters and numbers
      static std::string randomString( size_t len );
      /// @}

      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
      Model();

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      ~Model();
      /// @}

      /// @{
      /// Set of interfaces for interacting with a Cauldron model.\n
      /// Interfaces were simplified to allow easy access from C# using Swig.\n
      /// For interfaces which returns double or string values, user can request error code
      /// and error message from the model itself, after the interface call.

      /// @brief Compare projects and return all differences found
      /// @return full list of differences as a string
      std::string compareProject( Model & mdl1                                  ///< the model to compare with
                                , const std::set<std::string> & compareTblsList ///< list of tables to compare, empty to compare all tables
                                , const std::set<std::string> & ignoreTblsList  ///< list of tables to ignore them during comparison
                                , double relTol                                 ///< relative tolerance value to compare float point values
                                );

      /// @brief Copy matched given filter records from the given project to the current, all similar records in 
      ///        the currenct projects will be deleted and replaced
      /// @return empty string on success or error message
      std::string mergeProject( Model & mdl1                                               ///< the model to merge from
                              , const std::set<std::string> & procesTblsList               ///< list of tables to process, must not be empty
                              , const std::vector< std::vector< std::string > > & flitList ///< filter list (process matched to filter records only)
                              , size_t                                          & dlRecNum ///< deleted records num in the current project
                              , size_t                                          & cpRecNum ///< copied recrords num from the given project
                              );

      /// @brief Copy model, creates a deep copy of the model
      /// @param[in] otherModel - model to copy
      Model & operator = ( const Model & otherModel );

      /// @brief Sort record in the table
      /// @param tblName table name to sort
      /// @param colsName list of columns to use in sort
      /// @return NoError on success, or error code otherwise
      ErrorHandler::ReturnCode tableSort( const std::string & tblName, const std::vector<std::string> & colsName );

      // Set of universal access interfaces. Project file level
      /// @brief Get all table names in project
      /// @return list of tables from project as an array
      std::vector<std::string> tablesList();

      /// @brief Get list of all column names in the given table, and datatype for each column
      /// @param[in] tableName name of the table
      /// @param[out] colDataTypes for each table column it keeps data type lid double/string/integer
      /// @return list of column names for the given table on success, or empty list on any fail
      std::vector<std::string> tableColumnsList( const std::string & tableName, std::vector<datatype::DataType> & colDataTypes );

      /// @brief Get size of the given table
      /// @param[in] tableName name of the table in project file
      /// @return number of rows in table or UndefinedIntegerValue if any error happened.
      int tableSize( const std::string & tableName );

      /// @brief Get value from the table
      /// @param tableName name of the table in project file
      /// @param rowNumber row number in the table
      /// @param propName name of the column
      /// @return requested value from the table or  UndefinedIntegerValue if any error happened 
      long tableValueAsInteger( const std::string & tableName, size_t rowNumber, const std::string & propName );

      /// @brief Get value from the table
      /// @param tableName name of the table in project file
      /// @param rowNumber row number in the table
      /// @param propName name of the column
      /// @return requested value from the table or  UndefinedDoubleValue if any error happened 
      double tableValueAsDouble( const std::string & tableName, size_t rowNumber, const std::string & propName );

      /// @brief Get value from the table
      /// @param tableName name of the table in project file
      /// @param rowNumber row number in the table
      /// @param propName name of the column
      /// @return requested value from the table. or UndefinedStringValue if any error happened
      std::string tableValueAsString( const std::string & tableName, size_t rowNumber, const std::string & propName );

      /// @brief Set value in the table
      /// @param tableName name of the table in project file
      /// @param rowNumber row number in the table
      /// @param propName name of the column
      /// @param propValue value to be set in the table
      /// @return ErrorHandler::NoError on success, error code otherwise
      ErrorHandler::ReturnCode setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, long propValue );

      /// @brief Set value in the table
      /// @param tableName name of the table in project file
      /// @param rowNumber row number in the table
      /// @param propName name of the column
      /// @param propValue value to be set in the table
      /// @return ErrorHandler::NoError on success, error code otherwise
      ErrorHandler::ReturnCode setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue );

      /// @brief Set value in the table
      /// @return ErrorHandler::NoError on success, error code otherwise
      ErrorHandler::ReturnCode setTableValue( const std::string & tableName  ///< name of the table in project file
                                            , size_t              rowNumber  ///< row number in the table
                                            , const std::string & propName   ///< name of the column
                                            , const std::string & propValue  ///< value to be set in the table
                                            );

      /// @brief Delete all rows in given table
      /// @return ErrorHandler::NoError on success, error code otherwise
      ErrorHandler::ReturnCode clearTable( const std::string & tableName );

      /// @brief Remove a record from a table
      /// @param tableName name of the table in project file
      /// @param ind the record to remove
      /// @return ErrorHandler::NoError on success, error code otherwise
      ErrorHandler::ReturnCode removeRecordFromTable( const std::string & tableName, int ind );

      /// @brief Add a new row
      /// @param tableName name of the table in project file
      /// @return ErrorHandler::NoError on success, error code otherwise
      ErrorHandler::ReturnCode addRowToTable( const std::string & tableName );

      // IO file load/save methods

      /// @brief Clean all parameters of the model and load the new model parameters from the given project file
      ///        If file can't be opened, red or not valid it return error
      ///
      /// @param[in] projectFileName name of the Cauldron project1d/project3d file
      /// @return NoError in case of success, error code otherwise
      ReturnCode loadModelFromProjectFile( const char * projectFileName );

      /// @brief Save the model to the Cauldron project project1d/project3d file under the given file name
      ///        If file exist, it will be overwritten
      ///
      /// @param projectFileName the name for the Cauldron project file
      /// @param copyFiles if copyFiles is false - results files will be linked where it is possible otherwise they will be copied
      /// @return NoError in case of success, error code otherwise.
      ReturnCode saveModelToProjectFile( const char * projectFileName, bool copyFiles = false );

      /// @brief Get project file name
      /// @return project file name or empty string if project wasn't loaded or saved before
      std::string projectFileName();

      /// @brief Get the data-access project handle
      /// @return The project handle
      std::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle();

      // Access to some project functionality

      /// @brief Get model stratigraphy manager. It allows manipulate model startigraphy
      /// @return reference to the model stratigraphy. It created/deleted by the Model itself.
      StratigraphyManager & stratigraphyManager();

      /// @brief Get lithology manager. It keeps list of the model lithologies and allows to manipulate them
      /// @return reference to the lithologies manager. It created/deleted by the model itself.
      LithologyManager & lithologyManager();

      /// @brief Get fluid manager. It keeps a list of fluids in the model and allows to manipulate them
      /// @return reference to the fluids manager. It created/deleted by the model itself.
      FluidManager & fluidManager();

      /// @brief Get source rock manager. It keeps a list of source rocks in the model and allows to manipulate them
      /// @return reference to the source rock manager. It created/deleted by the model itself.
      SourceRockManager & sourceRockManager();
     
      /// @brief Get snapshot manager. It provides access to the snapshot table in project file
      /// @return reference to snapshot manager
      SnapshotManager & snapshotManager();
 
      /// @brief Get property manager. It provides access to the FilterTimeIoTable in project file
      /// @return reference to property manager
      PropertyManager & propertyManager();

      /// @brief Get input maps manager. It provides access to the GridMapIoTbl in project file
      /// @return reference to maps manager
      MapsManager & mapsManager();

      /// @brief Get input maps manager. It provides access to the GridMapIoTbl in project file
      /// @return reference to maps manager
      ReservoirManager & reservoirManager();
      /// @brief Get Biodegradation manager. It provides access to the BioDegradIoTbl in project file 
      /// @return reference to the Biodegradation manager. It created/deleted by the model itself.
      BiodegradeManager & biodegradeManager();

	  /// @brief Get Bottom Boundary manager. It provides access to the BasementIoTbl in project file 
	  /// @return reference to the Bottom Boundary manager. It created/deleted by the model itself.
	  BottomBoundaryManager & bottomBoundaryManager();

	  /// @brief Get Top Boundary manager. It provides access to the SurfaceDepthIoTbl and SurfaceTempIoTbl in project file 
	  /// @return reference to the Top Boundary manager. It created/deleted by the model itself.
	  TopBoundaryManager & topBoundaryManager();

     /// @brief Get CTC manager. It provides access to the CTCIoTbl in project file 
     /// @return reference to the CTC manager. It created/deleted by the model itself.
     CtcManager & ctcManager();

     /// @brief Get Fracture Pressure manager. It provides access to the RunOptionsIoTbl/PressureFuncIoTbl in project file 
     /// @return reference to the Fracture Pressure manager. It created/deleted by the model itself.
     FracturePressureManager & fracturePressureManager();

     /// @brief Get Project Data manager. It provides access to the ProjectIoTbl in project file 
     /// @return reference to the Project Data manager. It created/deleted by the model itself.
     ProjectDataManager& projectDataManager();


      // Request some project porperties

      /// @brief Get basin model origin areal position
      /// @param[out] x x coordinate [m] for the project origin
      /// @param[out] y y coordinate [m] for the project origin
      /// @return ErrorHandler::NoError on success, or error code otherwise
      ReturnCode origin( double & x, double & y );

      /// @brief Get dimensions of hi resolution grid
      /// @param[out] sizeI grid dimension in I direction
      /// @param[out] sizeJ grid dimension in J direction
      /// @return ErrorHandler::NoError on success or error code otherwise
      ReturnCode hiresGridArealSize( long & sizeI, long & sizeJ );

      /// @brief Get grid subsampling for the model
      /// @param[out] di grid subsampling step in I direction
      /// @param[out] dj grid subsampling step in J direction
      /// @return ErrorHandler::NoError on success, or error code otherwise
      ReturnCode subsampling( long & di, long & dj );

      /// @brief Set grid subsampling for the model
      /// @param di grid subsampling step in I direction
      /// @param dj grid subsampling step in J direction
      /// @return ErrorHandler::NoError on success, or error code otherwise
      ReturnCode setSubsampling( long di, long dj );

      /// @brief Get model window position
      /// @param[out] minWinI window start postion in I direction
      /// @param[out] maxWinI window end   postion in I direction
      /// @param[out] minWinJ window start postion in J direction
      /// @param[out] maxWinJ window end   postion in J direction
      /// @return ErrorHandler::NoError on success, or error code otherwise
      ReturnCode window( long & minWinI, long & maxWinI, long & minWinJ, long & maxWinJ );
 
      /// @brief Define model window position
      /// @param minWinI window start postion in I direction
      /// @param maxWinI window end   postion in I direction
      /// @param minWinJ window start postion in J direction
      /// @param maxWinJ window end   postion in J direction
      /// @return ErrorHandler::NoError on success, or error code otherwise
      ReturnCode setWindow( long minWinI, long maxWinI, long minWinJ, long maxWinJ );
      
      /// @brief Get basin model areal dimenstions
      /// @param[out] dimX length [m] of the model along X axis
      /// @param[out] dimY length [m] of the model along Y axis
      /// @return ErrorHandler::NoError on success, or error code otherwise
      ReturnCode arealSize( double & dimX, double & dimY );

      /// @brief Get the window extension for multi1D projects
      /// @param[out] x first x coordinate of the well
      /// @param[out] y first y coordinate of the well
      /// @param[out] xMin minimum x node
      /// @param[out] xMax maximum x node
      /// @param[out] yMin minimum y node
      /// @param[out] yMax maximum y node
      /// @param[out] xc window center x coordinate
      /// @param[out] yc window center y coordinate
      /// @return ErrorHandler::NoError on success, or error code otherwise
      ReturnCode windowSize( double x, double y, int & xMin, int & xMax, int & yMin, int & yMax, double & xc, double & yc );

      // Natural neighbour interpolation of the lithofractions

      /// @brief Transform lithofractions, set up interpolation points at the edge of the domain and interpolate
      /// @return ErrorHandler::NoError on success, or error code otherwise
      ReturnCode interpolateLithoFractions( const std::vector<double> & xin    ///< [in]  x coordinates of the wells 
                                          , const std::vector<double> & yin    ///< [in]  y coordinates of the wells
                                          , const std::vector<double> & lf1    ///< [in]  first litofraction
                                          , const std::vector<double> & lf2    ///< [in]  second litofraction
                                          , const std::vector<double> & lf3    ///< [in]  third litofraction
                                          , std::vector<double>       & xInt   ///< [out] x coordinates of the interpolated points
                                          , std::vector<double>       & yInt   ///< [out] y coordinates of the interpolated points
                                          , std::vector<double>       & rpInt  ///< [out] interpolated values
                                          , std::vector<double>       & r13Int ///< [out] interpolated values
                                          );

      /// @brief Correct and back-transform rp and r13 to real lithofraction values
      /// @return ErrorHandler::NoError on success, or error code otherwise
      ReturnCode backTransformLithoFractions( const std::vector<double> & rpInt      ///< [in]  interpolated values
                                            , const std::vector<double> & r13Int     ///< [in]  interpolated values 
                                            , std::vector<double>       & lf1CorrInt ///< [out] corrected interpolated first lithofraction
                                            , std::vector<double>       & lf2CorrInt ///< [out] corrected interpolated second lithofraction
                                            , std::vector<double>       & lf3CorrInt ///< [out] corrected interpolated third lithofraction
                                            );

      ///@}

      // Modify project methods
      /// @brief Create the unique copies of lithology for each given layer, alochtonous lithology and fault cut from the given lists
      /// @return array of newly created lithologies name in order of layers->alochtonous lithologies->fault cuts
      std::vector<std::string> copyLithology(
              const std::string                                       & litName       ///< the original lithology name
            , const std::vector< std::pair<std::string, size_t> >     & layersName    ///< list of layers name with lithology mixing ids
            , const std::vector<std::string>                          & alochtLitName ///< list of layers with alochtonous lithologies
            , const std::vector<std::pair<std::string, std::string> > & faultsName    ///< list of pairs map name - fault cut name
                                            );


      /// @brief determine if a particular point lies within the layer
      /// @return true is comprised, false otherwise
      bool checkPoint( const double x, const double y, const double z, const std::string & layerName );


      /// @brief get the depth values of one specific surface
      /// @return true is the grid values could be retrived, false otherwise
      bool getGridMapDepthValues( const mbapi::StratigraphyManager::SurfaceID s, std::vector<double> & v );

   private:
      /// @{
      /// Implementation part
      /// @brief Class which hides all interface implementation
      class ModelImpl;
      
      std::unique_ptr<ModelImpl> m_pimpl; ///< The actual object which will do all the job

      /// @brief Copy constructor is disabled, use the copy operator instead
      Model( const Model & );

      /// @}
   };
} // mbapi

#endif // CMB_API
