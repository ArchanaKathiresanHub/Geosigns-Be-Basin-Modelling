//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file SourceRockManagerImpl.C
/// @brief This file keeps API implementation for manipulating source rocks in Cauldron model

// CMB API
#include "cmbAPI.h"
#include "SourceRockManagerImpl.h"

// TableIO lib
#include "database.h"

// Genex6_kernel lib
#include "GenexSourceRock.h"

// STL
#include <stdexcept>
#include <string>

namespace mbapi
{
const char * SourceRockManagerImpl::s_sourceRockTableName     = "SourceRockLithoIoTbl";
const char * SourceRockManagerImpl::s_layerNameFieldName      = "LayerName";
const char * SourceRockManagerImpl::s_sourceRockTypeFieldName = "SourceRockType";
const char * SourceRockManagerImpl::s_baseSourceRockTypeFieldName = "BaseSourceRockType";
const char * SourceRockManagerImpl::s_tocIni                  = "TocIni";
const char * SourceRockManagerImpl::s_tocIniMap               = "TocIniGrid";
const char * SourceRockManagerImpl::s_hcIni                   = "HcVRe05";
const char * SourceRockManagerImpl::s_scIni                   = "ScVRe05";
const char * SourceRockManagerImpl::s_PreAsphaltStartAct      = "PreAsphaltStartAct";
const char * SourceRockManagerImpl::s_applyAdsorption         = "ApplyAdsorption";
const char * SourceRockManagerImpl::s_adsorptionTOCDependent  = "AdsorptionTOCDependent";
const char * SourceRockManagerImpl::s_computeOTGC             = "ComputeOTGC";
const char * SourceRockManagerImpl::s_adsorptionCapacityFunctionName = "AdsorptionCapacityFunctionName";
const char * SourceRockManagerImpl::s_whichAdsorptionSimulator= "WhichAdsorptionSimulator";
const char * SourceRockManagerImpl::s_asphalteneDiffusionEnergy = "AsphalteneDiffusionEnergy";
const char * SourceRockManagerImpl::s_resinDiffusionEnergy  = "ResinDiffusionEnergy";
const char * SourceRockManagerImpl::s_C15AroDiffusionEnergy = "C15AroDiffusionEnergy";
const char * SourceRockManagerImpl::s_C15SatDiffusionEnergy = "C15SatDiffusionEnergy";
const char * SourceRockManagerImpl::s_vreOptimization		= "VREoptimization";
const char * SourceRockManagerImpl::s_vreThreshold			= "VREthreshold";
const char * SourceRockManagerImpl::s_vesLimitIndicator     = "VESLimitIndicator";
const char * SourceRockManagerImpl::s_vesLimit              = "VESLimit";


// Constructor
SourceRockManagerImpl::SourceRockManagerImpl()
{
   //throw std::runtime_error( "Not implemented yet" );
   m_db = NULL;
}

// Copy operator
SourceRockManagerImpl & SourceRockManagerImpl::operator = ( const SourceRockManagerImpl & /*otherSrRockMgr*/ )
{
   throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "SourceRockManagerImpl::operator = () not implemented yet";
   return *this;
}

// Set project database. Reset all
void SourceRockManagerImpl::setDatabase( database::ProjectFileHandlerPtr pfh )
{
   m_db = pfh;
}

// Get list of source rocks in the model
// return array with IDs of different source rock lithologies defined in the model
std::vector<SourceRockManager::SourceRockID> SourceRockManagerImpl::sourceRockIDs( ) const
{
   std::vector<SourceRockID> srIDs;
   if ( !m_db ) return srIDs;

   // get pointer to the table
   database::Table * table = m_db->getTable( s_sourceRockTableName );

   // if table does not exist - return empty array
   if ( !table ) return srIDs;

   // fill IDs array with increasing indexes
   srIDs.resize( table->size(), 0 );

   for ( size_t i = 0; i < srIDs.size(); ++i ) srIDs[ i ] = static_cast<SourceRockID>( i );

   return srIDs;
}

// Search for source rock lithology record which has given layer name and source rock type name
// return ID of found source rock lithology on success or Utilities::Numerical::NoDataIDValue otherwise
SourceRockManager::SourceRockID SourceRockManagerImpl::findID( const std::string & lName
                                                             , const std::string & srTypeName
                                                             )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      size_t tblSize = table->size();
      for ( size_t i = 0; i < tblSize; ++i )
      {
         database::Record * rec = table->getRecord( static_cast<int>( i ) );
         if ( !rec )
         {
            throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << i;
         }

         if ( lName      == rec->getValue<std::string>( s_layerNameFieldName ) &&
              srTypeName == rec->getValue<std::string>( s_sourceRockTypeFieldName )
            )
         {
            return static_cast<SourceRockManager::SourceRockID>(i);
         }
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::NoDataIDValue;
}


// Create new source rock
// return ID of the new SourceRock
SourceRockManager::SourceRockID SourceRockManagerImpl::createNewSourceRockLithology()
{
   throw std::runtime_error( "Not implemented yet" );
}

// Get layer name for the source rock lithology
std::string SourceRockManagerImpl::layerName( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();
   std::string layName;

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      layName = rec->getValue<std::string>( s_layerNameFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return layName;
}

// Get source rock type name for source rock lithology
std::string SourceRockManagerImpl::sourceRockType( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();
   std::string tpName;

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      tpName = rec->getValue<std::string>( s_sourceRockTypeFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return tpName;
}

// Set both source rock type and base source rock type for source rock lithology
ErrorHandler::ReturnCode SourceRockManagerImpl::setSourceRockType(const SourceRockID id, const std::string & newSourceRock)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_sourceRockTypeFieldName, newSourceRock);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}

// Get base source rock type name for source rock lithology
std::string SourceRockManagerImpl::baseSourceRockType(SourceRockID id)
{
	if (errorCode() != NoError) resetError();
	std::string tpName;

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		tpName = rec->getValue<std::string>(s_baseSourceRockTypeFieldName);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return tpName;
}

// Set both source rock type and base source rock type for source rock lithology
ErrorHandler::ReturnCode SourceRockManagerImpl::setBaseSourceRockType(const SourceRockID id, const std::string & newBaseSourceRock)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_baseSourceRockTypeFieldName, newBaseSourceRock);		
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}

