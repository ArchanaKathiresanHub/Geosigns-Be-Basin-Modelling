using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Runtime.InteropServices;
using Shell.BasinModeling.Cauldron;

namespace Shell.BasinModeling.Cauldron.Test
{
   /// <summary>
   ///This is a test class for suaAPI
   ///</summary>
   [TestClass()]
   public class casaAPITest
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

      public double relativeError(double v1, double v2) { return Math.Abs((v1 - v2) / (v1 + v2)); }

      private bool m_isDebug = false;

      private void logMsg(string msg)
      {
         if (!m_isDebug) return;
         using (System.IO.StreamWriter file = new System.IO.StreamWriter(@".\logfile.txt", true))
         {
            file.WriteLine(msg);
         }
      }

      private void prepareScenarioUpToMC(ScenarioAnalysis sa, Cauldron.RSProxy.RSKrigingType krig, int proxyOrder, string proxyName)
      {
         DoubleVector obsVals = new DoubleVector();
         for (uint i = 0; i < 5; ++i ) // 5 cases
         {
            for ( uint j = 0; j < 2; ++j ) // 2 observables
            {
               obsVals.Add(s_mcObservables[i, j]);
            }
         }

         // define base case for scenario
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.defineBaseCase(m_projectFileName));

         Cauldron.VarSpace vrs = sa.varSpace();
         Cauldron.ObsSpace obs = sa.obsSpace();

         // vary 2 parameters
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
            CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VaryTopCrustHeatProduction(sa, "", m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         // add 2 observables for T & VRE
         Observable ob = ObsGridPropertyXYZ.createNewInstance(460001.0, 6750001.0, 2751.0, "Temperature", 0.01);
         ob.setReferenceValue(ObsValueDoubleScalar.createNewInstance(ob, 108.6), 2.0);
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, obs.addObservable(ob));

