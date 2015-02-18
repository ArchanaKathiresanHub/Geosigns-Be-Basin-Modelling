/*******************************************************************
 * 
 * Copyright (C) 2013 Shell International Exploration & Production.
 * All rights reserved.
 * 
 * Developed under licence for Shell by PDS BV.
 * 
 * Confidential and proprietary source code of Shell.
 * Do not distribute without written permission from Shell.
 * 
 *******************************************************************/

using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Shell.BasinModeling.Cauldron;
using Shell.Ocean.Petrel.BasinModeling.BPACauldronLink.Data;


namespace BasinModelingLinkTest
{
   [TestClass]
   public class EosPackCApiTest
   {
      private static string m_cfgFile = @"..\..\..\..\..\PTDiagCalculatorTest.cfg";
       
      #region Additional test attributes
      // 
      //You can use the following additional attributes as you write your tests:
      //
      //Use ClassInitialize to run code before running the first test in the class
      [ClassInitialize()]
      public static void EosPackCApiInitialize(TestContext testContext)
      {
         TextWriter ofs = new StreamWriter(m_cfgFile);
         ofs.WriteLine("///component based and general data for PVT");
         ofs.WriteLine("///");
         ofs.WriteLine("///This file contains tables describing 6 COMPONENT-based properties and additionally GENERAL properties ");
         ofs.WriteLine("///for PVT (see table headers).");
         ofs.WriteLine("///");
         ofs.WriteLine("///All tables have comma-separated columns. ");
         ofs.WriteLine("///");
         ofs.WriteLine("///COMPONENT TABLES:");
         ofs.WriteLine("///");
         ofs.WriteLine("///Every table with component data contains the component names in the first column followed by the property columns.");
         ofs.WriteLine("///");
         ofs.WriteLine("///The order of the 6 component property columns must be maintained. ");
         ofs.WriteLine("///However, it is possible to split the component tables, i.e. one can use one table containing all component property columns,");
         ofs.WriteLine("///or use up to 6 separate tables.");
         ofs.WriteLine("///");
         ofs.WriteLine("///GENERAL TABLES:");
         ofs.WriteLine("///");
         ofs.WriteLine("///General data must be contained in one table.");
         ofs.WriteLine("///");
         ofs.WriteLine("///All (numerical) data entries to be prescribed (either component or general data) ");
         ofs.WriteLine("///are generally interpreted as a piecewise polynomial depending on the ");
         ofs.WriteLine("///GORM (mass based gas/oil ratio), i.e. every data entry may be either: ");
         ofs.WriteLine("///a constant value,");
         ofs.WriteLine("///a polynomial (e.g. 2.0 + 4.0x^3 + 3.4x),");
         ofs.WriteLine("///a piecewise polynomial with an arbitrary number of semicolon separated pieces, each consisting ");
         ofs.WriteLine("///of a range [x_a:x_b] followed by a constant or polynomial.");
         ofs.WriteLine("///Example:");
         ofs.WriteLine("///[*:1.2]   1.7e-05 ; [1.2:1.8]   -2.7E-05x + 5.1E-05 ;   [1.8:*]   0.0");
         ofs.WriteLine("///(in the above example, the '*'s indicate positive/negative infinity)");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-Component-Properties]");
         ofs.WriteLine("ComponentName,MolecularWeight,AcentricFactor,VCrit");
         ofs.WriteLine("N2,         28.01352	         ,   0.04000   ,   8.980e-02");
         ofs.WriteLine("COx,        44.00980	         ,   0.22500   ,   9.400e-02");
         ofs.WriteLine("C1,         16.04288	         ,   0.00800   ,   9.900e-02");
         ofs.WriteLine("C2,         30.06982	         ,   0.09800   ,   1.480e-01");
         ofs.WriteLine("C3,         44.09676	         ,   0.15200   ,   2.030e-01");
         ofs.WriteLine("C4,         58.12370	         ,   0.19300   ,   2.550e-01");
         ofs.WriteLine("C5,         72.15064	         ,   0.25100   ,   3.040e-01");
         ofs.WriteLine("H2S,        34.08000	         ,   0.10000   ,   9.850e-02");
         ofs.WriteLine("C6-14Sat,   [0.0:2.5]   103.09 -   0.57x  ;   [2.5:*] 101.665,   0.37272   ,   4.7060e-01");
         ofs.WriteLine("C6-14Aro,   [0.0:2.5]   158.47 - 2.0552x  ;   [2.5:*] 153.332,   0.54832   ,   6.7260e-01");
         ofs.WriteLine("C15+Sat,    [0.0:2.5]   281.85 - 17.194x  ;   [2.5:*] 238.865,   0.86828   ,   1.19620e+00");
         ofs.WriteLine("C15+Aro,    [0.0:2.5]   474.52 - 11.129x  ;   [2.5:*] 446.6975,   1.14380   ,   2.12800e+00");
         ofs.WriteLine("resins,     [0.0:2.5]   618.35 - 7.7908x  ;   [2.5:*] 598.873,   1.16772   ,   2.89200e+00");
         ofs.WriteLine("asphaltenes,[0.0:2.5]   795.12 +  2.785x  ;   [2.5:*] 802.0825,   0.89142   ,   3.84400e+00");
         ofs.WriteLine("EndOfTable");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-Component-Properties]");
         ofs.WriteLine("ComponentName,VolumeShift");
         ofs.WriteLine("N2,                           -4.2300000E-03");
         ofs.WriteLine("COx,                          -1.6412000E-03");
         ofs.WriteLine("C1,                           -5.2000000E-03");
         ofs.WriteLine("C2,                           -5.7900010E-03");
         ofs.WriteLine("C3,                           -6.3499980E-03");
         ofs.WriteLine("C4,                           -6.4900000E-03");
         ofs.WriteLine("C5,                           -5.1200000E-03");
         ofs.WriteLine("H2S,                          -3.840000E-03");
         ofs.WriteLine("C6-14Sat,    [0.0:1.242]   1.737999554e-02 ; [1.242:1.8694]   -2.770163E-02x + 5.178542E-02 ;   [1.8694:*]   0.0   ");
         ofs.WriteLine("C6-14Aro,    [0.0:1.242]   2.214000542e-02 ; [1.242:1.8694]   -3.528849E-02x + 6.596831E-02 ;   [1.8694:*]   0.0  ");
         ofs.WriteLine("C15+Sat,     [0.0:1.242]   2.573999668e-02 ; [1.242:1.8694]   -4.102646E-02x + 7.669486E-02 ;   [1.8694:*]   0.0  ");
         ofs.WriteLine("C15+Aro,                                     [0.0:2.5]        -3.687332E-02x + 1.244384E-03 ;   [2.5:*] -9.09389E-02");
         ofs.WriteLine("resins,                                      [0.0:2.5]         -4.229062E-02x - 3.814919E-02;   [2.5:*] -1.43876E-01");
         ofs.WriteLine("asphaltenes,                                 [0.0:2.5]        -5.542006E-02x - 9.001542E-02 ;   [2.5:*] -2.28566E-01");
         ofs.WriteLine("EndOfTable");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-Component-Properties]");
         ofs.WriteLine("ComponentName,CritPressure");
         ofs.WriteLine("N2,            3394387");
         ofs.WriteLine("COx,           7376462");
         ofs.WriteLine("C1,            4600155");
         ofs.WriteLine("C2,            4883865");
         ofs.WriteLine("C3,            4245519");
         ofs.WriteLine("C4,            3799688");
         ofs.WriteLine("C5,            3374122");
         ofs.WriteLine("H2S,           8936864");
         ofs.WriteLine("C6-14Sat,      [0.0:2.0]   2.81e+06 + 8.71e+04x  +  8.24e+05x^2  -  1.15e+06x^3 +  3.68e+05x^4; [2.0:*]  2.97e+06");
         ofs.WriteLine("C6-14Aro,      [0.0:2.0]   2.07e+06 + 2.51e+05x  +  5.79e+05x^2  -  1.07e+06x^3 +  3.85e+05x^4; [2.0:*]  2.49e+06");
         ofs.WriteLine("C15+Sat,       [0.0:2.0]   1.64e+06 - 2.66e+05x  +  1.27e+06x^2  -  1.48e+06x^3 +  4.83e+05x^4; [2.0:*]  2.08e+06");
         ofs.WriteLine("C15+Aro,       [0.0:2.0]   1.34e+06 - 1.85e+05x  +  1.26e+06x^2  -  1.57e+06x^3 +  5.22e+05x^4; [2.0:*]  1.80e+06");
         ofs.WriteLine("resins,        [0.0:2.0]   1.29e+06 - 2.98e+05x  +  1.43e+06x^2  -  1.67e+06x^3 +  5.43e+05x^4; [2.0:*]  1.74e+06");
         ofs.WriteLine("asphaltenes,   [0.0:2.0]   1.25e+06 - 3.43e+05x  +  1.45e+06x^2  -  1.67e+06x^3 +  5.42e+05x^4; [2.0:*]  1.68e+06");
         ofs.WriteLine("EndOfTable");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-Component-Properties]");
         ofs.WriteLine("ComponentName,CritTemperature");
         ofs.WriteLine("N2,             126.200");
         ofs.WriteLine("COx,            304.200");
         ofs.WriteLine("C1,             190.600");
         ofs.WriteLine("C2,             305.400");
         ofs.WriteLine("C3,             369.800");
         ofs.WriteLine("C4,             425.200");
         ofs.WriteLine("C5,             469.600");
         ofs.WriteLine("H2S,            373.200");
         ofs.WriteLine("C6-14Sat,       [0.0:2.0]    582.13 +  15.00x -	160.73x^2 + 189.96x^3 -  61.28x^4 ; [2.0:*]	 508.426");
         ofs.WriteLine("C6-14Aro,       [0.0:2.0]    686.69 -  41.72x -	101.39x^2 + 179.72x^3 -  65.72x^4 ; [2.0:*]	 583.956");
         ofs.WriteLine("C15+Sat,        [0.0:2.0]    857.09 - 121.02x -	 56.60x^2 + 197.60x^3 -  79.33x^4 ; [2.0:*]	 700.218");
         ofs.WriteLine("C15+Aro,        [0.0:2.0]   1037.50 -  81.42x -	208.72x^2 + 351.63x^3 - 123.74x^4 ; [2.0:*]	 872.986");
         ofs.WriteLine("resins,         [0.0:2.0]   1160.30 -  41.11x -	319.82x^2 + 444.11x^3 - 148.68x^4 ; [2.0:*]	 972.8");
         ofs.WriteLine("asphaltenes,    [0.0:2.0]   1351.30 - 206.25x -	105.01x^2 + 341.23x^3 - 136.23x^4 ; [2.0:*]	 1068.92");
         ofs.WriteLine("EndOfTable");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-General-Properties]");
         ofs.WriteLine("EOS,OmegaA,OmegaB,LBC0,LBC1,LBC2,LBC3,LBC4");
         ofs.WriteLine("PR,   4.57240000E-01,   7.77960000E-02,   1.02300000E-01,   2.33640000E-02,   5.85330000E-02,   -4.07580000E-02,   9.33240000E-03");
         ofs.WriteLine("EndOfTable");
         ofs.Close();
      }
        
      //
      //Use ClassCleanup to run code after all tests in a class have run
      [ClassCleanup()]
      public static void EosPackCApiCleanup()
      {
         File.Delete(m_cfgFile);
      }
      #endregion

      [TestMethod]
      public void FlashVapourTest()
      {
          ComputeStruct computeStruct = new ComputeStruct();
          // Cricondentherm point plus some delta must give pure vapour phase
          computeStruct.pressure = 1e6 * 3.12139 ;  // in Pa
          computeStruct.temperature = 682.255 + 1;  // in K

          InitializeCompositionMasses(computeStruct);

          computeStruct.isGormPrescribed = false;
          CauldronAPI.EosPackComputeWithLumping(computeStruct);

          double sumVapour = SumPhase(computeStruct, BpaPhase.Vapour);
          double sumLiquid = SumPhase(computeStruct, BpaPhase.Liquid);

          Assert.IsTrue(Math.Abs(sumVapour - 7426542) < 0.00001, "Vapour mass not as expected");
          Assert.IsTrue(Math.Abs(sumLiquid) < 0.00001, "Liquid mass not as expected");
      }
      
      [TestMethod]
      public void FlashLiquidTest()
      {
          CauldronAPI.SetPvtPropertiesConfigFile(m_cfgFile);

          ComputeStruct computeStruct = new ComputeStruct();
          // Cricondenbar point plus some delta must give pure liquid phase
          computeStruct.pressure = 1e6 * (8.0467 + 0.001);  // in Pa
          computeStruct.temperature = 470.578;  // in K

          InitializeCompositionMasses(computeStruct);

          computeStruct.isGormPrescribed = false;
          CauldronAPI.EosPackComputeWithLumping(computeStruct);

          double sumVapour = SumPhase(computeStruct, BpaPhase.Vapour);
          Assert.IsTrue(Math.Abs(sumVapour) < 0.00001, "Vapour mass not as expected");

          double sumLiquid = SumPhase(computeStruct, BpaPhase.Liquid);
          Assert.IsTrue(Math.Abs(sumLiquid - 7426542) < 0.00001, "Liquid mass not as expected");
      }

      [TestMethod]
      public void FlashVapourLiquidWarmerTest()
      {
         CauldronAPI.SetPvtPropertiesConfigFile(m_cfgFile);

         ComputeStruct computeStruct = new ComputeStruct();
         computeStruct.pressure = 1e6 * 1;  // in Pa
         computeStruct.temperature = 273.15 + 290;  // in K

         InitializeCompositionMasses(computeStruct);

         computeStruct.isGormPrescribed = false;
         CauldronAPI.EosPackComputeWithLumping(computeStruct);
         
         double sumVapour = SumPhase(computeStruct, BpaPhase.Vapour);
         double sumLiquid = SumPhase(computeStruct, BpaPhase.Liquid);
         Assert.IsTrue(Math.Abs(sumVapour - 7400659.1 ) < 0.1, "Vapour mass not as expected");
         Assert.IsTrue(Math.Abs(sumLiquid - 25882.915) < 0.001, "Liquid mass not as expected");
      }

      [TestMethod]
      public void FlashVapourLiquidTest()
      {
          CauldronAPI.SetPvtPropertiesConfigFile(m_cfgFile);

          ComputeStruct computeStruct = new ComputeStruct();
          computeStruct.pressure = 1e6 * 1;  // in Pa
          computeStruct.temperature = 273.15 + 100;  // in K

          InitializeCompositionMasses(computeStruct);

          computeStruct.isGormPrescribed = false;
          CauldronAPI.EosPackComputeWithLumping(computeStruct);
          
          double sumVapour = SumPhase(computeStruct, BpaPhase.Vapour);
          Assert.IsTrue(Math.Abs(sumVapour - 851393.6564469377) < 0.00001, "Vapour mass not as expected");

          double sumLiquid = SumPhase(computeStruct, BpaPhase.Liquid);
          Assert.IsTrue(Math.Abs(sumLiquid - 6575148.3435530625) < 0.00001, "Liquid mass not as expected");
      }

      [TestMethod]
      public void FlashVapourLiquidTestUsingArrays()
      {
         CauldronAPI.SetPvtPropertiesConfigFile(m_cfgFile);

         double[] masses         = new double[(int)ComponentId.NUM_COMPONENTS];
         double[] phaseMasses    = new double[(int)ComponentId.NUM_COMPONENTS * (int)CauldronAPI.N_PHASES];
         double[] phaseDensity   = new double[(int)CauldronAPI.N_PHASES];
         double[] phaseViscosity = new double[(int)CauldronAPI.N_PHASES];

         InitializeCompositionMasses(masses);

         CauldronAPI.EosPackComputeWithLumpingArr(373.15, 1e6, masses, false, 0.0, phaseMasses, phaseDensity, phaseViscosity);

         double[] totPhaseMass = new double[(int)CauldronAPI.N_PHASES];

         for (int i = 0; i < (int)CauldronAPI.N_PHASES; ++i)
         {
            totPhaseMass[i] = 0.0;
            for (int j = 0; j < (int)ComponentId.NUM_COMPONENTS; ++j)
            {
               totPhaseMass[i] += phaseMasses[i * ((int)ComponentId.NUM_COMPONENTS) + j];
            }
         }
         double sumVapour = totPhaseMass[(int)BpaPhase.Vapour];
         Assert.IsTrue(Math.Abs(sumVapour - 851393.6564469377) < 0.00001, "Vapour mass not as expected");

         double sumLiquid = totPhaseMass[(int)BpaPhase.Liquid];
         Assert.IsTrue(Math.Abs(sumLiquid - 6575148.3435530625) < 0.00001, "Liquid mass not as expected");
      }

      [TestMethod]
      public void GormCalculationTest()
      {
         CauldronAPI.SetPvtPropertiesConfigFile(m_cfgFile);

         double[] compos = new double[(int)ComponentId.NUM_COMPONENTS];
         for (int i = 0; i < (int)ComponentId.NUM_COMPONENTS; ++i)
         {
            compos[i] = 1;
         }
         double gorm = CauldronAPI.Gorm(compos);
         Assert.IsTrue(Math.Abs(gorm - 6.0/15.0) < 0.000001, "Gorm not as expected");
      }

      [TestMethod]
      public void GetMolWeight()
      {
         CauldronAPI.SetPvtPropertiesConfigFile(m_cfgFile);

         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.Asphaltenes, 0.0 ) -   795.12 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.Resins     , 0.0 ) -   618.35 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15Aro     , 0.0 ) -   474.52 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15Sat     , 0.0 ) -   281.85 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14Aro   , 0.0 ) -   158.47 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14Sat   , 0.0 ) -   103.09 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C5         , 0.0 ) - 72.15064 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C4         , 0.0 ) -  58.1237 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C3         , 0.0 ) - 44.09676 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C2         , 0.0 ) - 30.06982 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C1         , 0.0 ) - 16.04288 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.Cox        , 0.0 ) -  44.0098 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.N2         , 0.0 ) - 28.01352 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.H2S        , 0.0 ) -    34.08 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.LSC        , 0.0 ) -   281.85 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15_AT     , 0.0 ) -   281.85 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14BT    , 0.0 ) -   158.47 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14DBT   , 0.0 ) -   158.47 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14BP    , 0.0 ) -   158.47 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15_AROS   , 0.0 ) -   281.85 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15_SATS   , 0.0 ) -   281.85 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14SATS  , 0.0 ) -   158.47 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14AROS  , 0.0 ) -   158.47 ) < 1.0e-5 );

         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.Asphaltenes, 2.3 ) -  801.5255 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.Resins     , 2.3 ) - 600.43116 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15Aro     , 2.3 ) -  448.9233 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15Sat     , 2.3 ) -  242.3038 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14Aro   , 2.3 ) - 153.74304 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14Sat   , 2.3 ) -   101.779 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C5         , 2.3 ) -  72.15064 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C4         , 2.3 ) -   58.1237 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C3         , 2.3 ) -  44.09676 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C2         , 2.3 ) -  30.06982 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C1         , 2.3 ) -  16.04288 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.Cox        , 2.3 ) -   44.0098 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.N2         , 2.3 ) -  28.01352 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.H2S        , 2.3 ) -     34.08 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.LSC        , 2.3 ) -  242.3038 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15_AT     , 2.3 ) -  242.3038 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14BT    , 2.3 ) - 153.74304 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14DBT   , 2.3 ) - 153.74304 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14BP    , 2.3 ) - 153.74304 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15_AROS   , 2.3 ) -  242.3038 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C15_SATS   , 2.3 ) -  242.3038 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14SATS  , 2.3 ) - 153.74304 ) < 1.0e-5 );
         Assert.IsTrue( Math.Abs( CauldronAPI.GetMolWeight( (int)BpaComponent.C6_14AROS  , 2.3 ) - 153.74304 ) < 1.0e-5 );
      }

      #region Helper Methods
      private static void InitializeCompositionMasses(double[] masses)
        {
           masses[(int)BpaComponent.Asphaltenes] = 1158F;
           masses[(int)BpaComponent.Resins] = 21116F;
           masses[(int)BpaComponent.C15Aro] = 2021F;
           masses[(int)BpaComponent.C15Sat] = 19731F;
           masses[(int)BpaComponent.C6_14Aro] = 339F;
           masses[(int)BpaComponent.C6_14Sat] = 6328815F;
           masses[(int)BpaComponent.C5] = 103238F;
           masses[(int)BpaComponent.C4] = 187596F;
           masses[(int)BpaComponent.C3] = 215881F;
           masses[(int)BpaComponent.C2] = 232280F;
           masses[(int)BpaComponent.C1] = 308969F;
           masses[(int)BpaComponent.Cox] = 0F;
           masses[(int)BpaComponent.N2] = 5398F;
           masses[(int)BpaComponent.H2S] = 0F;
           masses[(int)BpaComponent.LSC] = 0F;
           masses[(int)BpaComponent.C15_AT] = 0F;
           masses[(int)BpaComponent.C6_14BT] = 0F;
           masses[(int)BpaComponent.C6_14DBT] = 0F;
           masses[(int)BpaComponent.C6_14BP] = 0F;
           masses[(int)BpaComponent.C15_AROS] = 0F;
           masses[(int)BpaComponent.C15_SATS] = 0F;
           masses[(int)BpaComponent.C6_14SATS] = 0F;
           masses[(int)BpaComponent.C6_14AROS] = 0F;
        }
      
      private static void InitializeCompositionMasses(ComputeStruct computeStruct)
      {
          double[] masses = new double[(int) ComponentId.NUM_COMPONENTS];
          InitializeCompositionMasses(masses);

          Assert.AreEqual(masses.Length, (int) ComponentId.NUM_COMPONENTS);

          for (int i = 0; i < masses.Length; ++i)
              CauldronAPI.doubleArray_setitem(computeStruct.compMasses, i, masses[i]);
          
      }

      private static double GetMass(ComputeStruct computeStruct, BpaPhase phase, BpaComponent componentId)
      {
          return CauldronAPI.doubleArray_getitem(computeStruct.phaseCompMasses, (int) ComponentId.NUM_COMPONENTS * (int)phase + (int)componentId);
      }

      private static double SumPhase(ComputeStruct computeStruct, BpaPhase phase)
      {
          double result = 0;
          result += GetMass(computeStruct, phase, BpaComponent.Asphaltenes);
          result += GetMass(computeStruct, phase, BpaComponent.Resins);
          result += GetMass(computeStruct, phase, BpaComponent.C15Aro);
          result += GetMass(computeStruct, phase, BpaComponent.C15Sat);
          result += GetMass(computeStruct, phase, BpaComponent.C6_14Aro);
          result += GetMass(computeStruct, phase, BpaComponent.C6_14Sat);
          result += GetMass(computeStruct, phase, BpaComponent.C5);
          result += GetMass(computeStruct, phase, BpaComponent.C4);
          result += GetMass(computeStruct, phase, BpaComponent.C3);
          result += GetMass(computeStruct, phase, BpaComponent.C2);
          result += GetMass(computeStruct, phase, BpaComponent.C1);
          result += GetMass(computeStruct, phase, BpaComponent.Cox);
          result += GetMass(computeStruct, phase, BpaComponent.N2);
          result += GetMass(computeStruct, phase, BpaComponent.H2S);
          result += GetMass(computeStruct, phase, BpaComponent.LSC);
          result += GetMass(computeStruct, phase, BpaComponent.C15_AT);
          result += GetMass(computeStruct, phase, BpaComponent.C6_14BT);
          result += GetMass(computeStruct, phase, BpaComponent.C6_14DBT);
          result += GetMass(computeStruct, phase, BpaComponent.C6_14BP);
          result += GetMass(computeStruct, phase, BpaComponent.C15_AROS);
          result += GetMass(computeStruct, phase, BpaComponent.C15_SATS);
          result += GetMass(computeStruct, phase, BpaComponent.C6_14SATS);
          result += GetMass(computeStruct, phase, BpaComponent.C6_14AROS);

          return result;
      }
      #endregion
    }
}
