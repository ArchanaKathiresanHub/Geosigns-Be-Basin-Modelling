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

      public double relativeError(double v1, double v2)
      {
         return Math.Abs((v1 - v2)/(v1 + v2));
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

      private void prepareScenarioUpToMC(Cauldron.ScenarioAnalysis sa, Cauldron.RSProxy.RSKrigingType krig,
         int proxyOrder, string proxyName)
      {
         DoubleVector obsVals = new DoubleVector();
         // case 1
         obsVals.Add(65.1536);
         obsVals.Add(0.479763);
         // case 2
         obsVals.Add(49.8126);
         obsVals.Add(0.386869);
         // case 3
         obsVals.Add(80.4947);
         obsVals.Add(0.572657);
         // case 4
         obsVals.Add(65.1536);
         obsVals.Add(0.479763);
         // case 5
         obsVals.Add(65.1536);
         obsVals.Add(0.479763);

         // define base case for scenario
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.defineBaseCase(m_projectFileName));

         VarSpace vrs = sa.varSpace();
         ObsSpace obs = sa.obsSpace();

         // vary 2 parameters
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VarySourceRockTOC(sa, m_layerName, m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VaryTopCrustHeatProduction(sa, m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         // add 2 observables for T & VRE
         Observable ob = ObsGridPropertyXYZ.createNewInstance(460001.0, 6750001.0, 2751.0, "Temperature", 0.01);
         ob.setReferenceValue(ObsValueDoubleScalar.createNewInstance(ob, 108.6), 2.0);
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == obs.addObservable(ob));

         ob = ObsGridPropertyXYZ.createNewInstance(460001.0, 6750001.0, 2730.0, "Vr", 0.002);
         ob.setReferenceValue(ObsValueDoubleScalar.createNewInstance(ob, 1.1), 0.1);
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == obs.addObservable(ob));

         // set Tornado as DoE
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         // generate DoE
         DoEGenerator doe = sa.doeGenerator();
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == doe.generateDoE(sa.varSpace(), sa.doeCaseSet()));

         RunCaseSet rcs = sa.doeCaseSet();

         int off = 0;
         for (uint i = 0; i < rcs.size(); ++i)
         {
            RunCase rc = rcs.runCase(i);

            for (uint j = 0; j < 2; ++j)
            {
               ObsValue obVal = obs.observable(j).newObsValueFromDoubles(obsVals, ref off);
               rc.addObsValue(obVal);
            }
            rc.setRunStatus(RunCase.CaseStatus.Completed);
         }

         // Calculate Response Surface proxy
         StringVector doeList = new StringVector();
         string doeName = DoEGenerator.DoEName(DoEGenerator.DoEAlgorithm.Tornado);
         doeList.Add(doeName);

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

      // for debug run
      //public string m_projectFileName = @"d:\cauldron\cld-dev-64\libraries\CSharpAPI\csharp-test\Ottoland.project3d";
      //public string m_serialisedStateFileName = @"d:\cauldron\cld-dev-64\libraries\CSharpAPI\csharp-test\Ottoland_casa_state.txt";

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

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         DoEGenerator doe = sa.doeGenerator();
         VarSpace varPrms = sa.varSpace();

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VarySourceRockTOC(sa, m_layerName, m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VaryTopCrustHeatProduction(sa, m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         Assert.IsTrue(2 == varPrms.size());

         RunCaseSet expSet = sa.doeCaseSet();

         doe.generateDoE(varPrms, expSet);
         Assert.IsTrue(5 == expSet.size());

         for (uint i = 0; i < 5; ++i)
         {
            Assert.IsTrue(2 == expSet.runCase(i).parametersNumber());

            double val1 = expSet.runCase(i).parameter(0).asDoubleArray()[0];

            double val2 = expSet.runCase(i).parameter(1).asDoubleArray()[0];

            switch (i)
            {
               case 0:
                  Assert.IsTrue(Math.Abs(val1 - m_midTOC) < eps);
                  Assert.IsTrue(Math.Abs(val2 - m_midTCHP) < eps);
                  break;
               case 1:
                  Assert.IsTrue(Math.Abs(val1 - m_minTOC) < eps);
                  Assert.IsTrue(Math.Abs(val2 - m_midTCHP) < eps);
                  break;
               case 2:
                  Assert.IsTrue(Math.Abs(val1 - m_maxTOC) < eps);
                  Assert.IsTrue(Math.Abs(val2 - m_midTCHP) < eps);
                  break;
               case 3:
                  Assert.IsTrue(Math.Abs(val1 - m_midTOC) < eps);
                  Assert.IsTrue(Math.Abs(val2 - m_minTCHP) < eps);
                  break;
               case 4:
                  Assert.IsTrue(Math.Abs(val1 - m_midTOC) < eps);
                  Assert.IsTrue(Math.Abs(val2 - m_maxTCHP) < eps);
                  break;
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
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VarySourceRockTOC(sa, m_layerName, m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VaryTopCrustHeatProduction(sa, m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         // set up and generate DoE
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         VarSpace varPrms = sa.varSpace();
         RunCaseSet expSet = sa.doeCaseSet();
         DoEGenerator doe = sa.doeGenerator();

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == doe.generateDoE(varPrms, expSet));

         Assert.IsTrue(5 == expSet.size());

         string pathToCaseSet = @".\CaseSet";

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setScenarioLocation(pathToCaseSet));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.applyMutations(sa.doeCaseSet()));

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
            Assert.IsTrue(
               Math.Abs(sa.doeCaseSet().runCase(i).parameter(0).asDoubleArray()[0] - prm_toc.asDoubleArray()[0]) < eps);
            Assert.IsTrue(
               Math.Abs(sa.doeCaseSet().runCase(i).parameter(1).asDoubleArray()[0] - prm_tchp.asDoubleArray()[0]) < eps);
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
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VarySourceRockTOC(sa, m_layerName, m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VaryTopCrustHeatProduction(sa, m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         // set up and generate DoE
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         VarSpace varPrms = sa.varSpace();
         RunCaseSet expSet = sa.doeCaseSet();
         DoEGenerator doe = sa.doeGenerator();

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == doe.generateDoE(varPrms, expSet));

         Assert.IsTrue(5 == expSet.size());

         string pathToCaseSet = @".\CaseSet";

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setScenarioLocation(pathToCaseSet));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.applyMutations(sa.doeCaseSet()));

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
            rm.scheduleCase(caseSet.runCase(i));
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
         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();
         // prepare scenario - define Tornado DoE for 2 var parameters and 2 observables, create 1st order proxy
         prepareScenarioUpToMC(sa, RSProxy.RSKrigingType.NoKriging, 1, "TestFirstOrderTornadoRS");

         RSProxy proxy = sa.rsProxySet().rsProxy("TestFirstOrderTornadoRS");
         CoefficientsMapList cml = proxy.getCoefficientsMapList();

         // Check proxy coefficients for
         // first observable obs(1) = 65.1536 + 15.3411 * prm_1 +0 * prm_2 
         for (int i = 0; i < cml.Count; ++i) // go over all observables
         {
            CoefficientsMap cm = cml[i];
            var enumerator = cm.GetEnumerator();

            if (i == 0) // check Response Surfvace for the first observable
            {
               uint cpow = 0;
               while (enumerator.MoveNext())
               {
                  UnsignedIntVector prmIDs = enumerator.Current.Key;
                  double coef = enumerator.Current.Value.first;

                  switch (cpow)
                  {
                     case 0:
                        Assert.AreEqual<int>(prmIDs.Count, 0);
                        Assert.IsTrue(Math.Abs(65.15362 - coef) < eps);
                        break;

                     case 1:
                        Assert.AreEqual<int>(prmIDs.Count, 1);
                        Assert.AreEqual<uint>(prmIDs[0], 0);
                        Assert.IsTrue(Math.Abs(15.34105 - coef) < eps);
                        break;

                     case 2:
                        Assert.AreEqual<int>(prmIDs.Count, 1);
                        Assert.AreEqual<uint>(prmIDs[0], 1);
                        Assert.IsTrue(Math.Abs(0.0 - coef) < eps);
                        break;
                  }
                  ++cpow;
               }
            }
            else if (i == 1) // check Response Surfvace for the second observable
            {
               uint cpow = 0;
               while (enumerator.MoveNext())
               {
                  UnsignedIntVector prmIDs = enumerator.Current.Key;
                  double coef = enumerator.Current.Value.first;

                  switch (cpow)
                  {
                     case 0:
                        Assert.AreEqual<int>(prmIDs.Count, 0);
                        Assert.IsTrue(Math.Abs(0.479763 - coef) < eps);
                        break;

                     case 1:
                        Assert.AreEqual<int>(prmIDs.Count, 1);
                        Assert.AreEqual<uint>(prmIDs[0], 0);
                        Assert.IsTrue(Math.Abs(0.0928937 - coef) < eps);
                        break;

                     case 2:
                        Assert.AreEqual<int>(prmIDs.Count, 1);
                        Assert.AreEqual<uint>(prmIDs[0], 1);
                        Assert.IsTrue(Math.Abs(0.0 - coef) < eps);
                        break;
                  }
                  ++cpow;
               }
            }
         }
         // check response surface evaluation
         // create one new case
         RunCaseImpl nrc = new RunCaseImpl();
         DoubleVector prmVals = new DoubleVector();
         // set case parameters
         prmVals.Add(10.16);
         prmVals.Add(1.970);

         int off = 0;
         for (uint i = 0; i < prmVals.Count; ++i)
         {
            nrc.addParameter(sa.varSpace().continuousParameter(i).newParameterFromDoubles(prmVals, ref off));
         }

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == proxy.evaluateRSProxy(nrc));
         Assert.IsTrue(Math.Abs(nrc.obsValue(0).asDoubleArray()[0] - 65.6445336) < eps);
         Assert.IsTrue(Math.Abs(nrc.obsValue(1).asDoubleArray()[0] - 0.4827356) < eps);
      }
      
      [TestMethod]
      public void Empty_test()
      { }


      [TestMethod]
      public void MCSolver_MC_Test() // analog of casaAPI/test/MCTest.C
      {
         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();
         // prepare scenario - define Tornado DoE for 2 var parameters and 2 observables, create 1st order proxy
         prepareScenarioUpToMC(sa, RSProxy.RSKrigingType.NoKriging, 1, "TestFirstOrderTornadoRS");

         RSProxy proxy = sa.rsProxySet().rsProxy("TestFirstOrderTornadoRS");

         // create corresponded MC algorithm
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       sa.setMCAlgorithm(MonteCarloSolver.Algorithm.MonteCarlo, MonteCarloSolver.KrigingType.NoKriging,
                          MonteCarloSolver.PriorDistribution.NoPrior, MonteCarloSolver.MeasurementDistribution.Normal));

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       sa.mcSolver().runSimulation(proxy, sa.varSpace(), sa.varSpace(), sa.obsSpace(), 50, 10, 1.0));

         Assert.IsTrue(Math.Abs(sa.mcSolver().stdDevFactor() - 1.0) < eps); // VRE

         // Get MC samples
         // must be 50 samples
         Assert.IsTrue(sa.mcSolver().samplingsNumber() == 50);

         // check couple cases to be sure
         RunCase rc = sa.mcSolver().samplingPoint(7);

         // check RMSE value
         Assert.IsTrue(relativeError(sa.mcSolver().RMSE(7), 12.5415) < reps);

         // check generated parameters
         Assert.IsTrue(relativeError(rc.parameter(0).asDoubleArray()[0], 13.1922) < reps);
         Assert.IsTrue(relativeError(rc.parameter(1).asDoubleArray()[0], 4.48863) < reps);

         // check evaluated observables
         Assert.IsTrue(relativeError(rc.obsValue(0).asDoubleArray()[0], 74.948) < reps);
         Assert.IsTrue(relativeError(rc.obsValue(1).asDoubleArray()[0], 0.539071) < reps);

         rc = sa.mcSolver().samplingPoint(44);

         Assert.IsTrue(relativeError(sa.mcSolver().RMSE(44), 20.6406) < reps);

         // check generated parameters
         Assert.IsTrue(relativeError(rc.parameter(0).asDoubleArray()[0], 5.68843) < reps);
         Assert.IsTrue(relativeError(rc.parameter(1).asDoubleArray()[0], 3.83905) < reps);

         // check evaluated observables
         Assert.IsTrue(relativeError(rc.obsValue(0).asDoubleArray()[0], 51.9248) < reps);
         Assert.IsTrue(relativeError(rc.obsValue(1).asDoubleArray()[0], 0.399659) < reps);
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
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       sa.setMCAlgorithm(MonteCarloSolver.Algorithm.MonteCarlo,
                          MonteCarloSolver.KrigingType.GlobalKriging,
                          MonteCarloSolver.PriorDistribution.NoPrior, MonteCarloSolver.MeasurementDistribution.Normal));
         // Expect an error here because we define to use kriging in MonteCarlo and do not provide proxy with kriging
         Assert.IsTrue(ErrorHandler.ReturnCode.MonteCarloSolverError ==
                       sa.mcSolver().runSimulation(proxy, sa.varSpace(), sa.varSpace(), sa.obsSpace(), 50, 10, 1.0));
      }

      [TestMethod]
      public void MCSolver_MCMC_Test() // analog of casaAPI/test/MCTest.C
      {
         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();
         // prepare scenario - define Tornado DoE for 2 var parameters and 2 observables, create 1st order proxy
         prepareScenarioUpToMC(sa, RSProxy.RSKrigingType.NoKriging, 1, "TestFirstOrderTornadoRS");

         RSProxy proxy = sa.rsProxySet().rsProxy("TestFirstOrderTornadoRS");

         // create corresponded MC algorithm
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       sa.setMCAlgorithm(MonteCarloSolver.Algorithm.MCMC, MonteCarloSolver.KrigingType.NoKriging,
                          MonteCarloSolver.PriorDistribution.NoPrior, MonteCarloSolver.MeasurementDistribution.Normal));

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       sa.mcSolver().runSimulation(proxy, sa.varSpace(), sa.varSpace(), sa.obsSpace(), 50, 10, 1.0));

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
            ScenarioAnalysis sa = ScenarioAnalysis.loadScenario(m_serialisedStateFileName, "txt");

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
            ScenarioAnalysis sb = ScenarioAnalysis.loadScenario(@".\casa_state_reloaded_1.txt", "txt");
            Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sb.errorCode());

            sb.saveScenario(@".\casa_state_reloaded_2.txt", "txt");
         }

         string fileAContent = File.ReadAllText(@".\casa_state_reloaded_1.txt");
         string fileBContent = File.ReadAllText(@".\casa_state_reloaded_2.txt");

         Assert.IsTrue(fileAContent == fileBContent);
      }
   }
}
