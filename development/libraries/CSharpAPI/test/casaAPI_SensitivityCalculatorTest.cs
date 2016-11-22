using Shell.BasinModeling.Cauldron;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace Shell.BasinModeling.Cauldron.Test
{
   /// <summary>
   ///This is a test class for suaAPI
   ///</summary>
   [TestClass()]
   public class casaAPI_SensitivityCalculatorTest
   {
      private TestContext testContextInstance;

      /// <summary>
      ///Gets or sets the test context which provides
      ///information about and functionality for the current test run.
      ///</summary>
      public TestContext TestContext
      {
         get { return testContextInstance; }
         set { testContextInstance = value; }
      }

      // for regular run
      public static string s_projectFileName = @"..\..\..\..\..\Ottoland.project3d";
      public static string s_serialisedStateFileName = @"..\..\..\..\..\Ottoland_casa_state.txt";

      public static double[,] s_tornadoSensVals;
      public static string[]  s_observablesName;
      public static string[]  s_mostInfluentialPrmName;

      public static double[]  s_paretoSensValues;
      public static string[]  s_paretoIPNames;
      public static double[,] s_paretoCyclicSensValues;
      public static string[,] s_paretoCyclicIPName;

      public static ScenarioAnalysis s_sa = null;

      public double eps = 1.0e-6;
      public double reps = 1.0e-2;

      private static bool s_isDebug = false;

      private static void logMsg(string msg)
      {
         if (!s_isDebug) return;
         using (System.IO.StreamWriter file = new System.IO.StreamWriter(@".\logfileSensCalc.txt", true))
         {
            file.WriteLine(msg);
         }
      }

      #region Additional test attributes

      // Use ClassInitialize to run code before running the first test in the class
      [ClassInitialize()]
      public static void casaAPIInitialize(TestContext testContext)
      {
         s_tornadoSensVals = new double[,] { { 938.056207044973,   774.530289958, 13.8050391091801, -7.86912540787465 },
                                             { 2736.86328913469,  2690.100632245,  6.7636472092802, -7.88819910623228 },
                                             {  3107.5807962337,  3055.811030419,  9.6680017772506, -11.4628735269273 },
                                             {  3513.6237933331,  3527.630746258, -5.2423942781047,   3.3428064420950 },
                                             {  4195.2467722494,  4061.475197488, 10.9473164108952,  -8.9937994631101 },
                                             {     4.6439350000,     4.643935000,  0,                 0               },
                                             {     4.6439350000,     4.643935000,  0,                 0               },
                                             { 33926.2916696985, 32555.638842331,  2.27361743957069, -1.71679648219193},
                                             {  6252.5669336311,  5910.099202033,  2.95799521811843, -2.30205870213079}
                                           };

         s_observablesName = new string[] { @"Temperature(460001,6.75e+06,1293,0)"
                                          , @"Temperature(460001,6.75e+06,2129,0)"
                                          , @"Temperature(460001,6.75e+06,2362,0)"
                                          , @"Temperature(460001,6.75e+06,2751,0)"
                                          , @"Temperature(460001,6.75e+06,3200,0)"
                                          , @"Vr(460001,6.75e+06,1590,0)"
                                          , @"Vr(460001,6.75e+06,2722,0)"
                                          , @"OilExpelledCumulative(460001,6.75e+06,Lower Jurassic,0)"
                                          , @"HcGasExpelledCumulative(460001,6.75e+06,Lower Jurassic,0)" 
                                          };
         s_mostInfluentialPrmName = new string[] { @"TopCrustHeatProdRate [\mu W/m^3]"
                                                 , @"TopCrustHeatProdRate [\mu W/m^3]"
                                                 , @"TopCrustHeatProdRate [\mu W/m^3]"
                                                 , @"TopCrustHeatProdRate [\mu W/m^3]"
                                                 , @"TopCrustHeatProdRate [\mu W/m^3]"
                                                 , @"TopCrustHeatProdRate [\mu W/m^3]"
                                                 , @"TopCrustHeatProdRate [\mu W/m^3]"
                                                 , @"TopCrustHeatProdRate [\mu W/m^3]"
                                                 , @"TopCrustHeatProdRate [\mu W/m^3]"
                                                 };

         s_paretoSensValues = new double[] { 26.673177, 20.168892, 18.644706, 15.344448, 12.614685, 6.554091 };
         
         s_paretoIPNames = new string[] { @"TopCrustHeatProdRate [\mu W/m^3]"
                                        , @"EventStartTime [Ma]"
                                        , @"Lower Jurassic TOC [%]"
                                        , @"CrustThinningFactor [m/m]"
                                        , @"EventDuration [Ma]"
                                        , @"InitialCrustThickness [m]"
                                       };

         s_paretoCyclicSensValues = new double[,] { { 26.673177, 20.168892, 18.644706, 15.344448, 12.614685, 6.554091},
                                                    { 26.673177, 20.168892, 18.644706, 15.344448, 12.614685, 6.554091},
                                                    { 26.673177, 20.168892, 18.644706, 15.344448, 12.614685, 6.554091},
                                                    { 26.673177, 20.168892, 18.644706, 15.344448, 12.614685, 6.554091},
                                                    { 26.673177, 20.168892, 18.644706, 15.344448, 12.614685, 6.554091}
                                                  };

         s_paretoCyclicIPName = new string[,] { 
           { @"TopCrustHeatProdRate [\mu W/m^3]", @"EventStartTime [Ma]", @"Lower Jurassic TOC [%]", @"CrustThinningFactor [m/m]", @"EventDuration [Ma]", @"InitialCrustThickness [m]"},
           { @"TopCrustHeatProdRate [\mu W/m^3]", @"EventStartTime [Ma]", @"Lower Jurassic TOC [%]", @"CrustThinningFactor [m/m]", @"EventDuration [Ma]", @"InitialCrustThickness [m]"},
           { @"TopCrustHeatProdRate [\mu W/m^3]", @"EventStartTime [Ma]", @"Lower Jurassic TOC [%]", @"CrustThinningFactor [m/m]", @"EventDuration [Ma]", @"InitialCrustThickness [m]"},
           { @"TopCrustHeatProdRate [\mu W/m^3]", @"EventStartTime [Ma]", @"Lower Jurassic TOC [%]", @"CrustThinningFactor [m/m]", @"EventDuration [Ma]", @"InitialCrustThickness [m]"},
           { @"TopCrustHeatProdRate [\mu W/m^3]", @"EventStartTime [Ma]", @"Lower Jurassic TOC [%]", @"CrustThinningFactor [m/m]", @"EventDuration [Ma]", @"InitialCrustThickness [m]"}
         };

         s_sa = ScenarioAnalysis.loadScenario(s_serialisedStateFileName, "txt");
         logMsg("Deserialization completed");

         if (ErrorHandler.ReturnCode.NoError != s_sa.errorCode())
         {
            s_isDebug = true;
            logMsg("Serialization test failed with message:" + s_sa.errorMessage());
         }

      }

      // Use ClassCleanup to run code after all tests in a class have run
      [ClassCleanup()]
      public static void casaAPICleanup()
      {
      }

      //Use TestInitialize to run code before running each test
      [TestInitialize()]
      public void TestInitialize()
      {
         if (System.Diagnostics.Debugger.IsAttached)
         {
            s_isDebug = true;
            // for debug run
            s_projectFileName         = @"d:\cauldron\cld-dev-64\Ottoland.project3d";
            s_serialisedStateFileName = @"d:\cauldron\cld-dev-64\Ottoland_casa_state.txt";
         }
      }
      //
      //Use TestCleanup to run code after each test has run
      //[TestCleanup()]
      //public void MyTestCleanup()
      //{
      //}
      //

      #endregion

      [TestMethod]
      public void ScenarioAnalysis_SensitivityCalculatorTornadoTest() // test for Tornado sens. calc
      {
         logMsg("ScenarioAnalysis_SensitivityCalculatorTornadoTest started...");
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, s_sa.errorCode());

         SensitivityCalculator sensCalc = s_sa.sensitivityCalculator();
         StringVector doeNames = new StringVector();

         doeNames.Add( "BoxBehnken" );
         doeNames.Add( "SpaceFilling" );

         RunCaseSet cs = s_sa.doeCaseSet();
         uint csNum = cs.size();
         
         logMsg("Starting tornado calculation...");
         TornadoSensitivityData tornadoData = sensCalc.calculateTornado(cs, doeNames);
         logMsg("Tornado calculation completed.");

         if (!s_isDebug)
         {
            Assert.AreEqual<int>(tornadoData.Count, 9, "Wrong observables number in Tornado diagram data"); // number of observables
         }
         else { logMsg("Tornado observables number: " + tornadoData.Count.ToString()); }
         
         for (int i = 0; i < tornadoData.Count; ++i)
         {
            Observable obs = tornadoData[i].observable();
            StringVector obsNames = obs.name();

            int subObsNum = tornadoData[i].observableSubID();
            string obsName = obsNames[subObsNum];

            // first observable is a temperature at 1293 m
            if (!s_isDebug)
            {
               Assert.AreEqual<string>(obsName, s_observablesName[i], "Wrong first observable name for Tornado sensitivity");
            }
            else { logMsg("Observable name: " + obsName); }

            StringVector prmNames = tornadoData[i].varParametersNameList();
            //for (uint j = 0; j < prmNames.Count; ++j)
            for (uint j = 0; j < 1; ++j) // compare only one set of parameters sensitivities in this test
            {
               double minPrmAbsSens = tornadoData[i].minAbsSensitivityValue(j);
               double maxPrmAbsSens = tornadoData[i].maxAbsSensitivityValue(j);
               double minPrmRelSens = tornadoData[i].minRelSensitivityValue(j);
               double maxPrmRelSens = tornadoData[i].maxRelSensitivityValue(j);
               string name = prmNames[(int)j];
               
               VarParameter vprm = tornadoData[i].varParameter(j);
               int subPrmNum = tornadoData[i].varParameterSubID(j);

               // check results
               if (!s_isDebug)
               {
                  Assert.AreEqual(s_tornadoSensVals[i, 0], minPrmAbsSens, 1e-3, "Wrong min absolute value for the first parameter in the Tornado diagram data");
                  Assert.AreEqual(s_tornadoSensVals[i, 1], maxPrmAbsSens, 1e-3, "Wrong max absolute value for the first parameter in the Tornado diagram data");
                  Assert.AreEqual(s_tornadoSensVals[i, 2], minPrmRelSens, 1e-3, "Wrong min relative value for the first parameter in the Tornado diagram data");
                  Assert.AreEqual(s_tornadoSensVals[i, 3], maxPrmRelSens, 1e-3, "Wrong max relative value for the first parameter in the Tornado diagram data");
                  Assert.AreEqual<string>(name, s_mostInfluentialPrmName[i], "Wrong first parameter name in Tornado diagram data");
                  Assert.AreEqual<int>(subPrmNum, 0, "Wrong sub-parameter id for the first parameter in the Tornado diagram data");
               }
               else
               {
                  logMsg("minPrmAbsSens: " + minPrmAbsSens.ToString());
                  logMsg("maxPrmAbsSens: " + maxPrmAbsSens.ToString());
                  logMsg("minPrmRelSens: " + minPrmRelSens.ToString());
                  logMsg("maxPrmRelSens: " + maxPrmRelSens.ToString());
                  logMsg("name: " + name );
                  logMsg("subPrmNum: " + subPrmNum.ToString() );
               }
            }
         }
      }

      [TestMethod]
      public void ScenarioAnalysis_SensitivityCalculatorParetoTest() // test for Tornado sens. calc
      {
         RSProxySet proxySet = s_sa.rsProxySet();
         RSProxy secOrdProx = proxySet.rsProxy("SecondOrder");
         
         Assert.IsTrue( secOrdProx != null );

         SensitivityCalculator sensCalc = s_sa.sensitivityCalculator();

         ParetoSensitivityInfo paretoData = new ParetoSensitivityInfo();
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sensCalc.calculatePareto(secOrdProx, paretoData));
         for ( int i = 0; i < paretoData.m_vprmPtr.Count; ++i )
         {
            VarParameter prm = paretoData.m_vprmPtr[i];
            int prmSubId = paretoData.m_vprmSubID[i];
            String prmName = prm.name()[prmSubId];
            double prmSens = paretoData.getSensitivity(prm, prmSubId);
            
            if (!s_isDebug)
            {
               Assert.AreEqual(prmSens, s_paretoSensValues[i], reps);
               Assert.AreEqual(s_paretoIPNames[i], prmName);
            }
            else
            {
               logMsg(i.ToString() + ": prmSens " + prmSens.ToString());
               logMsg(i.ToString() + ": prmName " + prmName);
            }
      
         }
      }

      [TestMethod] // test for Pareto with variation of observable weights sens. calc
      public void ScenarioAnalysis_SensitivityCalculatorCyclicParetoTest()
      {
         RSProxy secOrdProx = s_sa.rsProxySet().rsProxy("SecondOrder");

         Assert.IsTrue(secOrdProx != null);

         SensitivityCalculator sensCalc = s_sa.sensitivityCalculator();

         //////////////////////////////////////////
         // Create pareto with weights 1.0
         ParetoSensitivityInfo paretoDataEQW = new ParetoSensitivityInfo();
         
         // set all observable SA weights to 1.0
         for ( uint o = 0; o < s_sa.obsSpace().size(); ++o )
         {
            s_sa.obsSpace().observable(o).setSAWeight(1.0);
         }

         // get pareto data for equal weighting
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sensCalc.calculatePareto(secOrdProx, paretoDataEQW));

         // list of parameters - can be different various weights value
         List<string> prmNamesEQW = new List<string>();
         // list of parameters sensitivities
         List<double>  sensDataEQW = new List<double>();

         for (int i = 0; i < paretoDataEQW.m_vprmPtr.Count; ++i)
         {
            VarParameter prm = paretoDataEQW.m_vprmPtr[i];
            int prmSubId = paretoDataEQW.m_vprmSubID[i];
            prmNamesEQW.Add( prm.name()[prmSubId] );
            sensDataEQW.Add( paretoDataEQW.getSensitivity(prm, prmSubId) );
         }

         //////////////////////////////////////////
         // Create set of 5 pareto charts with weights variation
         for ( uint p = 0; p < 5; ++p )
         {
            // keep results only till the next loop. It will simulate
            // user interaction and pareto recalculation
            List<double>  sensDataVW = new List<double>(); 
            List<string>  prmNamesVW = new List<string>();
            ParetoSensitivityInfo paretoDataVW = new ParetoSensitivityInfo();

            // variate in some way SA weights for observables
            // user updates SA weights
            for ( uint o = 0; o < s_sa.obsSpace().size(); ++o )
            {
               s_sa.obsSpace().observable(o).setSAWeight( 1.0 - 1.0/(2.0 + o + p) );
            }
            // get pareto data for the new weighting
            Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sensCalc.calculatePareto(secOrdProx, paretoDataVW));
            // collect new pareto data
            for (int i = 0; i < paretoDataVW.m_vprmPtr.Count; ++i)
            {
               VarParameter prm = paretoDataVW.m_vprmPtr[i];
               int prmSubId = paretoDataVW.m_vprmSubID[i];
               prmNamesVW.Add( prm.name()[prmSubId] );
               sensDataVW.Add(paretoDataVW.getSensitivity(prm, prmSubId));
            }

            // use new pareto (plot for example), in test case just check numbers for the
            // first parameter in the chart
            if (!s_isDebug)
            {
               for (int i = 0; i < paretoDataVW.m_vprmPtr.Count; ++i)
               {
                  Assert.AreEqual(sensDataVW[i], s_paretoCyclicSensValues[p,i], reps);
                  Assert.AreEqual(prmNamesVW[i], s_paretoCyclicIPName[p,i]);
               }
            }
            else
            {  
               string msg = "";
               for (int i = 0; i < paretoDataVW.m_vprmPtr.Count; ++i ) { msg += (i == 0 ? " " : ", ") + sensDataVW[i].ToString(); }
               logMsg( "{" + msg + "}" );
               msg = "";
               for (int i = 0; i < paretoDataVW.m_vprmPtr.Count; ++i ) { msg += (i == 0 ? " " : ", ") + prmNamesVW[i]; }
               logMsg("{" + msg + "}");
            }
         }
      }
   }
}
