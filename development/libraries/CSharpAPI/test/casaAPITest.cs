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
         get
         {
            return testContextInstance;
         }
         set
         {
            testContextInstance = value;
         }
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

      public double m_minTOC =  5.0;
      public double m_midTOC = 10.0;
      public double m_maxTOC = 15.0;

      public double m_minTCHP = 0.1;
      public double m_midTCHP = 2.5;
      public double m_maxTCHP = 4.9;

      public string m_layerName       = @"Lower Jurassic";
      public string m_projectFileName = @"..\..\..\csharp-test\Ottoland.project3d";
      public string m_fileName        = @"Ottoland.project3d";

      public double eps = 1.0e-6;

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
      public void DoE_Tornado_Test() // analog of API/test/DoeTest.C:Tornado2Prms
      {
         ScenarioAnalysis sa = new ScenarioAnalysis();
         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.defineBaseCase(m_projectFileName));
         
         Assert.IsTrue( ErrorHandler.ReturnCode.NoError == sa.setDoEAlgorithm( DoEGenerator.DoEAlgorithm.Tornado ) );
      
         DoEGenerator doe     = sa.doeGenerator();
         VarSpace     varPrms = sa.varSpace();

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == CauldronAPI.VarySourceRockTOC(sa, m_layerName, m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == CauldronAPI.VaryTopCrustHeatProduction(sa, m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         Assert.IsTrue( 2 == varPrms.size() );

         RunCaseSet expSet = sa.doeCaseSet();

         doe.generateDoE( varPrms, expSet );
         Assert.IsTrue( 5 == expSet.size() );

         for ( uint i = 0; i < 5; ++i )
         {
            Assert.IsTrue( 2 == expSet.runCase(i).parametersNumber() );

            Assert.IsTrue( expSet.runCase(i).parameter(0).isDouble() );
            Assert.IsTrue( expSet.runCase(i).parameter(1).isDouble() );

            double val1 = expSet.runCase(i).parameter(0).doubleValue();
            double val2 = expSet.runCase(i).parameter(1).doubleValue();
                        
            switch ( i )
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
      public void Mutator_Test() // analog of API/test/MutatorTest.C
      {   
         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.defineBaseCase(m_projectFileName));
   
         // vary 2 parameters
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == CauldronAPI.VarySourceRockTOC(sa, m_layerName, m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == CauldronAPI.VaryTopCrustHeatProduction(sa, m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         // set up and generate DoE
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         VarSpace   varPrms = sa.varSpace();
         RunCaseSet expSet  = sa.doeCaseSet();
         DoEGenerator doe   = sa.doeGenerator();

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == doe.generateDoE(varPrms, expSet));

         Assert.IsTrue(5 == expSet.size());

         string  pathToCaseSet = @".\CaseSet";

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setScenarioLocation(pathToCaseSet));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.applyMutations(sa.doeCaseSet()));

         pathToCaseSet += @"\Iteration_1";
         for ( uint i = 0; i < sa.doeCaseSet().size(); ++i )
         {
            string casePath = pathToCaseSet;
            
            casePath += @"\Case_" + (i+1).ToString() + @"\" + m_fileName;
            
            // check that all files were generated correctly
            Assert.IsTrue(File.Exists(casePath));

            // get parameters from project file
            Model caseModel = new Model();
            caseModel.loadModelFromProjectFile(casePath);

            PrmTopCrustHeatProduction prm_tchp = new PrmTopCrustHeatProduction(caseModel);
            PrmSourceRockTOC prm_toc = new PrmSourceRockTOC(caseModel, m_layerName);

            // get parameters from case set
            Assert.IsTrue(Math.Abs(sa.doeCaseSet().runCase(i).parameter(0).doubleValue() - prm_toc.doubleValue()) < eps);
            Assert.IsTrue(Math.Abs(sa.doeCaseSet().runCase(i).parameter(1).doubleValue() - prm_tchp.doubleValue()) < eps);
         }
   
         // cleaning files/folders
         Directory.Delete(pathToCaseSet,true); // delete folder ./CaseSet
         Assert.IsFalse(Directory.Exists(pathToCaseSet));
      }

      [TestMethod]
      public void RunManager_Test() // analog of API/test/RunManagerTest.C
      {
         // create new scenario analysis
         ScenarioAnalysis sa = new ScenarioAnalysis();

         Assert.AreEqual(ErrorHandler.ReturnCode.NoError, sa.defineBaseCase(m_projectFileName));
   
         // vary 2 parameters
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == CauldronAPI.VarySourceRockTOC(sa, m_layerName, m_minTOC, m_maxTOC, VarPrmContinuous.PDF.Block));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == CauldronAPI.VaryTopCrustHeatProduction(sa, m_minTCHP, m_maxTCHP, VarPrmContinuous.PDF.Block));

         // set up and generate DoE
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setDoEAlgorithm(DoEGenerator.DoEAlgorithm.Tornado));

         VarSpace   varPrms = sa.varSpace();
         RunCaseSet expSet  = sa.doeCaseSet();
         DoEGenerator doe   = sa.doeGenerator();

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == doe.generateDoE(varPrms, expSet));

         Assert.IsTrue(5 == expSet.size());

         string  pathToCaseSet = @".\CaseSet";

         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.setScenarioLocation(pathToCaseSet));
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError == sa.applyMutations(sa.doeCaseSet()));

         RunManager rm = sa.runManager();
 
         // set up simulation pipeline, the first is fastcauldron
         CauldronApp app = RunManager.createApplication( RunManager.ApplicationType.fastcauldron );
         app.addOption( "-itcoupled" );
         rm.addApplication( app );

         // then set up fastgenex6
         app = RunManager.createApplication( RunManager.ApplicationType.fastgenex6 );
         rm.addApplication( app );

         // and at the end set up track1d
         app = RunManager.createApplication( RunManager.ApplicationType.track1d );
         app.addOption( "-coordinates 460001,6750001" );
         app.addOption( "-age 0.0" );
         app.addOption( "-properties Temperature,Vr,OilExpelledCumulative,HcGasExpelledCumulative,TOC" );
         rm.addApplication( app );

         RunCaseSet caseSet = sa.doeCaseSet();
         for ( uint i = 0; i < sa.doeCaseSet().size(); ++i )
         {
            rm.scheduleCase( caseSet.runCase( i ) );
         }

         // check generated scripts
         pathToCaseSet += @"\Iteration_1";

         for ( uint i = 0; i < sa.doeCaseSet().size(); ++i )
         {
            string casePath = pathToCaseSet;
            
            casePath += @"\Case_" + (i+1).ToString() + @"\";

            for ( int j = 0; j < 3; ++j )
            { 
               string stageFile = casePath + @"Stage_" + j.ToString() + @".sh";
               Assert.IsTrue( File.Exists( stageFile ) );
            }
         }
 
         // cleaning files/folders
         Directory.Delete( pathToCaseSet, true ); // delete folder ./CaseSet
         Assert.IsFalse(Directory.Exists(pathToCaseSet));  
      }
   }
}
