using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using Shell.BasinModeling.Casa;

namespace Shell.BasinModeling.Casa.Test
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

      private bool m_isDebug = true;

      private void logMsg(string msg)
      {
         if (!m_isDebug) return;
         using (System.IO.StreamWriter file = new System.IO.StreamWriter(@".\logfile.txt", true))
         {
            file.WriteLine(msg);
         }
      }

      private void prepareScenarioUpToMC(ScenarioAnalysis sa, RSProxy.RSKrigingType krig, int proxyOrder, string proxyName)
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

         VarSpace vrs = sa.varSpace();
         ObsSpace obs = sa.obsSpace();

         // vary 2 parameters
         StringVector mapRng = new StringVector();
         DoubleVector dblRng = new DoubleVector();

         dblRng.Add(m_minTOC);
         dblRng.Add(m_maxTOC);

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                         CasaAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));

         dblRng[0] = m_minTCHP;
         dblRng[1] = m_maxTCHP;

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                       CasaAPI.VaryTopCrustHeatProduction(sa, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));

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
      public string m_projectFileName = @"..\..\..\Ottoland.project3d";
      public string m_serialisedStateFileName = @"..\..\..\Ottoland_casa_state.txt";
      public string m_bestMatchStateFileName = @"..\..\..\BestMatchCaseGeneration_state.txt";

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
            m_projectFileName = @"e:\Build\cld-dev-git\build\libraries\casaAPI\Ottoland.project3d";
            m_serialisedStateFileName = @"e:\Build\cld-dev-git\build\libraries\casaAPI\Ottoland_casa_state.txt";
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

         ErrorHandler.ReturnCode ret = CasaAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", dblRng, mapRng, VarPrmContinuous.PDF.Block);
         if ( ret != ErrorHandler.ReturnCode.NoError )
         {
            m_isDebug = true;
            logMsg( "DoE_Tornado_Test: Create TOC parameter: " + sa.errorCode().ToString() + ", msg: " + sa.errorMessage() );
         }
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, ret );

         dblRng[0] = m_minTCHP;
         dblRng[1] = m_maxTCHP;

         ret = CasaAPI.VaryTopCrustHeatProduction(sa, "", dblRng, mapRng, VarPrmContinuous.PDF.Block);
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
            CasaAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));
         
         dblRng[0] = m_minTCHP;
         dblRng[1] = m_maxTCHP;
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                         CasaAPI.VaryTopCrustHeatProduction(sa, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));

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
                       CasaAPI.VarySourceRockTOC(sa, "", m_layerName, 1, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));
         
         dblRng[0] = m_minTCHP;
         dblRng[1] = m_maxTCHP;
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError,
                       CasaAPI.VaryTopCrustHeatProduction(sa, "", dblRng, mapRng, VarPrmContinuous.PDF.Block));

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
         //                                       RMSE             prm 1             prm 2              obs 1             obs 2
         double[,] mcResults = new double[,] { { 10.7879731393561, 14.8356166909653, 4.66576505556989,  79.9903074873865, 0.569602955378105 },
                                               { 11.1051148601371, 14.5374527537145, 4.35741433802461,  79.0754779134743, 0.564063427220711 },
                                               { 12.1991752378916, 13.5121373836887, 4.64508876565977,  75.9295950420076, 0.545014298024076 },
                                               { 12.252786006686,  13.462009465236,  4.37472769324298,  75.7757920613317, 0.544082981452726 },
                                               { 12.3820271725145, 13.3412036658743, 4.3806905165491,   75.4051344996723, 0.541838554667546 },
                                               { 12.4729588487609, 13.2562393642637, 0.975454536517619, 75.1444461798274, 0.540260019900782 },
                                               { 13.0602471351978, 12.7080950648371, 2.86927833300457,  73.4626243588837, 0.530076156590595 },
                                               { 13.169224302649,  12.6064892427487, 0.807402162248774, 73.150876359494,  0.52818844234318  },
                                               { 13.1771567304307, 12.5990946181568, 3.11374800140448,  73.1281880983748, 0.528051059091811 },
                                               { 13.4219203848041, 12.3710061352131, 2.75138106467388,  72.4283647341223, 0.523813448784898 },
                                               { 13.4739312015058, 12.3225583534757, 2.8295763042489,   72.2797167657176, 0.522913347137554 },
                                               { 13.879040591195,  11.9454247763387, 0.758500818406509, 71.1225917530101, 0.515906657834641 },
                                               { 13.921838603541,  11.9056045643186, 1.28100410702939,  71.000414980288,  0.515166846079563 },
                                               { 13.9560563887553, 11.8737705378336, 1.46124853679731,  70.9027415018865, 0.514575408068303 },
                                               { 14.0902749775403, 11.7489271402011, 0.460475995894725, 70.5196957408365, 0.512255967552369 },
                                               { 14.5143304398321, 11.3547422368608, 3.06722247796826,  69.3102536785587, 0.50493248507019  },
                                               { 14.6770253392381, 11.2036033868344, 1.66815512241157,  68.8465279475193, 0.50212450660332  },
                                               { 14.7034675743775, 11.1790441186809, 1.21514473577663,  68.771174955378,  0.501668224872149 },
                                               { 15.0520635069024, 10.8553928334425, 1.31659160270215,  67.7781448454966, 0.495655172373962 },
                                               { 15.0772302614211, 10.8320354115435, 0.838549010396023, 67.7064793700518, 0.495221219503984 },
                                               { 16.0716255187624, 9.90996343609151, 1.44140075858651,  64.8773689142503, 0.478090228686457 },
                                               { 16.1933429404125, 9.79720211837081, 1.0515473648274,   64.5313935116065, 0.475995258716787 },
                                               { 16.2336600504122, 9.7598561993572,  1.33898039639104,  64.4168083894298, 0.475301416356618 },
                                               { 16.2852758998326, 9.71204751325424, 4.47305168460716,  64.2701213006418, 0.474413188339248 },
                                               { 16.357729234274,  9.64494450403496, 1.69219915712883,  64.0642351767251, 0.473166494951565 },
                                               { 17.0012840608235, 9.04921608760945, 1.65237505640001,  62.2364152921642, 0.462098575848479 },
                                               { 17.1086133271661, 8.94991349234308, 1.06111547356634,  61.931734076342,  0.460253652791544 },
                                               { 17.4414835707045, 8.64202291373108, 0.424968137904843, 60.9870611241388, 0.454533415309627 },
                                               { 17.5520852302491, 8.53974896314239, 0.501682704025924, 60.6732631662031, 0.45263328803643  },
                                               { 17.7835502699219, 8.32575513269596, 3.82228469042301,  60.0166851556891, 0.448657539459332 },
                                               { 18.1959997060957, 7.94457636851029, 4.14833748342992,  58.8471486596269, 0.441575695435279 },
                                               { 18.3458057444032, 7.80617018488442, 4.21490838577795,  58.4224894229642, 0.439004274630931 },
                                               { 18.4406977205434, 7.71851040561426, 0.850454244621484, 58.1535308116097, 0.437375661123826 },
                                               { 18.4815683841905, 7.6807573268569,  0.695143028095296, 58.0376964378356, 0.436674254224209 },
                                               { 19.3692650885978, 6.86114249631003, 4.01324896918924,  55.5229460186034, 0.421446794210445 },
                                               { 19.48151108693,   6.75755271673245, 4.289070125124,    55.2051108210057, 0.419522220413629 },
                                               { 19.6847160790538, 6.57004399463106, 0.620838103885983, 54.629794684767,  0.416038533367452 },
                                               { 19.7767592358923, 6.48512113913357, 0.462003589455877, 54.369233530301,  0.414460768619735 },
                                               { 19.9374944245574, 6.3368355763448,  1.16266210822815,  53.9142622836969, 0.411705800805795 },
                                               { 19.9832281645035, 6.29464764030332, 4.41504569086603,  53.7848208364551, 0.410921999579667 },
                                               { 20.0283856746772, 6.25299277319743, 3.78276617793339,  53.6570149566521, 0.410148102134681 },
                                               { 20.1804703108375, 6.11271552232118, 0.88304181689363,  53.2266148927411, 0.407541919146101 },
                                               { 20.2076835979003, 6.0876167505399,  3.98812296227795,  53.149606590174,  0.407075614084931 },
                                               { 20.2820709312114, 6.01901212028505, 1.09060841213079,  52.9391131775798, 0.405801022380352 },
                                               { 20.7799964962491, 5.55989262144654, 4.09759656595363,  51.5304381400485, 0.397271133035331 },
                                               { 20.9700267187797, 5.38471599746555, 4.57082283155173,  50.9929594705838, 0.394016561573713 },
                                               { 21.0465400507161, 5.31418982057115, 3.91781218973907,  50.7765703493746, 0.392706269838427 },
                                               { 21.2216084157889, 5.15283434294466, 3.57787672291092,  50.2814978593662, 0.3897084786907   },
                                               { 21.3227907132745, 5.05958615128558, 4.20714570764057,  49.9953928252359, 0.387976039187505 },
                                               { 21.3263404769108, 5.05631485187007, 3.92793487321935,  49.9853557916562, 0.387915262369924 }

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
