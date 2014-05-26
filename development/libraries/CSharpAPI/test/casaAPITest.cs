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

      #region Additional test attributes

      // Use ClassInitialize to run code before running the first test in the class
      [ClassInitialize()]
      public static void cusaAPIInitialize(TestContext testContext)
      {
      }

      // Use ClassCleanup to run code after all tests in a class have run
      [ClassCleanup()]
      public static void cuasAPICleanup()
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
      public void DoE_Tornado_Test()
      {
         ScenarioAnalysis sa = new ScenarioAnalysis();
         sa.defineBaseCase( @"Project.project3d" );
         
         Assert.IsTrue( ErrorHandler.ReturnCode.NoError == sa.setDoEAlgorithm( DoEGenerator.DoEAlgorithm.Tornado ) );
      
         DoEGenerator doe     = sa.doeGenerator();
         VarSpace     varPrms = sa.varSpace();

         Assert.IsTrue( ErrorHandler.ReturnCode.NoError == 
                        CauldronAPI.VariateSourceRockTOC(sa.baseCase(), "Layer1", 10, 40, VarPrmContinuous.PDF.Block, varPrms) );
         Assert.IsTrue(ErrorHandler.ReturnCode.NoError ==
                        CauldronAPI.VariateTopCrustHeatProduction(sa.baseCase(), 0.1, 4.0, VarPrmContinuous.PDF.Block, varPrms));

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
            
            double eps = 1.0e-6;
            
            switch ( i )
            {
               case 0: 
                  Assert.IsTrue( Math.Abs( val1 - 25.0 ) < eps ); 
                  Assert.IsTrue( Math.Abs( val2 - 2.05 ) < eps ); 
                  break;
               case 1: 
                  Assert.IsTrue( Math.Abs( val1 - 10.0 ) < eps ); 
                  Assert.IsTrue( Math.Abs( val2 - 2.05 ) < eps ); 
                  break;
               case 2: 
                  Assert.IsTrue( Math.Abs( val1 - 40.0 ) < eps ); 
                  Assert.IsTrue( Math.Abs( val2 - 2.05 ) < eps ); 
                  break;
               case 3: 
                  Assert.IsTrue( Math.Abs( val1 - 25.0 ) < eps ); 
                  Assert.IsTrue( Math.Abs( val2 - 0.1  ) < eps ); 
                  break;
               case 4: 
                  Assert.IsTrue( Math.Abs( val1 - 25.0 ) < eps ); 
                  Assert.IsTrue( Math.Abs( val2 - 4.0  ) < eps ); 
                  break;
            }
         }
      }
   }
}
