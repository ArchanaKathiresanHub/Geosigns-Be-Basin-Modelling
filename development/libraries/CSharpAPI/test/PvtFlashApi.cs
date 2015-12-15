/********************************************************************
 * 
 * Copyright (C) 2010 Shell International Exploration & Production.
 * All rights reserved.
 * 
 * Developed under licence for Shell by PDS BV.
 * 
 * Confidential and proprietary source code of Shell.
 * Do not distribute without written permission from Shell.
 * 
 ********************************************************************/

using System;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

using Shell.BasinModeling.Cauldron;
using Shell.Ocean.Petrel.BasinModeling.BPACauldronLink.Data;


namespace Shell.Ocean.Petrel.BasinModeling.BPACauldronLink.Util
{
    /// <summary>
    /// provides interface to PVT-flash functionality of the EosCauldron implementation
    ///
    /// The class EosPack class encapsulates the EosCauldron functionality for PVT-flash
    /// in the member function compute(), which computes from Pressure, Temperature and
    /// component masses the corresponding phase component masses, densities and viscosities.
    /// In order to get the (only) instance of this class, call member getInstance(), which calls the
    /// constructor only at first time being called, otherwise it only returns a reference
    /// to a static object.(singleton-pattern). During the only 
    /// constructor-execution, the component properties and additional parameters describing 
    /// the equation of state are read from a configuration file. The properties and parameters are
    /// possibly GORM-dependent and therefore stored as piecewise polynomials.
    /// </summary>
    internal static class EosPack
    {
        #region Fields
        internal const int NPhases = 2;
        internal const int NComponents = 23;
        #endregion

        #region Methods
        /// <summary>
        /// Computes from Pressure, Temperature and component masses the corresponding phase component masses, densities and viscosities 
        /// </summary>
        /// <param name="computeInfo"></param>
        /// <returns></returns>
        [DllImport("EosPack.dll", EntryPoint = "EosPackComputeWithLumping")]
        internal static extern bool ComputeWithLumping(ref ComputeStruct computeInfo);

        [SuppressMessage("Microsoft.Globalization", "CA2101")]
        [DllImport("EosPack.dll", EntryPoint = "SetPvtPropertiesConfigFile")]
        internal static extern bool SetPvtPropertiesConfigFile(string fileName);

        [DllImport("EosPack.dll", EntryPoint = "GetMolWeight")]
        internal static extern double GetMolWeight(int componentId, double gorm);

        [SuppressMessage("Microsoft.Globalization", "CA2101")]
        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        private static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("EosPack.dll", EntryPoint = "Gorm")]
        internal static extern double Gorm(ref ComputeStruct computeInfo);

        #endregion

        #region Constructor
        static EosPack()
        {
            LoadLibrary("EosPack.dll");
            SetPvtPropertiesConfigFile("..\\..\\..\\csharp-test\\PTDiagCalculatorTest.cfg");
            return; 

            DirectoryInfo directory = new DirectoryInfo(Path.Combine(
                new FileInfo(typeof(EosPack).Assembly.Location).Directory.FullName,
                @"Lib\BasinModeling\"));
            
            if (directory.Exists == false)
            {
                // If the directory does not exists, swith to the installation directory.
                directory = new FileInfo(typeof(EosPack).Assembly.Location).Directory;
            }

            FileInfo cSharpAPIFileInfo = new FileInfo(
                Path.Combine(
                    directory.FullName,
                    @"Cauldron.dll"));

            FileInfo pvtPropertiesFileInfo = new FileInfo(
                Path.Combine(
                    directory.FullName,
                    @"PVT_properties.cfg"));

            if (directory.Exists == false)
            {
                System.Console.WriteLine( "Could not resolve PVT-Flash library directory " + directory.FullName);
            }
            else
            {
                Environment.SetEnvironmentVariable("PATH", string.Format("{0};{1}", directory.FullName, Environment.GetEnvironmentVariable("PATH")));
                
                // Load the library
                IntPtr cSharpAPILibrary = LoadLibrary(cSharpAPIFileInfo.FullName);
                if (cSharpAPILibrary == IntPtr.Zero)
                {
                    System.Console.WriteLine("Could not resolve PVT-Flash library " + cSharpAPIFileInfo.FullName);
                    return;
                }

                if (pvtPropertiesFileInfo.Exists == false)
                {
                    System.Console.WriteLine("Could not resolve PVT-Flash configuration file " + pvtPropertiesFileInfo.FullName);
                    return;
                }

                SetPvtPropertiesConfigFile(pvtPropertiesFileInfo.FullName);
            }
        }
        #endregion

        #region Internal Types
        [StructLayout(LayoutKind.Sequential)]
        public struct ComputeStruct
        {
            #region Fields
            /// <summary>
            /// Input: Temperature (in K)
            /// </summary>
            public double Temperature;

            /// <summary>
            /// Input: Pressure (in Pa)
            /// </summary>
            public double Pressure;

            /// <summary>
            /// Input: Masses of feed components (in kg)
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.R8, SizeConst = NComponents)]
            public double[] CompMasses;

            /// <summary>
            /// Output: Masses of components per phase (in kg)
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.R8, SizeConst = NPhases * NComponents)]
            public double[] PhaseCompMasses;

            /// <summary>
            /// Output: Density per phase (in kg/m3)
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.R8, SizeConst = NPhases)]
            public double[] PhaseDensity;

            /// <summary>
            /// Output: Viscosity per phase (in Pa*s)
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.R8, SizeConst = NPhases)]
            public double[] PhaseViscosity;

            /// <summary>
            /// Input: IsGormPrescribed if true, a prescribed GORM is passed to the function used to evaluate the 
            ///	GORM-dependent component properties. if false (default) the GORM is computed as 
            /// the ratio (mass C1-C5)/(mass of oil components)
            /// </summary>
            public bool IsGormPrescribed;

            /// <summary>
            /// Input: prescribed GORM (only significant when IsGormPrescribed is true) 
            /// </summary>
            public double Gorm;
            #endregion

            #region Methods
            /// <summary>
            /// Creates the struct with the arrays set.
            /// </summary>
            /// <returns></returns>
            public static ComputeStruct New()
            {
                ComputeStruct result = new ComputeStruct();
                result.CompMasses = new double[NComponents];
                result.PhaseCompMasses = new double[NPhases * NComponents];
                result.PhaseDensity = new double[NPhases];
                result.PhaseViscosity = new double[NPhases];
                return result;
            }

            /// <summary>
            /// Access the PhaseCompMasses using a specified phase and component
            /// </summary>
            /// <param name="phase"></param>
            /// <param name="component"></param>
            /// <returns></returns>
            public double GetParticularPhaseCompMasses(BpaPhase phase, BpaComponent component)
            {
                return PhaseCompMasses[(int)phase * NComponents + (int)component];
            }
            #endregion
        }
        #endregion
    }
}

