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
      m_detectedReservoirRecord = false;
      m_detectedReservoir = false;

      m_genexData = 0;

      m_startGenexTime = -1;
      m_endGenexTime = -1;
      m_formationNodeArray = NULL;

      // Initializing other data members for Windows build

      for (size_t i = 0; i != NUM_COMPONENTS; ++i)
         m_expulsionGridMaps[i] = NULL;

      // Initializing other data members for Windows build
      for (size_t i = 0; i != NUMBEROFPROPERTYINDICES; ++i)
         m_gridMaps[i] = NULL;

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

      const GridMap * formationGridMap = m_gridMaps[DEPTHPROPERTY];
      assert (formationGridMap);
      assert (formationGridMap->getGrid () == m_migrator->getProjectHandle ()->getActivityOutputGrid ());

      int depth = formationGridMap->getDepth ();
      assert (depth > 1);

      const Grid * grid = m_migrator->getProjectHandle ()->getActivityOutputGrid ();

      m_formationNodeArray = new FormationNodeArray (this,
                                                     grid->numIGlobal () - 1, grid->numJGlobal () - 1,
                                                     grid->firstI (), Min (grid->lastI (), grid->numIGlobal () - 2),
                                                     grid->firstJ (), Min (grid->lastJ (), grid->numJGlobal () - 2), depth - 1);

   }

   bool Formation::computePropertyMaps (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot,
                                        bool lowResEqualsHighRes, bool isPressureRun, bool nonGeometricLoop, bool chemicalCompaction)
   {
      assert (topDepthGridMap);
      const Grid *grid = m_projectHandle->getActivityOutputGrid ();

      assert (topDepthGridMap->getGrid () == grid);

      unsigned int firstI = grid->firstI ();
      unsigned int lastI = grid->lastI ();
      unsigned int firstJ = grid->firstJ ();
      unsigned int lastJ = grid->lastJ ();
      unsigned int i, j;
      int depth;

      if (!hasVolumePropertyGridMap ("Ves", snapshot))
         return false;

      if ((m_gridMaps[VESPROPERTY] = getVolumePropertyGridMap ("Ves", snapshot)) == 0)
         return false;
      if ((m_gridMaps[MAXVESPROPERTY] = getVolumePropertyGridMap ("MaxVes", snapshot)) == 0)
         return false;
      if ((m_gridMaps[PRESSUREPROPERTY] = getVolumePropertyGridMap ("Pressure", snapshot)) == 0)
         return false;
      if ((m_gridMaps[TEMPERATUREPROPERTY] = getVolumePropertyGridMap ("Temperature", snapshot)) == 0)
         return false;

      // load or calculate the rest
      if (lowResEqualsHighRes) // no reason to perform all these computations as we can just load the maps
      {
         FormationNode * getFormationNode (int i, int j, int k);

         if ((m_gridMaps[DEPTHPROPERTY] = getVolumePropertyGridMap ("Depth", snapshot)) == 0)
            return false;
         if ((m_gridMaps[POROSITYPROPERTY] = getVolumePropertyGridMap ("Porosity", snapshot)) == 0)
            return false;
         if ((m_gridMaps[VERTICALPERMEABILITYPROPERTY] = getVolumePropertyGridMap ("Permeability", snapshot)) == 0)
            return false;
         if ((m_gridMaps[HORIZONTALPERMEABILITYPROPERTY] = getVolumePropertyGridMap ("HorizontalPermeability", snapshot)) == 0)
            return false;
	 if ((m_gridMaps[OVERPRESSUREPROPERTY] = getVolumePropertyGridMap("OverPressure", snapshot)) == 0)
	    return false;

         return true;
      }

      // Compute OverPressure if a pressure run.
      if (isPressureRun)
      {
         DerivedProperties::FormationPropertyPtr gridMap = getFormationPropertyPtr ("OverPressure", snapshot);

         if (gridMap == 0)
         {
            return false;
         }

         unsigned int depth = gridMap->lengthK ();
         assert (depth > 1);

         // Top series of domain nodes absent in fastmig so make sure they are not used
         if (!getTopFormation ())
            --depth;

         gridMap->retrieveData ();
         for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
         {
            for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
            {
               for (unsigned int k = 0; k < depth; ++k)
               {
                  LocalFormationNode * formationNode = getLocalFormationNode (i, j, k);

                  formationNode->setOverPressure (gridMap->get (i, j, k));
               }
            }
         }
         gridMap->restoreData ();
      }

      SmartGridMapRetrieval    vesPropertyGridMapRetrieval (m_gridMaps[VESPROPERTY], false);
      SmartGridMapRetrieval maxVesPropertyGridMapRetrieval (m_gridMaps[MAXVESPROPERTY], false);

      // load or calculate porosities
      if (chemicalCompaction)
      {
         if ((m_gridMaps[POROSITYPROPERTY] = getVolumePropertyGridMap ("Porosity", snapshot)) == 0)
            return false;
      }
      else
      {
         GridMap *porosityGridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, grid,
                                                                                    Interface::DefaultUndefinedMapValue, getGridMapDepth ());

         m_gridMaps[POROSITYPROPERTY] = porosityGridMap;
         SmartGridMapRetrieval porosityGridMapRetrieval (porosityGridMap, true);

         for (depth = porosityGridMap->getDepth () - 1; depth >= 0; --depth)
         {
            for (i = firstI; i <= lastI; ++i)
            {
               for (j = firstJ; j <= lastJ; ++j)
               {
                  double ves = m_gridMaps[VESPROPERTY]->getValue (i, j, (unsigned int) depth);
                  double maxVes = m_gridMaps[MAXVESPROPERTY]->getValue (i, j, (unsigned int) depth);

                  if (ves != m_gridMaps[VESPROPERTY]->getUndefinedValue () && maxVes != m_gridMaps[MAXVESPROPERTY]->getUndefinedValue ())
                  {
                     double porosity = getCompoundLithology (i, j)->porosity (ves, maxVes, 0.0, false);

                     porosityGridMap->setValue (i, j, (unsigned int) depth, porosity * Fraction2Percentage);
                  }
               }
            }
         }
      }

      getCompoundLithologyArray ().setCurrentLithologies (snapshot->getTime ());

      GridMap *depthGridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, grid,
                                                                              Interface::DefaultUndefinedMapValue, getGridMapDepth ());

      m_gridMaps[DEPTHPROPERTY] = depthGridMap;

      // to be used
      SmartGridMapRetrieval porosityGridMapRetrieval (m_gridMaps[POROSITYPROPERTY], false);

      // to be produced
      SmartGridMapRetrieval topDepthGridMapRetrieval (topDepthGridMap, true);
      SmartGridMapRetrieval depthGridMapRetrieval (depthGridMap, true);

      // calculate depths
      for (i = firstI; i <= lastI; ++i)
      {
         for (j = firstJ; j <= lastJ; ++j)
         {
            bool firstTime = true;
            double topDepth = topDepthGridMap->getValue (i, j);

            bool depthIsDefined = (topDepth != topDepthGridMap->getUndefinedValue ());

            for (depth = depthGridMap->getDepth () - 1; depth >= 0; --depth)
            {
               double porosityBottom;
               double porosityTop;
               double realThickness = Interface::DefaultUndefinedScalarValue;

               if (!firstTime && depthIsDefined)
               {
                  if (isPressureRun && nonGeometricLoop)
                  {
                     realThickness = getRealThickness (i, j, (unsigned int) depth, snapshot->getTime ());
                  }
                  else
                  {
                     porosityBottom = m_gridMaps[POROSITYPROPERTY]->getValue (i, j, (unsigned int) depth);
                     porosityTop = m_gridMaps[POROSITYPROPERTY]->getValue (i, j, (unsigned int) depth + 1);

                     if (porosityBottom != m_gridMaps[POROSITYPROPERTY]->getUndefinedValue () && porosityTop != m_gridMaps[POROSITYPROPERTY]->getUndefinedValue ())
                     {
                        // porosity expected in fractions
                        porosityBottom *= Percentage2Fraction;
                        porosityTop *= Percentage2Fraction;
                        realThickness = computeRealThickness (i, j, (unsigned int) depth, porosityTop, porosityBottom, snapshot->getTime ());
                     }
                  }

                  if (realThickness == Interface::DefaultUndefinedScalarValue)
                  {
                     realThickness = 0;
                  }

                  topDepth += realThickness;
               }

               depthGridMap->setValue (i, j, (unsigned int) depth, topDepth);

#if 0
               if (!depthIsDefined)
               {
                  cerr << "depthIsDefined = " << depthIsDefined << endl;
                  cerr << "porosityTop = " << porosityTop << endl;
                  cerr << "porosityBottom = " << porosityBottom << endl;
                  cerr << "realThickness = " << realThickness << endl;
                  cerr << "topDepth = " << topDepth << endl;
                  cerr << "undefined = " << topDepthGridMap->getUndefinedValue () << endl;
                  cerr << getName () << "->depth (" << i << ", " << j << ", " << depth << ") = " << topDepth << endl;
                  cerr << endl;
               }
#endif

               firstTime = false;
            }

            topDepthGridMap->setValue (i, j, topDepth);
         }
      }

      // calculate permeabilities
      GridMap *horizontalPermeabilityGridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, grid,
                                                                                               Interface::DefaultUndefinedMapValue, getGridMapDepth ());
      m_gridMaps[HORIZONTALPERMEABILITYPROPERTY] = horizontalPermeabilityGridMap;
      SmartGridMapRetrieval horizontalPermeabilityGridMapRetrieval (horizontalPermeabilityGridMap, true);

      GridMap *verticalPermeabilityGridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, grid,
                                                                                             Interface::DefaultUndefinedMapValue, getGridMapDepth ());
      m_gridMaps[VERTICALPERMEABILITYPROPERTY] = verticalPermeabilityGridMap;
      SmartGridMapRetrieval verticalPermeabilityGridMapRetrieval (verticalPermeabilityGridMap, true);

      for (depth = horizontalPermeabilityGridMap->getDepth () - 1; depth >= 0; --depth)
      {
         for (i = firstI; i <= lastI; ++i)
         {
            for (j = firstJ; j <= lastJ; ++j)
            {
               double ves = m_gridMaps[VESPROPERTY]->getValue (i, j, (unsigned int) depth);
               double maxVes = m_gridMaps[MAXVESPROPERTY]->getValue (i, j, (unsigned int) depth);
               double porosity = m_gridMaps[POROSITYPROPERTY]->getValue (i, j, (unsigned int) depth);

               if (ves != m_gridMaps[VESPROPERTY]->getUndefinedValue () &&
                   maxVes != m_gridMaps[MAXVESPROPERTY]->getUndefinedValue () &&
                   porosity != m_gridMaps[POROSITYPROPERTY]->getUndefinedValue ())
               {
                  double horizontalPermeability;
                  double verticalPermeability;

                  porosity *= Percentage2Fraction;

                  getCompoundLithology (i, j)->calcBulkPermeabilityNP (ves, maxVes, porosity, verticalPermeability, horizontalPermeability);
                  verticalPermeability *= GeoPhysics::M2TOMILLIDARCY;
                  horizontalPermeability *= GeoPhysics::M2TOMILLIDARCY;

                  horizontalPermeabilityGridMap->setValue (i, j, (unsigned int) depth, horizontalPermeability);
                  verticalPermeabilityGridMap->setValue (i, j, (unsigned int) depth, verticalPermeability);
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

      if (!hasVolumePropertyGridMap ("Ves", snapshot))
         return false;

      GridMap *capillaryPressureGas100GridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, grid,
                                                                                                Interface::DefaultUndefinedMapValue, getGridMapDepth ());

      GridMap *capillaryPressureOil100GridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, grid,
                                                                                                Interface::DefaultUndefinedMapValue, getGridMapDepth ());


      m_gridMaps[CAPILLARYPRESSUREGAS100PROPERTY] = capillaryPressureGas100GridMap;
      m_gridMaps[CAPILLARYPRESSUREOIL100PROPERTY] = capillaryPressureOil100GridMap;

      //Required properties to compute capillary pressures
      SmartGridMapRetrieval vesPropertyGridMapRetrieval (m_gridMaps[VESPROPERTY], false);
      SmartGridMapRetrieval maxVesPropertyGridMapRetrieval (m_gridMaps[MAXVESPROPERTY], false);
      SmartGridMapRetrieval porosityPropertyGridMapRetrieval (m_gridMaps[POROSITYPROPERTY], false);

      SmartGridMapRetrieval gasDensityPropertyGridMapRetrieval (m_gridMaps[GASDENSITYPROPERTY], false);
      SmartGridMapRetrieval oilDensityPropertyGridMapRetrieval (m_gridMaps[OILDENSITYPROPERTY], false);
      SmartGridMapRetrieval temperaturePropertyGridMapRetrieval (m_gridMaps[TEMPERATUREPROPERTY], false);
      SmartGridMapRetrieval pressurePropertyGridMapRetrieval (m_gridMaps[PRESSUREPROPERTY], false);
      SmartGridMapRetrieval verticalPermeabilityPropertyGridMapRetrieval (m_gridMaps[VERTICALPERMEABILITYPROPERTY], false);

      SmartGridMapRetrieval capillaryPressureGas100GridMapRetrieval (capillaryPressureGas100GridMap, true);
      SmartGridMapRetrieval capillaryPressureOil100GridMapRetrieval (capillaryPressureOil100GridMap, true);

      unsigned int firstI = grid->firstI ();
      unsigned int lastI = grid->lastI ();
      unsigned int firstJ = grid->firstJ ();
      unsigned int lastJ = grid->lastJ ();
      unsigned int i, j;
      int k;

      for (k = capillaryPressureGas100GridMap->getDepth () - 1; k >= 0; --k)
      {
         for (i = firstI; i <= lastI; ++i)
         {
            for (j = firstJ; j <= lastJ; ++j)
            {
               double ves = m_gridMaps[VESPROPERTY]->getValue (i, j, (unsigned int) k);
               double maxVes = m_gridMaps[MAXVESPROPERTY]->getValue (i, j, (unsigned int) k);
               double porosity = m_gridMaps[POROSITYPROPERTY]->getValue (i, j, (unsigned int) k);

               if (ves != m_gridMaps[VESPROPERTY]->getUndefinedValue () &&
                   maxVes != m_gridMaps[MAXVESPROPERTY]->getUndefinedValue () &&
                   porosity != m_gridMaps[POROSITYPROPERTY]->getUndefinedValue ())
               {
                  double capillaryPressureGas100 (Interface::DefaultUndefinedMapValue);
                  double capillaryPressureOil100 (Interface::DefaultUndefinedMapValue);
                  const GeoPhysics::FluidType * fluid = (GeoPhysics::FluidType *) getFluidType ();

                  double gasDensity = m_gridMaps[GASDENSITYPROPERTY]->getValue (i, j, (unsigned int) k);
                  double oilDensity = m_gridMaps[OILDENSITYPROPERTY]->getValue (i, j, (unsigned int) k);
                  double tempValue = m_gridMaps[TEMPERATUREPROPERTY]->getValue (i, j, (unsigned int) k);
                  double pressValue = m_gridMaps[PRESSUREPROPERTY]->getValue (i, j, (unsigned int) k);
                  double permeability = m_gridMaps[VERTICALPERMEABILITYPROPERTY]->getValue (i, j, (unsigned int) k);

                  double waterDensity = fluid->density (tempValue, pressValue);

                  // Do not assign any value and continue 
                  if (gasDensity == Interface::DefaultUndefinedMapValue ||
                      oilDensity == Interface::DefaultUndefinedMapValue ||
                      waterDensity <= 0.0)
                     continue;

                  double hcTempValueGas = pvtFlash::getCriticalTemperature (C1, 0);
                  double hcTempValueOil = pvtFlash::getCriticalTemperature (C6_14SAT, 0);

                  const double capC1 = getCompoundLithology (i, j)->capC1 ();
                  const double capC2 = getCompoundLithology (i, j)->capC2 ();

                  double capSealStrength_Air_Hg = CBMGenerics::capillarySealStrength::capSealStrength_Air_Hg (capC1, capC2, permeability);

                  double oilIFT = CBMGenerics::capillarySealStrength::capTension_H2O_HC (waterDensity, oilDensity, tempValue + CBMGenerics::C2K, hcTempValueOil);
                  // double gasIFT = CBMGenerics::capillarySealStrength::capTension_H2O_HC (waterDensity, gasDensity, tempValue + CBMGenerics::C2K, hcTempValueGas);

                  // Considers 180 deg. angle between H2O and HC (strictly speaking not true for oil)
                  capillaryPressureOil100 = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC (capSealStrength_Air_Hg, oilIFT);
                  capillaryPressureGas100 = capillaryPressureOil100 + capillaryEntryPressureOilGas (permeability, pressValue, capC1, capC2);

                  if (capillaryPressureOil100 == Interface::DefaultUndefinedMapValue && capillaryPressureGas100 == Interface::DefaultUndefinedMapValue)
                  {
                     continue;
                  }
                  else
                  {
                     capillaryPressureGas100GridMap->setValue (i, j, (unsigned int) k, capillaryPressureGas100);
                     capillaryPressureOil100GridMap->setValue (i, j, (unsigned int) k, capillaryPressureOil100);
                  }

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
         delete m_gridMaps[i];
         m_gridMaps[i] = 0;
      }
      return true;
   }

   bool Formation::computeHCDensityMaps ()
   {
      if (m_gridMaps[TEMPERATUREPROPERTY] == 0 || m_gridMaps[PRESSUREPROPERTY] == 0)
         return false;

      const Grid *grid = m_projectHandle->getActivityOutputGrid ();
      unsigned int firstI = grid->firstI ();
      unsigned int lastI = grid->lastI ();
      unsigned int firstJ = grid->firstJ ();
      unsigned int lastJ = grid->lastJ ();
      unsigned int i, j;
      int depth;

      GridMap *oilDensityGridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, grid,
                                                                                   Interface::DefaultUndefinedMapValue,
                                                                                   getGridMapDepth ());

      m_gridMaps[OILDENSITYPROPERTY] = oilDensityGridMap;
      GridMap *gasDensityGridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, grid,
                                                                                   Interface::DefaultUndefinedMapValue,
                                                                                   getGridMapDepth ());

      m_gridMaps[GASDENSITYPROPERTY] = gasDensityGridMap;

#if 0
      cerr << "Compute density maps formation " << getName () << endl;
#endif

      SmartGridMapRetrieval oilDensityGridMapRetrieval (oilDensityGridMap, true);
      SmartGridMapRetrieval gasDensityGridMapRetrieval (gasDensityGridMap, true);

      SmartGridMapRetrieval temperatureGridMapRetrieval (m_gridMaps[TEMPERATUREPROPERTY], false);
      SmartGridMapRetrieval pressureGridMapRetrieval (m_gridMaps[PRESSUREPROPERTY], false);

      double  compMasses[CBMGenerics::ComponentManager::NumberOfSpeciesToFlash];
      double  phaseCompMasses[CBMGenerics::ComponentManager::NumberOfPhases][CBMGenerics::ComponentManager::NumberOfSpeciesToFlash];
      double  phaseDensity[CBMGenerics::ComponentManager::NumberOfSpeciesToFlash];
      double  phaseViscosity[CBMGenerics::ComponentManager::NumberOfSpeciesToFlash];
   
      for(int nc=0; nc != CBMGenerics::ComponentManager::NumberOfSpeciesToFlash ;++nc)
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

      for (depth = oilDensityGridMap->getDepth () - 1; depth >= 0; --depth)
      {
         for (i = firstI; i <= lastI; ++i)
         {
            for (j = firstJ; j <= lastJ; ++j)
            {
               double tempValue = m_gridMaps[TEMPERATUREPROPERTY]->getValue (i, j, (unsigned int) depth);
               double pressValue = m_gridMaps[PRESSUREPROPERTY]->getValue (i, j, (unsigned int) depth);

               if (tempValue != m_gridMaps[TEMPERATUREPROPERTY]->getUndefinedValue () &&
                   pressValue != m_gridMaps[PRESSUREPROPERTY]->getUndefinedValue ())
               {
                  int gasComponent = pvtFlash::C1;
                  int oilComponent = pvtFlash::C6_14SAT;

                  int gasPhase, oilPhase;
                  double gasDensity, oilDensity;
                  double gasViscosity, oilViscosity;

                  pvtFlash::EosPack::getInstance ().compute (tempValue + CBMGenerics::C2K, pressValue * CBMGenerics::MPa2Pa,
                                                             compMasses, phaseCompMasses, phaseDensity, phaseViscosity);


                  gasDensityGridMap->setValue (i, j, (unsigned int) depth, phaseDensity[ CBMGenerics::ComponentManager::C1]);
                  oilDensityGridMap->setValue (i, j, (unsigned int) depth, phaseDensity[ CBMGenerics::ComponentManager::C6Minus14Sat]);
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

   int Formation::getGridMapDepth (void)
   {
      return getMaximumNumberOfElements () + 1;
   }

   /// Get the value of a property at specified indices.
   /// Will look in adjacent formation if k is out of range
   double Formation::getPropertyValue (PropertyIndex propertyIndex, int i, int j, int k)
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
         const GridMap * gridMap = getGridMap (propertyIndex);
         if (!gridMap)
         {
            value = Interface::DefaultUndefinedMapValue;
         }
         else
         {
            value = gridMap->getValue ((unsigned int) i, (unsigned int) j, (unsigned int) k);
            if (value == gridMap->getUndefinedValue ()) value = Interface::DefaultUndefinedMapValue;
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
         for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
         {
            for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
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
            getLocalFormationNode (i, j, depthIndex)->prescribeTargetFormationNode ();
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

   bool Formation::retrievePropertyMaps (void)
   {
      assert (m_gridMaps[DEPTHPROPERTY]);
      m_gridMaps[DEPTHPROPERTY]->retrieveData (true);

      assert (m_gridMaps[VERTICALPERMEABILITYPROPERTY]);
      m_gridMaps[VERTICALPERMEABILITYPROPERTY]->retrieveData (true);

      assert (m_gridMaps[HORIZONTALPERMEABILITYPROPERTY]);
      m_gridMaps[HORIZONTALPERMEABILITYPROPERTY]->retrieveData (true);

      assert (m_gridMaps[POROSITYPROPERTY]);
      m_gridMaps[POROSITYPROPERTY]->retrieveData (true);

      assert (m_gridMaps[TEMPERATUREPROPERTY]);
      m_gridMaps[TEMPERATUREPROPERTY]->retrieveData (true);

      assert (m_gridMaps[PRESSUREPROPERTY]);
      m_gridMaps[PRESSUREPROPERTY]->retrieveData (true);

      assert (m_gridMaps[GASDENSITYPROPERTY]);
      m_gridMaps[GASDENSITYPROPERTY]->retrieveData (true);

      assert (m_gridMaps[OILDENSITYPROPERTY]);
      m_gridMaps[OILDENSITYPROPERTY]->retrieveData (true);

      return true;
   }

   bool Formation::retrieveCapillaryPressureMaps ()
   {
      if (performHDynamicAndCapillary ())
      {
         assert (m_gridMaps[CAPILLARYPRESSUREGAS100PROPERTY]);
         m_gridMaps[CAPILLARYPRESSUREGAS100PROPERTY]->retrieveData (true);

         assert (m_gridMaps[CAPILLARYPRESSUREOIL100PROPERTY]);
         m_gridMaps[CAPILLARYPRESSUREOIL100PROPERTY]->retrieveData (true);

      }

      return true;
   }

   bool Formation::restorePropertyMaps (void)
   {
      assert (m_gridMaps[DEPTHPROPERTY]);
      m_gridMaps[DEPTHPROPERTY]->restoreData ();

      assert (m_gridMaps[VERTICALPERMEABILITYPROPERTY]);
      m_gridMaps[VERTICALPERMEABILITYPROPERTY]->restoreData ();

      assert (m_gridMaps[HORIZONTALPERMEABILITYPROPERTY]);
      m_gridMaps[HORIZONTALPERMEABILITYPROPERTY]->restoreData ();

      assert (m_gridMaps[POROSITYPROPERTY]);
      m_gridMaps[POROSITYPROPERTY]->restoreData ();

      if (performHDynamicAndCapillary ())
      {
         assert (m_gridMaps[TEMPERATUREPROPERTY]);
         m_gridMaps[TEMPERATUREPROPERTY]->restoreData ();

         assert (m_gridMaps[PRESSUREPROPERTY]);
         m_gridMaps[PRESSUREPROPERTY]->restoreData ();

         assert (m_gridMaps[GASDENSITYPROPERTY]);
         m_gridMaps[GASDENSITYPROPERTY]->restoreData ();

         assert (m_gridMaps[OILDENSITYPROPERTY]);
         m_gridMaps[OILDENSITYPROPERTY]->restoreData ();
      }
      return true;
   }

   bool Formation::restoreCapillaryPressureMaps ()
   {
      if (performHDynamicAndCapillary ())
      {
         assert (m_gridMaps[CAPILLARYPRESSUREGAS100PROPERTY]);
         m_gridMaps[CAPILLARYPRESSUREGAS100PROPERTY]->restoreData ();

         assert (m_gridMaps[CAPILLARYPRESSUREOIL100PROPERTY]);
         m_gridMaps[CAPILLARYPRESSUREOIL100PROPERTY]->restoreData ();

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

   //
   // Loop through the uppermost cells and check capillary pressure across the boundary
   //
   bool Formation::detectReservoir (Formation * topFormation,
                                    const double minOilColumnHeight, const double minGasColumnHeight, const bool pressureRun)
   {
	  
      int upperIndex = getNodeDepth () - 1;
      int lowerIndex = 0;

      for (int i = (int) m_formationNodeArray->firstILocal (); i <= (int) m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int) m_formationNodeArray->firstJLocal (); j <= (int) m_formationNodeArray->lastJLocal (); ++j)
         {
            getLocalFormationNode (i, j, upperIndex)->detectReservoir (topFormation->getLocalFormationNode (i, j, lowerIndex),
                                                                       minOilColumnHeight, minGasColumnHeight, pressureRun);
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

      RequestHandling::StartRequestHandling(getMigrator(), "detectReservoirCrests");

      int upperIndex = getNodeDepth() - 1;
      bool reservoirCrestDetected = false;

      // First  oil 
      for (int i = (int)m_formationNodeArray->firstILocal(); i <= (int)m_formationNodeArray->lastILocal(); ++i)
      {
         for (int j = (int)m_formationNodeArray->firstJLocal(); j <= (int)m_formationNodeArray->lastJLocal(); ++j)
         {
            reservoirCrestDetected = getLocalFormationNode(i, j, upperIndex)->detectReservoirCrests(OIL);
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
               reservoirCrestDetected = getLocalFormationNode(i, j, upperIndex)->detectReservoirCrests(GAS);
               if (reservoirCrestDetected) break;
            }
            if (reservoirCrestDetected) break;
         }
      }

      m_detectedReservoir = MaximumAll ((int) reservoirCrestDetected);
      //cerr << " Rank, Formation, m_detectedReservoir " << GetRank () << " " << getName () << " " << m_detectedReservoir << endl;

      RequestHandling::FinishRequestHandling ();

      return m_detectedReservoir;

   }

   bool Formation::getDetectedReservoir () const
   {
      return m_detectedReservoir;
   }

   // add the detected reservoir to the reservoir vector
   void Formation::addDetectedReservoir ()
   {
      if (!m_detectedReservoirRecord)
      {
         //add a record to the reservoir list
         Migrator * migrator = getMigrator ();
         database::Record * record = migrator->addDetectedReservoirRecord (this);
         migrator->getProjectHandle ()->addDetectedReservoirs (record);
         m_detectedReservoirRecord = true;
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

   void Formation::identifyAsReservoir (void) const
   {
      int depthIndex = getNodeDepth () - 1;

      for (int i = (int) m_formationNodeArray->firstILocal (); i <= (int) m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int) m_formationNodeArray->firstJLocal (); j <= (int) m_formationNodeArray->lastJLocal (); ++j)
         {
            getLocalFormationNode (i, j, depthIndex)->identifyAsReservoir ();
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

         if (gridMapEnd && gridMapStart)
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
#if 0
      cerr << GetRankString () << ": " << getName () << "(" << depthIndex << ")->computeAdjacentNodes ()" << endl;
#endif
      
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
            if (targetFormationNode && (targetFormationNode->getReservoirGas () || targetFormationNode->getReservoirOil ()) &&
                !targetFormationNode->goesOutOfBounds () &&
                targetFormationNode->getFormation () == targetReservoir->getFormation ())
            {
               assert (targetFormationNode->getK () == targetFormationNode->getFormation ()->getNodeDepth () - 1); // will fail for reservoir offsets

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

               Column *shallowestColumn = 0;
               double shallowestDepth = Interface::DefaultUndefinedScalarValue;

               for (offsetIndex = 0; offsetIndex < 4; ++offsetIndex)
               {
                  Column *targetColumn = targetReservoir->getColumn (targetFormationNode->getI () + offsets[offsetIndex][0],
                                                                     targetFormationNode->getJ () + offsets[offsetIndex][1]);

                  if (IsValid (targetColumn) && !targetColumn->isSealing () && (shallowestColumn == 0 || targetColumn->getTopDepth () < shallowestDepth))
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
      const DataAccess::Interface::Reservoir * dataAccessReservoir = * leakingReservoirList->begin();
      
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

            // If the "leaking" node has a reservoir flag (e.g. because it's a trap crest) then the HC path will be forced to be lateral.
            // But we know it should leak so we force it to do so by probing the node right above it.
            // If no reservoir offsets then this node will belong to the seal.
            if (formationNode->getReservoirGas () or formationNode->getReservoirOil ())
            {
               formationNode = getLocalFormationNode (i, j, depthIndex + 1);
            }

            FormationNode *targetFormationNode = formationNode->getTargetFormationNode ();

            // check if targetReservoir is the reservoir to migrate to for given i, j
            if (targetFormationNode && (targetFormationNode->getReservoirGas () || targetFormationNode->getReservoirOil ()) &&
                !targetFormationNode->goesOutOfBounds () &&
                targetFormationNode->getFormation () == targetReservoir->getFormation ())
            {
               assert (targetFormationNode->getK () == targetFormationNode->getFormation ()->getNodeDepth () - 1); // will fail for reservoir offsets

               // calculate the composition to migrate
               Composition composition;
               for (int componentId = FIRST_COMPONENT; componentId < NUM_COMPONENTS; ++componentId)
               {
                  if (!ComponentsUsed[componentId])
                     continue;

                  double sum = 0;
                  int offsetIndex = 0;

                  for (offsetIndex = 0; offsetIndex < 4; ++offsetIndex)
                  {
                     LocalColumn * leakingColumn = leakingReservoir->getLocalColumn (i + offsets[offsetIndex][0], j + offsets[offsetIndex][1]);
                     if (!IsValid (leakingColumn)) continue;

                     if (leakingColumn->containsComponent ((ComponentId) componentId))
                     {
                        sum += leakingColumn->getComponent ((ComponentId) componentId);
                     }
                  }

                  double weight = sum * surfaceFraction;

                  composition.add ((ComponentId) componentId, weight);
               }

               int offsetIndex;

               Column *shallowestColumn = 0;
               double shallowestDepth = Interface::DefaultUndefinedScalarValue;

               for (offsetIndex = 0; offsetIndex < 4; ++offsetIndex)
               {
                  Column *targetColumn = targetReservoir->getColumn (targetFormationNode->getI () + offsets[offsetIndex][0],
                                                                     targetFormationNode->getJ () + offsets[offsetIndex][1]);

                  if (IsValid (targetColumn) && !targetColumn->isSealing () && (shallowestColumn == 0 || targetColumn->getTopDepth () < shallowestDepth))
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

      if (sourceRockIsActive && m_isInitialised) {

         bool status = m_isInitialised;
 
         if (start->getTime () == m_startGenexTime && end->getTime () == m_endGenexTime) {
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
      if (startProperty && endProperty) {
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
      
         if (startProperty && endProperty) {
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
