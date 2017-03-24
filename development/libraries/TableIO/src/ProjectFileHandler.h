//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef TABLE_IO__PROJECT_FILE_HANDLER__H
#define TABLE_IO__PROJECT_FILE_HANDLER__H

// TableIO
#include "database.h"

// Sytem
#include <memory>
#include <ostream>
#include <string>
#include <memory>

namespace database {

   /// \brief Handles databases for both the project file and the output tables file.
   ///
   /// The tables are spread over two database objects, one for the input project file
   /// and another for the output tables file.
   class ProjectFileHandler {

   public :

      /// \brief The name of the table that will contain the file name for the output tables.
      static const std::string OutputTablesFileIoTableName;

      /// \brief The name of the table that will contain the list of tables that are in the output tables file.
      static const std::string OutputTablesIoTableName;

      //// \brief The default name for the output tables file.
      static const std::string OutputTablesFileName;


      /// \brief Constructor creates the empty database from schema
      ProjectFileHandler ();

      /// \brief Constructor loads the project file into the database.
      ///
      /// Also loads the output tables if output tables have been set in project file.
      ProjectFileHandler ( const std::string& fileName );

      /// \brief Constructor loads the project file into the database.
      ///
      /// Loads the output tables if output tables have been set in project file.
      /// Also make additional tables as output tables if they are not already.
      ProjectFileHandler ( const std::string&              fileName,
                           const std::vector<std::string>& outputTableNames );

      /// \brief Disable copy construction.
      ProjectFileHandler ( const ProjectFileHandler& copy ) = delete;

      /// \brief Destructor.
      ~ProjectFileHandler ();


      /// \brief Disable copying.
      ProjectFileHandler& operator=( const ProjectFileHandler& copy ) = delete;

      /// \brief Save the database to a file.
      ///
      /// Both input and output database objects will be saved.
      bool saveToFile ( const std::string& fileName );

      /// \brief Write the intput database to a stream.
      bool saveInputDataBaseToStream ( std::ostream& os );

      /// \brief Get a table by its name.
      ///
      /// A nullptr will be returned if the table cannot be found
      /// in either the input or output databases.
      database::Table* getTable ( const std::string& tableName ) const;

      /// \brief Set a table as an output table.
      ///
      /// If the table is in the intput database then the table is moved
      /// from here to the output database. The records are copied.
      /// An output database will be created is it does not exist already.
      /// A true will be returned if the table was set as an output table.
      bool setTableAsOutput ( const std::string& tableName );

      /// \brief Merges tables from the output database into the input databse.
      ///
      /// The tables containing information about the output tables file and the
      /// output table names will be cleared. The output database will be deleted.
      void mergeTablesToInput ();

      /// \brief Get a set of all the tables.
      const std::vector<std::string>& getAllTableNames () const;

      /// \brief Get the name of the project file used to create this object.
      const std::string& getFileName () const;

   private :

      /// \brief Load the project file into the intput tables database.
      ///
      /// When opening a project file, all the tables will in in the input schema
      /// so the list of all table names can be created here.
      void loadFromFile ( const std::string& fileName );

      /// \brief Load the output tables file into the output tables database.
      ///
      /// Tables are moved from the input schema to the output schema.
      void loadOutputTables ();

      /// \brief Helper function to strip the extension from the project file name.
      std::string stripExtension ( const std::string& fileName ) const;


      /// \brief Contains tables that are part of the project file
      std::unique_ptr<database::Database> m_inputDataBase;

      /// \brief Contains tables that are to be saved in a file separated from the project file.
      std::unique_ptr<database::Database> m_outputDataBase;

      /// \brief A list of all the tables contained in both input and output databases.
      std::vector<std::string> m_allTableNames;

   };

   typedef std::shared_ptr<ProjectFileHandler> ProjectFileHandlerPtr;

   typedef std::shared_ptr<const ProjectFileHandler> ProjectFileHandlerConstPtr;

}

//
// Inline functions.
//

inline const std::vector<std::string>& database::ProjectFileHandler::getAllTableNames () const {
   return m_allTableNames;
}


inline const std::string& database::ProjectFileHandler::getFileName () const {
   return m_inputDataBase->getFileName();
}

#endif // TABLE_IO__PROJECT_FILE_HANDLER__H
