using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Runtime.InteropServices;

namespace PTDiagCalculator.Test
{
    
    
    /// <summary>
    ///This is a test class for PTDiagramCalculator
    ///</summary>
    [TestClass()]
    public class PTDiagCalculatorTest
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
        // 
        //You can use the following additional attributes as you write your tests:
        //
        //Use ClassInitialize to run code before running the first test in the class
        //[ClassInitialize()]
        //public static void MyClassInitialize(TestContext testContext)
        //{
        //}
        //
        //Use ClassCleanup to run code after all tests in a class have run
        //[ClassCleanup()]
        //public static void MyClassCleanup()
        //{
        //}
        //
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

        [DllImport("EosPack.dll", EntryPoint = "SetPvtPropertiesConfigFile")]
        private static extern void EosPackSetPvtPropertiesConfigFile(string fileName);

        [DllImport("EosPack.dll", EntryPoint = "BuildPTDiagram")]
        private static extern bool BuildPTDiagram( int diagType, double T, double P, [In] double[] comp, [Out] double[] points, 
                                                   [In, Out] int[] szIso, [Out] double[] isolines );

        [TestMethod]
        public void TestPTDiagramCalculator()
        {
           string pvtPropertiesConfigFile = "..\\..\\..\\csharp-test\\PTDiagCalculatorTest.cfg";
           EosPackSetPvtPropertiesConfigFile(pvtPropertiesConfigFile);
           // define composition
           double[] comp = new double[] { 4.63937e+10, 3.76229e+11, 7.53617e+10, 1.60934e+11, 7.01346e+10, 1.93474e+11, 6.20802e+09, 6.26855e+09, 6.3693e+09, 
                                          7.4708e+09, 7.78099e+09, 0, 2.65935e+09, 5.4181e+08, 8.16853e+09, 2.68247e+10, 4.82603e+09, 5.65998e+07, 2.17633e+07,
                                          0, 0, 0, 0 };
           // allocate space for special points
           double[] points = new double[8]; // 4 points - Critical point, Bubble point, Cricondentherm point, Cricondenbar point
           // allocate space for isolines. It will be 11 isolines 0 : 0.1 : 1, up to 400 points (2 double per point) per isoline
           int[] szIso = new int[] { 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400 };
           double[] isolines = new double[11 * 400 * 2];
           // Call diagram builder
           bool ret = BuildPTDiagram(0, 588.15, 12665600, comp, points, szIso, isolines);
           // check results
           Assert.IsTrue( ret, "PTDiagramBuilder failed to create diagram for unknown reason" );
           
           int[] checkSz = new int[] { 65, 55, 55, 54, 57, 60, 69, 77, 87, 93, 125 };
           for ( int i = 0; i < 11; ++i )
           {
              Assert.AreEqual( szIso[i], checkSz[i] );
           }
           Assert.IsTrue(Math.Abs(points[0] - 909.194      ) < 1e-3, "Wrong critical point temperature value " + Convert.ToString(points[0]));
           Assert.IsTrue(Math.Abs(points[1] - 11080924.440 ) < 1e-3, "Wrong critical point pressure value "    + Convert.ToString(points[1]));
           Assert.IsTrue(Math.Abs(points[2] - 588.15       ) < 1e-2, "Wrong bubble point temperature value "   + Convert.ToString(points[2]));
           Assert.IsTrue(Math.Abs(points[3] - 10057971.025 ) < 1e-3, "Wrong bubble point pressure value "      + Convert.ToString(points[3]));
      }
    }
}
