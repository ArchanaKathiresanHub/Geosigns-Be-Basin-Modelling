using Shell.BasinModeling.EOS;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Shell.BasinModeling.EOS.Test
{


   /// <summary>
   ///This is a test class for PTDiagramCalculator
   ///</summary>
   [TestClass()]
   public class PTDiagCalculatorTest
   {
      private TestContext testContextInstance;

      // define composition
      private static double[] m_Composition = { 4.63937e+10, 3.76229e+11, 7.53617e+10, 1.60934e+11, 7.01346e+10, 1.93474e+11, 
                                                6.20802e+09, 6.26855e+09, 6.3693e+09,  7.4708e+09, 7.78099e+09, 0, 2.65935e+09, 
                                                5.4181e+08, 8.16853e+09, 2.68247e+10, 4.82603e+09, 5.65998e+07, 2.17633e+07,
                                                0, 0, 0, 0 };

      private static double[] m_CompositionByOlivie = {  
                                    46393676972.0415, // asphaltene
                                    376228852155.787, // resin
                                    75361672187.7905, // C15PlusAro
                                    160933558529.36,  // C15PlusSat
                                    70134580791.6047, // C6Minus14Aro
                                    193473629375.084, // C6Minus14Sat 
                                    6208020563.40107, // C5
                                    6268552951.99726, // C4
                                    6369301385.92029, // C3
                                    7470802663.62787, // C2
                                    7780993477.8834,  // C1
                                    0.0,              // COx
                                    2659345319.13799, // N2
                                    541810267.726579, // H2S
                                    8168527698.32282, // LSC
                                    26824731512.8973, // C15PlusAT
                                    4826027990.49316, // C6Minus14BT
                                    56599766.62811,   // C6Minus14DBT
                                    21763290.0290721, // C6Minus14BP
                                    0.0,              // C15PlusAroS
                                    0.0,              // C15PlusSatS
                                    0.0,              // C6Minus14SatS
                                    0.0               // C6Minus14AroS
                                 };

      private static double[] m_TrapCond = { 588.15, 12665600 };
      private static int[] m_IsolinesSizes = { 66, 49, 55, 56, 58, 62, 70, 79, 89, 93, 124 };
      private static double[] m_CritPoint = { 909.17673, 10959091.306859 };
      private static double[] m_BubblePoint = { 588.15, 10057971.0253 };
      private static double m_eps = 1e-3;  // threshold for comparison
      private static string m_cfgFile = @".\PTDiagCalculatorTest.cfg";

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
      [ClassInitialize()]
      public static void PTDiagCalculatorInitialize(TestContext testContext)
      {
         TextWriter ofs = new StreamWriter(m_cfgFile);
         ofs.WriteLine("///component based and general data for PVT");
         ofs.WriteLine("///");
         ofs.WriteLine("///This file contains tables describing 6 COMPONENT-based properties and additionally GENERAL properties ");
         ofs.WriteLine("///for PVT (see table headers).");
         ofs.WriteLine("///");
         ofs.WriteLine("///All tables have comma-separated columns. ");
         ofs.WriteLine("///");
         ofs.WriteLine("///COMPONENT TABLES:");
         ofs.WriteLine("///");
         ofs.WriteLine("///Every table with component data contains the component names in the first column followed by the property columns.");
         ofs.WriteLine("///");
         ofs.WriteLine("///The order of the 6 component property columns must be maintained. ");
         ofs.WriteLine("///However, it is possible to split the component tables, i.e. one can use one table containing all component property columns,");
         ofs.WriteLine("///or use up to 6 separate tables.");
         ofs.WriteLine("///");
         ofs.WriteLine("///GENERAL TABLES:");
         ofs.WriteLine("///");
         ofs.WriteLine("///General data must be contained in one table.");
         ofs.WriteLine("///");
         ofs.WriteLine("///All (numerical) data entries to be prescribed (either component or general data) ");
         ofs.WriteLine("///are generally interpreted as a piecewise polynomial depending on the ");
         ofs.WriteLine("///GORM (mass based gas/oil ratio), i.e. every data entry may be either: ");
         ofs.WriteLine("///a constant value,");
         ofs.WriteLine("///a polynomial (e.g. 2.0 + 4.0x^3 + 3.4x),");
         ofs.WriteLine("///a piecewise polynomial with an arbitrary number of semicolon separated pieces, each consisting ");
         ofs.WriteLine("///of a range [x_a:x_b] followed by a constant or polynomial.");
         ofs.WriteLine("///Example:");
         ofs.WriteLine("///[*:1.2]   1.7e-05 ; [1.2:1.8]   -2.7E-05x + 5.1E-05 ;   [1.8:*]   0.0");
         ofs.WriteLine("///(in the above example, the '*'s indicate positive/negative infinity)");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-Component-Properties]");
         ofs.WriteLine("ComponentName,MolecularWeight,AcentricFactor,VCrit");
         ofs.WriteLine("N2,         28.01352	         ,   0.04000   ,   8.980e-02");
         ofs.WriteLine("COx,        44.00980	         ,   0.22500   ,   9.400e-02");
         ofs.WriteLine("C1,         16.04288	         ,   0.00800   ,   9.900e-02");
         ofs.WriteLine("C2,         30.06982	         ,   0.09800   ,   1.480e-01");
         ofs.WriteLine("C3,         44.09676	         ,   0.15200   ,   2.030e-01");
         ofs.WriteLine("C4,         58.12370	         ,   0.19300   ,   2.550e-01");
         ofs.WriteLine("C5,         72.15064	         ,   0.25100   ,   3.040e-01");
         ofs.WriteLine("H2S,        34.08000	         ,   0.10000   ,   9.850e-02");
         ofs.WriteLine("C6-14Sat,   [0.0:2.5]   103.09 -   0.57x  ;   [2.5:*] 101.665,   0.37272   ,   4.7060e-01");
         ofs.WriteLine("C6-14Aro,   [0.0:2.5]   158.47 - 2.0552x  ;   [2.5:*] 153.332,   0.54832   ,   6.7260e-01");
         ofs.WriteLine("C15+Sat,    [0.0:2.5]   281.85 - 17.194x  ;   [2.5:*] 238.865,   0.86828   ,   1.19620e+00");
         ofs.WriteLine("C15+Aro,    [0.0:2.5]   474.52 - 11.129x  ;   [2.5:*] 446.6975,   1.14380   ,   2.12800e+00");
         ofs.WriteLine("resins,     [0.0:2.5]   618.35 - 7.7908x  ;   [2.5:*] 598.873,   1.16772   ,   2.89200e+00");
         ofs.WriteLine("asphaltenes,[0.0:2.5]   795.12 +  2.785x  ;   [2.5:*] 802.0825,   0.89142   ,   3.84400e+00");
         ofs.WriteLine("EndOfTable");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-Component-Properties]");
         ofs.WriteLine("ComponentName,VolumeShift");
         ofs.WriteLine("N2,                           -4.2300000E-03");
         ofs.WriteLine("COx,                          -1.6412000E-03");
         ofs.WriteLine("C1,                           -5.2000000E-03");
         ofs.WriteLine("C2,                           -5.7900010E-03");
         ofs.WriteLine("C3,                           -6.3499980E-03");
         ofs.WriteLine("C4,                           -6.4900000E-03");
         ofs.WriteLine("C5,                           -5.1200000E-03");
         ofs.WriteLine("H2S,                          -3.840000E-03");
         ofs.WriteLine("C6-14Sat,    [0.0:1.242]   1.737999554e-02 ; [1.242:1.8694]   -2.770163E-02x + 5.178542E-02 ;   [1.8694:*]   0.0   ");
         ofs.WriteLine("C6-14Aro,    [0.0:1.242]   2.214000542e-02 ; [1.242:1.8694]   -3.528849E-02x + 6.596831E-02 ;   [1.8694:*]   0.0  ");
         ofs.WriteLine("C15+Sat,     [0.0:1.242]   2.573999668e-02 ; [1.242:1.8694]   -4.102646E-02x + 7.669486E-02 ;   [1.8694:*]   0.0  ");
         ofs.WriteLine("C15+Aro,                                     [0.0:2.5]        -3.687332E-02x + 1.244384E-03 ;   [2.5:*] -9.09389E-02");
         ofs.WriteLine("resins,                                      [0.0:2.5]         -4.229062E-02x - 3.814919E-02;   [2.5:*] -1.43876E-01");
         ofs.WriteLine("asphaltenes,                                 [0.0:2.5]        -5.542006E-02x - 9.001542E-02 ;   [2.5:*] -2.28566E-01");
         ofs.WriteLine("EndOfTable");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-Component-Properties]");
         ofs.WriteLine("ComponentName,CritPressure");
         ofs.WriteLine("N2,            3394387");
         ofs.WriteLine("COx,           7376462");
         ofs.WriteLine("C1,            4600155");
         ofs.WriteLine("C2,            4883865");
         ofs.WriteLine("C3,            4245519");
         ofs.WriteLine("C4,            3799688");
         ofs.WriteLine("C5,            3374122");
         ofs.WriteLine("H2S,           8936864");
         ofs.WriteLine("C6-14Sat,      [0.0:2.0]   2.81e+06 + 8.71e+04x  +  8.24e+05x^2  -  1.15e+06x^3 +  3.68e+05x^4; [2.0:*]  2.97e+06");
         ofs.WriteLine("C6-14Aro,      [0.0:2.0]   2.07e+06 + 2.51e+05x  +  5.79e+05x^2  -  1.07e+06x^3 +  3.85e+05x^4; [2.0:*]  2.49e+06");
         ofs.WriteLine("C15+Sat,       [0.0:2.0]   1.64e+06 - 2.66e+05x  +  1.27e+06x^2  -  1.48e+06x^3 +  4.83e+05x^4; [2.0:*]  2.08e+06");
         ofs.WriteLine("C15+Aro,       [0.0:2.0]   1.34e+06 - 1.85e+05x  +  1.26e+06x^2  -  1.57e+06x^3 +  5.22e+05x^4; [2.0:*]  1.80e+06");
         ofs.WriteLine("resins,        [0.0:2.0]   1.29e+06 - 2.98e+05x  +  1.43e+06x^2  -  1.67e+06x^3 +  5.43e+05x^4; [2.0:*]  1.74e+06");
         ofs.WriteLine("asphaltenes,   [0.0:2.0]   1.25e+06 - 3.43e+05x  +  1.45e+06x^2  -  1.67e+06x^3 +  5.42e+05x^4; [2.0:*]  1.68e+06");
         ofs.WriteLine("EndOfTable");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-Component-Properties]");
         ofs.WriteLine("ComponentName,CritTemperature");
         ofs.WriteLine("N2,             126.200");
         ofs.WriteLine("COx,            304.200");
         ofs.WriteLine("C1,             190.600");
         ofs.WriteLine("C2,             305.400");
         ofs.WriteLine("C3,             369.800");
         ofs.WriteLine("C4,             425.200");
         ofs.WriteLine("C5,             469.600");
         ofs.WriteLine("H2S,            373.200");
         ofs.WriteLine("C6-14Sat,       [0.0:2.0]    582.13 +  15.00x -	160.73x^2 + 189.96x^3 -  61.28x^4 ; [2.0:*]	 508.426");
         ofs.WriteLine("C6-14Aro,       [0.0:2.0]    686.69 -  41.72x -	101.39x^2 + 179.72x^3 -  65.72x^4 ; [2.0:*]	 583.956");
         ofs.WriteLine("C15+Sat,        [0.0:2.0]    857.09 - 121.02x -	 56.60x^2 + 197.60x^3 -  79.33x^4 ; [2.0:*]	 700.218");
         ofs.WriteLine("C15+Aro,        [0.0:2.0]   1037.50 -  81.42x -	208.72x^2 + 351.63x^3 - 123.74x^4 ; [2.0:*]	 872.986");
         ofs.WriteLine("resins,         [0.0:2.0]   1160.30 -  41.11x -	319.82x^2 + 444.11x^3 - 148.68x^4 ; [2.0:*]	 972.8");
         ofs.WriteLine("asphaltenes,    [0.0:2.0]   1351.30 - 206.25x -	105.01x^2 + 341.23x^3 - 136.23x^4 ; [2.0:*]	 1068.92");
         ofs.WriteLine("EndOfTable");
         ofs.WriteLine("");
         ofs.WriteLine("Table:[PVT-General-Properties]");
         ofs.WriteLine("EOS,OmegaA,OmegaB,LBC0,LBC1,LBC2,LBC3,LBC4");
         ofs.WriteLine("PR,   4.57240000E-01,   7.77960000E-02,   1.02300000E-01,   2.33640000E-02,   5.85330000E-02,   -4.07580000E-02,   9.33240000E-03");
         ofs.WriteLine("EndOfTable" );
         ofs.Close();
      }
      //
      //Use ClassCleanup to run code after all tests in a class have run
      [ClassCleanup()]
      public static void PTDiagCalculatorCleanup()
      {
         File.Delete(m_cfgFile);
      }
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

      [TestMethod]
      public void TestPTDiagramCalculatorCAPI()
      {
         EosPackAPI.SetPvtPropertiesConfigFile(m_cfgFile);

         // composition is created as class member array in m_Composition

         // allocate space for special points
         double[] points = new double[8]; // 4 points - Critical point, Bubble point, Cricondentherm point, Cricondenbar point
         // allocate space for isolines. It will be 11 isolines 0 : 0.1 : 1, up to 400 points (2 double per point) per isoline
         int[] szIso = new int[] { 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400 };
         double[] isolines = new double[11 * 400 * 2];
         // Call diagram builder
         bool ret = EosPackAPI.BuildPTDiagram(0, m_TrapCond[0], m_TrapCond[1], m_Composition, points, szIso, isolines);
         // check results
         Assert.IsTrue(ret, "PTDiagramBuilder failed to create diagram for unknown reason");

         for (int i = 0; i < 11; ++i)
         {
            Assert.AreEqual(szIso[i], m_IsolinesSizes[i]);
         }
         Assert.IsTrue(Math.Abs(points[0] - m_CritPoint[0]) < m_eps, "Wrong critical point temperature value " + Convert.ToString(points[0]));
         Assert.IsTrue(Math.Abs(points[1] - m_CritPoint[1]) < m_eps, "Wrong critical point pressure value " + Convert.ToString(points[1]));
         Assert.IsTrue(Math.Abs(points[2] - m_BubblePoint[0]) < m_eps, "Wrong bubble point temperature value " + Convert.ToString(points[2]));
         Assert.IsTrue(Math.Abs(points[3] - m_BubblePoint[1]) < m_eps, "Wrong bubble point pressure value " + Convert.ToString(points[3]));
         
         // search critical point
         for ( int i = 0; i < 8; ++i ) points[i] = 0.0;

         ret = EosPackAPI.FindCriticalPoint( 0, m_Composition, points );
         Assert.IsTrue( ret ); // critical point search shouldn't failed here
         Assert.IsTrue( Math.Abs(points[0] - m_CritPoint[0])   < 1e-3 );// Wrong critical point temperature value
         Assert.IsTrue( Math.Abs(points[1] - m_CritPoint[1])   < 1e-3 );// Wrong critical point pressure value
      }

      [TestMethod]
      public void TestPTDiagramCalculatorObjectOneByOneCountourLine()
      {
         EosPackAPI.SetPvtPropertiesConfigFile(m_cfgFile);

         DoubleVector comp = new DoubleVector(23);
         for (int i = 0; i < 23; ++i)
         {
            comp.Insert(i, m_Composition[i]);
         }

         PTDiagramCalculator diagramBuilder = new PTDiagramCalculator(PTDiagramCalculator.DiagramType.MassFractionDiagram, comp);
         diagramBuilder.setAoverBTerm(2.0);
         diagramBuilder.setNonLinSolverConvPrms(1e-6, 500, 0.3);
         diagramBuilder.findBubbleDewLines(m_TrapCond[0], m_TrapCond[1], new DoubleVector());

         // Check special points
         PairOfDoubles critPt = diagramBuilder.getCriticalPoint();
         Assert.IsTrue(Math.Abs(critPt.first - m_CritPoint[0]) < m_eps, "Wrong critical point temperature value " + Convert.ToString(critPt.first));
         Assert.IsTrue(Math.Abs(critPt.second - m_CritPoint[1]) < m_eps, "Wrong critical point pressure value " + Convert.ToString(critPt.second));
         double[] bubbleP = new double[1];
         Assert.IsTrue(diagramBuilder.getBubblePressure(m_BubblePoint[0], bubbleP));
         Assert.IsTrue(Math.Abs(bubbleP[0] - m_BubblePoint[1]) < m_eps, "Wrong bubble point pressure value " + Convert.ToString(bubbleP[0]));

         for (int i = 0; i < 11; ++i)
         {
            PairOfDoublesVector isoLine = diagramBuilder.calcContourLine(i * 0.1);
            Assert.AreEqual(m_IsolinesSizes[i], isoLine.Count);
         }
      }

      [TestMethod]
      public void TestPTDiagramCalculatorObjectAllCountourLinesInOneGo()
      {
         EosPackAPI.SetPvtPropertiesConfigFile(m_cfgFile);

         DoubleVector comp = new DoubleVector(23);
         for (int i = 0; i < 23; ++i)
         {
            comp.Insert(i, m_Composition[i]);
         }

         PTDiagramCalculator diagramBuilder = new PTDiagramCalculator(PTDiagramCalculator.DiagramType.MassFractionDiagram, comp);
         diagramBuilder.setAoverBTerm(2.0);
         diagramBuilder.setNonLinSolverConvPrms(1e-6, 500, 0.3);
         diagramBuilder.findBubbleDewLines(m_TrapCond[0], m_TrapCond[1], new DoubleVector());

         int[] realSz = new int[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
         DoubleVector values = new DoubleVector(11);
         for (int i = 0; i < 11; ++i)
         {
            values.Insert(i, 0.1 * i);
         }

         DoubleVector isoLines = diagramBuilder.calcContourLines(values);

         int isoline = 0;
         for (int i = 0; i < isoLines.Count; ++i)
         {
            if (isoLines[i] < 0 && isoLines[i + 1] < 0)
            {
               isoline++;
            }
            else
            {
               realSz[isoline] += 1;
            }
            ++i;
         }

         for (int i = 0; i < 11; ++i)
         {
            Assert.AreEqual(realSz[i], m_IsolinesSizes[i]);
         }
      }

      [TestMethod]
      public void TestPTDiagramCalculatorTuneAB()
      {
         DoubleVector comp = new DoubleVector(23);
         for (int i = 0; i < 23; ++i)
         {
            comp.Insert(i, m_CompositionByOlivie[i]);
         }

         PTDiagramCalculator diagramBuilder = new PTDiagramCalculator( PTDiagramCalculator.DiagramType.MoleMassFractionDiagram, comp );
         diagramBuilder.setAoverBTerm( 2.0 );
         diagramBuilder.setNonLinSolverConvPrms( 1e-6, 400, 0.2 );
         double AB = diagramBuilder.searchAoverBTerm();

         Assert.IsTrue( Math.Abs( AB - 3.76929 ) < m_eps );
      }

      [TestMethod]
      public void TestPTDiagramCalculatorFastSearchCritPt()
      {
         DoubleVector comp = new DoubleVector(23);
         for (int i = 0; i < 23; ++i)
         {
            comp.Insert(i, m_CompositionByOlivie[i]);
         }

         PTDiagramCalculator diagramBuilder = new PTDiagramCalculator( PTDiagramCalculator.DiagramType.MoleMassFractionDiagram, comp );
         diagramBuilder.setAoverBTerm( 2.0 );
         diagramBuilder.setNonLinSolverConvPrms( 1e-6, 400, 0.2 );

         PairOfDoubles critPt = diagramBuilder.searchCriticalPoint();
         double critT = critPt.first;
         double critP = critPt.second;
         Assert.IsTrue( Math.Abs( critT - 903.652 ) < m_eps );
         Assert.IsTrue( Math.Abs( critP - 11130482.494 ) < m_eps );
      }
   }
}
