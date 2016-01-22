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

         StringVector mapRng = new StringVector();
         DoubleVector dblRng = new DoubleVector();
         dblRng.Add(m_minTCHP);
         dblRng.Add(m_maxTCHP);

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CauldronAPI.VaryTopCrustHeatProduction(sa, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));

         // add 2 observables for T & VRE
         Observable ob = ObsGridPropertyXYZ.createNewInstance(460001.0, 6750001.0, 2751.0, "Temperature", 0.0);
         ob.setReferenceValue(ObsValueDoubleScalar.createNewInstance(ob, 108.6), 2.0);
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, obs.addObservable(ob));

         ob = ObsGridPropertyXYZ.createNewInstance(460001.0, 6750001.0, 2730.0, "Vr", 0.0);
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

         DoEGenerator doe = sa.doeGenerator();
         VarSpace varPrms = sa.varSpace();

         ErrorHandler.ReturnCode ret = CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block);
         if ( ret != ErrorHandler.ReturnCode.NoError )
         {
            m_isDebug = true;
            logMsg( "DoE_Tornado_Test: Create TOC parameter: " + sa.errorCode().ToString() + ", msg: " + sa.errorMessage() );
         }
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, ret );

         StringVector mapRng = new StringVector();
         DoubleVector dblRng = new DoubleVector();
         dblRng.Add(m_minTCHP);
         dblRng.Add(m_maxTCHP);

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

         // vary 2 parameters
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
            CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         
         StringVector mapRng = new StringVector();
         DoubleVector dblRng = new DoubleVector();
         dblRng.Add(m_minTCHP);
         dblRng.Add(m_maxTCHP);
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

         // vary 2 parameters
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                       CauldronAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         
         StringVector mapRng = new StringVector();
         DoubleVector dblRng = new DoubleVector();
         dblRng.Add(m_minTCHP);
         dblRng.Add(m_maxTCHP);
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
         prmVals.Add( -1 + (10.16 - m_minTOC)/(m_maxTOC-m_minTOC) ); // For TOC -1:1 range is used inside the parameter
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
         //                                       RMSE             prm 1             prm 2             obs 1             obs 2
         double[,] mcResults = new double[,] { { 10.847120958689,  14.7799716425875, 3.69058738898254, 79.8195767935034, 0.568569137153305 },
                                               { 11.9165961895442, 13.776522349076,  4.78341260706223, 76.7407836366584, 0.549926253419013 },
                                               { 12.0109371098716, 13.6882243810147, 4.11399096870442, 76.469866928073,  0.548285783129995 },
                                               { 12.0664648493268, 13.6362682578697, 4.13861701951217, 76.3104546314783, 0.547320500709309 },
                                               { 12.1204815889351, 13.5857362972528, 4.73401379696825, 76.1554119645939, 0.5463816775194   },
                                               { 12.4029575083419, 13.3216445377765, 3.32743709854253, 75.3451229872513, 0.541475169538442 },
                                               { 12.606748503291,  13.1312768753522, 4.55014247079906, 74.7610350217244, 0.537938366811794 },
                                               { 12.64455051635,   13.0959789335645, 4.48862828771201, 74.652733523752,  0.537282573410909 },
                                               { 12.9552873374259, 12.8059859219899, 4.1117210860181,  73.7629740657086, 0.531894851247466 },
                                               { 13.0151359362289, 12.7501643359592, 4.88329961289153, 73.5917017172335, 0.53085775316492  },
                                               { 13.7580070374516, 12.0580596108872, 2.91186590043343, 71.4681790787201, 0.51799927789875  },
                                               { 13.800881355477,  12.0181565668905, 3.83329493024231, 71.3457481600992, 0.517257927224946 },
                                               { 13.9791789893154, 11.8522602324524, 3.4158371617078,  70.8367433678129, 0.514175772406687 },
                                               { 14.3136626377492, 11.5412287987335, 3.3458498698207,  69.8824336125621, 0.50839718160591  },
                                               { 14.3390032401012, 11.5176745297941, 4.22934814973615, 69.8101641690595, 0.507959571554138 },
                                               { 14.8407686489863, 11.0515414337589, 3.22065279753176, 68.3799699424732, 0.499299377989519 },
                                               { 14.8411183101777, 11.051216770538,  4.58045757324999, 68.3789738075325, 0.499293346136472 },
                                               { 15.1425403748377, 10.7714258456639, 4.16415771034784, 67.5205164939244, 0.49409516650142  },
                                               { 15.1627465480066, 10.7526754630346, 4.31392772913826, 67.4629863824373, 0.493746806892627 },
                                               { 15.3478830554577, 10.5809097901734, 2.78188086214046, 66.935973227308,  0.490555606809674 },
                                               { 15.4508051531493, 10.4854455819435, 3.58544910964974, 66.6430689889748, 0.488781996377812 },
                                               { 15.465883796857,  10.4714610120017, 4.74641963189648, 66.6001613916338, 0.488522179849778 },
                                               { 15.6002210793228, 10.3468870671809, 2.22309062394272, 66.217942368395,  0.48620774544374  },
                                               { 15.6301221365876, 10.319163049181,  3.66029641868947, 66.1328792591277, 0.485692666458125 },
                                               { 16.0917543852704, 9.89131421709498, 3.38275458648381, 64.820149194033,  0.477743748576564 },
                                               { 16.0957195762782, 9.88764057075626, 4.3277754702562,  64.8088776756001, 0.477675496635966 },
                                               { 16.107807197066,  9.87644184327594, 3.18871442041409, 64.7745176279577, 0.477467437717855 },
                                               { 16.2657489360303, 9.73013374190742, 4.19849745054188, 64.3256136482577, 0.47474920876415  },
                                               { 16.6105704861003, 9.41082973504958, 4.41487327790818, 63.3459219013765, 0.468816923481539 },
                                               { 16.7836093954068, 9.25065493417931, 2.99367623451439, 62.8544719755983, 0.465841067891131 },
                                               { 17.070139481911,  8.98550846674646, 3.96081525003686, 62.0409469327562, 0.460914964701989 },
                                               { 17.1262624407699, 8.93358559214614, 4.38063114868328, 61.8816366496787, 0.459950299999365 },
                                               { 17.6794036478568, 8.42203353980263, 3.60160186373642, 60.3120875271578, 0.450446276729285 },
                                               { 17.8367723523726, 8.27655837410774, 3.30304765237699, 59.8657391690211, 0.447743522720873 },
                                               { 18.0066660026412, 8.11953388649968, 3.26770201527524, 59.3839550658972, 0.4448261961705   },
                                               { 18.1956004811674, 7.94494524266689, 4.3470416514606,  58.848280443003,  0.44158254867446  },
                                               { 18.2566260250387, 7.88856093114782, 3.24691727035265, 58.675281534557,  0.440534995827609 },
                                               { 18.4930849546295, 7.6701195152663,  0.34659656853707, 58.0050573979352, 0.43647661645023  },
                                               { 18.7866279266765, 7.39901624113773, 3.09620214924863, 57.1732556212212, 0.43143984294085  },
                                               { 18.9048967014549, 7.28981036013851, 2.23788469136545, 56.8381890450806, 0.429410928718941 },
                                               { 19.4740153500561, 6.76447008007717, 0.21179269636964, 55.2263347443936, 0.419650736723738 },
                                               { 19.8921973536103, 6.37862223168163, 3.07917151439978, 54.0424725174679, 0.412482146717967 },
                                               { 20.1209404191503, 6.16762170054059, 4.68884544759226, 53.3950785778157, 0.408562010050004 },
                                               { 20.5140074386574, 5.80513070298663, 0.45090394759577, 52.2828800742106, 0.401827362304648 },
                                               { 20.6019559372532, 5.72403820941656, 3.83904842223875, 52.034071274514,  0.400320761085108 },
                                               { 20.6225512241834, 5.70504917783427, 3.82917845834406, 51.9758089379229, 0.399967967665147 },
                                               { 20.7258132360557, 5.60984498706084, 0.51152304117577, 51.6837024877499, 0.398199188045606 },
                                               { 20.7850145737082, 5.55526647765669, 3.41782583407434, 51.516244159411,  0.397185184835088 },
                                               { 21.0197420887767, 5.33889042455692, 3.51784408778324, 50.8523569895298, 0.393165177419758 },
                                               { 21.3682551744864, 5.01768876428489, 0.22087950680754, 49.8668428434666, 0.387197636013896 }
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
         double[,] mcResults = new double[,] { { 10.6199, 14.9939, 0.353319, 80.4759, 0.572543 },
                                               { 10.6241, 14.9899, 2.22,     80.4637, 0.57247  },
                                               { 10.6246, 14.9894, 0.823811, 80.4621, 0.57246  },
                                               { 10.625,  14.989,  1.07942,  80.4609, 0.572452 },
                                               { 10.6293, 14.985,  0.152547, 80.4485, 0.572378 },
                                               { 10.6296, 14.9847, 0.244148, 80.4477, 0.572373 },
                                               { 10.6358, 14.9788, 1.79458,  80.4297, 0.572264 },
                                               { 10.64,   14.9749, 0.925276, 80.4178, 0.572192 },
                                               { 10.6417, 14.9733, 1.1165,   80.4128, 0.572161 },
                                               { 10.642,  14.973,  0.426355, 80.4119, 0.572156 },
                                               { 10.6421, 14.9729, 4.25969,  80.4116, 0.572154 },
                                               { 10.6422, 14.9728, 3.57137,  80.4113, 0.572152 },
                                               { 10.6425, 14.9725, 2.14452,  80.4103, 0.572146 },
                                               { 10.6426, 14.9725, 3.47253,  80.4102, 0.572146 },
                                               { 10.6441, 14.9711, 0.140859, 80.406,  0.57212  },
                                               { 10.6455, 14.9697, 0.531559, 80.4016, 0.572094 },
                                               { 10.6463, 14.969,  3.96393,  80.3995, 0.572081 },
                                               { 10.6464, 14.9689, 2.5711,   80.3993, 0.572079 },
                                               { 10.6492, 14.9662, 0.380807, 80.3911, 0.57203  },
                                               { 10.6498, 14.9657, 1.21838,  80.3895, 0.57202  },
                                               { 10.6503, 14.9652, 4.19547,  80.3878, 0.57201  },
                                               { 10.6504, 14.9651, 1.01329,  80.3875, 0.572008 },
                                               { 10.6522, 14.9634, 1.05815,  80.3823, 0.571977 },
                                               { 10.6558, 14.96,   0.23379,  80.372,  0.571914 },
                                               { 10.6562, 14.9597, 2.57613,  80.3709, 0.571908 },
                                               { 10.6563, 14.9595, 1.13229,  80.3705, 0.571905 },
                                               { 10.6588, 14.9572, 2.57458,  80.3634, 0.571862 },
                                               { 10.6592, 14.9568, 2.15885,  80.3622, 0.571855 },
                                               { 10.6593, 14.9568, 2.28329,  80.362,  0.571854 },
                                               { 10.6631, 14.9531, 2.25454,  80.3509, 0.571787 },
                                               { 10.6664, 14.95,   2.18242,  80.3413, 0.571729 },
                                               { 10.6688, 14.9478, 2.22615,  80.3345, 0.571687 },
                                               { 10.6698, 14.9468, 2.69632,  80.3315, 0.571669 },
                                               { 10.6728, 14.944,  0.530648, 80.3228, 0.571616 },
                                               { 10.68,   14.9372, 3.96848,  80.3021, 0.571491 },
                                               { 10.685,  14.9326, 1.17187,  80.2878, 0.571404 },
                                               { 10.6868, 14.9308, 0.40862,  80.2824, 0.571372 },
                                               { 10.6877, 14.93,   2.6942,   80.28,   0.571357 },
                                               { 10.6911, 14.9268, 0.56615,  80.27,   0.571297 },
                                               { 10.6912, 14.9267, 2.42626,  80.2696, 0.571294 },
                                               { 10.7057, 14.913,  3.32914,  80.2278, 0.571041 },
                                               { 10.7062, 14.9126, 0.597936, 80.2264, 0.571032 },
                                               { 10.7141, 14.9052, 0.541473, 80.2037, 0.570895 },
                                               { 10.7147, 14.9046, 0.293939, 80.2018, 0.570884 },
                                               { 10.7196, 14.8999, 3.53698,  80.1877, 0.570798 },
                                               { 10.7215, 14.8981, 0.884713, 80.1821, 0.570765 },
                                               { 10.7285, 14.8916, 4.25707,  80.162,  0.570642 },
                                               { 10.7324, 14.8879, 2.23697,  80.1507, 0.570574 },
                                               { 10.7336, 14.8868, 0.268091, 80.1474, 0.570554 },
                                               { 10.7351, 14.8854, 3.33775,  80.143,  0.570527 }
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
