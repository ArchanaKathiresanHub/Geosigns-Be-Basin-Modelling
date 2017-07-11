/*******************************************************************
 * 
 * Copyright (C) 2017 Shell International Exploration & Production.
 * All rights reserved.
 * 
 * Developed under license for Shell by PDS BV.
 * 
 * Confidential and proprietary source code of Shell.
 * Do not distribute without written permission from Shell.
 * 
 *******************************************************************/


using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Runtime.InteropServices;
using Shell.BasinModeling.CauldronIO;
using System.Threading;


namespace Shell.BasinModeling.CauldronIO.Test
{
    [TestClass()]
    [DeploymentItem( "VisualizationIOAPI.dll" )]
    public class PVTDataTests
    {
        private float m_eps      = 0.001F;
        private bool  m_saveLog = false;

        private void logMsg( string msg )
        {
            if ( !m_saveLog ) { return; }
            using ( System.IO.StreamWriter file = new System.IO.StreamWriter( @".\PVTDataTests.log", true ) )
            {
                file.WriteLine( msg );
            }
        }

        [TestMethod]
        public void ExtractPVTData()
        {
            // Trap data to check with. TrapID:        1                 2                 3                 4                 5                 6                 7
            float [] trapsTemp     = new float [] { 0, 115.2955F,        121.4564F,        125.6874F,        126.732F,         129.4684F,        138.1147F,        132.1923F };
            float [] trapsPres     = new float [] { 0, 35.98489F,        38.30693F,        40.15776F,        40.15736F,        41.08578F,        46.01517F,        42.55643F };
            double [] trapsTotMass = new double[] { 0, 851817842850.365, 45779104113.6582, 31168626300.7513, 17498290626.3735, 6981004234.18165, 4938315084.46984, 3418834239.95966 };

            try
            { 
                Project project = ImportFromXML.importFromXML("../../../csharp-test/pvtdata/pvtdata.xml");
                TrapperList trapperTable = project.getTrapperTable();
                TrapList    trapTable    = project.getTrapTable();

                foreach ( Trapper trap in trapperTable )
                {
                    int persistentTrapID = trap.getPersistentID();
                    int curTrapID        = trap.getID();
                    float trapAge        = trap.getAge();

                    logMsg( $"Processing trap with ID: {persistentTrapID} for age: {trapAge}" );

                    if ( trapAge > m_eps ) { continue; } // process only current time traps

                    // collect masses from trap, for this look in trap table and find trap with the same age and trap ID
                    Trap curTrap = null;                   
                    foreach ( Trap tt in trapTable )
                    {
                        if ( tt.getID() == curTrapID && tt.getAge() == trapAge )
                        {
                            curTrap = tt;
                            break;
                        }
                    }

                    // Check that we found the necessary trap
                    Assert.AreNotEqual( null, curTrap, $"Can't find trap with ID: {curTrapID} and age: {trapAge} in Trap table" );

                    float temp_C    = curTrap.getTemperature();
                    float press_MPa = curTrap.getPressure();
                    logMsg( $"   Temperature: {temp_C} [C], pressure: {press_MPa} [MPa]" );

                    double [] masses = new double[(uint)SpeciesNamesId.NUMBER_OF_SPECIES];
                    // collect composition
                    logMsg( "   Composition [kg]" );
                    double totMass = 0;

                    for ( SpeciesNamesId sp = SpeciesNamesId.FIRST_COMPONENT; sp < SpeciesNamesId.NUMBER_OF_SPECIES; ++sp )
                    {
                        uint i = (uint)sp;
                        masses[i]  = curTrap.getMass( sp );
                        totMass += masses[i];
                        logMsg( $"     species id {i}: {masses[i]}" );
                    }
                    logMsg( $"Total mass in trap: {totMass}" );

                    logMsg( $"   Check total mass {totMass} == {trapsTotMass[curTrapID]}" );
                    Assert.AreEqual( totMass, trapsTotMass[curTrapID], m_eps, $"Total mass {totMass} for trap {curTrapID} calculated doesn't match expected: {trapsTotMass[curTrapID]}" );

                    logMsg( $"   Check expected temperature {trapsTemp[curTrapID]} == {temp_C}" );
                    Assert.AreEqual( temp_C, trapsTemp[curTrapID], m_eps, $"Temperature {temp_C} for trap {curTrapID} doesn't match expected: {trapsTemp[curTrapID]}" );

                    logMsg( $"   Check expected pressure for trap {curTrapID} {trapsPres[curTrapID]} == {press_MPa}" );
                    Assert.AreEqual( press_MPa, trapsPres[curTrapID], m_eps, $"Pressure {press_MPa} for trap {curTrapID} doesn't match expected: {trapsPres[curTrapID]}" );
                       
                    // To call phase diagram calculation the following data from this test is needed:
                    //
                    // masses[SpeciesNamesId.NUMBER_OF_SPECIES] - HC composition in trap [kg]
                    // double temp_K = temp_C + 273.15 - temperature in trap [K]
                    // press_MPa - pressure in trap [MPa]
                    // int diagType = 0;  // type of diagram, 0 - mass, 1 - mole, 2 - volume
                    //
                    // The following arrays are used for returning of calculated P/T phase diagram data:
                    //
                    // 4 special points on diagram - Critical point, Bubble point, Cricondentherm point, Cricondenbar point
                    // double[] points = new double[8]; 
                    //
                    // Memory space for isolines. 
                    // For this example it will be 11 isolines 0 : 0.1 : 1, up to 400 points (2 doubles per point) per isoline
                    // szIso - in/out array which pass to diagram calculator the number of allocated points for isolines,
                    //         on output - it contains the exact number of points per isoline
                    // int[] szIso = new int[] { 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400 }; // in/out array
                    //
                    // in this exampe memory array keeps space for 11 isolines, 400 points per isoline, 2 doubles per point
                    // double[] isolines = new double[11 * 400 * 2]; 
                    // 
                    // Call diagram builder
                    // bool ret = EosPackAPI.BuildPTDiagram( diagType, temp_K, press_MPa, masses, points, szIso, isolines );
                }
            }
            catch( System.Exception ex )
            {
                m_saveLog = true;
                logMsg( "Exception in test ExtractPVTData: " + ex.Message );
                Assert.Fail();
            }
        }
    }
}
