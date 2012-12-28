// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef TRAPPER_IO_TABLE_REC_H
#define TRAPPER_IO_TABLE_REC_H

#include "ComponentManager.h"
#include "Interface/ProjectHandle.h"

#include <numeric>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
/// \brief Wrapper class for the record in TrapperIoTable table
///////////////////////////////////////////////////////////////////////////////
class TrapperIoTableRec
{
public:
   static const char * PhaseNames[4];
   static const char * SpeciesNames[CBMGenerics::ComponentManager::NumberOfSpecies];

   TrapperIoTableRec();
   TrapperIoTableRec( database::Table * trapperIoTbl );
   virtual ~TrapperIoTableRec() {;}

   void loadRec( const database::Table::iterator & tit );

   double pressure() const      { return m_Pressure; }
   double temperature() const  { return m_Temperature; }

   void setPressure(    double p ) { m_Pressure = p; }
   void setTemperature( double t ) { m_Temperature = t; }

   void setTrapID(        int trapId )                        { m_TrapId = trapId; }
   void setReservoirName( const std::string & reservoirName ) { m_ReservoirName = reservoirName; }
   void setAge(           double age )                        { m_Age = age; }
 
   double totMass() const { return std::accumulate( m_TotMass, m_TotMass + 4, 0.0 ); }

   double totMass( int phase ) const
   { 
      assert( phase < CBMGenerics::ComponentManager::NumberOfPhases );
      return m_TotMass[phase*2] + m_TotMass[phase*2+1];
   }

   double compMass( int compNum, int phase )
   {
      assert( phase < CBMGenerics::ComponentManager::NumberOfPhases );
      assert( compNum < CBMGenerics::ComponentManager::NumberOfSpecies );
      return m_CompMass[phase*2][compNum] + m_CompMass[phase*2+1][compNum];
   }

   double compMass( int compNum ) const
   {
      assert( compNum < CBMGenerics::ComponentManager::NumberOfSpecies );
      return m_CompMass[0][compNum] + m_CompMass[1][compNum] + m_CompMass[2][compNum] + m_CompMass[3][compNum];
   }

   void setCompositionComp( int phase, int compNum, double val ) { m_CompMass[phase][compNum] = val; m_TotMass[phase] += val; }

   std::string toString() const;
   std::string name() const;

private:
   // position in table corresponded values:
   std::vector<int> m_iCompMass[4];
   int              m_iTotMass[4];

   int m_iTrapId;
   int m_iResName;
   int m_iAge;
   int m_iPressure;
   int m_iTemperature;

   // values in row
   int                 m_TrapId;
   std::string         m_ReservoirName;
   double              m_Age;
   double              m_Pressure;
   double              m_Temperature;
   double              m_TotMass[4];

   std::vector<double> m_CompMass[4];
};


#endif

