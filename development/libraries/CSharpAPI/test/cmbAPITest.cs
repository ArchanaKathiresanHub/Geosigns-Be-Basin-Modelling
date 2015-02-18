using Shell.BasinModeling.Cauldron;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Shell.BasinModeling.Cauldron.Test
{
   /// <summary>
   ///This is a test class for cmbAPI
   ///</summary>
   [TestClass()]
   public class cmbAPITest
   {
      private TestContext testContextInstance;

      private const double eps = 1e-5;

      /// <summary>
      ///Gets or sets the test context which provides
      ///information about and functionality for the current test run.
      ///</summary>
      public TestContext TestContext
      {
         get { return testContextInstance;  }
         set { testContextInstance = value; }
      }

      private void logMsg( string msg )
      {
         using (System.IO.StreamWriter file = new System.IO.StreamWriter(@".\logfile.txt", true))
         {
            file.WriteLine(msg);
         }
      }
 
      public string m_projectFileName = @"..\..\..\..\..\Project.project3d";
      public string m_ottolandProject = @"..\..\..\..\..\Ottoland.project3d";

      private bool compareFiles( string projFile1, string projFile2 )
      {
         FileInfo f1 = new FileInfo(projFile1);
         FileInfo f2 = new FileInfo(projFile2);

         if ( f1.Length != f2.Length ) return false;

         string file1Content = File.ReadAllText(projFile1);
         string file2Content = File.ReadAllText(projFile2);

         string[] file1Lines = file1Content.Split('\n');
         string[] file2Lines = file2Content.Split('\n');

         if ( file1Lines.Length != file2Lines.Length ) return false;

         for ( int i = 2; i < file1Lines.Length; ++i )
         {
            if ( file1Lines[i] != file2Lines[i] ) return false;
         }
         return true;
      }
 
      private bool compareModels( Model model1, Model model2 )
      {
         string projFile1 = @".\project_model_1.project3d";
         string projFile2 = @".\project_model_2.project3d";
   
         model1.saveModelToProjectFile( projFile1 );
         model2.saveModelToProjectFile( projFile2 );

         bool isTheSame = compareFiles( projFile1, projFile2 );

         // clean files copy
         File.Delete( projFile1 );
         File.Delete( projFile2 );

         return isTheSame;
      }

      #region Additional test attributes

      // Use ClassInitialize to run code before running the first test in the class
      [ClassInitialize()]
      public static void cmbAPIInitialize(TestContext testContext)
      {
      }

      // Use ClassCleanup to run code after all tests in a class have run
      [ClassCleanup()]
      public static void cmbAPICleanup()
      {
      }

      //Use TestInitialize to run code before running each test
      [TestInitialize()]
      public void MyTestInitialize()
      {
         if (System.Diagnostics.Debugger.IsAttached)
         {
            // for debug run
            m_projectFileName = @"d:\cauldron\cld-dev-64\Project.project3d";
            m_ottolandProject = @"d:\cauldron\cld-dev-64\Ottoland.project3d";
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
      public void TestCMB_API_LoadFile()
      {
         string fileName = m_projectFileName;
         Model cmb = new Model();
         Model.ReturnCode ret = cmb.loadModelFromProjectFile(fileName);
         Assert.AreEqual( ret, Model.ReturnCode.NoError );
      }

      [TestMethod]
      public void TestCMB_API_ModelLoadSaveProjectRoundTrip()
      {
         string fileName = m_projectFileName;
         Model cmb = new Model();
         Model.ReturnCode ret = cmb.loadModelFromProjectFile(fileName);
         Assert.AreEqual(ret, Model.ReturnCode.NoError);

         //logMsg(cmb.errorMessage());

         string newFileName1 = @".\Project_tmp1.project3d";
         ret = cmb.saveModelToProjectFile( newFileName1 );
         Assert.AreEqual(ret, Model.ReturnCode.NoError);

         Model anotherModel = new Model();
         ret = anotherModel.loadModelFromProjectFile(newFileName1);
         Assert.AreEqual(ret, Model.ReturnCode.NoError);

         Assert.IsTrue( compareModels( cmb, anotherModel ) );
         
         File.Delete( newFileName1 );
      }
      
      [TestMethod]
      public void TestCMB_API_BasementPropertySetting()
      {
         string fileName = m_projectFileName;
         Model testModel = new Model();
         
         // load project file
         Model.ReturnCode ret = testModel.loadModelFromProjectFile( fileName );
         Assert.AreEqual(ret, Model.ReturnCode.NoError);

         // check that BasementIoTable has only 1 record       
         Assert.AreEqual( testModel.tableSize( "BasementIoTbl" ), 1 );

         // Get bottom boundary condition model name. Must be "Fixed Temperature"
         string modelName = testModel.tableValueAsString( "BasementIoTbl", 0, "BottomBoundaryModel" );
         Assert.AreEqual( Model.ReturnCode.NoError, testModel.errorCode() );
         Assert.IsTrue( modelName == "Fixed Temperature" );

         // Get value for top crust heat production rate
         double topCrustHeatProd = testModel.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" );
         Assert.AreEqual( Model.ReturnCode.NoError, testModel.errorCode() );
         // must be 2.5
         Assert.IsTrue( Math.Abs( topCrustHeatProd - 2.5) < eps );

         // change it value to 4.0
         Assert.AreEqual( Model.ReturnCode.NoError, testModel.setTableValue( "BasementIoTbl", 0, "TopCrustHeatProd", 4.0 ) );
         // and save as a new project
         Assert.AreEqual( Model.ReturnCode.NoError, testModel.saveModelToProjectFile( "Project_basem_prop.project3d" ) );

         // reload project to another model
         Model modifModel = new Model();
         Assert.AreEqual( Model.ReturnCode.NoError, modifModel.loadModelFromProjectFile( "Project_basem_prop.project3d" ) );

         // get value of top crust production rate
         topCrustHeatProd = modifModel.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" );
         Assert.AreEqual( Model.ReturnCode.NoError, modifModel.errorCode() );
         // must be 4.0 (as it was set before)
         Assert.IsTrue( Math.Abs( topCrustHeatProd - 4.0) < eps );

         // delete copy of the project
         File.Delete( "Project_basem_prop.project3d" );
      }

      // Test set/get TOC in source rock lithology
      [TestMethod]
      public void TestCMB_API_SourceRockTOCSettings()
      {
         string fileName = m_ottolandProject;
         Model testModel = new Model();
   
         // load test project
         Assert.AreEqual(Model.ReturnCode.NoError, testModel.loadModelFromProjectFile(fileName));

         SourceRockManager srMgr = testModel.sourceRockManager();

         // get current values for TOC. Test project has 2 source rock lithologies with TOC 70.2 & 10.0
         double toc1st = srMgr.tocIni( 0 );
         Assert.AreEqual( Model.ReturnCode.NoError, srMgr.errorCode() );

         Assert.IsTrue( Math.Abs( toc1st - 70.2 ) < eps );
         
         toc1st = srMgr.tocIni( 1 );
         Assert.AreEqual( Model.ReturnCode.NoError, srMgr.errorCode() );
         Assert.IsTrue( Math.Abs( toc1st - 10.0 ) < eps );

         // change TOC values to some others - 19 & 70
         Assert.AreEqual( Model.ReturnCode.NoError, srMgr.setTOCIni( "Lower Jurassic", 19.0 ) );
         Assert.AreEqual( Model.ReturnCode.NoError, srMgr.setTOCIni( "Westphalian", 70.0 ) );
      
         // save as a new temporary project file
         Assert.AreEqual( Model.ReturnCode.NoError, testModel.saveModelToProjectFile( "Ottoland_changedTOC.project3d" ) );

         // reload this new project file into other model
         Model modifModel = new Model();
         Assert.AreEqual( Model.ReturnCode.NoError, modifModel.loadModelFromProjectFile( "Ottoland_changedTOC.project3d" ) );
   
         SourceRockManager srModMgr = modifModel.sourceRockManager();

         // check values for the TOC
         toc1st = srModMgr.tocIni( 0 );
         Assert.AreEqual( Model.ReturnCode.NoError, srModMgr.errorCode() );
         Assert.IsTrue( Math.Abs( toc1st - 70.0 ) < eps );
   
         toc1st = srModMgr.tocIni( 1 );
         Assert.AreEqual(Model.ReturnCode.NoError, srModMgr.errorCode() );
         Assert.IsTrue( Math.Abs( toc1st - 19.0 ) <  eps );

         // delete temporary project file
         File.Delete( "Ottoland_changedTOC.project3d" );
      }
   }
}
