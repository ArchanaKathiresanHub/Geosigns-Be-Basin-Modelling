// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "TrapperIoTableRec.h"

#include <sstream>

const char * TrapperIoTableRec::PhaseNames[4] = { "FreeGas", "Condensate", "SolutionGas", "StockTankOil" };

TrapperIoTableRec::TrapperIoTableRec()
{
   m_iTrapId      = -1;
   m_iResName     = -1;
   m_iAge         = -1;
   m_iPressure    = -1;
   m_iTemperature = -1;
   
   for ( int p = 0; p < 4; ++p )
   {
      m_iTotMass[p] = -1;
      m_TotMass[p] = 0.0;

      m_iCompMass[p].resize( ComponentId::NUMBER_OF_SPECIES );
      std::fill( m_iCompMass[p].begin(), m_iCompMass[p].end(), -1 );

      m_CompMass[p].resize( ComponentId::NUMBER_OF_SPECIES );
      std::fill( m_CompMass[p].begin(), m_CompMass[p].end(), 0.0 );
   }
} 

TrapperIoTableRec::TrapperIoTableRec( database::Table * trapperIoTbl )
{
   const database::TableDefinition & trapperIoTblDef = trapperIoTbl->getTableDefinition();

   m_iTrapId      = trapperIoTblDef.getIndex( "PersistentTrapID" );
   m_iResName     = trapperIoTblDef.getIndex( "ReservoirName" );
   m_iAge         = trapperIoTblDef.getIndex( "Age" );
   m_iPressure    = trapperIoTblDef.getIndex( "Pressure" );
   m_iTemperature = trapperIoTblDef.getIndex( "Temperature" );

   for ( int p = 0; p < 4; ++p )
   {
      std::string fracPref;
      switch ( p )
      {
         case 0: fracPref = "FreeGas";      break;
         case 1: fracPref = "Condensate";   break;
         case 2: fracPref = "SolutionGas";  break;
         case 3: fracPref = "StockTankOil"; break;
         default: assert(0); break;
      }
      m_iTotMass[p] = trapperIoTblDef.getIndex( fracPref + "Mass" );
      assert( m_iTotMass[p] > -1 );

      m_iCompMass[p].resize( ComponentId::NUMBER_OF_SPECIES );
      for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
      {
         m_iCompMass[p][i] = trapperIoTblDef.getIndex( fracPref + "Mass" + CBMGenerics::ComponentManager::getInstance().getSpeciesInputName( i ) );
         assert( m_iCompMass[p][i] > -1 );
      }
   }
}

void TrapperIoTableRec::loadRec( const database::Table::iterator & tit )
{
   if ( *tit != 0 )
   {
      m_TrapId        = (*tit)->getValue<int>( m_iTrapId );
      m_ReservoirName = (*tit)->getValue<std::string>( m_iResName );
      m_Age           = (*tit)->getValue<double>( m_iAge );
      m_Pressure      = (*tit)->getValue<double>( m_iPressure );
      m_Temperature   = (*tit)->getValue<double>( m_iTemperature );

      for ( int p = 0; p < 4; ++p )
      {
         m_TotMass[p] = (*tit)->getValue<double>( m_iTotMass[p] );
   
         m_CompMass[p].resize( ComponentId::NUMBER_OF_SPECIES );
         for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
         {
            m_CompMass[p][i] = (*tit)->getValue<double>( m_iCompMass[p][i] );
         }
      }
   }
}


std::string TrapperIoTableRec::toString() const
{
   std::ostringstream oss;
   oss << "Persistent Trap ID: " << m_TrapId        << std::endl;
   oss << "Reservoir Name: "     << m_ReservoirName << std::endl;
   oss << "Age: "                << m_Age           << std::endl;
   oss << "Pressure: "           << m_Pressure      << std::endl;
   oss << "Temperature: "        << m_Temperature   << std::endl;

   oss << "  Total masses by phases: " << std::endl;
   oss << "     FreeGas: "      << m_TotMass[0] << std::endl;
   oss << "     Condensate: "   << m_TotMass[1] << std::endl;
   oss << "     SolutionGas: "  << m_TotMass[2] << std::endl;
   oss << "     StockTankOil: " << m_TotMass[3] << std::endl;

   oss << "  Masses by components: " << std::endl << "      " << std::endl;
   for ( int i = ComponentId::NUMBER_OF_SPECIES-1; i >=0; --i )
   {
      oss << "      " << CBMGenerics::ComponentManager::getInstance().getSpeciesName( i ) << ": " << compMass(i) << std::endl;
   }
   oss << std::endl;
   return oss.str();
}

std::string TrapperIoTableRec::name() const
{
   std::ostringstream oss;
   oss << m_ReservoirName << "_trap_" << m_TrapId << "_age_" << m_Age;
   return oss.str();
}
