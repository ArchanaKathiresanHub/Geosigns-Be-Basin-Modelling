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

#include <set>

namespace casa
{
   const std::string RunCaseSetImpl::userDefinedName = "UserDefined";

   RunCaseSetImpl::RunCaseSetImpl()
   {
   }

   RunCaseSetImpl::~RunCaseSetImpl()
   {
   }

   // Get number of cases
   size_t RunCaseSetImpl::size() const
   {
      return m_filter.empty() ? m_caseSet.size() : m_expIndSet.size();
   }


   // Read only access to i-th element
   std::shared_ptr<RunCase> RunCaseSetImpl::operator[] ( size_t i ) const
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

      // check first do we have such experiment?
      ListOfDoEIndexesSet::iterator ret = m_expSet.find( expName );
      if ( ret == m_expSet.end() )
      {
        m_filter = "";
        return;
      }
      m_expIndSet = ret->second;
   }

   void RunCaseSetImpl::filterByDoeList(const std::vector<std::string>& doeList)
   {
     if ( doeList.empty() )
     {
       return;
     }
     m_filter = "DoeList"; // trigger correct getters
     m_expIndSet.clear();

     std::set<size_t> uniqueRunCaseIndex;
     for ( const std::string& doeName : doeList )
     {
       ListOfDoEIndexesSet::iterator ret = m_expSet.find(doeName);
       if ( ret ==  m_expSet.end() )
       {
         continue;
       }
       uniqueRunCaseIndex.insert(ret->second.begin(), ret->second.end());
     }

     m_expIndSet.insert(m_expIndSet.end(), uniqueRunCaseIndex.begin(), uniqueRunCaseIndex.end());
   }

   // Get all experiment names for this case set as an array
   std::vector< std::string > RunCaseSetImpl::experimentNames() const
   {
     return m_experimentNames;
   }

   std::vector<size_t> RunCaseSetImpl::experimentIndexSet( const std::string & expName ) const
   {
     ListOfDoEIndexesSet::const_iterator ret = m_expSet.find(expName);
     if ( ret == m_expSet.end() )
     {
       return {};
     }

     return ret->second;
   }

   // Move new cases to the collection and clear array
   void RunCaseSetImpl::addNewCases( std::vector<std::shared_ptr<RunCase>> & newCases, const std::string & expLabel )
   {
      // assign new ids to
      size_t startID = m_caseSet.empty() ? 0 : (m_caseSet.back()->id() + 1);

      for ( size_t i = 0; i < newCases.size(); ++i )
      {
         RunCaseImpl * rc = dynamic_cast<RunCaseImpl*>( newCases[i].get() );
         if ( rc ) { rc->setID( startID + i ); }
      }

      if ( m_expSet.count( expLabel ) > 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Experiment with name: " << expLabel << " already exists in the set";
      }
      else
      {
         std::vector<size_t> newIndSet( newCases.size() );

         size_t pos = m_caseSet.size(); // position of new inserted elements

         if ( pos )
         {
            // add cases with unique parameters set only
            for ( size_t i = 0; i < newCases.size(); ++i ) // go over all new cases
            {
               bool found = false;
                for ( size_t j = 0; j < pos && !found; ++j ) // check only cases which were in set before
                {
                 if ( *(m_caseSet[j]) == *(newCases[i]) ) // duplicated case
                   {
                      newIndSet[i] = j;   // copy only index of the case
                      found = true;
                   }
               }
               if ( !found ) // normal case, add it to the list
               {
                  newIndSet[i] = m_caseSet.size();
                  m_caseSet.push_back( newCases[i] );
               }
            }
         }
         else // nothing yet to compare with, just add all of new cases
         {
            m_caseSet.insert( m_caseSet.end(), newCases.begin(), newCases.end() );
            for ( size_t i = 0; i < newCases.size(); ++i ) newIndSet[i] = i;
         }

         m_expSet[expLabel] = newIndSet; // put list of experiment case indexes to the map
         m_experimentNames.push_back(expLabel);

         newCases.clear(); // clean container
      }
   }

   void RunCaseSetImpl::addUserDefinedCase(std::shared_ptr<RunCase> newCase)
   {
     m_caseSet.push_back(newCase);

     ListOfDoEIndexesSet::iterator ret = m_expSet.find( userDefinedName );
     bool firstUserDefinedCase = ret == m_expSet.end();

     if (firstUserDefinedCase)
     {
       m_experimentNames.push_back( userDefinedName );
       m_expSet[userDefinedName] = {m_caseSet.size() - 1};
     }
     else
     {
       m_expSet[userDefinedName].push_back( m_caseSet.size() - 1 );
     }
   }

   // collect completed cases for given DoEs name list
   std::vector<const RunCase*> RunCaseSetImpl::collectCompletedCases( const std::vector<std::string> & doeList )
   {
      std::vector<const RunCase *> rcs( m_caseSet.size(), 0 ); // at first use it as mask array
      std::vector<size_t>          rcsIndSet; // here we will collect experiments index

      // the first step collect cases as unique indexes set and use pointers array as a mask array
      for ( size_t i = 0; i < doeList.size(); ++i )
      {
         // look for DoE name
         ListOfDoEIndexesSet::iterator ret = m_expSet.find( doeList[i] );

         if ( ret == m_expSet.end() ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Can not find DoE with name: " << doeList[i];

         const std::vector<size_t> & doeIndSet = ret->second;

         // collect RunCases pointer and index for the DoE
         for ( size_t j = 0; j < doeIndSet.size(); ++j )
         {
            if ( !rcs[ doeIndSet[j] ] ) //use RunCase pointer as a mask value to avoid duplicated cases
            {
               rcs[doeIndSet[j]] = m_caseSet[doeIndSet[j]].get();
               rcsIndSet.push_back( doeIndSet[j] );
            }
         }
      }

      // the second step - check cases for completion and collect them in to array
      rcs.clear(); // clear mask array and use it as container of the RunCases now
      for ( size_t i = 0; i < rcsIndSet.size(); ++i )
      {
         if ( m_caseSet[rcsIndSet[i]]->runStatus() == RunCase::Completed )
         {
            rcs.push_back( m_caseSet[rcsIndSet[i]].get());
         }
      }
      return rcs;
   }

   // Serialize object to the given stream
   bool RunCaseSetImpl::save( CasaSerializer & sz ) const
   {
      bool ok = true;

      // initial implementation of serialization, must exist in all future versions of serialization
      size_t setSize = m_caseSet.size();
      ok = ok ? sz.save( setSize, "RunCaseSetSize" ) : ok;

      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         ok = ok ? sz.save( *(m_caseSet[i]), "RunCase" ) : ok;
      }

      setSize = m_expSet.size();
      ok = ok ? sz.save( setSize, "MapOfDoEIndSetSize" ) : ok;
      for ( std::vector<std::string>::const_iterator it = m_experimentNames.begin(); it != m_experimentNames.end() && ok; ++it )
      {
         ok = ok ? sz.save( *it,  "DoEName" )   : ok;
         ok = ok ? sz.save( m_expSet.find(*it)->second, "DoEIndSet" ) : ok;
      }
      ok = ok ? sz.save( m_filter,    "CurrentExpFilter" ) : ok;
      ok = ok ? sz.save( m_expIndSet, "CurDoEIndSet" )     : ok;
      return ok;
   }

   // Create a new instance and deserialize it from the given stream
   RunCaseSetImpl::RunCaseSetImpl(CasaDeserializer & dz, const char * objName)
   {
      // read from file object name and version
      unsigned int objVer = version();
      bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

      // RunCases
      size_t setSize;
      ok = ok ? dz.load( setSize, "RunCaseSetSize" ) : ok;

      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         std::shared_ptr<RunCase>  rc( new RunCaseImpl( dz, "RunCase" ) );
         assert( rc.get() );

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
         m_experimentNames.push_back(dn);
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
