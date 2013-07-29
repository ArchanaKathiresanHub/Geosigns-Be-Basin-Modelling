#include "ExperimentResultsTable.h"

#include <iostream>

ExperimentResultsTable :: ExperimentResultsTable( 
      const std::vector< std::string > & probeNames, char fieldSeparator, int fieldWidth )
   : m_probeNames( probeNames )
   , m_table()
   , m_printer(fieldSeparator, fieldWidth)
{}

void ExperimentResultsTable :: clear()
{
   m_table.clear();
}

void ExperimentResultsTable :: add( const Entry & entry)
{
   m_table.push_back(entry);
}


void ExperimentResultsTable :: print( std::ostream & ofs ) 
{
   // sort the entries on position, time, and probe number
   // Note: the whole idea is to print entries with the same position and time on the same record
   std::sort( m_table.begin(), m_table.end() );

   // print labels
   m_printer.newField(true, ofs);
   ofs << "X";
   m_printer.newField(false, ofs);
   ofs << "Y";
   m_printer.newField(false, ofs);
   ofs << "Z";
   m_printer.newField(false, ofs);
   ofs << "Age";
   for (unsigned j = 0; j < m_probeNames.size(); ++j)
   {
      m_printer.newField(false, ofs);
      ofs << m_probeNames[j];
   }

   // print the entries
   PositionAndTime prevPos;
   ProbeID prevLabel = m_probeNames.size();
   for (unsigned k = 0; k < m_table.size(); ++k)
   {
      using std::tr1::get;
      const PositionAndTime pos = get<0>(m_table[k]);
      const ProbeID label = get<1>(m_table[k]);
      const double value = get<2>(m_table[k]);

      if (k == 0 || pos != prevPos || label <  prevLabel )
      {  
         // finish the last one whenever the position changes or the labels don't increase
         for (ProbeID l = prevLabel; l < m_probeNames.size(); ++l)
         {
            m_printer.newField(false, ofs);
            ofs << ' ';
         }
         m_printer.newRecord(ofs);

         // and start a new record 
         m_printer.newField(true, ofs);
         ofs << get<0>(pos);
         m_printer.newField(false, ofs);
         ofs << get<1>(pos);
         m_printer.newField(false, ofs);
         ofs << get<2>(pos);
         m_printer.newField(false, ofs);
         ofs << get<3>(pos);

         prevLabel = 0;
      }

      // print empty fields from the previous label up to the current label
      for (ProbeID l = prevLabel; l < label; ++l)
      {
         m_printer.newField(false, ofs);
         ofs << ' ';
      }

      // print the value
      m_printer.newField(false, ofs);
      ofs << value;

      // remember next iteration the position and label we've printed now
      prevPos = pos;
      prevLabel = label + 1;
   }

   // end the last record
   for (ProbeID l = prevLabel; l < m_probeNames.size(); ++l)
   {
      m_printer.newField(false, ofs);
      ofs << ' ';
   }
   m_printer.newRecord(ofs);
}