double SourceRockManagerImpl::tocIni( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      return rec->getValue<double>( s_tocIni );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }
   return Utilities::Numerical::IbsNoDataValue;
}

// get layer TOC map name
std::string SourceRockManagerImpl::tocInitMapName( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      return rec->getValue<std::string>( s_tocIniMap );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::NoDataStringValue;
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setTOCIni( SourceRockID id, double newTOC )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      if ( (newTOC < 0.0 || newTOC > 100.0 ) && !IsValueUndefined( newTOC ) )
      {
         throw Exception( OutOfRangeValue ) << "TOC value must be in range [0:100] but given is: " << newTOC;
      }
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord(  static_cast<int>( id ) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      rec->setValue( s_tocIni, newTOC );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setTOCInitMapName( SourceRockID id, const std::string & mapName )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord(  static_cast<int>( id ) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      rec->setValue( s_tocIniMap, mapName );
      rec->setValue( s_tocIni,    Utilities::Numerical::IbsNoDataValue ); // TOC must be -9999
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

double SourceRockManagerImpl::hiIni( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      double hcIni = rec->getValue<double>( s_hcIni );
      return Genex6::GenexSourceRock::convertHCtoHI( hcIni );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::IbsNoDataValue;
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setHIIni( SourceRockID id, double newHI )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      if ( newHI < 0.0 || newHI > 1000.0 )
      {
         throw Exception( OutOfRangeValue ) << "HI value must be in range [0:1000], but given is: " << newHI;
      }
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>( id ) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      double hcIni = Genex6::GenexSourceRock::convertHItoHC( newHI );
      rec->setValue( s_hcIni, hcIni );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

double SourceRockManagerImpl::hcIni( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      return rec->getValue<double>( s_hcIni );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::IbsNoDataValue;
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setHCIni( SourceRockID id, double newHC )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      if ( newHC < 0.0 || newHC > 2.0 )
      {
         throw Exception( OutOfRangeValue ) << "H/C value must be in range [0:2] but given is: " << newHC;
      }

      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      rec->setValue( s_hcIni, newHC );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

double SourceRockManagerImpl::scIni(SourceRockID id)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		// if record does not exist report error
		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}

		return rec->getValue<double>(s_scIni);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return Utilities::Numerical::IbsNoDataValue;
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setSCIni(SourceRockID id, double newSC)
{
	if (errorCode() != NoError) resetError();

	try
	{
		if (newSC < 0.0 || newSC > 2.0)
		{
			throw Exception(OutOfRangeValue) << "H/C value must be in range [0:2] but given is: " << newSC;
		}

		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_scIni, newSC);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}

// Get pre-asphaltene activation energy [kJ/mol]
double SourceRockManagerImpl::preAsphActEnergy( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      return rec->getValue<double>( s_PreAsphaltStartAct );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::IbsNoDataValue;
}

// Set pre-asphaltene activation energy (must be in range 200-220 kJ/mol)
ErrorHandler::ReturnCode SourceRockManagerImpl::setPreAsphActEnergy( SourceRockID id, double newVal )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      if ( newVal < 100.0 || newVal > 300.0 )
      {
         throw Exception( OutOfRangeValue ) << "pre-asphaltene activation energy  value must be in range [100:300]" <<
                                               " but given is: " << newVal;
      }

      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      rec->setValue( s_PreAsphaltStartAct, newVal );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Get asphaltene diffusion energy [kJ/mol]
double SourceRockManagerImpl::getAsphalteneDiffusionEnergy(SourceRockID id)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		// if record does not exist report error
		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}

		return rec->getValue<double>(s_asphalteneDiffusionEnergy);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return Utilities::Numerical::IbsNoDataValue;
}

// Set asphaltene diffusion energy (must be in range 200-220 kJ/mol)
ErrorHandler::ReturnCode SourceRockManagerImpl::setAsphalteneDiffusionEnergy(SourceRockID id, double newVal)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_asphalteneDiffusionEnergy, newVal);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}

