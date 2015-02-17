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
      //public string m_projectFileName = @"..\..\..\csharp-test\Ottoland.project3d";
      //public string m_serialisedStateFileName = @"..\..\..\csharp-test\Ottoland_casa_state.txt";

      // for debug run
      public string m_projectFileName         = @"d:\cauldron\cld-dev-64\libraries\CSharpAPI\csharp-test\Ottoland.project3d";
      public string m_serialisedStateFileName = @"d:\cauldron\cld-dev-64\libraries\CSharpAPI\csharp-test\Ottoland_casa_state.txt";

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
      //[TestInitialize()]
      //public void MyTestInitialize()
      //{
      //}
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
         ScenarioAnalysis sa = ScenarioAnalysis.loadScenario(m_serialisedStateFileName, "txt");
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.errorCode());

         SensitivityCalculator sensCalc = sa.sensitivityCalculator();
         StringVector doeNames = new StringVector();
         doeNames.Add("Tornado");
         doeNames.Add("FullFactorial");
         RunCaseSet cs = sa.doeCaseSet();
         uint csNum = cs.size();

         TornadoSensitivityData tornadoData = sensCalc.calculateTornado(sa.doeCaseSet(), doeNames);

         Assert.AreEqual<int>(tornadoData.Count, 9, "Wrong observables number in Tornado diagram data" ); // number of observables
         
         //for (int i = 0; i < tornadoData.Count; ++i)
         for (int i = 0; i < 1; ++i) // do test only for 1 observable
         {
            Observable obs = tornadoData[i].observable();
            StringVector obsNames = obs.name();

            int subObsNum = tornadoData[i].observableSubID();
            string obsName = obsNames[subObsNum];

            // first observable is a temperature at 1293 m
            Assert.AreEqual<string>(obsName, "Temperature(460001,6.75e+06,1293,0)", 
                                    "Wrong first observable name for Tornado sensitivity");

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
               Assert.IsTrue(Math.Abs(minPrmAbsSens - 55.9668) < 1e-3, "Wrong min absolute value for the first parameter in the Tornado diagram data");
               Assert.IsTrue(Math.Abs(maxPrmAbsSens - 81.6257) < 1e-3, "Wrong max absolute value for the first parameter in the Tornado diagram data");
               Assert.IsTrue(Math.Abs(minPrmRelSens + 40.6002) < 1e-3, "Wrong min relative value for the first parameter in the Tornado diagram data");
               Assert.IsTrue(Math.Abs(maxPrmRelSens - 40.3462) < 1e-3, "Wrong max relative value for the first parameter in the Tornado diagram data");
               Assert.AreEqual<string>(name, "TopCrustHeatProdRate [\\mu W/m^3]", "Wrong first parameter name in Tornado diagram data");
               Assert.AreEqual<int>(subPrmNum, 0, "Wrong sub-parameter id for the first parameter in the Tornado diagram data");
            }
         }
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
            
            switch ( i )
            {
               case 0:
                  Assert.IsTrue( Math.Abs(prmSens - 77.7633296939624) < eps );
                  Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmName );
                  break;

               case 1:
                  Assert.IsTrue( Math.Abs(prmSens - 10.2448067481032) < eps );
                  Assert.AreEqual( @"EventStartTime [Ma]", prmName );
                  break;

               case 2:
                  Assert.IsTrue( Math.Abs(prmSens - 7.87589293181969) < eps );
                  Assert.AreEqual(@"InitialCrustThickness [m]", prmName);
                  break;

               case 3:
                  Assert.IsTrue( Math.Abs(prmSens - 3.95385427910523) < eps );
                  Assert.AreEqual( @"CrustThinningFactor [m/m]", prmName );
                  break;

               case 4:
                  Assert.IsTrue(Math.Abs(prmSens - 0.10563669785741404) < eps);
                  Assert.AreEqual( @"EventDuration [Ma]", prmName );
                  break;

               case 5:
                  Assert.IsTrue(Math.Abs(prmSens - 0.056479649152066574) < eps);
                  Assert.AreEqual( @"Lower Jurassic TOC [%]", prmName );
                  break;
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
            switch( p )
            {
               case 0:
                  Assert.IsTrue(Math.Abs(sensDataVW[0]- 77.721115907) < eps);
                  Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                  break;

               case 1:
                  Assert.IsTrue(Math.Abs(sensDataVW[0] - 77.72500210) < eps);
                  Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                  break;

               case 2:
                  Assert.IsTrue(Math.Abs(sensDataVW[0] - 77.72823309) < eps);
                  Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                  break;

               case 3:
                  Assert.IsTrue(Math.Abs(sensDataVW[0] - 77.73096169) < eps);
                  Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                  break;

               case 4:
                  Assert.IsTrue(Math.Abs(sensDataVW[0] - 77.73329662) < eps);
                  Assert.AreEqual(@"TopCrustHeatProdRate [\mu W/m^3]", prmNamesVW[0]);
                  break;
            }            
         }
      }
   }
}
