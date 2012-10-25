#ifndef _INTERFACE_CRUST_FORMATION_H_
#define _INTERFACE_CRUST_FORMATION_H_

#include "Interface/DAObject.h"
#include "Interface/Interface.h"
#include "Interface/BasementFormation.h"


namespace DataAccess
{
   namespace Interface
   {

      /// A specialised formation providing the extra functionality requierd by the crust formation.
      class CrustFormation : virtual public BasementFormation
      {
      public:

         /// constructor.
         CrustFormation (ProjectHandle * projectHandle, database::Record* record);

         /// destructor.
         ~CrustFormation (void);

         /// Return the present day, user-supplied Crust thickness GridMap.
         const GridMap * getInputThicknessMap (void) const;

         /// Return the user-supplied heat production map for the crust.
         const GridMap * getCrustHeatProductionMap () const;

         const GridMap * getCrustThicknessMeltOnsetMap () const;
         const GridMap * getBasaltThicknessMap () const;
         GridMap * loadCrustHeatProductionMap () const;
         GridMap * loadCrustThicknessMeltOnsetMap () const;
         GridMap * loadBasaltThicknessMap () const;
         const GridMap * getInitialThicknessMap (void) const;


         PaleoFormationPropertyList * getPaleoThicknessHistory () const;

         virtual int getDepositionSequence () const;

         /// Return the heat production decay constant for the crust.
         ///
         /// The heat production has an exponential decay w.r.t. depth.
         virtual double getHeatProductionDecayConstant () const;

         void asString (string & str) const;

         void setInitialCrustalThickness( const double aInitCrustalThickness );
         double getInitialCrustalThickness() const;
         
      protected:

         mutable const GridMap* m_inputThicknessMap;
         mutable const GridMap* m_initialThicknessMap;

         mutable const GridMap* basaltThickness;        
         mutable const GridMap* crustalThicknessMeltOnset;        
 
         /// crustal thickness at the age of basin.
         double m_initialCrustalThickness;

      };

      inline void Interface::CrustFormation::setInitialCrustalThickness (const double aInitCrustalThickness ) {
         m_initialCrustalThickness = aInitCrustalThickness;
      }
      
      inline double Interface::CrustFormation::getInitialCrustalThickness () const {
         return m_initialCrustalThickness;
      }
   }
}
#endif // _INTERFACE_CRUST_FORMATION_H_