// Get resin diffusion energy [kJ/mol]
double SourceRockManagerImpl::getResinDiffusionEnergy(SourceRockID id)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		// if record does not exist report error
		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}

		return rec->getValue<double>(s_resinDiffusionEnergy);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return Utilities::Numerical::IbsNoDataValue;
}

// Set resin diffusion energy (must be in range 200-220 kJ/mol)
ErrorHandler::ReturnCode SourceRockManagerImpl::setResinDiffusionEnergy(SourceRockID id, double newVal)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_resinDiffusionEnergy, newVal);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}

// Get C15Aro diffusion energy [kJ/mol]
double SourceRockManagerImpl::getC15AroDiffusionEnergy(SourceRockID id)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		// if record does not exist report error
		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}

		return rec->getValue<double>(s_C15AroDiffusionEnergy);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return Utilities::Numerical::IbsNoDataValue;
}

// Set C15Aro diffusion energy (must be in range 200-220 kJ/mol)
ErrorHandler::ReturnCode SourceRockManagerImpl::setC15AroDiffusionEnergy(SourceRockID id, double newVal)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_C15AroDiffusionEnergy, newVal);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}

// Get C15Sat diffusion energy [kJ/mol]
double SourceRockManagerImpl::getC15SatDiffusionEnergy(SourceRockID id)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		// if record does not exist report error
		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}

		return rec->getValue<double>(s_C15SatDiffusionEnergy);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return Utilities::Numerical::IbsNoDataValue;
}

// Set C15Sat diffusion energy (must be in range 200-220 kJ/mol)
ErrorHandler::ReturnCode SourceRockManagerImpl::setC15SatDiffusionEnergy(SourceRockID id, double newVal)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_C15SatDiffusionEnergy, newVal);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}

