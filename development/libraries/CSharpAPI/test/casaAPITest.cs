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
         StringVector mapRng = new StringVector();
         DoubleVector dblRng = new DoubleVector();

         dblRng.Add(m_minTOC);
         dblRng.Add(m_maxTOC);

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                         CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));

         dblRng[0] = m_minTCHP;
         dblRng[1] = m_maxTCHP;

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VaryTopCrustHeatProduction(sa, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));

         // add 2 observables for T & VRE
         Observable ob = ObsGridPropertyXYZ.createNewInstance(460001.0, 6750001.0, 2751.0, "Temperature", 0.0);
         ob.setReferenceValue(ObsValueDoubleScalar.createNewInstance(ob, 108.6), ObsValueDoubleScalar.createNewInstance(ob, 2.0));
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, obs.addObservable(ob));

         ob = ObsGridPropertyXYZ.createNewInstance(460001.0, 6750001.0, 2730.0, "Vr", 0.0);
         ob.setReferenceValue(ObsValueDoubleScalar.createNewInstance(ob, 1.1), ObsValueDoubleScalar.createNewInstance(ob, 0.1));
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
               obs.updateObsValueValidateStatus(j, obs.observable(j).isValid(obVal));
               rcs.runCase(i).addObsValue( obVal );
            }
            rcs.runCase(i).setRunStatus(RunCase.CaseStatus.Completed);
         }
         // Calculate Response Surface proxy
         StringVector doeList = new StringVector();
         doeList.Add(DoEGenerator.DoEName(DoEGenerator.DoEAlgorithm.Tornado));

         ErrorHandler.ReturnCode retCode = sa.addRSAlgorithm(proxyName, proxyOrder, krig, doeList);
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

         DoEGenerator doe     = sa.doeGenerator();
         VarSpace     varPrms = sa.varSpace();
         StringVector mapRng  = new StringVector();
         DoubleVector dblRng  = new DoubleVector();

         dblRng.Add(m_minTOC);
         dblRng.Add(m_maxTOC);

         ErrorHandler.ReturnCode ret = CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", dblRng, mapRng, VarPrmContinuous.PDF.Block);
         if ( ret != ErrorHandler.ReturnCode.NoError )
         {
            m_isDebug = true;
            logMsg( "DoE_Tornado_Test: Create TOC parameter: " + sa.errorCode().ToString() + ", msg: " + sa.errorMessage() );
         }
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, ret );

         dblRng[0] = m_minTCHP;
         dblRng[1] = m_maxTCHP;

         ret = CauldronAPI.VaryTopCrustHeatProduction(sa, "", dblRng, mapRng, VarPrmContinuous.PDF.Block);
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
         StringVector mapRng = new StringVector();
         DoubleVector dblRng = new DoubleVector();

         dblRng.Add( m_minTOC );
         dblRng.Add( m_maxTOC );

         // vary 2 parameters
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
            CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));
         
         dblRng[0] = m_minTCHP;
         dblRng[1] = m_maxTCHP;
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                         CauldronAPI.VaryTopCrustHeatProduction(sa, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));

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

         StringVector mapRng = new StringVector();
         DoubleVector dblRng = new DoubleVector();
         dblRng.Add(m_minTOC);
         dblRng.Add(m_maxTOC);
         // vary 2 parameters
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                       CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));
         
         dblRng[0] = m_minTCHP;
         dblRng[1] = m_maxTCHP;
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                       CauldronAPI.VaryTopCrustHeatProduction(sa, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));

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
               string stageFile = casePath + @"Stage_" + j.ToString() + @".bat";
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
         prmVals.Add( 10.16 );
         prmVals.Add( 1.970 );

         int off = 0;
         for (uint i = 0; i < prmVals.Count; ++i)
         {
            nrc.addParameter(sa.varSpace().continuousParameter(i).newParameterFromDoubles(prmVals, ref off));
         }

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, proxy.evaluateRSProxy(nrc));
         Assert.AreEqual(65.6445336, nrc.obsValue(0).asDoubleArray()[0], eps);
         Assert.AreEqual(0.482735608, nrc.obsValue(1).asDoubleArray()[0], eps);
      }
      
      [TestMethod]
      public void Empty_test()
      { }


      [TestMethod]
      public void MCSolver_MC_Test() // analog of casaAPI/test/MCTest.C
      {
         //                                       RMSE             prm 1             prm 2             obs 1             obs 2
         double[,] mcResults = new double[,] { { 10.8438860461543, 14.7830145362452, 3.69058738898254, 79.8289130302528, 0.568625670465992 },
                                               { 11.8138137845091, 13.8727579754610, 4.78341260706223, 77.0360547478893, 0.551714195874495 },
                                               { 12.0076853996612, 13.6912672746723, 4.11399096870442, 76.4792031648224, 0.548342316442682 },
                                               { 12.0176187638116, 13.6819719236378, 4.73401379696825, 76.4506830758248, 0.548169619974882 },
                                               { 12.0632124587602, 13.6393111515273, 4.13861701951217, 76.3197908682276, 0.547377034021996 },
                                               { 12.3997011912424, 13.3246874314342, 3.32743709854253, 75.3544592240006, 0.541531702851129 },
                                               { 12.5037096578221, 13.2275125017373, 4.55014247079906, 75.0563061329553, 0.539726309267276 },
                                               { 12.5414988464063, 13.1922145599496, 4.48862828771201, 74.9480046349829, 0.539070515866391 },
                                               { 12.9119645004691, 12.8463999623443, 4.88329961289153, 73.8869728284644, 0.532645695620402 },
                                               { 12.9520252348307, 12.8090288156475, 4.11172108601810, 73.7723103024579, 0.531951384560152 },
                                               { 13.5829036818290, 12.2210727573150, 2.91186590043343, 71.9683376447215, 0.521027866543604 },
                                               { 13.6257589782159, 12.1811697133184, 3.83329493024231, 71.8459067261006, 0.520286515869799 },
                                               { 14.1383260596859, 11.7042419451614, 3.34584986982070, 70.3825921785635, 0.511425770250764 },
                                               { 14.6655845129706, 11.2142299169659, 4.58045757324999, 68.8791323735338, 0.502321934781325 },
                                               { 14.6914909597234, 11.1901677073199, 3.41583716170780, 68.8053044612760, 0.501874887800755 },
                                               { 14.7781289537093, 11.1097065351398, 3.22065279753176, 68.5584326881812, 0.500380015775055 },
                                               { 14.9442824362002, 10.9554377960859, 4.22934814973615, 68.0851038003288, 0.497513887725921 },
                                               { 14.9871026129686, 10.9156886094624, 4.31392772913826, 67.9631449484386, 0.496775395537480 },
                                               { 15.5374934892312, 10.4050521685618, 2.22309062394272, 66.3964051141030, 0.487288383229276 },
                                               { 15.6904866578017, 10.2631978138639, 3.66029641868947, 65.9611661644753, 0.484652899544214 },
                                               { 15.7487466184369, 10.2091891119557, 4.16415771034784, 65.7954561251937, 0.483649482673203 },
                                               { 16.0450277346513, 9.93460694465688, 3.18871442041409, 64.9529803736657, 0.478548075503391 },
                                               { 16.0620694382349, 9.91881726504091, 2.78188086214046, 64.9045343207711, 0.478254722203742 },
                                               { 16.0724233101205, 9.90922427829359, 4.74641963189648, 64.8751010229032, 0.478076496021561 },
                                               { 16.1651129349410, 9.82335305681096, 3.58544910964974, 64.6116300824379, 0.476481111771879 },
                                               { 16.6710257446797, 9.35486449973243, 4.41487327790818, 63.1742088067240, 0.467777156567629 },
                                               { 16.7028520426684, 9.32540383704811, 4.32777547025620, 63.0838173068694, 0.467229812807750 },
                                               { 16.7207681073944, 9.30882003556024, 2.99367623451439, 63.0329347213063, 0.466921705676667 },
                                               { 16.8067671217477, 9.22922169196245, 3.38275458648381, 62.7887102874961, 0.465442863970632 },
                                               { 16.9809389780047, 9.06804121677488, 4.19849745054188, 62.2941747417209, 0.462448324158217 },
                                               { 17.1306319609077, 8.92954323142931, 3.96081525003686, 61.8692338381037, 0.459875197788079 },
                                               { 17.5823002062942, 8.51181135725520, 3.30304765237699, 60.5875447244440, 0.452114240844173 },
                                               { 17.6164911733549, 8.48019864118357, 3.60160186373642, 60.4905502728658, 0.451526914514821 },
                                               { 17.7342289068931, 8.37134885843800, 4.38063114868328, 60.1565762809480, 0.449504616171148 },
                                               { 17.7521446431924, 8.35478686964714, 3.26770201527524, 60.1057606213200, 0.449196914293800 },
                                               { 18.0020347748410, 8.12381391429528, 3.24691727035265, 59.3970870899799, 0.444905713950909 },
                                               { 18.2561724383803, 7.88898000734974, 4.34704165146060, 58.6765673483506, 0.440542781760549 },
                                               { 18.4923654468928, 7.67078411814585, 0.34659656853707, 58.0070965391363, 0.436488963974208 },
                                               { 18.5318977218937, 7.63426922428519, 3.09620214924863, 57.8950611766441, 0.435810561064150 },
                                               { 18.6501370933779, 7.52506334328597, 2.23788469136545, 57.5599946005034, 0.433781646842241 },
                                               { 19.4732951810615, 6.76513468295672, 0.21179269636964, 55.2283738855946, 0.419663084247716 },
                                               { 19.9307926938012, 6.34301784245452, 3.07917151439978, 53.9332307743974, 0.411820659891394 },
                                               { 20.1816185795138, 6.11165646522345, 4.68884544759226, 53.2233654831632, 0.407522243136093 },
                                               { 20.5132866701182, 5.80579530586618, 0.45090394759577, 52.2849192154117, 0.401839709828627 },
                                               { 20.6405723046043, 5.68843382018945, 3.83904842223875, 51.9248295314435, 0.399659274258536 },
                                               { 20.6611681693404, 5.66944478860716, 3.82917845834406, 51.8665671948524, 0.399306480838575 },
                                               { 20.7250923563972, 5.61050958994038, 0.51152304117577, 51.6857416289510, 0.398211535569584 },
                                               { 20.8236360165256, 5.51966208842958, 3.41782583407434, 51.4070024163405, 0.396523698008516 },
                                               { 20.9811153561768, 5.37449481378403, 3.51784408778324, 50.9615987326003, 0.393826664246331 },
                                               { 21.3689763714581, 5.01702416140535, 0.22087950680754, 49.8648037022655, 0.387185288489918 }
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
         //                                       RMSE    prm 1    prm 2     obs 1    obs 2
         double[,] mcResults = new double[,] { { 10.6138, 14.9996, 3.621,    80.4935, 0.57265  },
                                               { 10.6181, 14.9955, 4.54124,  80.4808, 0.572573 },
                                               { 10.6185, 14.9951, 0.621135, 80.4798, 0.572567 },
                                               { 10.6197, 14.994,  0.511065, 80.4764, 0.572546 },
                                               { 10.6218, 14.9921, 3.30716,  80.4704, 0.57251  },
                                               { 10.622,  14.9918, 2.02334,  80.4696, 0.572505 },
                                               { 10.6231, 14.9908, 2.12795,  80.4665, 0.572487 },
                                               { 10.6236, 14.9903, 3.03292,  80.465,  0.572478 },
                                               { 10.6247, 14.9893, 0.662088, 80.4617, 0.572458 },
                                               { 10.6287, 14.9856, 0.925191, 80.4503, 0.572389 },
                                               { 10.6319, 14.9825, 0.512678, 80.441,  0.572332 },
                                               { 10.6335, 14.9811, 3.33186,  80.4365, 0.572305 },
                                               { 10.6344, 14.9802, 4.60552,  80.4339, 0.572289 },
                                               { 10.635,  14.9796, 1.12382,  80.4321, 0.572278 },
                                               { 10.6358, 14.9789, 2.46228,  80.4298, 0.572264 },
                                               { 10.6367, 14.978,  1.76094,  80.4272, 0.572249 },
                                               { 10.6378, 14.977,  1.59355,  80.424,  0.572229 },
                                               { 10.6396, 14.9753, 3.49808,  80.4188, 0.572198 },
                                               { 10.6435, 14.9716, 1.7125,   80.4076, 0.57213  },
                                               { 10.6439, 14.9712, 1.89484,  80.4063, 0.572122 },
                                               { 10.6476, 14.9678, 0.467267, 80.3957, 0.572058 },
                                               { 10.6502, 14.9653, 1.86743,  80.3881, 0.572012 },
                                               { 10.6533, 14.9624, 3.81712,  80.3794, 0.571959 },
                                               { 10.654,  14.9617, 1.95494,  80.3772, 0.571946 },
                                               { 10.6554, 14.9604, 1.76527,  80.3733, 0.571922 },
                                               { 10.656,  14.9599, 1.50376,  80.3716, 0.571912 },
                                               { 10.6654, 14.951,  0.586551, 80.3443, 0.571747 },
                                               { 10.6661, 14.9503, 0.255906, 80.3422, 0.571733 },
                                               { 10.668,  14.9486, 0.667534, 80.3368, 0.571701 },
                                               { 10.6715, 14.9452, 0.69706,  80.3266, 0.571639 },
                                               { 10.6749, 14.942,  2.04532,  80.3168, 0.57158  },
                                               { 10.6756, 14.9413, 2.07706,  80.3147, 0.571567 },
                                               { 10.6786, 14.9385, 4.8162,   80.306,  0.571515 },
                                               { 10.6911, 14.9268, 1.98568,  80.2701, 0.571297 },
                                               { 10.7021, 14.9164, 3.18953,  80.2383, 0.571104 },
                                               { 10.7115, 14.9076, 0.226664, 80.211,  0.57094  },
                                               { 10.7264, 14.8936, 3.84456,  80.1682, 0.57068  },
                                               { 10.7268, 14.8931, 0.521945, 80.1668, 0.570672 },
                                               { 10.7272, 14.8928, 0.754967, 80.1658, 0.570666 },
                                               { 10.729,  14.8911, 0.114227, 80.1604, 0.570633 },
                                               { 10.7319, 14.8884, 0.598605, 80.1522, 0.570583 },
                                               { 10.735,  14.8854, 3.42631,  80.1432, 0.570529 },
                                               { 10.7406, 14.8802, 4.62221,  80.127,  0.57043  },
                                               { 10.7552, 14.8664, 3.65213,  80.0849, 0.570176 },
                                               { 10.7743, 14.8485, 0.375491, 80.0297, 0.569842 },
                                               { 10.8009, 14.8235, 2.39751,  79.9531, 0.569378 },
                                               { 10.8052, 14.8194, 0.527,    79.9405, 0.569301 },
                                               { 10.83,   14.7961, 4.44749,  79.8689, 0.568868 },
                                               { 10.852,  14.7754, 0.351537, 79.8055, 0.568484 },
                                               { 10.9136, 14.7175, 0.939383, 79.6278, 0.567408 }
                                             };

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

         m_isDebug = true;
         if (m_isDebug) { logMsg("MCSolver_MCMC_Test StdDev " + sa.mcSolver().stdDevFactor().ToString()); }
         else           { Assert.AreEqual(1.0, sa.mcSolver().stdDevFactor(), eps);  }
         m_isDebug = false;

         // Get MC samples
         // must be 50 samples
         Assert.AreEqual(50U, sa.mcSolver().samplingsNumber());

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
         ScenarioAnalysis sa = ScenarioAnalysis.loadScenario(m_bestMatchStateFileName, "txt");

         if (ErrorHandler.ReturnCode.NoError != sa.errorCode())
         {
            m_isDebug = true;
            logMsg("Deserialization test failed with message:" + sa.errorMessage());
         }
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.errorCode());
         
         // clean any previous failed run
         string pathToCaseSet = @".\CaseSetBMCSH";

         if ( Directory.Exists(pathToCaseSet) )
         {
            Directory.Delete(pathToCaseSet, true); // delete folder ./CaseSet
         }

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.setScenarioLocation( pathToCaseSet ) );
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.saveCalibratedCase("NVGBestMatchCase.project3d", 1));

         Assert.IsTrue(File.Exists(pathToCaseSet+@"\Calibrated_NVGBestMatchCase\NVGBestMatchCase.project3d"));
         Assert.IsTrue(File.Exists(pathToCaseSet+@"\Calibrated_NVGBestMatchCase\Inputs.HDF"));
         Assert.IsTrue(File.Exists(pathToCaseSet+@"\Calibrated_NVGBestMatchCase\MAP-72981789-4.FLT"));

         // cleaning files/folders
         Directory.Delete(pathToCaseSet, true); // delete folder ./CaseSet       
      }
   }
}
