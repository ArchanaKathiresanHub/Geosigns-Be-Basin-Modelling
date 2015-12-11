//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RequestHandling.h"

#include "Formation.h"
#include "Surface.h"
#include "rankings.h"
#include "migration.h"
#include "ObjectFactory.h"
#include "Reservoir.h"
#include "Column.h"
#include "BrooksCorey.h"

#include "FiniteElement.h"
#include "FiniteElementTypes.h"

#include "Interface/ProjectHandle.h"
#include "Interface/RunParameters.h"
#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/PropertyValue.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "PropertyRetriever.h"
#include "DerivedFormationProperty.h"

#include "Migrator.h"
#include "MigrationPropertyManager.h"
#include "PrimaryFormationProperty.h"
#include "PrimarySurfaceProperty.h"
#include "SmartGridMapRetrieval.h"

#include "LinearGridInterpolator.h"
#include "GeoPhysicsSourceRock.h"

#include "consts.h"

#include <assert.h>
#include <math.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace CBMGenerics;
using namespace DataAccess;
using namespace FiniteElementMethod;

using Interface::Property;
using Interface::Snapshot;
using Interface::GridMap;
using Interface::Grid;
using Interface::ProjectHandle;
using Interface::PropertyValueList;
using Interface::FormationList;

namespace migration
{
   Formation::Formation (ProjectHandle * projectHandle, Migrator * const migrator, database::Record * record) :
      Interface::Formation (projectHandle, record),
      GeoPhysics::Formation (projectHandle, record),
      m_migrator (migrator), m_index (-1)
   {
      m_isInitialised = false;
      m_detectedReservoir = false;

      m_genexData = 0;

      m_startGenexTime = -1;
      m_endGenexTime = -1;
      m_formationNodeArray = NULL;

      // Initializing other data members for Windows build

      for (size_t i = 0; i != NUM_COMPONENTS; ++i)
         m_expulsionGridMaps[i] = NULL;

      // Initializing other data members for Windows build
      //for (size_t i = 0; i != NUMBEROFPROPERTYINDICES; ++i)
      //   m_derivedFormationPropertyPtr[i] = NULL;

   }

   Formation::~Formation (void)
   {
      if (m_genexData != 0) {
         delete m_genexData;
      }
   }

   /// Can't be done in the constructor as we need a propertyvalue to calculate the vertical extent of the formation grid
   void Formation::createFormationNodes (void)
   {
      if (m_formationNodeArray) return;

      // Using the GeoPhysicsFormation function to get number of elements.
      int depth = getMaximumNumberOfElements ();
      assert (depth > 0);

      // High-resolution grid
      const Grid * grid = m_migrator->getProjectHandle ()->getActivityOutputGrid ();

      m_formationNodeArray = new FormationNodeArray (this,
                                                     grid->numIGlobal () - 1, grid->numJGlobal () - 1,
                                                     grid->firstI (), Min (grid->lastI (), grid->numIGlobal () - 2),
                                                     grid->firstJ (), Min (grid->lastJ (), grid->numJGlobal () - 2), depth);

   }

