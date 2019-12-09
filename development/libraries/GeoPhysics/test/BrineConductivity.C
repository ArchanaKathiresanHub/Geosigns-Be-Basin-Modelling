//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/BrinePhases.h"
#include "../src/BrineConductivity.h"
#include "../src/GeoPhysicsObjectFactory.h"
#include "FluidThermalConductivitySample.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace GeoPhysics;

namespace BrineConductivity_UnitTests
{
   void getThermalConductivityTable(ibs::Interpolator2d& thermalConductivitytbl)
   {
      ObjectFactory factory;
      ObjectFactory* factoryptr = &factory;
      std::unique_ptr<ProjectHandle> projectHandle( dynamic_cast<ProjectHandle*>(OpenCauldronProject("ConductivityProject.project3d", factoryptr)) );

      database::Table* fluidTbl = projectHandle->getTable("FluidtypeIoTbl");
      database::Table::iterator tblIter;
      database::Record * fluidRecord = 0;

      for (tblIter = fluidTbl->begin(); tblIter != fluidTbl->end(); ++tblIter)
      {
         fluidRecord = *tblIter;
         if (fluidRecord)
            break;
      }

      std::unique_ptr<FluidType> geoPhysicsFluidType (new FluidType(*projectHandle, fluidRecord));

      DataAccess::Interface::FluidThermalConductivitySampleList* thermalConductivitySamples;
      thermalConductivitySamples = projectHandle->getFluidThermalConductivitySampleList(projectHandle->findFluid(geoPhysicsFluidType->getThermalConductivityFluidName()));
      if ((*thermalConductivitySamples).size() != GeoPhysics::Brine::Conductivity::s_thCondArraySize)
      {
         throw formattingexception::GeneralException() << "\nBasin_Error:  Size of FltThCondIoTbl in project file is not correct\n\n";
      }

      DataAccess::Interface::FluidThermalConductivitySampleList::const_iterator thermalConductivitySampleIter;

      for (thermalConductivitySampleIter = thermalConductivitySamples->begin();
         thermalConductivitySampleIter != thermalConductivitySamples->end();
         ++thermalConductivitySampleIter)
      {
         const DataAccess::Interface::FluidThermalConductivitySample * sample = *thermalConductivitySampleIter;
         thermalConductivitytbl.addPoint(sample->getTemperature(), sample->getPressure(), sample->getThermalConductivity());
      }

      return;
   }

   /// Testing that conductivity values across (but also outside the
   /// allowed range of) the parameter space are not negative.
   TEST(BrineConductivity, testing_non_negative)
   {
      GeoPhysics::Brine::PhaseStateScalar phase(0.0);
      GeoPhysics::Brine::Conductivity conductivity;
      ibs::Interpolator2d thermalConductivitytbl;
      getThermalConductivityTable(thermalConductivitytbl);
      conductivity.setTable(thermalConductivitytbl);

      for (int i = 0; i <= 4000; ++i)
      {
         phase.set( -1000.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( -100.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 0.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 80.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 150.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 280.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 450.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 680.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 900.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 1200.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 2000.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
         phase.set( 6800.0, 0.1*double(i) );
         EXPECT_GE(conductivity.get(phase), 0.0);
      }
   }

   /// Testing exact values for the scalar interface
   TEST ( BrineConductivity, testing_conductivity_exact_values )
   {
      const double salinity = 0.2;
      GeoPhysics::Brine::Conductivity conductivity;
      GeoPhysics::Brine::PhaseStateScalar phase( salinity );
      ibs::Interpolator2d thermalConductivitytbl;
      getThermalConductivityTable(thermalConductivitytbl);
      conductivity.setTable(thermalConductivitytbl);

      phase.set( 666.0, 271.0 );
      EXPECT_NEAR( conductivity.get(phase), 0.2536, 1.0e-11 ); // vapor

      phase.set( 130.0, GeoPhysics::Brine::s_MinPressure );
      EXPECT_NEAR( conductivity.get(phase), 0.03, 1.0e-11 ); // vapor

      phase.set( 600.0, 100.0 );
      EXPECT_NEAR( conductivity.get(phase), 0.29, 1.0e-11 ); // vapor

      phase.set( GeoPhysics::Brine::s_MinTemperature, 127.0 );
      EXPECT_NEAR( conductivity.get(phase), 0.62, 1.0e-11 ); // aqueousBatzleWang

      phase.set( GeoPhysics::Brine::s_MinTemperature, GeoPhysics::Brine::s_MaxPressure );
      EXPECT_NEAR( conductivity.get(phase), 0.62, 1.0e-11 ); // aqueousBatzleWang

      phase.set( 130.0, GeoPhysics::Brine::s_MaxPressure );
      EXPECT_NEAR( conductivity.get(phase), 0.74, 1.0e-11 ); // aqueousBatzleWang

      phase.set( 50.0, 100.0 );
      EXPECT_NEAR( conductivity.get(phase), 0.69, 1.0e-11 ); // aqueousBatzleWang

      phase.set( 392.0, 60.0 );
      EXPECT_NEAR( conductivity.get(phase), 0.486, 1.0e-11 ); // transition
   }


   /// Testing continuity across T1 and T2.
   TEST(BrineConductivity, testing_conductivity_continuity)
   {
      GeoPhysics::Brine::PhaseStateScalar phase(0.0);
      GeoPhysics::Brine::Conductivity conductivity;

      ibs::Interpolator2d thermalConductivitytbl;
      getThermalConductivityTable(thermalConductivitytbl);
      conductivity.setTable(thermalConductivitytbl);

      const double epsilon = 1.0e-15;

      for (int i = 0; i < 100; ++i)
      {
         double pressure = 0.1 * double(10 * i);

         if (pressure < 0.1)
         {
            pressure = 0.1;
         }

         const double highTemp = phase.findT2(pressure);
         const double lowTemp  = phase.findT1(highTemp);

         phase.set( lowTemp * (1.0 - epsilon), pressure );
         double cond1 = conductivity.get( phase );
         phase.set( lowTemp * (1.0 + epsilon), pressure );
         double cond2 = conductivity.get( phase );
         EXPECT_NEAR( cond1, cond2, 1.0e-10 );

         phase.set( highTemp * (1.0 - epsilon), pressure );
         cond1 = conductivity.get( phase );
         phase.set( highTemp * (1.0 + epsilon), pressure );
         cond2 = conductivity.get( phase );
         EXPECT_NEAR( cond1, cond2, 1.0e-10 );
      }
   }
}
