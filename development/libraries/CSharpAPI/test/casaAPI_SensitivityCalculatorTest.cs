using Shell.BasinModeling.Cauldron;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Runtime.InteropServices;

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
      public string m_projectFileName = @"..\..\..\csharp-test\Ottoland.project3d";
      public string m_serialisedStateFileName = @"..\..\..\csharp-test\Ottoland_casa_state.txt";

      // for debug run
      //public string m_projectFileName         = @"c:\Temp\dev-x64_Debug\libraries\CSharpAPI\csharp-test\Ottoland.project3d";
      //public string m_serialisedStateFileName = @"c:\Temp\dev-x64_Debug\libraries\CSharpAPI\csharp-test\Ottoland_casa_state.txt";

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
         SensitivityCalculator sensCalc = sa.sensitivityCalculator();
         StringVector doeNames = new StringVector();
         doeNames.Add("Tornado");
         doeNames.Add("FullFactorial");
         RunCaseSet cs = sa.doeCaseSet();
         uint csNum = cs.size();

         TornadoSensitivityData tornadoData = sensCalc.calculateTornado(sa.doeCaseSet(), doeNames);

         Assert.AreEqual<int>(tornadoData.Count, 9, "Wrong observables number in Tornado diagram data" ); // number of observables
         
         for (int i = 0; i < tornadoData.Count; ++i)
         {
            Observable obs = tornadoData[i].observable();
            StringVector obsNames = obs.name();

            int subObsNum = tornadoData[i].observableSubID();
            string obsName = obsNames[subObsNum];

            // first observable is a temperature at 1293 m
            Assert.AreEqual<string>(obsName, "Temperature(460001,6.75e+06,1293,0)", 
                                    "Wrong first observable name for Tornado sensitivity");

            StringVector prmNames = tornadoData[i].varParametersNameList();
            for (uint j = 0; j < prmNames.Count; ++j)
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

               break; // compare only one set of parameters sensitivities in this test
            }
            break; // do test only for 1 observable
         }
      }

      [TestMethod]
      public void ScenarioAnalysis_SensitivityCalculatorParetoTest() // test for Tornado sens. calc
      {
         ScenarioAnalysis sa = ScenarioAnalysis.loadScenario(m_serialisedStateFileName, "txt");
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
   }
}
