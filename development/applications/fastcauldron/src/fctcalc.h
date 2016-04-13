#ifndef _FASTCAULDRON__FCTCALC_H_
#define _FASTCAULDRON__FCTCALC_H_

#include "propinterface.h"

class FCTCalc
{
public:

   FCTCalc ( AppCtx* Application_Context );

   void printTestVals();

   void decompact();

   const PropListVec & getVolumeOutputProperties() const;
   const PropListVec & getMapOutputProperties() const;
   

private:

   AppCtx* cauldron;

   bool getGrid ( const double                    currentTime,
                  const SnapshotEntrySetIterator& majorSnapshots );

   void writeCauldronSnapShotTime ( const double time );

   PropListVec m_mapOutputProperties;
   PropListVec m_volumeOutputProperties;

};

#endif /* _FASTCAULDRON__FCTCALC_H_ */
