using System.Runtime.InteropServices;

namespace PvtFlash
{
    internal static class EosPack
    {
        /// <summary>
        /// provides interface to PVT-flash functionality of the EosCauldron implementation
        ///
        /// The class EosPack class encapsulates the EosCauldron functionality for PVT-flash
        /// in the member function compute(), which computes from pressure, temperature and
        /// component masses the corresponding phase component masses, densities and viscosities.
        /// In order to get the (only) instance of this class, call member getInstance(), which calls the
        /// constructor only at first time being called, otherwise it only returns a reference
        /// to a static object.(singleton-pattern). During the only 
        /// constructor-execution, the component properties and additional parameters describing 
        /// the equation of state are read from a configuration file. The properties and parameters are
        /// possibly GORM-dependent and therefore stored as piecewise polynomials.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct ComputeStruct
        {
            /// <summary>
            /// Input: Temperature (in K)
            /// </summary>
            public double temperature;
            /// <summary>
            /// Input: Pressure (in Pa)
            /// </summary>
            public double pressure;

            /// <summary>
            /// Input: Masses of feed components (in kg)
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = NPhases)]
            public double[] compMasses;

            /// <summary>
            /// Input: isGormPrescribed if true, a prescribed GORM is passed to the function used to evaluate the 
            ///	GORM-dependent component properties. if false (default) the GORM is computed as 
            /// the ratio (mass C1-C5)/(mass of oil components)
            /// </summary>
            public bool isGormPrescribed;

            /// <summary>
            /// Input: prescribed GORM (only significant when isGormPrescribed is true) 
            /// </summary>
            public double gorm;

            /// <summary>
            /// Output: Masses of components per phase (in kg)
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = NPhases * NUM_COMPONENTS)]
            public double[] phaseCompMasses;

            /// <summary>
            /// Output: Density per phase (in kg/m3)
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = NPhases)]
            public double[] phaseDensity;

            /// <summary>
            /// Output: Viscosity per phase (in Pa*s)
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = NPhases)]
            public double[] phaseViscosity;

            /// <summary>
            /// Creates the struct with the arrays set.
            /// </summary>
            /// <returns></returns>
            public static ComputeStruct New()
            {
                ComputeStruct result = new ComputeStruct();
                result.compMasses = new double[NPhases];
                result.phaseCompMasses = new double[NPhases * NUM_COMPONENTS];
                result.phaseDensity = new double[NPhases];
                result.phaseViscosity = new double[NPhases];
                return result;
            }

            /// <summary>
            /// Access the phaseCompMasses using a specified phase and component
            /// </summary>
            /// <param name="phase"></param>
            /// <param name="component"></param>
            /// <returns></returns>
            public double PhaseCompMasses(PVTPhase phase, ComponentId component)
            {
                return phaseCompMasses[(int)phase * NPhases + (int)component];
            }
        }

        /// <summary>
        /// Computes from pressure, temperature and component masses the corresponding phase component masses, densities and viscosities 
        /// </summary>
        /// <param name="computeInfo"></param>
        /// <returns></returns>
        [DllImport("EosPack.dll", EntryPoint = "EosPackCompute")]
        internal static extern bool Compute(ref ComputeStruct computeInfo);

        internal const int NPhases = 2;
        internal enum PVTPhase
        {
            Vapour = 0,
            Liquid = 1
        };

        internal const int NUM_COMPONENTS = 13;
        internal enum ComponentId
        {
            FIRST_COMPONENT = 0,
            ASPHALTENES = 0,
            RESINS = 1,
            C15_ARO = 2,
            C15_SAT = 3,
            C6_14ARO = 4,
            C6_14SAT = 5,
            C5 = 6,
            C4 = 7,
            C3 = 8,
            C2 = 9,
            C1 = 10,
            COX = 11,
            N2 = 12,
            LAST_COMPONENT = 12,
            NUM_COMPONENTS = 13
        };
    }
}

