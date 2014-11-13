//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunCaseSetImpl.C
/// @brief This file keeps definitions for RunCaseSet API implementation

#include "RunCaseSetImpl.h"
#include "RunCaseImpl.h"

#include <utility>

namespace casa
{

   // Destructor
   RunCaseSetImpl::~RunCaseSetImpl()
   {
      for ( size_t i = 0; i < m_caseSet.size(); ++i ) delete m_caseSet[i];
      m_caseSet.clear();
   }

   // Get number of cases
   size_t RunCaseSetImpl::size() const
   {
      return m_filter.empty() ? m_caseSet.size() : m_expIndSet.size();
   }


   // Read only access to i-th element
   RunCase * RunCaseSetImpl::operator[] ( size_t i ) const
   {
      if ( !m_filter.empty() )
      {
         return i < m_expIndSet.size() ? m_caseSet[m_expIndSet[i]] : NULL;
      }

      return i < m_caseSet.size() ? m_caseSet[i] : NULL;
   }


   // Set filter for experiments by experiment name
   void RunCaseSetImpl::filterByExperimentName( const std::string & expName )
   {
      // reset previous filter any case
      m_expIndSet.clear();
      m_filter = expName;

      if ( m_filter.empty() ) return;

      ListOfDoEIndexesSet::iterator ret;

      // check first do we have such experiment?
      ret = m_expSet.find( expName );
      if ( ret != m_expSet.end() )
      {
         m_expIndSet.assign( ret->second.begin(), ret->second.end() );
      }
   }

   // Get all experiment names for this case set as an array
   std::vector< std::string > RunCaseSetImpl::experimentNames() const
   {
      std::vector<std::string> expSet;

      // loop over all experiments and collect names
      for ( ListOfDoEIndexesSet::const_iterator it = m_expSet.begin(); it != m_expSet.end(); ++it )
      {
         expSet.push_back( it->first );
      }

      return expSet;
   }

   // Move a new Cases to the collection and clear array 
   void RunCaseSetImpl::addNewCases( std::vector<RunCase*> & newCases, const std::string & expLabel )
   {

      if ( m_expSet.count( expLabel ) > 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Experiment with name:" << expLabel << "already exist in the set";
      }
      else
      {
         std::vector<size_t> newIndSet( newCases.size() );

         size_t pos = m_caseSet.size(); // position of new inserted elements
         m_caseSet.insert( m_caseSet.end(), newCases.begin(), newCases.end() );

         // add experiment indexes to experiments set
         for ( size_t i = 0; i < newCases.size(); ++i )
         {
            newIndSet[i] = i + pos;
         }
         m_expSet[expLabel] = newIndSet;

         newCases.clear(); // clean container
      }
   }

   // Serialize object to the given stream
   bool RunCaseSetImpl::save( CasaSerializer & sz, unsigned int fileVersion ) const
   {
      bool ok = true;

      // initial implementation of serialization, must exist in all future versions of serialization
      if ( fileVersion >= 0 )
      {
         size_t setSize = m_caseSet.size();
         ok = ok ? sz.save( setSize, "RunCaseSetSize" ) : ok;

         for ( size_t i = 0; i < setSize && ok; ++i )
         {
            ok = ok ? sz.save( *(m_caseSet[i]), "RunCase" ) : ok;
         }

         setSize = m_expSet.size();
         ok = ok ? sz.save( setSize, "MapOfDoEIndSetSize" ) : ok;
         for ( ListOfDoEIndexesSet::const_iterator it = m_expSet.begin(); it != m_expSet.end() && ok; ++it )
         {
            ok = ok ? sz.save( it->first,  "DoEName" )   : ok;
            ok = ok ? sz.save( it->second, "DoEIndSet" ) : ok;
         }
         ok = ok ? sz.save( m_filter,    "CurrentExpFilter" ) : ok;
         ok = ok ? sz.save( m_expIndSet, "CurDoEIndSet" )     : ok;
      }
      return ok;
   }

   // Create a new instance and deserialize it from the given stream
   RunCaseSetImpl::RunCaseSetImpl(CasaDeserializer & dz, const char * objName)
   {
      // read from file object name and version
      std::string  objNameInFile;
      std::string  objType;
      unsigned int objVer;

      bool ok = dz.loadObjectDescription( objType, objNameInFile, objVer );
      if ( objType.compare( typeid(*this).name() ) || objNameInFile.compare( objName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Deserialization error. Can not load object: " << objName;
      }

      if ( version() < objVer )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Version of object in file is newer. No forward compatibility!";
      }

      // RunCases
      size_t setSize;
      ok = ok ? dz.load( setSize, "RunCaseSetSize" ) : ok;

      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         RunCaseImpl * rc = new RunCaseImpl( dz, "RunCase" );
         assert( rc );

         m_caseSet.push_back( rc );
      }

      ok = ok ? dz.load( setSize, "MapOfDoEIndSetSize" ) : ok;
      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         std::string dn;
         std::vector<size_t> is;
         ok = ok ? dz.load( dn, "DoEName" ) : ok;
         ok = ok ? dz.load( is, "DoEIndSet" ) : ok;
         m_expSet[dn] = is;
      }
      ok = ok ? dz.load( m_filter,    "CurrentExpFilter" ) : ok;
      ok = ok ? dz.load( m_expIndSet, "CurDoEIndSet"     ) : ok;

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "RunCaseSetImpl deserialization error";
      }
   }
}