         ob = ObsGridPropertyXYZ.createNewInstance(460001.0, 6750001.0, 2730.0, "Vr", 0.002);
         ob.setReferenceValue(ObsValueDoubleScalar.createNewInstance(ob, 1.1), 0.1);
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, obs.addObservable(ob));

         // set Tornado as DoE
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         // generate DoE
         DoEGenerator doe = sa.doeGenerator();
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, doe.generateDoE(sa.varSpace(), sa.doeCaseSet()));

         RunCaseSet rcs = sa.doeCaseSet();

         int off = 0;
         for (uint i = 0; i < rcs.size(); ++i)
         {
            for (uint j = 0; j < 2; ++j)
            {
               ObsValue obVal = obs.observable(j).newObsValueFromDoubles(obsVals, ref off);
               rcs.runCase(i).addObsValue( obVal );
            }
            rcs.runCase(i).setRunStatus(RunCase.CaseStatus.Completed);
         }
         // Calculate Response Surface proxy
         StringVector doeList = new StringVector();
         doeList.Add(DoEGenerator.DoEName(DoEGenerator.DoEAlgorithm.Tornado));

         ErrorHandler.ReturnCode retCode = sa.addRSAlgorithm(proxyName, proxyOrder, RSProxy.RSKrigingType.NoKriging, doeList);
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == retCode );
      }

      public double m_minTOC = 5.0;
      public double m_midTOC = 10.0;
      public double m_maxTOC = 15.0;

      public double m_minTCHP = 0.1;
      public double m_midTCHP = 2.5;
      public double m_maxTCHP = 4.9;

      public string m_layerName = @"Lower Jurassic";
      public string m_fileName = @"Ottoland.project3d";

      // for regular run
      public string m_projectFileName = @"..\..\..\..\..\Ottoland.project3d";
      public string m_serialisedStateFileName = @"..\..\..\..\..\Ottoland_casa_state.txt";
      public string m_bestMatchStateFileName = @"..\..\..\..\..\BestMatchCaseGeneration_state.txt";

      // for debug run
      //public string m_projectFileName = @"d:\cauldron\cld-dev-64\libraries\CSharpAPI\csharp-test\Ottoland.project3d";
      //public string m_serialisedStateFileName = @"d:\cauldron\cld-dev-64\libraries\CSharpAPI\csharp-test\Ottoland_casa_state.txt";

      public double eps = 1.0e-6;
      public double reps = 1.0e-2;

      public static double[,] s_mcObservables;

      #region Additional test attributes

      // Use ClassInitialize to run code before running the first test in the class
      [ClassInitialize()]
      public static void casaAPIInitialize(TestContext testContext)
      {
         s_mcObservables = new double[,] { { 65.1536, 0.479763 }, // case 1
                                           { 49.8126, 0.386869 }, // case 2
                                           { 80.4947, 0.572657 }, // case 3
                                           { 65.1536, 0.479763 }, // case 4
                                           { 65.1536, 0.479763 }  // case 5
                                         };
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
            m_projectFileName = @"d:\cauldron\cld-dev-64\Ottoland.project3d";
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
      public void DoE_Tornado_Test() // analog of casaAPI/test/DoeTest.C:Tornado2Prms
      {
         ScenarioAnalysis sa = new ScenarioAnalysis();
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.defineBaseCase(m_projectFileName));
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         DoEGenerator doe = sa.doeGenerator();
         VarSpace varPrms = sa.varSpace();

         ErrorHandler.ReturnCode ret = CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block);
         if ( ret != ErrorHandler.ReturnCode.NoError )
         {
            m_isDebug = true;
            logMsg( "DoE_Tornado_Test: Create TOC parameter: " + sa.errorCode().ToString() + ", msg: " + sa.errorMessage() );
         }
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, ret );

         ret = CauldronAPI.VaryTopCrustHeatProduction(sa, "", m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block);
         if (ret != ErrorHandler.ReturnCode.NoError)
         {
            m_isDebug = true;
            logMsg("DoE_Tornado_Test: Create TCHP parameter: " + sa.errorCode().ToString() + ", msg: " + sa.errorMessage());
         }
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, ret );

         Assert.AreEqual(2U, varPrms.size());

         RunCaseSet expSet = sa.doeCaseSet();
         doe.generateDoE(varPrms, expSet);
         Assert.AreEqual(5U, expSet.size() );

         for (uint i = 0; i < expSet.size(); ++i)
         {
            Assert.AreEqual(2U, expSet.runCase(i).parametersNumber());

            double v1 = expSet.runCase(i).parameter(0).asDoubleArray()[0];
            double v2 = expSet.runCase(i).parameter(1).asDoubleArray()[0];

            switch (i)
            {
               case 0: Assert.AreEqual(m_midTOC, v1, eps); Assert.AreEqual(m_midTCHP, v2, eps); break;
               case 1: Assert.AreEqual(m_minTOC, v1, eps); Assert.AreEqual(m_midTCHP, v2, eps); break;
               case 2: Assert.AreEqual(m_maxTOC, v1, eps); Assert.AreEqual(m_midTCHP, v2, eps); break;
               case 3: Assert.AreEqual(m_midTOC, v1, eps); Assert.AreEqual(m_minTCHP, v2, eps); break;
               case 4: Assert.AreEqual(m_midTOC, v1, eps); Assert.AreEqual(m_maxTCHP, v2, eps); break;
            }
         }
      }

      [TestMethod]
      public void Mutator_Test() // analog of casaAPIAPI/test/MutatorTest.C
      {
         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.defineBaseCase(m_projectFileName));

         // vary 2 parameters
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
            CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
            CauldronAPI.VaryTopCrustHeatProduction(sa, "", m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         // set up and generate DoE
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         VarSpace varPrms = sa.varSpace();
         RunCaseSet expSet = sa.doeCaseSet();
         DoEGenerator doe = sa.doeGenerator();

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, doe.generateDoE(varPrms, expSet));

         Assert.AreEqual(expSet.size(), 5U);

         string pathToCaseSet = @".\CaseSet";

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setScenarioLocation(pathToCaseSet));
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.applyMutations(sa.doeCaseSet()));

         pathToCaseSet += @"\Iteration_1";
         for (uint i = 0; i < sa.doeCaseSet().size(); ++i)
         {
            string casePath = pathToCaseSet;

            casePath += @"\Case_" + (i + 1).ToString() + @"\" + m_fileName;

            // check that all files were generated correctly
            Assert.IsTrue(File.Exists(casePath));

            // get parameters from project file
            Model caseModel = new Model();
            caseModel.loadModelFromProjectFile(casePath);

            PrmTopCrustHeatProduction prm_tchp = new PrmTopCrustHeatProduction(caseModel);
            PrmSourceRockTOC prm_toc = new PrmSourceRockTOC(caseModel, m_layerName);

            // get parameters from case set
            Assert.AreEqual(prm_toc.asDoubleArray()[0],  sa.doeCaseSet().runCase(i).parameter(0).asDoubleArray()[0], eps);
            Assert.AreEqual(prm_tchp.asDoubleArray()[0], sa.doeCaseSet().runCase(i).parameter(1).asDoubleArray()[0], eps);
         }

         // cleaning files/folders
         Directory.Delete(pathToCaseSet, true); // delete folder ./CaseSet
         Assert.IsFalse(Directory.Exists(pathToCaseSet));
      }

      [TestMethod]
      public void RunManager_Test() // analog of API/test/RunManagerTest.C
      {
         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.defineBaseCase(m_projectFileName));

         // vary 2 parameters
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                       CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                       CauldronAPI.VaryTopCrustHeatProduction(sa, "", m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         // set up and generate DoE
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         VarSpace varPrms = sa.varSpace();
         RunCaseSet expSet = sa.doeCaseSet();
         DoEGenerator doe = sa.doeGenerator();

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, doe.generateDoE(varPrms, expSet));

         Assert.AreEqual(5U, expSet.size());

         string pathToCaseSet = @".\CaseSet";

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setScenarioLocation(pathToCaseSet));
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.applyMutations(sa.doeCaseSet()));

         RunManager rm = sa.runManager();

         // set up simulation pipeline, the first is fastcauldron
         CauldronApp app = RunManager.createApplication(RunManager.ApplicationType.fastcauldron);
         app.addOption("-itcoupled");
         rm.addApplication(app);

         // then set up fastgenex6
         app = RunManager.createApplication(RunManager.ApplicationType.fastgenex6);
         rm.addApplication(app);

         // and at the end set up track1d
         app = RunManager.createApplication(RunManager.ApplicationType.track1d);
         app.addOption("-coordinates 460001,6750001");
         app.addOption("-age 0.0");
         app.addOption("-properties Temperature,Vr,OilExpelledCumulative,HcGasExpelledCumulative,TOC");
         rm.addApplication(app);

         RunCaseSet caseSet = sa.doeCaseSet();
         for (uint i = 0; i < sa.doeCaseSet().size(); ++i)
         {
            rm.scheduleCase(caseSet.runCase(i), sa.scenarioID() );
         }

         // check generated scripts
         pathToCaseSet += @"\Iteration_1";

         for (uint i = 0; i < sa.doeCaseSet().size(); ++i)
         {
            string casePath = pathToCaseSet;

            casePath += @"\Case_" + (i + 1).ToString() + @"\";

            for (int j = 0; j < 3; ++j)
            {
               string stageFile = casePath + @"Stage_" + j.ToString() + @".sh";
               Assert.IsTrue(File.Exists(stageFile));
            }
         }

         // cleaning files/folders
         Directory.Delete(pathToCaseSet, true); // delete folder ./CaseSet
         Assert.IsFalse(Directory.Exists(pathToCaseSet));
      }

      [TestMethod]
      public void RSProxy_Test() // analog of casaAPIAPI/test/RSProxyTest.C
      {
         double[,] rsCoeff = new double[,] { { 65.15362, 15.34105,  0.0 },  // obs 1
                                             { 0.479763, 0.0928937, 0.0 }   // obs 2
                                           };
         int [,]  prmsNum = new int [,] { { 0, 1, 1 },
                                          { 0, 1, 1 }
                                        };
         uint [,] pIDs = new uint [,] { {0, 1}, {0, 1} };

         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();

         // prepare scenario - define Tornado DoE for 2 var parameters and 2 observables, create 1st order proxy
         prepareScenarioUpToMC(sa, RSProxy.RSKrigingType.NoKriging, 1, "TestFirstOrderTornadoRS");

         RSProxy proxy = sa.rsProxySet().rsProxy("TestFirstOrderTornadoRS");
         CoefficientsMapList cml = proxy.getCoefficientsMapList();
         Assert.AreEqual<int>(2, cml.Count);

         // Check proxy coefficients for
         // first observable obs(1) = 65.15362 + 15.34105 * prm_1 +0 * prm_2 
         for (int i = 0; i < cml.Count; ++i) // go over all observables
         {
            CoefficientsMap cm = cml[i];
            var enumerator = cm.GetEnumerator();

            uint cpow = 0;
            uint pID = 0;
            while (enumerator.MoveNext())
            {
               UnsignedIntVector prmIDs = enumerator.Current.Key;
               double coef = enumerator.Current.Value.first;

               Assert.AreEqual(rsCoeff[i, cpow], coef, eps);
               Assert.AreEqual<int>(prmsNum[i, cpow], prmIDs.Count);

               for (int p = 0; p < prmIDs.Count; ++p)
               {
                  Assert.AreEqual<uint>(pIDs[i, pID], prmIDs[0]);
                  pID++;
               }
               cpow++;
            }
         }

         // check response surface evaluation
         // create one new case
         RunCaseImpl nrc = new RunCaseImpl();
         DoubleVector prmVals = new DoubleVector();
         // set case parameters
         prmVals.Add( -1 + (10.16 - m_minTOC)/(m_maxTOC-m_minTOC) ); // For TOC -1:1 range is used insede the parameter
         prmVals.Add(1.970);

         int off = 0;
         for (uint i = 0; i < prmVals.Count; ++i)
         {
            nrc.addParameter(sa.varSpace().continuousParameter(i).newParameterFromDoubles(prmVals, ref off));
         }

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, proxy.evaluateRSProxy(nrc));
         Assert.AreEqual(57.7285518,  nrc.obsValue(0).asDoubleArray()[0], eps);
         Assert.AreEqual(0.434802304, nrc.obsValue(1).asDoubleArray()[0], eps);
      }
      
      [TestMethod]
      public void Empty_test()
      { }


      [TestMethod]
      public void MCSolver_MC_Test() // analog of casaAPI/test/MCTest.C
      {
         /////////!!!!!!!!!!!!!!!!!!!!!!! temporary disabled
         return;
         //                                       RMSE      prm 1     prm 2     obs 1     obs 2
         double[,] mcResults = new double[,] { { 10.76400, 14.85817, 4.66576, 80.05950, 0.57002 },
                                               { 11.12912, 14.51489, 4.35741, 79.00627, 0.56364 },
                                               { 12.22329, 13.48958, 4.64508, 75.86039, 0.54459 },
                                               { 12.22866, 13.48456, 4.37472, 75.84499, 0.54450 },
                                               { 12.28734, 13.42969, 0.97545, 75.67664, 0.54348 },
                                               { 12.40616, 13.31864, 4.38069, 75.33593, 0.54141 },
                                               { 12.98320, 12.77994, 0.80740, 73.68307, 0.53141 },
                                               { 13.13891, 12.63474, 2.86927, 73.23756, 0.52871 },
                                               { 13.25585, 12.52574, 3.11374, 72.90313, 0.52668 },
                                               { 13.50066, 12.29765, 2.75138, 72.20330, 0.52245 },
                                               { 13.55269, 12.24920, 2.82957, 72.05465, 0.52155 },
                                               { 13.69266, 12.11888, 0.75850, 71.65479, 0.51912 },
                                               { 13.90380, 11.92238, 0.46047, 71.05189, 0.51547 },
                                               { 13.91151, 11.91520, 1.28100, 71.02988, 0.51534 },
                                               { 13.94573, 11.88337, 1.46124, 70.93220, 0.51475 },
                                               { 14.51674, 11.35250, 1.21514, 69.30337, 0.50489 },
                                               { 14.59328, 11.28139, 3.06722, 69.08519, 0.50356 },
                                               { 14.66668, 11.21320, 1.66815, 68.87599, 0.50230 },
                                               { 15.06688, 10.84163, 0.83854, 67.73594, 0.49539 },
                                               { 15.12019, 10.79216, 1.31659, 67.58414, 0.49448 },
                                               { 16.13987, 9.846734, 1.44140, 64.68336, 0.47691 },
                                               { 16.18297, 9.806806, 1.05154, 64.56086, 0.47617 },
                                               { 16.30192, 9.696627, 1.33898, 64.22280, 0.47412 },
                                               { 16.42600, 9.581715, 1.69219, 63.87023, 0.47199 },
                                               { 16.45101, 9.558561, 4.47305, 63.79919, 0.47156 },
                                               { 16.92167, 9.122883, 1.06111, 62.46244, 0.46346 },
                                               { 17.06962, 8.985987, 1.65237, 62.04241, 0.46092 },
                                               { 17.25446, 8.814993, 0.42496, 61.51777, 0.45774 },
                                               { 17.36503, 8.712719, 0.50168, 61.20397, 0.45584 },
                                               { 17.94960, 8.172268, 3.82228, 59.54575, 0.44580 },
                                               { 18.25346, 7.891480, 0.85045, 58.68424, 0.44058 },
                                               { 18.29432, 7.853727, 0.69514, 58.56840, 0.43988 },
                                               { 18.36212, 7.791090, 4.14833, 58.37622, 0.43872 },
                                               { 18.51196, 7.652683, 4.21490, 57.95156, 0.43615 },
                                               { 19.53558, 6.707656, 4.01324, 55.05201, 0.41859 },
                                               { 19.57631, 6.670069, 4.28907, 54.93669, 0.41789 },
                                               { 19.63389, 6.616938, 0.62083, 54.77367, 0.41690 },
                                               { 19.72593, 6.532015, 0.46200, 54.51311, 0.41533 },
                                               { 19.78643, 6.476194, 3.98812, 54.34184, 0.41429 },
                                               { 19.88666, 6.383730, 1.16266, 54.05814, 0.41257 },
                                               { 19.88839, 6.382131, 4.41504, 54.05323, 0.41254 },
                                               { 19.93354, 6.340476, 3.78276, 53.92543, 0.41177 },
                                               { 20.12962, 6.159610, 0.88304, 53.37049, 0.40841 },
                                               { 20.23122, 6.065906, 1.09060, 53.08299, 0.40667 },
                                               { 20.82055, 5.522500, 4.09759, 51.41570, 0.39657 },
                                               { 20.92946, 5.422108, 4.57082, 51.10768, 0.39471 },
                                               { 20.95163, 5.401673, 3.91781, 51.04498, 0.39433 },
                                               { 21.28221, 5.096978, 4.20714, 50.11012, 0.38867 },
                                               { 21.28576, 5.093707, 3.92793, 50.10008, 0.38860 },
                                               { 21.31653, 5.065350, 3.57787, 50.01308, 0.38808 },
                                             };

         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();

         // prepare scenario - define Tornado DoE for 2 var parameters and 2 observables, create 1st order proxy
         prepareScenarioUpToMC(sa, RSProxy.RSKrigingType.NoKriging, 1, "TestFirstOrderTornadoRS");

         RSProxy proxy = sa.rsProxySet().rsProxy("TestFirstOrderTornadoRS");

         // create corresponded MC algorithm
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                       sa.setMCAlgorithm(MonteCarloSolver.Algorithm.MonteCarlo, MonteCarloSolver.KrigingType.NoKriging,
                          MonteCarloSolver.PriorDistribution.NoPrior, MonteCarloSolver.MeasurementDistribution.Normal));

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                       sa.mcSolver().runSimulation(proxy, sa.varSpace(), sa.varSpace(), sa.obsSpace(), 50, 10, 1.0));

         Assert.AreEqual(1.0, sa.mcSolver().stdDevFactor(), eps); // VRE

         // Get MC samples
         // must be 50 samples
         Assert.AreEqual(50U, sa.mcSolver().samplingsNumber() );

         // go over all samplings and compare results
         for (uint i = 0; i < sa.mcSolver().samplingsNumber(); ++i)
         {
            RunCase rc = sa.mcSolver().samplingPoint(i);

            if (!m_isDebug)
            {
               // check RMSE value
               Assert.IsTrue(relativeError(mcResults[i, 0], sa.mcSolver().RMSE(i)) < reps);

               // check generated parameters
               Assert.IsTrue(relativeError(mcResults[i, 1], rc.parameter(0).asDoubleArray()[0]) < reps);
               Assert.IsTrue(relativeError(mcResults[i, 2], rc.parameter(1).asDoubleArray()[0]) < reps);

               // check evaluated observables
               Assert.IsTrue(relativeError(mcResults[i, 3], rc.obsValue(0).asDoubleArray()[0]) < reps);
               Assert.IsTrue(relativeError(mcResults[i, 4], rc.obsValue(1).asDoubleArray()[0]) < reps);
            }
            else
            {
               string msg = "{ " + sa.mcSolver().RMSE(i).ToString() + ", " +
                                   rc.parameter(0).asDoubleArray()[0].ToString() + ", " +
                                   rc.parameter(1).asDoubleArray()[0].ToString() + ", " +
                                   rc.obsValue(0).asDoubleArray()[0].ToString() + ", " +
                                   rc.obsValue(1).asDoubleArray()[0].ToString() + " }";
               logMsg(msg);
            }
         }
      }

      [TestMethod]
      public void MCSolver_MC_NonMatchingKriging_Test() // analog of casaAPI/test/MCTest.C
      {
         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();
         // prepare scenario - define Tornado DoE for 2 var parameters and 2 observables, create 1st order proxy
         prepareScenarioUpToMC(sa, RSProxy.RSKrigingType.NoKriging, 1, "TestFirstOrderTornadoRS");

         RSProxy proxy = sa.rsProxySet().rsProxy("TestFirstOrderTornadoRS");

         // create corresponded MC algorithm
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setMCAlgorithm(MonteCarloSolver.Algorithm.MonteCarlo,
                                                                            MonteCarloSolver.KrigingType.GlobalKriging,
                                                                            MonteCarloSolver.PriorDistribution.NoPrior,
                                                                            MonteCarloSolver.MeasurementDistribution.Normal));
         // Expect an error here because we define to use kriging in MonteCarlo and do not provide proxy with kriging
         Assert.AreEqual(ErrorHandler.ReturnCode.MonteCarloSolverError, sa.mcSolver().runSimulation(proxy, sa.varSpace(), sa.varSpace(), sa.obsSpace(), 50, 10, 1.0));
      }

      [TestMethod]
      public void MCSolver_MCMC_Test() // analog of casaAPI/test/MCTest.C
      {
         /////////!!!!!!!!!!!!!!!!!!!!!!! temporary disabled
         return;
         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();
         // prepare scenario - define Tornado DoE for 2 var parameters and 2 observables, create 1st order proxy
         prepareScenarioUpToMC(sa, RSProxy.RSKrigingType.NoKriging, 1, "TestFirstOrderTornadoRS");

         RSProxy proxy = sa.rsProxySet().rsProxy("TestFirstOrderTornadoRS");

         // create corresponded MC algorithm
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setMCAlgorithm(MonteCarloSolver.Algorithm.MCMC,
                                                                            MonteCarloSolver.KrigingType.NoKriging,
                                                                            MonteCarloSolver.PriorDistribution.NoPrior,
                                                                            MonteCarloSolver.MeasurementDistribution.Normal));

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.mcSolver().runSimulation(proxy, sa.varSpace(), sa.varSpace(), sa.obsSpace(), 50, 10, 1.0));

         sa.saveScenario("MCMC.txt", "txt");

         // Get MCMC samples
         // must be 50 samples
         Assert.IsTrue(sa.mcSolver().samplingsNumber() == 50);

         // check couple cases to be sure
         RunCase rc = sa.mcSolver().samplingPoint(7);

         // check RMSE value
         double rmse = sa.mcSolver().RMSE(7);
         Assert.IsTrue(relativeError(rmse, 10.6236) < reps);
         	
         // check generated parameters
         Assert.IsTrue(relativeError(rc.parameter(0).asDoubleArray()[0], 14.9903) < reps);
         Assert.IsTrue(relativeError(rc.parameter(1).asDoubleArray()[0], 3.03292) < reps);

         // check evaluated observables
         Assert.IsTrue(relativeError(rc.obsValue(0).asDoubleArray()[0], 80.4651) < reps);
         Assert.IsTrue(relativeError(rc.obsValue(1).asDoubleArray()[0], 0.572478) < reps);

         rc = sa.mcSolver().samplingPoint(44);

         rmse = sa.mcSolver().RMSE(44);
         Assert.IsTrue(relativeError(rmse, 10.758168) < reps);
	
         // check generated parameters
         Assert.IsTrue(relativeError(rc.parameter(0).asDoubleArray()[0], 14.8485) < reps);
         Assert.IsTrue(relativeError(rc.parameter(1).asDoubleArray()[0], 0.375491) < reps);

         // check evaluated observables
         Assert.IsTrue(relativeError(rc.obsValue(0).asDoubleArray()[0], 80.0297) < reps);
         Assert.IsTrue(relativeError(rc.obsValue(1).asDoubleArray()[0], 0.569842) < reps);
      }

      [TestMethod]
      public void ScenarioAnalysis_Serialzation_Test() // analog of casaAPI/test/SerializationTest.C
      {
         {
            ScenarioAnalysis sa = ScenarioAnalysis.loadScenario(m_serialisedStateFileName, "");

            if ( ErrorHandler.ReturnCode.NoError != sa.errorCode() )
            {
               m_isDebug = true;
               logMsg("Serialization test failed with message:" + sa.errorMessage());
            }
            Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.errorCode());
            Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.saveScenario(@".\casa_state_reloaded_1.txt", "txt") );
         }
         {
            // do round trip - load and save it
            ScenarioAnalysis sb = ScenarioAnalysis.loadScenario(@".\casa_state_reloaded_1.txt", "");
            Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sb.errorCode());

            sb.saveScenario(@".\casa_state_reloaded_2.txt", "txt");
         }

         string fileAContent = File.ReadAllText(@".\casa_state_reloaded_1.txt");
         string fileBContent = File.ReadAllText(@".\casa_state_reloaded_2.txt");

         Assert.IsTrue(fileAContent == fileBContent);
      }

      [TestMethod]
      public void ScenarioAnalysis_DeserialzationTestFromByteArray() // deserialize casa state file from Byte [] array
      {
         Byte[] buf = File.ReadAllBytes(m_serialisedStateFileName);
         ScenarioAnalysis sa = ScenarioAnalysis.loadScenario( buf, (uint)(buf.Length), "");

         if ( ErrorHandler.ReturnCode.NoError != sa.errorCode() )
         {
            m_isDebug = true;
            logMsg("Deserialization test failed with message:" + sa.errorMessage());
         }
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.errorCode());
      }

      [TestMethod]
      public void ScenarioAnalysis_GenerateCalibratedCase() // load state file after MCMC and generate calibrated case
      {
         /////////!!!!!!!!!!!!!!!!!!!!!!! temporary disabled
         return;
         ScenarioAnalysis sa = ScenarioAnalysis.loadScenario(m_bestMatchStateFileName, "txt");

         if (ErrorHandler.ReturnCode.NoError != sa.errorCode())
         {
            m_isDebug = true;
            logMsg("Deserialization test failed with message:" + sa.errorMessage());
         }
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.errorCode());
         
         // clean any previous failed run
         string pathToCaseSet = @".\CaseSet";

         if ( Directory.Exists(pathToCaseSet) )
         {
            Directory.Delete(pathToCaseSet, true); // delete folder ./CaseSet
         }

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setScenarioLocation( @".\CaseSet" ) );
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.saveCalibratedCase("NVGBestMatchCase.project3d", 1));

         Assert.IsTrue(File.Exists(@".\CaseSet\Calibrated_NVGBestMatchCase\NVGBestMatchCase.project3d"));
         Assert.IsTrue(File.Exists(@".\CaseSet\Calibrated_NVGBestMatchCase\Inputs.HDF"));
         Assert.IsTrue(File.Exists(@".\CaseSet\Calibrated_NVGBestMatchCase\MAP-72981789-4.FLT"));

         // cleaning files/folders
         Directory.Delete(pathToCaseSet, true); // delete folder ./CaseSet       
      }
   }
}
