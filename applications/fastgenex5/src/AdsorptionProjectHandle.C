#include "AdsorptionProjectHandle.h"

#include <iostream>
#include <algorithm>

#include "LangmuirAdsorptionIsothermSample.h"

#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

using namespace std;

AdsorptionProjectHandle::AdsorptionProjectHandle ( Interface::ProjectHandle* projectHandle ) {

   m_projectHandle = projectHandle;

   loadLangmuirIsotherms ();
   loadLangmuirTOCEntries ();
   loadPointHistories ();
   loadIrreducibleWaterSaturationSample ();
   loadSGDensitySample ();
}

AdsorptionProjectHandle::~AdsorptionProjectHandle () {
   deleteLangmuirIsotherms ();
   deleteLangmuirTOCEntries ();
   deletePointHistories ();
   deleteIrreducibleWaterSaturationSample ();
   deleteSGDensitySample ();
}


ConstLangmuirAdsorptionIsothermSampleList* AdsorptionProjectHandle::getLangmuirAdsorptionIsothermSampleList ( const std::string& langmuirName ) const {

   ConstLangmuirAdsorptionIsothermSampleList* samples = new ConstLangmuirAdsorptionIsothermSampleList;
   LangmuirAdsorptionIsothermSampleList::const_iterator sampleIter;

   for ( sampleIter = m_langmuirIsotherms.begin (); sampleIter != m_langmuirIsotherms.end (); ++sampleIter ) {

      const LangmuirAdsorptionIsothermSample* sample = *sampleIter;

      if ( sample->getLangmuirName () == langmuirName ) {
         samples->push_back ( sample );
      }

   }

   std::sort ( samples->begin (), samples->end (), LangmuirAdsorptionIsothermSampleLessThan ());

   return samples;
}

const LangmuirAdsorptionTOCEntry* AdsorptionProjectHandle::getLangmuirAdsorptionTOCEntry ( const std::string& langmuirName ) const {

   LangmuirAdsorptionTOCEntry* adsorption = 0;
   LangmuirAdsorptionTOCEntryList::const_iterator langIter;

   for ( langIter = m_langmuirTocAdsorptionEntries.begin (); langIter != m_langmuirTocAdsorptionEntries.end (); ++langIter ) {

      if ((*langIter)->getLangmuirName () == langmuirName ) {
         adsorption = *langIter;
      }

   } 

   return adsorption;
}


PointAdsorptionHistoryList::iterator AdsorptionProjectHandle::getPointAdsorptionHistoryListBegin () {
   return m_adsorptionPointHistoryList.begin ();
}

PointAdsorptionHistoryList::iterator AdsorptionProjectHandle::getPointAdsorptionHistoryListEnd () {
   return m_adsorptionPointHistoryList.end ();
}

const IrreducibleWaterSaturationSample* AdsorptionProjectHandle::getIrreducibleWaterSaturationSample () const {
   return m_irreducibleWaterSample;
}


void AdsorptionProjectHandle::loadLangmuirIsotherms () {

   database::Table* langmuirIsothermTable = getDALProjectHandle ()->getDataBase ()->getTable ( "LangmuirAdsorptionCapacityIsothermSetIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = langmuirIsothermTable->begin (); tblIter != langmuirIsothermTable->end (); ++tblIter ) {

      LangmuirAdsorptionIsothermSample* sample = new LangmuirAdsorptionIsothermSample ( this, *tblIter );

      m_langmuirIsotherms.push_back ( sample );
   }

   std::sort ( m_langmuirIsotherms.begin (), m_langmuirIsotherms.end (), LangmuirAdsorptionIsothermSampleLessThan ());
}

void AdsorptionProjectHandle::loadLangmuirTOCEntries () {

   database::Table* langmuirIsothermTable = getDALProjectHandle ()->getDataBase ()->getTable ( "LangmuirAdsorptionCapacityTOCFunctionIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = langmuirIsothermTable->begin (); tblIter != langmuirIsothermTable->end (); ++tblIter ) {

      LangmuirAdsorptionTOCEntry* sample = new LangmuirAdsorptionTOCEntry ( this, *tblIter );

      m_langmuirTocAdsorptionEntries.push_back ( sample );
   }

}


void AdsorptionProjectHandle::loadPointHistories () {

   database::Table* pointHistoryTable = getDALProjectHandle ()->getDataBase ()->getTable ( "AdsorptionHistoryIoTbl" );
   database::Table::iterator tableIter;

   for ( tableIter = pointHistoryTable->begin (); tableIter != pointHistoryTable->end (); ++tableIter ) {
      PointAdsorptionHistory* newHistory = new PointAdsorptionHistory ( this, *tableIter );
      m_adsorptionPointHistoryList.push_back ( newHistory );
   }

}

void AdsorptionProjectHandle::loadIrreducibleWaterSaturationSample () {

   database::Table* irreducibleWaterSaturationTable = getDALProjectHandle ()->getDataBase ()->getTable ( "IrreducibleWaterSaturationIoTbl" );

   if ( irreducibleWaterSaturationTable != 0 and irreducibleWaterSaturationTable->size () >= 1 ) {
      m_irreducibleWaterSample = new IrreducibleWaterSaturationSample ( this, irreducibleWaterSaturationTable->getRecord ( 0 ));
   } else {
      m_irreducibleWaterSample = 0;
   }

}


void AdsorptionProjectHandle::loadSGDensitySample () {

   database::Table* densityTable = getDALProjectHandle ()->getDataBase ()->getTable ( "SGDensityIoTbl" );

   if ( densityTable != 0 and densityTable->size () > 0 ) {
      m_sgDensitySample = new SGDensitySample ( this, densityTable->getRecord ( 0 ));
   } else {
      m_sgDensitySample = 0;
   }

}

void AdsorptionProjectHandle::deleteSGDensitySample () {

   if ( m_sgDensitySample != 0 ) {
      delete m_sgDensitySample;
      m_sgDensitySample = 0;
   }

}


void AdsorptionProjectHandle::deleteIrreducibleWaterSaturationSample () {

   if ( m_irreducibleWaterSample != 0 ) {
      delete m_irreducibleWaterSample;
   }

}

void AdsorptionProjectHandle::deleteLangmuirIsotherms () {

   LangmuirAdsorptionIsothermSampleList::iterator langIter;

   for ( langIter = m_langmuirIsotherms.begin (); langIter != m_langmuirIsotherms.end (); ++langIter ) {
      delete (*langIter);
   } 

}

void AdsorptionProjectHandle::deleteLangmuirTOCEntries () {

   LangmuirAdsorptionTOCEntryList::iterator langIter;

   for ( langIter = m_langmuirTocAdsorptionEntries.begin (); langIter != m_langmuirTocAdsorptionEntries.end (); ++langIter ) {
      delete (*langIter);
   } 

}

void AdsorptionProjectHandle::deletePointHistories () {

   PointAdsorptionHistoryList::iterator histIter;

   for ( histIter = m_adsorptionPointHistoryList.begin (); histIter != m_adsorptionPointHistoryList.end (); ++histIter ) {
      delete (*histIter);
   }

   m_adsorptionPointHistoryList.clear ();
}

bool AdsorptionProjectHandle::LangmuirAdsorptionIsothermSampleLessThan::operator ()( const LangmuirAdsorptionIsothermSample* s1, 
                                                                                     const LangmuirAdsorptionIsothermSample* s2 ) const {
   return s1->getLangmuirTemperature () < s2->getLangmuirTemperature ();
}

