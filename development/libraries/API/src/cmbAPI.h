//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
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

#include <memory>

#include "ErrorHandler.h"
#include "LithologyManager.h"
#include "SourceRockManager.h"
#include "StratigraphyManager.h"
#include "FluidManager.h"

/// @mainpage Cauldron APIs
/// @tableofcontents
/// This documentation describes set of APIs which allows 3d party applications interact with Cauldron.
///
/// The following APIs are available now:
/// -# \subpage CMB_API_Descr
/// -# \subpage CASA_API_Descr
/// -# \subpage ErrorHandlerPage

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
///
/// @page ClassHierachyPage Cauldron Model hierarchy description.
/// The top level class is the mbapi::Model . It includes and provides access to the following set of classes:
///   -# mbapi::StratigraphyManager - for manipulating by layers and surfaces
///   -# mbapi::FluidManager - for manipulating fluids
///   -# mbapi::SourceRockManager - for manipulating source rocks
///   -# mbapi::LithologyManager for manipulating lithologies

/// @brief Namespace which keeps API to manipulate Cauldron model
namespace mbapi {

#define UndefinedDoubleValue  -9999      ///< Undefined value for floating point numbers
#define UndefinedIntegerValue -1         ///< Undefined value for integer numbers
#define UndefinedStringValue  "undef"    ///< Undefined value for strings

   /// @class Model cmbAPI.h "cmbAPI.h"
   /// @brief Class Model keeps the Cauldron data model insides and provides a set of interfaces to work with the model
   class Model : public ErrorHandler
   {
   public:
      /// @{
      /// Types definitions
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

      /// @brief Copy model, creates a deep copy of the model
      /// @param[in] otherModel - model to copy
      Model & operator = ( const Model & otherModel );

      // Set of universal access interfaces. Project file level

      /// @brief Get size of the given table
      /// @param[in] tableName name of the table in project file
      /// @return number of rows in table or UndefinedIntegerValue if any error happened.
      int tableSize( const std::string & tableName );

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
      ErrorHandler::ReturnCode setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue );

      /// @brief Set value in the table
      /// @param tableName name of the table in project file
      /// @param rowNumber row number in the table
      /// @param propName name of the column
      /// @param propValue value to be set in the table
      /// @return ErrorHandler::NoError on success, error code otherwise
      ErrorHandler::ReturnCode setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, const std::string & propValue );



      // IO file load/save methods

      /// @brief Clean all parameters of the model and load the new model parameters from the given project file
      ///        If file can't be opened, red or not valid it return error
      ///
      /// @param[in] projectFileName name of the Cauldron project1d/project3d file
      /// @return NoError in case of success, error code otherwise
      ReturnCode loadModelFromProjectFile( const char * projectFileName );

      /// @brief Save the model to the Cauldron project project1d/project3d file under the given file name
      ///        If file exist, it will be overwritten, if the writing is not possible or failed, interface will
      ///        throw an exception
      ///
      /// @param projectFileName the name for the Cauldron project file
      /// @return NoError in case of success, error code otherwise. The error message could be retrieved with getAPIErrorMessage call
      ReturnCode saveModelToProjectFile( const char * projectFileName );

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
     
      /// @}

   private:
      /// @{
      /// Implementation part
      /// @brief Class which hides all interface implementation
      class ModelImpl;
      
      std::auto_ptr<ModelImpl> m_pimpl; ///< The actual object which will do all the job

      /// @brief Copy constructor is disabled, use the copy operator instead
      Model( const Model & );

      /// @}
   };
} // mbapi

#endif // CMB_API