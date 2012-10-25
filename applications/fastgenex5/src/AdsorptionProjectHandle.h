#ifndef _ADSORPTION_PROJECT_HANDLE_H_
#define _ADSORPTION_PROJECT_HANDLE_H_

#include <vector>

#include "database.h"

#include "Interface/ProjectHandle.h"

#include "LangmuirAdsorptionIsothermSample.h"
#include "LangmuirAdsorptionTOCEntry.h"
#include "PointAdsorptionHistory.h"
#include "IrreducibleWaterSaturationSample.h"
#include "SGDensitySample.h"

using namespace DataAccess;

// class LangmuirAdsorptionIsothermSample;
// class AdsorptionSimulatorParameters;

/// Class containing representation of the adsorption-project file.
typedef std::vector<LangmuirAdsorptionIsothermSample*> LangmuirAdsorptionIsothermSampleList;

typedef std::vector<const LangmuirAdsorptionIsothermSample*> ConstLangmuirAdsorptionIsothermSampleList;

typedef std::vector<LangmuirAdsorptionTOCEntry*> LangmuirAdsorptionTOCEntryList;

typedef std::vector<PointAdsorptionHistory*> PointAdsorptionHistoryList;

class AdsorptionProjectHandle {

   /// Functor class for ordering langmuir-adsorption-isotherms.
   class LangmuirAdsorptionIsothermSampleLessThan {

   public :

      bool operator ()( const LangmuirAdsorptionIsothermSample* s1, 
                        const LangmuirAdsorptionIsothermSample* s2 ) const;

   };

public :

   AdsorptionProjectHandle ( Interface::ProjectHandle* projectHandle );
   ~AdsorptionProjectHandle ();


   /// Return a const pointer to the adsorption-simulator-parameters object.

   ConstLangmuirAdsorptionIsothermSampleList* getLangmuirAdsorptionIsothermSampleList ( const std::string& formationName ) const;

   const LangmuirAdsorptionTOCEntry* getLangmuirAdsorptionTOCEntry ( const std::string& formationName ) const;


   PointAdsorptionHistoryList::iterator getPointAdsorptionHistoryListBegin ();

   PointAdsorptionHistoryList::iterator getPointAdsorptionHistoryListEnd ();

   const IrreducibleWaterSaturationSample* getIrreducibleWaterSaturationSample () const;

   const SGDensitySample* getSGDensitySample () const;

   const Interface::ProjectHandle* getDALProjectHandle () const;


private :

   void loadLangmuirIsotherms ();
   void loadLangmuirTOCEntries ();
   void loadPointHistories ();
   void loadIrreducibleWaterSaturationSample ();
   void loadSGDensitySample ();

   void deleteLangmuirIsotherms ();
   void deleteLangmuirTOCEntries ();
   void deletePointHistories ();
   void deleteIrreducibleWaterSaturationSample ();
   void deleteSGDensitySample ();

   Interface::ProjectHandle* m_projectHandle;

   LangmuirAdsorptionIsothermSampleList m_langmuirIsotherms;
   LangmuirAdsorptionTOCEntryList m_langmuirTocAdsorptionEntries;

   PointAdsorptionHistoryList m_adsorptionPointHistoryList;

   IrreducibleWaterSaturationSample* m_irreducibleWaterSample;
   SGDensitySample* m_sgDensitySample;

};

inline const SGDensitySample* AdsorptionProjectHandle::getSGDensitySample () const {
   return m_sgDensitySample;
}


inline const Interface::ProjectHandle* AdsorptionProjectHandle::getDALProjectHandle () const {
   return m_projectHandle;
}


#endif // _ADSORPTION_PROJECT_HANDLE_H_
