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
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Shell.BasinModeling.CSharpAPI;
using Shell.Ocean.Petrel.BasinModeling.BPACauldronLink.Data;


namespace BasinModelingLinkTest
{
    [TestClass]
    public class EosPackCApiTest
    {
        [ClassInitialize()]
        public static void EosPackCApiInitialize(TestContext testContext)
        {
            string pvtPropertiesConfigFile = "..\\..\\..\\csharp-test\\PTDiagCalculatorTest.cfg";
            CauldronAPI.SetPvtPropertiesConfigFile(pvtPropertiesConfigFile);
        }

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
        public void FlashVapourLiquidWarmerTest()
        {
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
        public void FlashLiquidTest()
        {
            ComputeStruct computeStruct = new ComputeStruct();
            // Cricondenbar point plus some delta must give pure liquid phase
            computeStruct.pressure = 1e6 * (8.0467 + 0.001);  // in Pa
            computeStruct.temperature = 470.578;  // in K

            InitializeCompositionMasses(computeStruct);

            computeStruct.isGormPrescribed = false;
            CauldronAPI.EosPackComputeWithLumping( computeStruct);
            
            double sumVapour = SumPhase(computeStruct, BpaPhase.Vapour);
            Assert.IsTrue(Math.Abs(sumVapour) < 0.00001, "Vapour mass not as expected");

            double sumLiquid = SumPhase(computeStruct, BpaPhase.Liquid);
            Assert.IsTrue(Math.Abs(sumLiquid - 7426542) < 0.00001, "Liquid mass not as expected");
        }

        [TestMethod]
        public void FlashVapourLiquidTest()
        {
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

        #region Helper Methods
        private static void InitializeCompositionMasses(ComputeStruct computeStruct)
        {
            double[] masses = new double[(int) ComponentId.NUM_COMPONENTS];

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
