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
//      public string m_projectFileName = @"..\..\..\csharp-test\Ottoland.project3d";
//      public string m_serialisedStateFileName = @"..\..\..\csharp-test\Ottoland_casa_state.txt";

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
      public void ScenarioAnalysis_SensitivityCalculatorTest() // test for Tornado sens. calc
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
   }
}
