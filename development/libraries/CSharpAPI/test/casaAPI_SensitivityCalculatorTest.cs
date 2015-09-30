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


      private bool m_isDebug = false;

      private void logMsg(string msg)
      {
         if (!m_isDebug) return;
         using (System.IO.StreamWriter file = new System.IO.StreamWriter(@".\logfile.txt", true))
         {
            file.WriteLine(msg);
         }
      }

      // for regular run
      public string m_projectFileName = @"..\..\..\..\..\Ottoland.project3d";
      public string m_serialisedStateFileName = @"..\..\..\..\..\Ottoland_casa_state.txt";

      public double eps = 1.0e-6;
      public double reps = 1.0e-2;

      #region Additional test attributes

      // Use ClassInitialize to run code before running the first test in the class
      [ClassInitialize()]
      public static void casaAPIInitialize(TestContext testContext)
      {
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
            m_isDebug = true;
            // for debug run
            m_projectFileName         = @"d:\cauldron\cld-dev-64\Ottoland.project3d";
            m_serialisedStateFileName = @"d:\cauldron\cld-dev-64\Ottoland_casa_state.txt";
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
         /*m_isDebug = true;
         logMsg("ScenarioAnalysis_SensitivityCalculatorTornadoTest started...");
         ScenarioAnalysis sa = ScenarioAnalysis.loadScenario(m_serialisedStateFileName, "txt");
         if (ErrorHandler.ReturnCode.NoError != sa.errorCode())
         {
            m_isDebug = true;
            logMsg("Serialization test failed with message:" + sa.errorMessage());
         }
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.errorCode());

         logMsg("Deserialization completed");

         SensitivityCalculator sensCalc = sa.sensitivityCalculator();
         StringVector doeNames = new StringVector();
         doeNames.Add("Tornado");
         doeNames.Add("FullFactorial");
         RunCaseSet cs = sa.doeCaseSet();
         uint csNum = cs.size();
         
         logMsg("Starting tornado calculation...");
         TornadoSensitivityData tornadoData = sensCalc.calculateTornado(sa.doeCaseSet(), doeNames);
         logMsg("Tornado calculation completed.");

         if (!m_isDebug)
         {
            Assert.AreEqual<int>(tornadoData.Count, 9, "Wrong observables number in Tornado diagram data"); // number of observables
         }
         else { logMsg("Tornado observables number: " + tornadoData.Count.ToString()); }
         
         //for (int i = 0; i < tornadoData.Count; ++i)
         for (int i = 0; i < 1; ++i) // do test only for 1 observable
         {
            Observable obs = tornadoData[i].observable();
            StringVector obsNames = obs.name();

            int subObsNum = tornadoData[i].observableSubID();
            string obsName = obsNames[subObsNum];

            // first observable is a temperature at 1293 m
            if (!m_isDebug)
            {
               Assert.AreEqual<string>(obsName, "Temperature(460001,6.75e+06,1293,0)",
                                    "Wrong first observable name for Tornado sensitivity");
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
               if (!m_isDebug)
               {
                  Assert.IsTrue(Math.Abs(minPrmAbsSens - 55.730) < 1e-3, "Wrong min absolute value for the first parameter in the Tornado diagram data");
                  Assert.IsTrue(Math.Abs(maxPrmAbsSens - 81.3728) < 1e-3, "Wrong max absolute value for the first parameter in the Tornado diagram data");
                  Assert.IsTrue(Math.Abs(minPrmRelSens + 42.0058) < 1e-3, "Wrong min relative value for the first parameter in the Tornado diagram data");
                  Assert.IsTrue(Math.Abs(maxPrmRelSens - 41.7406) < 1e-3, "Wrong max relative value for the first parameter in the Tornado diagram data");
                  Assert.AreEqual<string>(name, "TopCrustHeatProdRate [\\mu W/m^3]", "Wrong first parameter name in Tornado diagram data");
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
         }*/
      }

      [TestMethod]
      public void ScenarioAnalysis_SensitivityCalculatorParetoTest() // test for Tornado sens. calc
      {
         ScenarioAnalysis sa = ScenarioAnalysis.loadScenario(m_serialisedStateFileName, "txt");
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.errorCode());

         RSProxySet proxySet = sa.rsProxySet();

         RSProxy secOrdProx = proxySet.rsProxy( "SecondOrder" );
         
         Assert.IsTrue( secOrdProx != null );

         SensitivityCalculator sensCalc = sa.sensitivityCalculator();

         ParetoSensitivityInfo paretoData = new ParetoSensitivityInfo();
         Assert.AreEqual( ErrorHandler.ReturnCode.NoError, sensCalc.calculatePareto(secOrdProx, paretoData ) );
         for ( int i = 0; i < paretoData.m_vprmPtr.Count; ++i )
         {
            VarParameter prm = paretoData.m_vprmPtr[i];
            int prmSubId = paretoData.m_vprmSubID[i];
            String prmName = prm.name()[prmSubId];
            double prmSens = paretoData.getSensitivity(prm, prmSubId);
            
            if (!m_isDebug)
            {
               switch ( i )
               {
                  case 0:
                     Assert.IsTrue(Math.Abs(prmSens - 73.01612776) < eps);
                     Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmName);
                     break;
                  case 1:
                     Assert.IsTrue(Math.Abs(prmSens - 10.926499) < eps);
                     Assert.AreEqual( @"EventStartTime [Ma]", prmName );
                     break;
                  case 2:
                     Assert.IsTrue(Math.Abs(prmSens - 7.737194643) < eps);
                     Assert.AreEqual(@"InitialCrustThickness [m]", prmName);
                     break;
                  case 3:
                     Assert.IsTrue(Math.Abs(prmSens - 5.3394003) < eps);
                     Assert.AreEqual( @"CrustThinningFactor [m/m]", prmName );
                     break;
                  case 4:
                     Assert.IsTrue(Math.Abs(prmSens - 2.3014749) < eps);
                     Assert.AreEqual( @"EventDuration [Ma]", prmName );
                     break;
                  case 5:
                     Assert.IsTrue(Math.Abs(prmSens - 0.67930294) < eps);
                     Assert.AreEqual( @"Lower Jurassic TOC [%]", prmName );
                     break;
               }
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
         ScenarioAnalysis sa = ScenarioAnalysis.loadScenario(m_serialisedStateFileName, "txt");
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.errorCode());

         RSProxySet proxySet = sa.rsProxySet();

         RSProxy secOrdProx = proxySet.rsProxy("SecondOrder");

         Assert.IsTrue(secOrdProx != null);

         SensitivityCalculator sensCalc = sa.sensitivityCalculator();

         //////////////////////////////////////////
         // Create pareto with weights 1.0
         ParetoSensitivityInfo paretoDataEQW = new ParetoSensitivityInfo();
         
         // set all observable SA weights to 1.0
         for ( uint o = 0; o < sa.obsSpace().size(); ++o )
         {
            sa.obsSpace().observable(o).setSAWeight(1.0);
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
            sensDataEQW.Add(paretoDataEQW.getSensitivity(prm, prmSubId));
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
            for ( uint o = 0; o < sa.obsSpace().size(); ++o )
            {
               sa.obsSpace().observable(0).setSAWeight( 1.0 - 1.0/(2.0 + o + p) );
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
            if (!m_isDebug)
            {
               switch( p )
               {
                  case 0:
                     Assert.IsTrue(Math.Abs(sensDataVW[0] - 72.9345672) < eps);
                     Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                     break;
                  case 1:
                     Assert.IsTrue(Math.Abs(sensDataVW[0] - 72.9420498) < eps);
                     Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                     break;
                  case 2:
                     Assert.IsTrue(Math.Abs(sensDataVW[0] - 72.94827492) < eps);
                     Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                     break;
                  case 3:
                     Assert.IsTrue(Math.Abs(sensDataVW[0] - 72.95353485 ) < eps);
                     Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                     break;
                  case 4:
                     Assert.IsTrue(Math.Abs(sensDataVW[0] - 72.958037956) < eps);
                     Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                     break;
               }
            }
            else
            {
               logMsg(p.ToString() + ": sensDataVW[0] " + sensDataVW[0].ToString());
               logMsg(p.ToString() + ": prmNamesVW[0] " + prmNamesVW[0]);
            }
         }
      }
   }
}