   bool Formation::computePropertyMaps (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot,
                                        bool lowResEqualsHighRes, const bool isOverPressureRun, bool nonGeometricLoop, bool chemicalCompaction)
   {
      assert (topDepthGridMap);
      const Grid *grid = m_projectHandle->getActivityOutputGrid ();

      assert (topDepthGridMap->getGrid () == grid);

      // Calculate all properties at once
      if ( (m_formationPropertyPtr[DEPTHPROPERTY]                  = getFormationPropertyPtr ("Depth", snapshot)) == 0 )  
         return false;
      if ( (m_formationPropertyPtr[PRESSUREPROPERTY]               = getFormationPropertyPtr ("Pressure", snapshot)) == 0 )  
         return false;
      if ( (m_formationPropertyPtr[TEMPERATUREPROPERTY]            = getFormationPropertyPtr ("Temperature", snapshot)) == 0 )  
         return false;
      if ( (m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY] = getFormationPropertyPtr ("HorizontalPermeability", snapshot)) == 0 )  
         return false;
      if ( (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]   = getFormationPropertyPtr ("Permeability", snapshot)) == 0 )  
         return false;
      if ( (m_formationPropertyPtr[POROSITYPROPERTY]               = getFormationPropertyPtr ("Porosity", snapshot)) == 0 )  
         return false;
      if (isOverPressureRun)
      {
         if ( (m_formationPropertyPtr[OVERPRESSUREPROPERTY]        = getFormationPropertyPtr ("OverPressure", snapshot)) == 0 )
            return false;
      }

      
      unsigned int depth = getMaximumNumberOfElements () - 1;
      assert (depth >= 0);

      // Using the PropertyRetriever class which ensures the retrieval and later on the restoration of property pointers
      DerivedProperties::PropertyRetriever depthPropertyRetriever         (m_formationPropertyPtr[DEPTHPROPERTY]);
      DerivedProperties::PropertyRetriever pressurePropertyRetriever      (m_formationPropertyPtr[PRESSUREPROPERTY]);
      DerivedProperties::PropertyRetriever temperaturePropertyRetriever   (m_formationPropertyPtr[TEMPERATUREPROPERTY]);
      DerivedProperties::PropertyRetriever porosityPropertyRetriever      (m_formationPropertyPtr[POROSITYPROPERTY]);
      DerivedProperties::PropertyRetriever vPermeabilityPropertyRetriever (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]);
      DerivedProperties::PropertyRetriever hPermeabilityPropertyRetriever (m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]);
      if (isOverPressureRun)
         DerivedProperties::PropertyRetriever depthPropertyRetriever      (m_formationPropertyPtr[OVERPRESSUREPROPERTY]);


      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            for (unsigned int k = 0; k <= depth; ++k)
            {
               LocalFormationNode * formationNode = getLocalFormationNode (i, j, k);

               formationNode->setDepth                  (m_formationPropertyPtr[DEPTHPROPERTY]                  ->get (i, j, k));
               formationNode->setPressure               (m_formationPropertyPtr[PRESSUREPROPERTY]               ->get (i, j, k));
               formationNode->setTemperature            (m_formationPropertyPtr[TEMPERATUREPROPERTY]            ->get (i, j, k));
               formationNode->setPorosity               (m_formationPropertyPtr[POROSITYPROPERTY]               ->get (i, j, k));
               formationNode->setVerticalPermeability   (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]   ->get (i, j, k));
               if (k == depth)
                  formationNode->setVerticalPermeability(m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]   ->get (i, j, k+1),true);
               formationNode->setHorizontalPermeability (m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY] ->get (i, j, k));
               if (isOverPressureRun)
               {
                  formationNode->setOverPressure        (m_formationPropertyPtr[OVERPRESSUREPROPERTY]           ->get (i, j, k));
               }
            }
         }
      }
      return true;
   }

   bool Formation::computeCapillaryPressureMaps (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot)
   {
      assert (topDepthGridMap);
      const Grid *grid = m_projectHandle->getActivityOutputGrid ();

      assert (topDepthGridMap->getGrid () == grid);

      // For capillary pressure we 
      unsigned int depth = getMaximumNumberOfElements ();
      assert (depth > 0);

      const DataModel::AbstractProperty* gasPcE = m_migrator->getPropertyManager ().getProperty ( "CapillaryEntryPressureGas" );
      const DataModel::AbstractProperty* oilPcE = m_migrator->getPropertyManager ().getProperty ( "CapillaryEntryPressureOil" );

      DerivedProperties::DerivedFormationPropertyPtr ptrGasPcE = DerivedProperties::DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty (gasPcE, snapshot, this, grid, depth + 1) );
      assert (ptrGasPcE);
      DerivedProperties::DerivedFormationPropertyPtr ptrOilPcE = DerivedProperties::DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty (oilPcE, snapshot, this, grid, depth + 1) );
      assert (ptrOilPcE);
      m_formationPropertyPtr[CAPILLARYENTRYPRESSUREGASPROPERTY] = ptrGasPcE;
      m_formationPropertyPtr[CAPILLARYENTRYPRESSUREOILPROPERTY] = ptrOilPcE;

      for (int k = depth; k >= 0; --k)
      {
         for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
         {
            for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
            {
               // If at the top choose the formation node right below it. We will still calculate and save values for the top node,
               // but these values will be stored in the arrays of the node below it.
               LocalFormationNode * formationNode = (k == depth) ? getLocalFormationNode (i,j,k-1) : getLocalFormationNode (i,j,k);
               if (!formationNode)
                  continue;

               // When treating the top node of a formation:
               // For continuous properties (P, T, rho) we just take the value of the node above (belogning to the formation above)
               // For vertical permeability we call the getVerticalPermeability function specifying that we are interested in node on top.
               double pressure      = (k == depth) ? formationNode->getPressure ()             : getLocalFormationNode (i,j,k)->getPressure ();
               double temperature   = (k == depth) ? formationNode->getTemperature ()          : getLocalFormationNode (i,j,k)->getTemperature ();
               double vPermeability = (k == depth) ? formationNode->getVerticalPermeability () : getLocalFormationNode (i,j,k)->getVerticalPermeability (true);
               double oilDensity    = (k == depth) ? formationNode->getOilDensity ()           : getLocalFormationNode (i,j,k)->getOilDensity ();

               // Fluid type the same independent of the position of the node inside the formation.
               const GeoPhysics::FluidType * fluid = (GeoPhysics::FluidType *) getFluidType ();               
               double waterDensity = fluid->density (temperature, pressure);

               // Do not assign any value and continue 
               if (formationNode->getGasDensity () == Interface::DefaultUndefinedMapValue or
                   formationNode->getOilDensity () == Interface::DefaultUndefinedMapValue or
                   waterDensity <= 0.0)
                  continue;

               // Critical temperatures and c1, c2 are independent of the exact position of the node inside the formation.
               double hcTempValueGas = pvtFlash::getCriticalTemperature (C1, 0);
               double hcTempValueOil = pvtFlash::getCriticalTemperature (C6_14SAT, 0);

               const double capC1 = getCompoundLithology (i, j)->capC1 ();
               const double capC2 = getCompoundLithology (i, j)->capC2 ();

               double capSealStrength_Air_Hg = CBMGenerics::capillarySealStrength::capSealStrength_Air_Hg (capC1, capC2, vPermeability);

               double oilIFT = CBMGenerics::capillarySealStrength::capTension_H2O_HC (waterDensity, oilDensity, temperature + CBMGenerics::C2K, hcTempValueOil);

               // Considers 180 deg. angle between H2O and HC (strictly speaking not true for oil)
               double capillaryEntryPressureOil = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC (capSealStrength_Air_Hg, oilIFT);
               double capillaryEntryPressureGas = capillaryEntryPressureOil + capillaryEntryPressureOilGas (vPermeability, pressure, capC1, capC2);

               if (capillaryEntryPressureOil == Interface::DefaultUndefinedMapValue and capillaryEntryPressureGas == Interface::DefaultUndefinedMapValue)
               {
                  continue;
               }
               else
               {
                  ptrGasPcE->set( i, j, (unsigned int) k, capillaryEntryPressureGas);
                  ptrOilPcE->set( i, j, (unsigned int) k, capillaryEntryPressureOil);

                  (k == depth) ? formationNode->setCapillaryEntryPressureGas (capillaryEntryPressureGas, true) : formationNode->setCapillaryEntryPressureGas (capillaryEntryPressureGas);
                  (k == depth) ? formationNode->setCapillaryEntryPressureOil (capillaryEntryPressureOil, true) : formationNode->setCapillaryEntryPressureGas (capillaryEntryPressureOil);
               }
            }
         }
      }

      return true;
   }

   /// To be moved into GeoPhysics library
   double Formation::capillaryEntryPressureOilGas (const double permeability, const double brinePressure, const double capC1, const double capC2) const
   {
      double pceHgAir;
      double pceog;
      double hgAigToOilGasConversionFactor;

      // Ratio of interfacial-tensions and cos(contact-angle).
      hgAigToOilGasConversionFactor = BrooksCorey::oilGasInterfacialTension (brinePressure) * BrooksCorey::CosOilGasContactAngle /
         //------------------------------------------------------------------------------------------//
         (BrooksCorey::MercuryAirInterfacialTension * BrooksCorey::CosMercuryAirContactAngle);

      pceHgAir = CBMGenerics::capillarySealStrength::capSealStrength_Air_Hg (capC1, capC2, permeability);
      pceog = pceHgAir * hgAigToOilGasConversionFactor;

      return pceog;
   }

   bool Formation::removeComputedPropertyMaps (void)
   {
      for (unsigned int i = 0; i < NUMBEROFPROPERTYINDICES; ++i)
      {
         //delete m_gridMaps[i];
         //m_gridMaps[i] = 0;
      }
      return true;
   }

   bool Formation::computeHCDensityMaps ()
   {
      const Grid *grid = m_projectHandle->getActivityOutputGrid ();

      int depth = getMaximumNumberOfElements () - 1;
      assert (depth >= 0);

      double compMasses     [CBMGenerics::ComponentManager::NumberOfSpecies];
      double phaseCompMasses[CBMGenerics::ComponentManager::NumberOfPhases][CBMGenerics::ComponentManager::NumberOfSpecies];
      double phaseDensity   [CBMGenerics::ComponentManager::NumberOfSpecies];
      double phaseViscosity [CBMGenerics::ComponentManager::NumberOfSpecies];      

      for(int nc = 0; nc != CBMGenerics::ComponentManager::NumberOfSpecies ;++nc)
      {
						 
         if(nc == CBMGenerics::ComponentManager::C1 or nc == CBMGenerics::ComponentManager::C6Minus14Sat) 
            compMasses[nc] = 1;
         else
            compMasses[nc] = 0;
						 
         phaseCompMasses[0][nc]=0;                    
         phaseCompMasses[1][nc]=0;
         phaseDensity[nc]=0; 
         phaseViscosity[nc]=0;
      }

      for (int k = depth; k >= 0; --k)
      {
         for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
         {
            for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
            {
               LocalFormationNode * formationNode = getLocalFormationNode (i,j,k);
               if (!formationNode)
                  continue;

               double temperature = formationNode->getTemperature ();
               double pressure    = formationNode->getPressure ();

               if (temperature != Interface::DefaultUndefinedMapValue and
                   pressure    != Interface::DefaultUndefinedMapValue)
               {
                  bool flashSuccess = pvtFlash::EosPack::getInstance ().computeWithLumping (temperature + CBMGenerics::C2K,
                                                                                            pressure * CBMGenerics::MPa2Pa,
                                                                                            compMasses, phaseCompMasses,
                                                                                            phaseDensity, phaseViscosity);

                  if (phaseDensity[0] == 0 or phaseDensity[1] == 0)
                  {
                     std::cout << "Formation::computeHCDensityMaps () : Density 0\n";
                     assert (phaseDensity[0] != 0);
                     assert (phaseDensity[0] != 0);
                  }

                  if (phaseDensity[0] > phaseDensity[1])
                  {
                     std::cout << "Formation::computeHCDensityMaps () : Gas density higher than oil\n";
                     assert (phaseDensity[0] < phaseDensity[1]);
                  }

                  // Some sort of bug in EosPack? The non-zero density values are at the front of the phaseDensity
                  // and phaseViscosity arrays. Should be at the places where the commented lines suggest.
                  // Report, possibly make an item and fix!
                  formationNode->setGasDensity (phaseDensity[0]);
                  formationNode->setOilDensity (phaseDensity[1]);

                  //formationNode->setGasDensity (phaseDensity[CBMGenerics::ComponentManager::C1]);
                  //formationNode->setOilDensity (phaseDensity[CBMGenerics::ComponentManager::C6Minus14Sat]);
               }
            }
         }
      }
      return true;
   }

   bool Formation::computeFiniteElement (int i, int j, int k)
   {
      double dx = getDeltaI ();
      double dy = getDeltaJ ();

      FiniteElement & element = getFiniteElement (i, j, k);

      int oi;
      for (oi = 0; oi < NumberOfNodeCorners; ++oi)
      {
         double depth = getDepth (i + NodeCornerOffsets[oi][0], j + NodeCornerOffsets[oi][1], k + NodeCornerOffsets[oi][2]);

         if (depth == Interface::DefaultUndefinedMapValue)
         {
            // undefined node
            return false;
         }

         element.setGeometryPoint (oi + 1, NodeCornerOffsets[oi][0] * dx, NodeCornerOffsets[oi][1] * dy, depth);
      }

      return true;
   }

   double Formation::getFiniteElementValue (int i, int j, int k, double iOffset, double jOffset, double kOffset, PropertyIndex propertyIndex)
   {
      FormationNode * nd = getFormationNode (i, j, k);
      double value;

      if (!nd)
      {
         assert (0);
      }

      value = nd->getFiniteElementValue (iOffset, jOffset, kOffset, propertyIndex);

      return value;
   }

   FiniteElementMethod::FiniteElement & Formation::getFiniteElement (int i, int j, int k)
   {
      return getLocalFormationNode (i, j, k)->getFiniteElement ();
   }

   void Formation::getThreeVectorValue (FormationNodeThreeVectorValueRequest & request, FormationNodeThreeVectorValueRequest & response)
   {
      getLocalFormationNode (request.i, request.j, request.k)->getThreeVectorValue (request, response);
   }

   void Formation::getThreeVector (FormationNodeThreeVectorRequest & request, FormationNodeThreeVectorRequest & response)
   {
      getLocalFormationNode (request.i, request.j, request.k)->getThreeVector (request, response);
   }

   FormationNode * Formation::getFormationNode (int i, int j, int k)
   {
      if (k >= (int) getNodeDepth ())
      {
         return getTopFormation ()->getFormationNode (i, j, k - getNodeDepth ());
      }
      else if (k < 0)
      {
         return getBottomFormation ()->getFormationNode (i, j, k + getBottomFormation ()->getNodeDepth ());
      }
      else
      {
         assert (m_formationNodeArray);
         return m_formationNodeArray->getFormationNode (i, j, k);
      }
   }

   LocalFormationNode * Formation::getLocalFormationNode (int i, int j, int k) const
   {
      if (k >= (int) getNodeDepth ())
      {
         if (getTopFormation ())
            return getTopFormation ()->getLocalFormationNode (i, j, k - getNodeDepth ());
         else
            return 0;
      }
      else if (k < 0)
      {
         if (getBottomFormation ())
            return getBottomFormation ()->getLocalFormationNode (i, j, k + getBottomFormation ()->getNodeDepth ());
         else
            return 0;
      }
      else
      {
         assert (m_formationNodeArray);
         return m_formationNodeArray->getLocalFormationNode (i, j, k);
      }
   }

   int Formation::getNodeDepth (void) const
   {
      assert (m_formationNodeArray);
      return m_formationNodeArray->depth ();
   }

   int Formation::getGridMapDepth (void) const
   {
      return getMaximumNumberOfElements () + 1;
   }

   /// Get the value of a property at specified indices.
   /// Will look in adjacent formation if k is out of range
   double Formation::getPropertyValue (PropertyIndex propertyIndex, int i, int j, int k) const
   {
      double value;
      if (k >= getGridMapDepth ())
      {
         value = getTopFormation ()->getPropertyValue (propertyIndex, i, j, k - (getGridMapDepth () + 1));
      }
      else if (k < 0)
      {
         value = getBottomFormation ()->getPropertyValue (propertyIndex, i, j, k + getBottomFormation ()->getGridMapDepth () - 1);
      }
      else
      {
         if (!m_formationPropertyPtr[propertyIndex])
         {
            value = Interface::DefaultUndefinedMapValue;
         }
         else
         {
            value = m_formationPropertyPtr[propertyIndex]->get ((unsigned int) i, (unsigned int) j, (unsigned int) k);
            if (value == Interface::DefaultUndefinedMapValue) value = Interface::DefaultUndefinedMapValue;
         }
      }

      return value;
   }

   double Formation::getMinOilColumnHeight (void) const
   {
      return m_migrator->getProjectHandle ()->getRunParameters ()->getMinOilColumnHeight ();
   }

   double Formation::getMinGasColumnHeight (void) const
   {
      return m_migrator->getProjectHandle ()->getRunParameters ()->getMinGasColumnHeight ();
   }

   FiniteElementMethod::ThreeVector & Formation::getAnalogFlowDirection (int i, int j, int k)
   {
      return getFormationNode (i, j, k)->getAnalogFlowDirection ();
   }

   int Formation::getIndex (void)
   {
      if (m_index == -1)
      {
         m_index = m_migrator->getIndex (this);
      }

      return m_index;
   }

   bool Formation::computeTargetFormationNodes (Formation * targetFormation)
   {
      targetFormation->prescribeTargetFormationNodes (); // The path has to stop somewhere if nowhere else

      bool allComputed = false;
      do
      {
         allComputed = true;
         for (Formation * formation = targetFormation; formation != getBottomFormation (); formation = (Formation *) formation->getBottomFormation ())
         {
            if (formation->m_detectedReservoir)
               formation->setEndOfPath ();

            int minDepthIndex = (formation == this ? formation->getNodeDepth () - 1 : 0);
            int maxDepthIndex = (formation == targetFormation ? formation->getNodeDepth () - 2 : formation->getNodeDepth () - 1);

            for (int depthIndex = maxDepthIndex; depthIndex >= minDepthIndex; --depthIndex)
            {
               allComputed &= formation->computeTargetFormationNodes (depthIndex);
            }
         }
      } while (!RequestHandling::AllProcessorsFinished (allComputed));

      return allComputed;
   }

   bool Formation::computeTargetFormationNodes (int depthIndex)
   {
      const int MaxLoops = Max (2, NumProcessors () / 2);
      RequestHandling::StartRequestHandling (getMigrator (), "computeTargetFormationNodes");

      bool allComputed = false;

      // depends on computations performed on other processors.
      // hence, keep on going (max. 10 times) until all target columns have been computed as it may not go right the first time
      do
      {
         allComputed = true;
         int targetNodesToCompute = 0;
         for (int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
         {
            for (int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
            {
               if (!computeTargetFormationNode (i, j, depthIndex))
               {
                  allComputed = false;
                  ++targetNodesToCompute;
               }
            }
         }
      } while (!allComputed);

      RequestHandling::FinishRequestHandling ();

      return allComputed;
   }

   bool Formation::computeTargetFormationNode (unsigned int i, unsigned int j, int depthIndex)
   {
      LocalFormationNode * formationNode = getLocalFormationNode (i, j, depthIndex);

      if (!IsValid (formationNode))
         return true;
      else
         return formationNode->computeTargetFormationNode ();
   }

   void Formation::prescribeTargetFormationNodes (void)
   {
      int depthIndex = m_formationNodeArray->depth () - 1;

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            getLocalFormationNode (i, j, depthIndex)->setEndOfPath ();
         }
      }
   }

   void Formation::setEndOfPath (void)
   {
      int depthIndex = m_formationNodeArray->depth () - 1;
      
      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            getLocalFormationNode (i, j, depthIndex)->setEndOfPath ();
         }
      }
   }

   bool Formation::hasVolumePropertyGridMap (const string & propertyName, double snapshotAge) const
   {
      const Property* prop = m_projectHandle->findProperty (propertyName);
      if (!prop)
      {
         cerr << "ERROR:::Could not find the property " << propertyName << endl;
         return 0;
      }

      const Snapshot * snapshot = m_projectHandle->findSnapshot (snapshotAge);
      if (!snapshot)
      {
         cerr << "ERROR:::Could not find snapshot with age " << snapshotAge << endl;
         return 0;
      }

      return hasVolumePropertyGridMap (prop, snapshot);
   }

   bool Formation::hasVolumePropertyGridMap (const Property* prop, double snapshotAge) const
   {
      const Snapshot * snapshot = m_projectHandle->findSnapshot (snapshotAge);
      if (!snapshot)
      {
         cerr << "ERROR:::Could not find snapshot with age " << snapshotAge << endl;
         return 0;
      }

      return hasVolumePropertyGridMap (prop, snapshot);
   }

   bool Formation::hasVolumePropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
   {
      const Property* prop = m_projectHandle->findProperty (propertyName);
      if (!prop)
      {
         cerr << "ERROR:::Could not find the property " << propertyName << endl;
         return 0;
      }

      return hasVolumePropertyGridMap (prop, snapshot);
   }


   bool Formation::hasVolumePropertyGridMap (const Property* prop, const Snapshot * snapshot) const
   {
      PropertyValueList * propertyValues =
         m_projectHandle->getPropertyValues (Interface::FORMATION,
                                             prop, snapshot, 0, this, 0,
                                             Interface::VOLUME);

      bool state = (propertyValues->size () == 1);

      if (!state)
         cerr << "ERROR:::Could not find the property " << prop->getName () <<
            " of formation " << getName () <<
            " at snapshot " << snapshot->getTime () << endl;

      delete propertyValues;
      return state;
   }

   const GridMap * Formation::getVolumePropertyGridMap (const string & propertyName, double snapshotAge) const
   {
      const Property* prop = m_projectHandle->findProperty (propertyName);
      if (!prop)
      {
         cerr << "ERROR:::Could not find the property " << propertyName << endl;
         return 0;
      }

      const Snapshot * snapshot = m_projectHandle->findSnapshot (snapshotAge);
      if (!snapshot)
      {
         cerr << "ERROR:::Could not find snapshot with age " << snapshotAge << endl;
         return 0;
      }

      return getVolumePropertyGridMap (prop, snapshot);
   }

   const GridMap * Formation::getVolumePropertyGridMap (const Property* prop, double snapshotAge) const
   {
      const Snapshot * snapshot = m_projectHandle->findSnapshot (snapshotAge);
      if (!snapshot)
      {
         cerr << "ERROR:::Could not find snapshot with age " << snapshotAge << endl;
         return 0;
      }

      return getVolumePropertyGridMap (prop, snapshot);
   }

   const GridMap * Formation::getVolumePropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
   {
      const Property* prop = m_projectHandle->findProperty (propertyName);
      if (!prop)
      {
         cerr << "ERROR:::Could not find the property " << propertyName << endl;
         return 0;
      }

      return getVolumePropertyGridMap (prop, snapshot);
   }


   const GridMap * Formation::getVolumePropertyGridMap (const Property* prop, const Snapshot * snapshot) const
   {
      auto_ptr<PropertyValueList> propertyValues (m_projectHandle->getPropertyValues (Interface::FORMATION,
                                                                                      prop, snapshot, 0, this, 0,
                                                                                      Interface::VOLUME));

      if (propertyValues->size () != 1)
      {
         cerr << "ERROR: could not load volume property " << prop->getName () << " for formation " << getName () <<
            " at snapshot " << snapshot->getTime () << ": No or too much data available" << endl;
         return 0;
      }

      const GridMap *gridMap = (*propertyValues)[0]->getGridMap ();

      if (!gridMap)
      {
         cerr << "ERROR: could not load volume property " << prop->getName () << " for formation " << getName () <<
            " at snapshot " << snapshot->getTime () << ": Could not load grid map" << endl;
         return 0;
      }

      return gridMap;
   }

   FormationSurfaceGridMaps Formation::getFormationSurfaceGridMaps (const Property* prop, const Snapshot* snapshot) const
   {
      unsigned int index = 0;

      const GridMap* top = getFormationPrimaryPropertyGridMap (prop, snapshot);
      const GridMap* base = top;

      if (top) {

         index = top->getDepth () - 1;

      }
      else {
         top = getSurfacePropertyGridMap (prop, snapshot, getTopSurface ());
         base = getSurfacePropertyGridMap (prop, snapshot, getBottomSurface ());
         if (!top) {
            //     cout << "No top property " << prop->getName() << " " << this->getName() << " " << snapshot->getTime() << endl;
         }
         if (!base) {
            //      cout << "No base property " << prop->getName() << " " << this->getName() << " " << snapshot->getTime() << endl;
         }
      }

      return FormationSurfaceGridMaps (SurfaceGridMap (top, index), SurfaceGridMap (base, (unsigned int) 0), this);
   }    


   SurfaceGridMap Formation::getTopSurfaceGridMap (const Property* prop, const Snapshot* snapshot) const
   {
      unsigned int index = 0;
      const GridMap* top = getFormationPrimaryPropertyGridMap (prop, snapshot);
  
      if (top) {
         index = top->getDepth () - 1;
      }
      else {
         top = getSurfacePropertyGridMap (prop, snapshot, getTopSurface ());
      }

      return SurfaceGridMap (top, index);
   }

   SurfaceGridMap Formation::getBaseSurfaceGridMap (const Property* prop, const Snapshot* snapshot) const
   {
      const GridMap* base = getFormationPrimaryPropertyGridMap (prop, snapshot);

      if (!base)
         base = getSurfacePropertyGridMap (prop, snapshot, getBottomSurface ());

      return SurfaceGridMap (base, (unsigned int) 0);
   }


   Formation * Formation::getTopFormation () const
   {
      const Interface::Surface * surface = getTopSurface ();

      return CastToFormation (surface ? surface->getTopFormation () : 0);
   }

   Formation * Formation::getBottomFormation () const
   {
      const Interface::Surface * surface = getBottomSurface ();
      
      return CastToFormation (surface ? surface->getBottomFormation () : 0);
   }

   bool Formation::isActive (const DataAccess::Interface::Snapshot * snapshot) const
   {
      return *(getBottomSurface ()->getSnapshot ()) < *snapshot;
   }

   void Formation::getValue (FormationNodeValueRequest & request, FormationNodeValueRequest & response)
   {
      LocalFormationNode * nd = getLocalFormationNode (request.i, request.j, request.k);

      if (!nd)
      {
         assert (0);
      }

      nd->getValue (request, response);
   }

   void Formation::setValue (FormationNodeValueRequest & request)
   {
      LocalFormationNode * nd = getLocalFormationNode (request.i, request.j, request.k);

      if (!nd)
      {
         assert (0);
      }

      nd->setValue (request);
   }

   DerivedProperties::FormationPropertyPtr Formation::getFormationPropertyPtr (const string & propertyName, const Interface::Snapshot * snapshot) const
   {

      const DataAccess::Interface::Property* property = m_migrator->getProjectHandle()->findProperty (propertyName);

      DerivedProperties::FormationPropertyPtr theProperty =
         m_migrator->getPropertyManager ().getFormationProperty (property, snapshot, this);

      return theProperty;
   }
 
   // Use this method for getting a formation map of "primary" properties. If the formation map is not available calculate a surface map on-the-fly.
   const GridMap* Formation::getFormationPrimaryPropertyGridMap (const Property* prop, const Interface::Snapshot * snapshot) const {

      const GridMap* theMap = 0;

      DerivedProperties::FormationPropertyPtr theProperty = m_migrator->getPropertyManager ().getFormationProperty (prop, snapshot, this);
      if (theProperty != 0)
      {
         const DerivedProperties::PrimaryFormationProperty * thePrimaryProperty = dynamic_cast<const DerivedProperties::PrimaryFormationProperty *>(theProperty.get ());

         if (thePrimaryProperty != 0)
         {
            theMap = thePrimaryProperty->getGridMap ();
         }
      }

      return theMap;
   }

   const GridMap* Formation::getSurfacePropertyGridMap (const Property* prop, const Snapshot* snapshot, const Interface::Surface* surface) const
   {

      const GridMap* theMap = 0;

      if (prop->getName().find("Permeability") == std::string::npos) {

         DerivedProperties::SurfacePropertyPtr theProperty = m_migrator->getPropertyManager().getSurfaceProperty(prop, snapshot, surface);
 
         if (theProperty != 0) {
      
            const DerivedProperties::PrimarySurfaceProperty * thePrimaryProperty = dynamic_cast<const DerivedProperties::PrimarySurfaceProperty *>(theProperty.get());
            //assert (thePrimaryProperty != 0);

            if (thePrimaryProperty != 0) {
               theMap = thePrimaryProperty->getGridMap();
            }
            else {
               theMap = m_migrator->getPropertyManager().produceDerivedGridMap(theProperty);
            }

         }
         else {
            DerivedProperties::FormationSurfacePropertyPtr theFormationProperty = m_migrator->getPropertyManager().getFormationSurfaceProperty(prop, snapshot, this, surface);

            if (theFormationProperty != 0)
            {
               theMap = m_migrator->getPropertyManager().produceDerivedGridMap(theFormationProperty);
                  }

         }
      }
      else {
         DerivedProperties::FormationSurfacePropertyPtr theFormationProperty = m_migrator->getPropertyManager ().getFormationSurfaceProperty (prop, snapshot, this, surface);

         if (theFormationProperty != 0) {
            theMap = m_migrator->getPropertyManager ().produceDerivedGridMap (theFormationProperty);
         }
      }

      return theMap;
   }

   bool Formation::saveComputedSMFlowPaths (Formation * targetFormation, const Interface::Snapshot * end)
   {
      Formation * formation;

      Formation * formationAboveTarget = (Formation *) targetFormation->getTopFormation ();

      for (formation = this; formation != formationAboveTarget; formation = (Formation *) formation->getTopFormation ())
      {
         formation->saveComputedSMFlowPathsByGridOffsets (end);
      }

      return true;
   }

   bool Formation::saveComputedSMFlowPathsByGridOffsets (const Interface::Snapshot * end)
   {
      ((Interface::Snapshot *) end)->setAppendFile (true);
      if (!m_formationNodeArray)
         return true;

      int nodeDepth = getNodeDepth ();

      Interface::PropertyValue * propertyValue =
         m_migrator->getProjectHandle ()->createVolumePropertyValue ("FlowDirectionIJK", end, 0, this, nodeDepth + 1);
      assert (propertyValue);

      GridMap *gridMap = propertyValue->getGridMap ();

      assert (gridMap);

      gridMap->retrieveData ();

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            for (unsigned int k = 0; k < nodeDepth; ++k)
            {
               LocalFormationNode * node = getLocalFormationNode (i, j, k);
               double gridMapValue =
                  100 * node->getAdjacentFormationNodeGridOffset (2) +     // K
                  10 * node->getAdjacentFormationNodeGridOffset (1) +     // J
                  1 * node->getAdjacentFormationNodeGridOffset (0);      // I

               gridMap->setValue (i, j, k, gridMapValue);
            }
         }
      }
      gridMap->restoreData ();

      return true;
   }

   bool Formation::retrievePropertyMaps (bool retrieveCapillary)
   {
      assert (m_formationPropertyPtr[DEPTHPROPERTY]);
      m_formationPropertyPtr[DEPTHPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]);
      m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]);
      m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[POROSITYPROPERTY]);
      m_formationPropertyPtr[POROSITYPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[TEMPERATUREPROPERTY]);
      m_formationPropertyPtr[TEMPERATUREPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[PRESSUREPROPERTY]);
      m_formationPropertyPtr[PRESSUREPROPERTY]->retrieveData ();

      if (retrieveCapillary)
      {
         assert (m_formationPropertyPtr[CAPILLARYENTRYPRESSUREGASPROPERTY]);
         m_formationPropertyPtr[CAPILLARYENTRYPRESSUREGASPROPERTY]->retrieveData ();

         assert (m_formationPropertyPtr[CAPILLARYENTRYPRESSUREOILPROPERTY]);
         m_formationPropertyPtr[CAPILLARYENTRYPRESSUREOILPROPERTY]->retrieveData ();
      }
      return true;
   }

   bool Formation::restorePropertyMaps (bool restoreCapillary)
   {
      assert (m_formationPropertyPtr[DEPTHPROPERTY]);
      m_formationPropertyPtr[DEPTHPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]);
      m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]);
      m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[POROSITYPROPERTY]);
      m_formationPropertyPtr[POROSITYPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[TEMPERATUREPROPERTY]);
      m_formationPropertyPtr[TEMPERATUREPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[PRESSUREPROPERTY]);
      m_formationPropertyPtr[PRESSUREPROPERTY]->restoreData ();

      if (restoreCapillary)
      {
         assert (m_formationPropertyPtr[CAPILLARYENTRYPRESSUREGASPROPERTY]);
         m_formationPropertyPtr[CAPILLARYENTRYPRESSUREGASPROPERTY]->restoreData ();

         assert (m_formationPropertyPtr[CAPILLARYENTRYPRESSUREOILPROPERTY]);
         m_formationPropertyPtr[CAPILLARYENTRYPRESSUREOILPROPERTY]->restoreData ();
      }
      return true;
   }

   bool Formation::clearNodeProperties (void)
   {
      if (m_formationNodeArray) m_formationNodeArray->clearProperties ();
      return true;
   }

   bool Formation::computeNodeProperties ()
   {
      if (m_formationNodeArray) m_formationNodeArray->computeProperties ();
      return true;
   }

   bool Formation::clearNodeReservoirProperties (void)
   {
      if (m_formationNodeArray) m_formationNodeArray->clearReservoirProperties ();
      return true;
   }

   //function that returns the first non zero thickness element in the reservoir
   LocalFormationNode * Formation::validReservoirNode (const int i, const int j) const
   {
      //reservoir formation, loop downwards until a formation node with thickness is found in the reservoir formation
      LocalFormationNode * reservoirFormationNode = 0;
      bool validReservoirFormationNode = false;
      int k = getNodeDepth () - 1;
		
      while (k >= 0 && validReservoirFormationNode == false)
      {
         reservoirFormationNode = getLocalFormationNode (i, j, k);
         if (reservoirFormationNode && !reservoirFormationNode->hasNoThickness ()) validReservoirFormationNode = true;
         else
            --k;
      }

      if (validReservoirFormationNode)
         return reservoirFormationNode;
      else
         return 0;
   }


   //function that returns the first non zero thickness element in the seal
   LocalFormationNode * Formation::validSealNode (const int i, const int j, const Formation * topFormation, const Formation * topActiveFormation) const
   {
      //seal formation, loop upwards until a formation node with thickness is found in one of the formations above the reservoir
      LocalFormationNode * sealFormationNode = 0;
      bool validSealFormationNode = false;
      int k = 0;
      int ktopseal = topActiveFormation->getNodeDepth() -1;
      //return topFormation->getLocalFormationNode (i, j, k);
      
      while (validSealFormationNode == false)
      {
         sealFormationNode = topFormation->getLocalFormationNode (i, j, k);
         if (sealFormationNode && !sealFormationNode->hasNoThickness ())
         { 
            validSealFormationNode = true;
         }
         else
         {
            if (!sealFormationNode ||(sealFormationNode->getFormation () == topActiveFormation && sealFormationNode->getK() == ktopseal)) break;
            ++k;
         }
      }

      if (validSealFormationNode)
         return sealFormationNode;
      else
         return 0;
   }

   //
   // Loop through the uppermost cells and check capillary pressure across the boundary
   //
   bool Formation::detectReservoir (Formation * topFormation,
                                    const double minOilColumnHeight, const double minGasColumnHeight, const bool pressureRun, const Formation * topActiveFormation)
   {
      for (int i = (int) m_formationNodeArray->firstILocal (); i <= (int) m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int) m_formationNodeArray->firstJLocal (); j <= (int) m_formationNodeArray->lastJLocal (); ++j)
         {
            LocalFormationNode * reservoirFormationNode = validReservoirNode (i, j);
            LocalFormationNode * sealFormationNode = validSealNode (i, j, topFormation, topActiveFormation);

            if (reservoirFormationNode && sealFormationNode) reservoirFormationNode->detectReservoir (sealFormationNode,minOilColumnHeight, minGasColumnHeight, pressureRun);
         }
      }

      return false;
   }

   //
   // Loop through the uppermost cells and check if a trap crests exist with m_height_oil > minOilColumnHeight OR m_height_gas > minGasColumnHeight 
   // Stop as soon as a trap crest is found. 

   bool Formation::detectReservoirCrests()
   {
      //cout << " Rank, Formation, m_detectedReservoir " << GetRank () << " " << getName () << " " << m_detectedReservoir << endl;
      bool reservoirCrestDetected = false;
      
      if (!m_detectedReservoir)
      {     
         RequestHandling::StartRequestHandling(getMigrator(), "detectReservoirCrests");
      
         int upperIndex = getNodeDepth() - 1;
         // First  oil 
         for (int i = (int)m_formationNodeArray->firstILocal(); i <= (int)m_formationNodeArray->lastILocal(); ++i)
         {
            for (int j = (int)m_formationNodeArray->firstJLocal(); j <= (int)m_formationNodeArray->lastJLocal(); ++j)
            {
               LocalFormationNode * reservoirFormationNode = validReservoirNode (i, j);
               if (reservoirFormationNode) reservoirCrestDetected = reservoirFormationNode->detectReservoirCrests (OIL);
               if (reservoirCrestDetected) break;
            }
            if (reservoirCrestDetected) break;
         }

         // Then gas 
         if (!reservoirCrestDetected) 
         {
            for (int i = (int)m_formationNodeArray->firstILocal(); i <= (int)m_formationNodeArray->lastILocal(); ++i)
            {
               for (int j = (int)m_formationNodeArray->firstJLocal(); j <= (int)m_formationNodeArray->lastJLocal(); ++j)
               {
                  LocalFormationNode * reservoirFormationNode = validReservoirNode (i, j);
                  if (reservoirFormationNode) reservoirCrestDetected = reservoirFormationNode->detectReservoirCrests (GAS);
                  if (reservoirCrestDetected) break;
               }
               if (reservoirCrestDetected) break;
            }       
         }
         RequestHandling::FinishRequestHandling ();
      }
      else
      {
         reservoirCrestDetected = m_detectedReservoir;
      }

      return MaximumAll ((int) reservoirCrestDetected);

   }

   bool Formation::getDetectedReservoir () const
   {
      return m_detectedReservoir;
   }
   
   void Formation::setDetectedReservoir (bool detectedReservoir)
   {
      m_detectedReservoir = detectedReservoir;
   }

   // add the detected reservoir to the reservoir vector
   void Formation::addDetectedReservoir (const Interface::Snapshot * start)
   {
      if (!m_detectedReservoir)
      {
         //add a record to the reservoir list
         database::Record * record = m_migrator->addDetectedReservoirRecord (this, start);
         Reservoir* reservoir = (Reservoir*) m_projectHandle->addDetectedReservoirs (record, this);
         // Offsets and net to gross
         reservoir->computeDepthOffsets (m_projectHandle->findSnapshot (0.));
         reservoir->computeNetToGross ();
         //all processes should arrive here
         MPI_Barrier (PETSC_COMM_WORLD);
         
         m_detectedReservoir = true;
      }
   }

   void Formation::saveReservoir (const Interface::Snapshot * curSnapshot)
   {
      if (!performHDynamicAndCapillary ())
         return;

      if (static_cast <int>(curSnapshot->getTime ()) != 0) return;
      int upperIndex = getNodeDepth () - 1;

      char fname[256];

      sprintf (fname, "%s_%d.txt", getName ().c_str (), static_cast <int>(curSnapshot->getTime ()));
      FILE *fres = fopen (fname, "w");
      LocalFormationNode *fnode;

      for (int i = (int) m_formationNodeArray->firstILocal (); i <= (int) m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int) m_formationNodeArray->firstJLocal (); j <= (int) m_formationNodeArray->lastJLocal (); ++j)
         {
            fprintf (fres, "%d %d\t", i, j);
            fnode = getLocalFormationNode (i, j, upperIndex);

            double depth = 0;
            int top = fnode->getFormation ()->getGridMapDepth ()-1;
            depth = fnode->getFormation ()->getPropertyValue (DEPTHPROPERTY, fnode->getI (), fnode->getJ (), top);

            fprintf (fres, "%d(%lf)\t\t%d(%lf) \t\t %d \t\t %d \t\t %lf\n", (fnode->getReservoirOil () ? 1 : 0), fnode->getHeightOil (),
                     (fnode->getReservoirGas () ? 1 : 0), fnode->getHeightGas (), (fnode->getIsCrest (OIL) ? 1 : 0), (fnode->getIsCrest (GAS) ? 1 : 0), depth);
         }
      }

      fclose (fres);
   }

   // In the case of reservoir offsets the node to be flagged is the first one 'under' the top reservoir surface for each column
   void Formation::identifyAsReservoir (void) const
   {
      int depthIndex = getNodeDepth () - 1;
      assert (depthIndex >= 0);

      for (int i = (int) m_formationNodeArray->firstILocal (); i <= (int) m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int) m_formationNodeArray->firstJLocal (); j <= (int) m_formationNodeArray->lastJLocal (); ++j)
         {
            const Interface::Formation * formation = dynamic_cast<const Interface::Formation *> (this);
            assert (formation);

            Interface::ReservoirList * reservoirs = m_projectHandle->getReservoirs (formation);

            const Reservoir * reservoir = dynamic_cast<const migration::Reservoir *> (*reservoirs->begin ());
            assert (reservoir);
                        
            double formationThickness = getDepth (i,j,0) - getDepth (i,j,depthIndex+1);

            if (getLocalFormationNode (i, j, depthIndex)->hasThickness () and
                ((reservoir->getLocalColumn (i, j)->getTopDepthOffset () * formationThickness + getDepth (i,j,depthIndex+1)) < getDepth (i,j,depthIndex) or depthIndex == 0)) // Top node is flagged
            {
               getLocalFormationNode (i, j, depthIndex)->identifyAsReservoir ();
            }
            else // There is top offset or zero-thickness elements, so the correct node needs to be found
            { 
               int depth = depthIndex;
               while (depth > 0 and
                      (getDepth (i,j,depth) < (reservoir->getLocalColumn (i, j)->getTopDepthOffset () * formationThickness + getDepth (i,j,depthIndex+1))
                       or !getLocalFormationNode (i,j,depth)->hasThickness ()))
               {
                  --depth;
                  if (depth == 0)
                     break;
               }

               getLocalFormationNode (i, j, depth)->identifyAsReservoir ();
            }
         }
      }
   }

   void Formation::loadExpulsionMaps (const Interface::Snapshot * begin, const Interface::Snapshot * end)
   {
      for (int componentId = FIRST_COMPONENT; componentId < NUM_COMPONENTS; ++componentId)
      {
         if (!ComponentsUsed[componentId]) continue;

         string propertyName = ComponentNames[componentId];
         propertyName += "ExpelledCumulative";

         const GridMap * gridMapEnd = getPropertyGridMap (propertyName, end);
         const GridMap * gridMapStart = getPropertyGridMap (propertyName, begin);

         if (gridMapEnd and gridMapStart)
         {
            Interface::SubtractionFunctor subtract;
            m_expulsionGridMaps[componentId] = m_migrator->getProjectHandle ()->getFactory ()->produceGridMap (0, 0, gridMapEnd, gridMapStart, subtract);
         }
         else if (gridMapEnd)
         {
            Interface::AddConstant addZero (0);
            m_expulsionGridMaps[componentId] = m_migrator->getProjectHandle ()->getFactory ()->produceGridMap (0, 0, gridMapEnd, addZero);
         }
         else if (gridMapStart)
         {
            Interface::SubtractFromConstant subtractFromZero (0);
            m_expulsionGridMaps[componentId] = m_migrator->getProjectHandle ()->getFactory ()->produceGridMap (0, 0, gridMapStart, subtractFromZero);
         }

         if (m_expulsionGridMaps[componentId])
         {
            m_expulsionGridMaps[componentId]->retrieveData (true); // divided over different nodes
         }
      }
   }

   /// remove the expulsion gridmaps
   void Formation::unloadExpulsionMaps ()
   {
      for (int componentId = FIRST_COMPONENT; componentId < NUM_COMPONENTS; ++componentId)
      {
         if (!ComponentsUsed[componentId])
            continue;
         if (!m_expulsionGridMaps || !m_expulsionGridMaps[componentId])
            continue;

         m_expulsionGridMaps[componentId]->restoreData (false); // no need to save
         delete m_expulsionGridMaps[componentId];
         m_expulsionGridMaps[componentId] = 0;
      }
   }

   bool Formation::computeFaults (const Interface::Snapshot * end)
   {
      const GridMap * gridMap = computeFaultGridMap (getGrid (), end);
      if (!gridMap) return false;

      unsigned int gridMapDepth = gridMap->getDepth (); // should be 1

      gridMap->retrieveData ();

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            FaultStatus fs;

            double gmValue = gridMap->getValue (i, j, gridMapDepth - 1);
            if (gmValue == gridMap->getUndefinedValue ())
            {
               fs = NOFAULT;
            }
            else
            {
               fs = FaultStatus ((int) gmValue);
            }

            for (unsigned int k = 0; k < getNodeDepth (); ++k)
            {
               getLocalFormationNode (i, j, k)->setFaultStatus (fs);
            }
         }
      }

      gridMap->restoreData ();
      delete gridMap; // required, not a PropertyValue GridMap

      return true;
   }

   bool Formation::computeAnalogFlowDirections (Formation * targetFormation, const Interface::Snapshot * begin,
                                                const Interface::Snapshot * end)
   {
      //DebugProgress (getName (), ": computeAnalogFlowDirections () upto ", targetFormation->getName (), end->getTime ());
      Formation * formation;

      Formation * formationAboveTarget = (Formation *) targetFormation->getTopFormation ();

      for (formation = this; formation != formationAboveTarget; formation = (Formation *) formation->getTopFormation ())
      {
         int firstDepthIndex = 0;
         if (formation == this) firstDepthIndex = getNodeDepth () - 1;

         int lastDepthIndex = formation->getNodeDepth () - 1;
         for (int depthIndex = firstDepthIndex; depthIndex <= lastDepthIndex; ++depthIndex)
         {
            formation->computeAnalogFlowDirections (depthIndex, begin, end); // also computes formation node depths
         }
      }

      return true;
   }

   bool Formation::computeAdjacentNodes (Formation * targetFormation, const Interface::Snapshot * begin,
                                         const Interface::Snapshot * end)
   {
      Formation * formation;

      Formation * formationAboveTarget = (Formation *) targetFormation->getTopFormation ();

      for (formation = this; formation != formationAboveTarget; formation = (Formation *) formation->getTopFormation ())
      {
         int firstDepthIndex = 0;
         if (formation == this) firstDepthIndex = getNodeDepth () - 1;

         int lastDepthIndex = formation->getNodeDepth () - 1;
         if (formation == targetFormation) lastDepthIndex = formation->getNodeDepth () - 2;

         for (int depthIndex = firstDepthIndex; depthIndex <= lastDepthIndex; ++depthIndex)
         {
            formation->computeAdjacentNodes (depthIndex, begin, end);
         }
      }

      return true;
   }

   bool migration::Formation::computeAnalogFlowDirections (int depthIndex, const Interface::Snapshot * begin,
                                                           const Interface::Snapshot * end)
   {
#if 0
      cerr << getName () << "(" << depthIndex << ")->computeAnalogFlowDirections ()" << endl;
#endif
      for (int i = (int) m_formationNodeArray->firstILocal (); i <= (int) m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int) m_formationNodeArray->firstJLocal (); j <= (int) m_formationNodeArray->lastJLocal (); ++j)
         {
            getLocalFormationNode (i, j, depthIndex)->computeAnalogFlowDirection ();
         }
      }
 
      return true;
   }

   bool Formation::computeAdjacentNodes (int depthIndex, const Interface::Snapshot * begin, const Interface::Snapshot * end)
   {
      RequestHandling::StartRequestHandling (getMigrator (), "computeAdjacentNodes");

      for (int i = (int) m_formationNodeArray->firstILocal (); i <= (int) m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int) m_formationNodeArray->firstJLocal (); j <= (int) m_formationNodeArray->lastJLocal (); ++j)
         {
            getLocalFormationNode (i, j, depthIndex)->computeAdjacentNode ();
         }
      }

      RequestHandling::FinishRequestHandling ();

      return true;
   }

   /// migrate the contents of the charge grid maps to their appropriate columns in the target reservoir
   void Formation::migrateExpelledChargesToReservoir (unsigned int direction, Reservoir * targetReservoir) const
   {
      if (direction == EXPELLEDNONE)
         return;

      const double surfaceFraction = 0.25;
      double expulsionFraction = (direction == EXPELLEDUPANDDOWNWARD ? 1.0 : 0.5);

      unsigned int offsets[4][2] = { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } };
      int depthIndex = m_formationNodeArray->depth () - 1;

      RequestHandling::StartRequestHandling (getMigrator (), "migrateExpelledChargesToReservoir");

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            LocalFormationNode * formationNode = getLocalFormationNode (i, j, depthIndex);
            if (!IsValid (formationNode)) continue;

            FormationNode *targetFormationNode = formationNode->getTargetFormationNode ();

            // check if targetReservoir is the reservoir to migrate to for given i, j
            if (targetFormationNode and targetFormationNode->isEndOfPath () and
                !targetFormationNode->goesOutOfBounds () and
                targetFormationNode->getFormation () == targetReservoir->getFormation ())
            {
               //std::cout << "Well inside migrateExpelledChargesToReservoir for reservoir " << targetReservoir->getFormation ()->getName () << std::endl;
               //std::cout << "It's being charged by SR" << getName () << std::endl;

               unsigned int iTarget = targetFormationNode->getI ();
               unsigned int jTarget = targetFormationNode->getJ ();
               unsigned int kTarget = targetFormationNode->getK ();

               assert (getDepth (iTarget, jTarget, kTarget) >= targetReservoir->getColumn (iTarget, jTarget)->getTopDepth ());

               // calculate the composition to migrate
               Composition composition;
               for (int componentId = FIRST_COMPONENT; componentId < NUM_COMPONENTS; ++componentId)
               {
                  if (!ComponentsUsed[componentId])
                     continue;
                  if (!m_expulsionGridMaps || !m_expulsionGridMaps[componentId])
                     continue;

                  GridMap *densityMap = m_expulsionGridMaps[componentId];

                  double surface = densityMap->getGrid ()->getSurface (1, 1);

                  double sum = 0;

                  int offsetIndex = 0;

                  for (offsetIndex = 0; offsetIndex < 4; ++offsetIndex)
                  {
                     double value;

                     if ((value = densityMap->getValue (i + offsets[offsetIndex][0], j + offsets[offsetIndex][1])) != densityMap->getUndefinedValue ())
                     {
                        sum += value;
                     }
                  }

                  double weight = sum * surfaceFraction * expulsionFraction * surface;

                  composition.add ((ComponentId) componentId, weight);

               }

               int offsetIndex;

               Column * shallowestColumn = 0;
               double shallowestDepth = Interface::DefaultUndefinedScalarValue;

               for (offsetIndex = 0; offsetIndex < 4; ++offsetIndex)
               {
                  Column *targetColumn = targetReservoir->getColumn (iTarget + offsets[offsetIndex][0],
                                                                     jTarget + offsets[offsetIndex][1]);

                  // If the primary column (same i,j as the targetFormationNode) of the element
                  // is wasting then put all the HCs there. Gas phase dictates path for all HCs.
                  // Phase-dependent correction needed.
                  if (offsetIndex == 0 and targetColumn->isWasting (GAS))
                  {
                     shallowestColumn = targetColumn;
                     break;
                  }

                  if (IsValid (targetColumn) and !targetColumn->isSealing () and (shallowestColumn == 0 || targetColumn->getTopDepth () < shallowestDepth))
                  {
                     shallowestColumn = targetColumn;
                     shallowestDepth = targetColumn->getTopDepth ();
                  }
               }

               if (shallowestColumn)
               {
                  shallowestColumn->addCompositionToBeMigrated (composition);
               }
               else
               {
                  targetReservoir->addBlocked (composition);
               }
            }
         }
      }
 
      RequestHandling::FinishRequestHandling ();
   }


   void Formation::migrateLeakedChargesToReservoir (Reservoir * targetReservoir) const
   {
      // LeakingReservoirList should contain only the reservoir corresponding
      // to the formation whose member function is being executed
      DataAccess::Interface::ReservoirList * leakingReservoirList = m_migrator->getReservoirs(this);
      assert (!leakingReservoirList->empty());

      // leakingReservoir is the reservoir hosted in "this" formation
      const DataAccess::Interface::Reservoir * dataAccessReservoir = (*leakingReservoirList)[0];
      
      const Reservoir * leakingReservoir = dynamic_cast<const migration::Reservoir *> (dataAccessReservoir);

      const double surfaceFraction = 0.25;

      unsigned int offsets[4][2] = { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } };

      int depthIndex = m_formationNodeArray->depth () - 1;

      RequestHandling::StartRequestHandling (getMigrator (), "migrateLeakedChargesToReservoir");

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            LocalColumn * leakingColumn = leakingReservoir->getLocalColumn (i, j);
            if (!IsValid (leakingColumn)) continue;

            LocalFormationNode * formationNode = getLocalFormationNode (i, j, depthIndex);
            if (!IsValid (formationNode)) continue;

            // The "leaking" node probably has a reservoir flag (e.g. because it's a trap crest) or isEndOfPath.
            // Then the HC path will be forced to be lateral.
            // But we know it should leak so we force it to do so by probing the node right above it.
            // If no reservoir offsets then this node will belong to the seal.
            formationNode = getLocalFormationNode (i, j, depthIndex + 1);
            
            FormationNode *targetFormationNode = formationNode->getTargetFormationNode ();

            // check if targetReservoir is the reservoir to migrate to for given i, j
            if (targetFormationNode and targetFormationNode->isEndOfPath () and
                !targetFormationNode->goesOutOfBounds () and
                targetFormationNode->getFormation () == targetReservoir->getFormation ())
            {
               unsigned int iTarget = targetFormationNode->getI ();
               unsigned int jTarget = targetFormationNode->getJ ();
               unsigned int kTarget = targetFormationNode->getK ();

               assert (getDepth (iTarget, jTarget, kTarget) >= targetReservoir->getColumn (iTarget, jTarget)->getTopDepth ());

               Composition leakingCompositions[2][2];

               for(int offsetIndex = 0; offsetIndex < 4; ++offsetIndex)
               {
                  if ( IsValid (leakingReservoir->getColumn (i + offsets[offsetIndex][0], j + offsets[offsetIndex][1])))
                  {
                     leakingCompositions [offsets[offsetIndex][0]][offsets[offsetIndex][1]] =
                        leakingReservoir->getColumn (i + offsets[offsetIndex][0], j + offsets[offsetIndex][1])->getComposition ();
                  }
               }

               // calculate the composition to migrate
               Composition composition;
               for (int componentId = FIRST_COMPONENT; componentId < NUM_COMPONENTS; ++componentId)
               {
                  if (!ComponentsUsed[componentId])
                     continue;

                  double sum = 0;

                  for (int offsetIndex = 0; offsetIndex < 4; ++offsetIndex)
                  {
                     sum += leakingCompositions[offsets[offsetIndex][0]][offsets[offsetIndex][1]].getWeight ((ComponentId) componentId);
                  }
                  if (sum)
                  {
                     double weight = sum * surfaceFraction;
                     composition.add ((ComponentId) componentId, weight);
                  }
               }               
               
               if (!composition.isEmpty ())
               {
                  int offsetIndex;

                  Column *shallowestColumn = 0;
                  double shallowestDepth = Interface::DefaultUndefinedScalarValue;

                  for (offsetIndex = 0; offsetIndex < 4; ++offsetIndex)
                  {
                     Column *targetColumn = targetReservoir->getColumn (iTarget + offsets[offsetIndex][0],
                                                                        jTarget + offsets[offsetIndex][1]);

                     if (IsValid (targetColumn) and !targetColumn->isSealing () and (shallowestColumn == 0 || targetColumn->getTopDepth () < shallowestDepth))
                     {
                        shallowestColumn = targetColumn;
                        shallowestDepth = targetColumn->getTopDepth ();
                     }
                  }

                  if (shallowestColumn)
                  {
                     shallowestColumn->addCompositionToBeMigrated (composition);
                  }
                  else
                  {
                     targetReservoir->addBlocked (composition);
                  }
               }
            }
         }
      }
 
      RequestHandling::FinishRequestHandling ();
   }

   bool Formation::preprocessSourceRock (const double startTime, const bool printDebug) {

      const double depoTime = (getTopSurface () ? getTopSurface ()->getSnapshot ()->getTime () : 0);
      bool sourceRockIsActive = (depoTime > startTime) || fabs (depoTime - startTime) < Genex6::Constants::ZERO;

      if (sourceRockIsActive) {
         if (GetRank () == 0) {
            cout << "Preprocessing formation " << getName () << " at the time interval: " << depoTime << " Ma to " << startTime << " Ma ..." << endl;
         }
 
         if( m_genexData == 0 ) {
            m_genexData = m_projectHandle->getFactory()->produceGridMap ( 0, 0, m_projectHandle->getActivityOutputGrid (), 99999.0, NUM_COMPONENTS );
         }
         const GeoPhysics::GeoPhysicsSourceRock * sourceRock = dynamic_cast<const GeoPhysics::GeoPhysicsSourceRock *>(getSourceRock1 ());
         GeoPhysics::GeoPhysicsSourceRock * sourceRock1 = const_cast<GeoPhysics::GeoPhysicsSourceRock *>(sourceRock);
      
         Interface::SnapshotList * snapshots = m_projectHandle->getSnapshots (Interface::MINOR | Interface::MAJOR);
      
         // present day map
         DerivedProperties::FormationPropertyPtr vrProperty = m_migrator->getPropertyManager().getFormationProperty( m_migrator->getPropertyManager().getProperty("Vr"), *(snapshots->begin()), this);
         const GridMap * gridMapEnd = m_migrator->getPropertyManager().produceDerivedGridMap(vrProperty);
      
         if (!gridMapEnd) {
         
            if (GetRank () == 0) {
               cout << getName () << ": Cannot find Vr present day  map" << endl;
            }
            return false;
         }
         m_isInitialised = true;
      
         sourceRock1->setFormationData (this);
         sourceRock1->initialize (false);
         sourceRock1->preprocess (gridMapEnd, gridMapEnd, false);
      
         Interface::SnapshotList::reverse_iterator snapshotIter;
      
         const Interface::Snapshot * start;
         const Interface::Snapshot * end = 0;
         bool status = true;
      
         for (snapshotIter = snapshots->rbegin (); snapshotIter != snapshots->rend () - 1; ++snapshotIter) {
         
            start = (*snapshotIter);
            end = 0;
         
            if (start->getTime () > startTime) {
               if ((depoTime > start->getTime ()) || (fabs (depoTime - start->getTime ()) < Genex6::Constants::ZERO)) {
               
                  start = (*snapshotIter);
                  end = *(snapshotIter + 1);
               
                  status = calculateGenexTimeInterval (start, end, printDebug);
               }
            }
         }
      
         delete snapshots; 

         return status;
      }
      return true;
   }


   bool Formation::calculateGenexTimeInterval (const Interface::Snapshot * start, const Interface::Snapshot * end, const bool printDebug) {

      const double depoTime = (getTopSurface () ? getTopSurface ()->getSnapshot ()->getTime () : 0);
      bool sourceRockIsActive = (depoTime > start->getTime ()) || fabs (depoTime - start->getTime ()) < Genex6::Constants::ZERO;

      if (sourceRockIsActive and m_isInitialised) {

         bool status = m_isInitialised;
 
         if (start->getTime () == m_startGenexTime and end->getTime () == m_endGenexTime) {
            if (GetRank () == 0 and printDebug) {
               cout << getName () << ": Genex interval start = " << start->getTime () << ", end = " << end->getTime () << " is already calculated" << endl;
            }
            return true;
         }       

         Genex6::LinearGridInterpolator vesInterp;
         Genex6::LinearGridInterpolator temperatureInterp;
         Genex6::LinearGridInterpolator thicknessInterp;
      
         Genex6::LinearGridInterpolator hydrostaticPressureInterp;
         Genex6::LinearGridInterpolator lithostaticPressureInterp;
         Genex6::LinearGridInterpolator porePressureInterp;
         Genex6::LinearGridInterpolator porosityInterp;
         Genex6::LinearGridInterpolator permeabilityInterp;
         Genex6::LinearGridInterpolator vreInterp;
      
         status = extractGenexDataInterval (start, end,
                                            thicknessInterp,
                                            vesInterp,
                                            temperatureInterp,
                                            hydrostaticPressureInterp,
                                            lithostaticPressureInterp,
                                            porePressureInterp,
                                            porosityInterp,
                                            permeabilityInterp,
                                            vreInterp);
      
         if (!status) {
            if (GetRank () == 0) {
               cout << getName () << ": Cannot extract  genex interval start = " << start->getTime () << ", end = " << end->getTime () << endl;
            }
        
            return false;
         }
      
         if (GetRank () == 0 and printDebug) {
            cout << getName () << ": Calculate genex interval start = " << start->getTime () << ", end = " << end->getTime () << endl;
         }

         const GeoPhysics::GeoPhysicsSourceRock * sourceRock = dynamic_cast<const GeoPhysics::GeoPhysicsSourceRock *>(getSourceRock1 ());
         GeoPhysics::GeoPhysicsSourceRock * sourceRock1 = const_cast<GeoPhysics::GeoPhysicsSourceRock *>(sourceRock);
      
         sourceRock1->computeTimeInterval (start->getTime (), end->getTime (),
                                           &vesInterp,
                                           &temperatureInterp,
                                           &thicknessInterp,
                                           &lithostaticPressureInterp,
                                           &hydrostaticPressureInterp,
                                           &porePressureInterp,
                                           &porosityInterp,
                                           &permeabilityInterp,
                                           &vreInterp,
                                           m_genexData);
      
      
      
         m_startGenexTime = start->getTime ();
         m_endGenexTime = end->getTime ();
      
         return status;
      } 
   
      return true;
   }

   bool Formation::extractGenexDataInterval (const Snapshot *intervalStart,
                                             const Snapshot *intervalEnd,
                                             Genex6::LinearGridInterpolator& thickness,
                                             Genex6::LinearGridInterpolator& ves,
                                             Genex6::LinearGridInterpolator& temperature,
                                             Genex6::LinearGridInterpolator& hydrostaticPressure,
                                             Genex6::LinearGridInterpolator& lithostaticPressure,
                                             Genex6::LinearGridInterpolator& porePressure,
                                             Genex6::LinearGridInterpolator& porosity,
                                             Genex6::LinearGridInterpolator& permeability,
                                             Genex6::LinearGridInterpolator& vre) {
 
      const DataModel::AbstractProperty* property = m_migrator->getPropertyManager ().getProperty ("ErosionFactor");
      DerivedProperties::FormationMapPropertyPtr startProperty = m_migrator->getPropertyManager ().getFormationMapProperty (property, intervalStart, this);
      DerivedProperties::FormationMapPropertyPtr endProperty = m_migrator->getPropertyManager ().getFormationMapProperty (property, intervalEnd, this);

      bool status = true;
      if (startProperty and endProperty) {
         startProperty->retrieveData ();
         endProperty->retrieveData ();

         thickness.compute (intervalStart, startProperty, intervalEnd, endProperty);

         startProperty->restoreData ();
         endProperty->restoreData ();
      }
      else {
         if (GetRank () == 0) {
            if (!startProperty) {
               cout << "Property map ErosionFactor " << getTopSurface ()->getName () << " at " << intervalStart->getTime () << " not found" << endl;
            }
            if (!endProperty) {
               cout << "Property map ErosionFactor " << getTopSurface ()->getName () << " at " << intervalEnd->getTime () << " not found" << endl;
            }
         }
         status = false;
      }
   
      status = computeInterpolator ( "Ves", intervalStart, intervalEnd, ves ) and status;
      status = computeInterpolator ( "Temperature", intervalStart, intervalEnd, temperature ) and status;
      status = computeInterpolator ( "HydroStaticPressure", intervalStart, intervalEnd, hydrostaticPressure ) and status;
      status = computeInterpolator ( "LithoStaticPressure", intervalStart, intervalEnd, lithostaticPressure ) and status;
      status = computeInterpolator ( "Pressure", intervalStart, intervalEnd, porePressure ) and status;

      status = computeInterpolator ( "Porosity", intervalStart, intervalEnd, porosity ) and status;
      status = computeInterpolator ( "Permeability", intervalStart, intervalEnd, permeability ) and status;
      status = computeInterpolator ( "VrVec2", intervalStart, intervalEnd, vre ) and status;

      return status;
   }

   bool Formation::computeInterpolator (const string & propertyName, const Snapshot *intervalStart, const Snapshot *intervalEnd, Genex6::LinearGridInterpolator& interpolator) {

      const DataModel::AbstractProperty* property;
   
      property = m_migrator->getPropertyManager ().getProperty (propertyName);


      if( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY ) {
         DerivedProperties::SurfacePropertyPtr startSurfaceProperty = m_migrator->getPropertyManager().getSurfaceProperty(property, intervalStart, getTopSurface());
         DerivedProperties::SurfacePropertyPtr endSurfaceProperty = m_migrator->getPropertyManager().getSurfaceProperty(property, intervalEnd, getTopSurface());

            startSurfaceProperty->retrieveData();
            endSurfaceProperty->retrieveData();

            interpolator.compute (intervalStart, startSurfaceProperty, intervalEnd, endSurfaceProperty);

            startSurfaceProperty->restoreData();
            endSurfaceProperty->restoreData();
      } else if( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {
         DerivedProperties::FormationSurfacePropertyPtr startProperty = m_migrator->getPropertyManager().getFormationSurfaceProperty(property, intervalStart,
                                                                                                                                     this, getTopSurface() );
         DerivedProperties::FormationSurfacePropertyPtr endProperty = m_migrator->getPropertyManager().getFormationSurfaceProperty(property, intervalEnd,
                                                                                                                                   this, getTopSurface() );
      
         if (startProperty and endProperty) {
            startProperty->retrieveData ();
            endProperty->retrieveData ();
            
            interpolator.compute (intervalStart, startProperty, intervalEnd, endProperty);
            
            startProperty->restoreData ();
            endProperty->restoreData ();
         }
         else {
            if (GetRank () == 0) {
               if (!startProperty) {
                  cout << "Property map " << propertyName << " " << getTopSurface ()->getName () << " at " << intervalStart->getTime () << " not found" << endl;
               }
               if (!endProperty) {
                  cout << "Property map " << propertyName << " " << getTopSurface ()->getName () << " at " << intervalEnd->getTime () << " not found" << endl;
               }
            }
            return false;
         }

      } else {

         if (GetRank () == 0) {
            cout << "Property map " << propertyName << " " << getTopSurface()->getName() << " at " << intervalStart->getTime() << " not found" << endl;
         }
         return false;
      }

      return true;
   }

   double Formation::getDeltaI (void)
   {
      return getGrid ()->deltaI ();
   }

   double Formation::getDeltaJ (void)
   {
      return getGrid ()->deltaJ ();
   }

   const Grid * Formation::getGrid (void) const
   {
      return m_projectHandle->getHighResolutionOutputGrid (); // not to be changed to getActivityOutputGrid ()!!
   }

   const Interface::GridMap * Formation::getPropertyGridMap (const string & propertyName,
                                                             const Interface::Snapshot * snapshot) const

   {
      return m_migrator->getPropertyGridMap (propertyName, snapshot, 0, this, 0);
   }

}