// Get VRE optimization for source rock lithology
std::string SourceRockManagerImpl::getVREoptimization(SourceRockID id)
{
	if (errorCode() != NoError) resetError();
	std::string vreName;

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		vreName = rec->getValue<std::string>(s_vreOptimization);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return vreName;
}


// Get VRE threshold value 
double SourceRockManagerImpl::getVREthreshold(SourceRockID id)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		// if record does not exist report error
		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}

		return rec->getValue<double>(s_vreThreshold);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return Utilities::Numerical::IbsNoDataValue;
}

// Set VRE threshold value
ErrorHandler::ReturnCode SourceRockManagerImpl::setVREthreshold(SourceRockID id, double newVRE)
{
	if (errorCode() != NoError) resetError();

	try
	{
		
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_vreThreshold, newVRE);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}


// Get VES limit indicator for source rock lithology
std::string SourceRockManagerImpl::getVESlimitIndicator(SourceRockID id)
{
	if (errorCode() != NoError) resetError();
	std::string vesName;

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		vesName = rec->getValue<std::string>(s_vesLimitIndicator);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return vesName;
}


// Get VES limit value (MPa)
double SourceRockManagerImpl::getVESlimit(SourceRockID id)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		// if record does not exist report error
		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}

		return rec->getValue<double>(s_vesLimit);
	}
	catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

	return Utilities::Numerical::IbsNoDataValue;
}

// Set VES limit value (must be in range 0-70 MPa)
ErrorHandler::ReturnCode SourceRockManagerImpl::setVESlimit(SourceRockID id, double newVES)
{
	if (errorCode() != NoError) resetError();

	try
	{
		if (newVES < 0.0 || newVES > 70.0)
		{
			throw Exception(OutOfRangeValue) << "VES limit value must be in range [0:70] but given is: " << newVES;
		}

		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_vesLimit, newVES);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}



// Get adsorption related fields
ErrorHandler::ReturnCode SourceRockManagerImpl::getAdsoptionList(SourceRockID id, int & applyAdsorption, int & adsorptionTOCDependent, int & computeOTGC, std::string & adsorptionCapacityFunctionName)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		applyAdsorption = rec->getValue<int>(s_applyAdsorption);
		adsorptionTOCDependent = rec->getValue<int>(s_adsorptionTOCDependent);
		computeOTGC = rec->getValue<int>(s_computeOTGC);
		adsorptionCapacityFunctionName = rec->getValue<std::string>(s_adsorptionCapacityFunctionName);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;
}
// Set adsorption TOC Dependent fields
ErrorHandler::ReturnCode SourceRockManagerImpl::setAdsorptionTOCDependent(SourceRockID id, int newAdsorptionTOCDependent)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_adsorptionTOCDependent, newAdsorptionTOCDependent);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;

}
// Set adsorption capacity function name fields
ErrorHandler::ReturnCode SourceRockManagerImpl::setAdsorptionCapacityFunctionName(SourceRockID id, const std::string & newAdsorptionCapacityFunctionName)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_adsorptionCapacityFunctionName, newAdsorptionCapacityFunctionName);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;

}
// Set adsorption simulator name fields
ErrorHandler::ReturnCode SourceRockManagerImpl::setWhichAdsorptionSimulator(SourceRockID id, const std::string & newWhichAdsorptionSimulator)
{
	if (errorCode() != NoError) resetError();

	try
	{
		// get pointer to the table
		database::Table * table = m_db->getTable(s_sourceRockTableName);

		// if table does not exist - report error
		if (!table)
		{
			throw Exception(NonexistingID) << s_sourceRockTableName << " table could not be found in project";
		}

		database::Record * rec = table->getRecord(static_cast<int>(id));
		if (!rec)
		{
			throw Exception(NonexistingID) << "No source rock lithology with such ID: " << id;
		}
		rec->setValue(s_whichAdsorptionSimulator, newWhichAdsorptionSimulator);
	}
	catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	return NoError;

}

}
