#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
namespace Genex6
{
   namespace Constants
   {
      const int SIMOTGC   = 0x0001;
      const int SIMGENEX  = 0x0002;
      const int SIMGENEX5 = 0x0004;
      const int SIMOTGC5  = 0x0008;
      const int SIMTSR    = 0x0010;

      const double s_undefinedValue = 9999.0;
      enum {TOTAL_OM, AROMATIC_IMMOBILE_OM, MOBILE_OM, NUMBER_OF_ORGANIC_MATTER_TYPES};
      //const double s_asphalteneRatio= 2.5; //purely empirical; values around 2 to 3 satis
      //const double s_BiotGeo = 10.0;
      //const double s_CharLengthGeo= 0.5;
      //'=old Biot Number/(Characteristic Length)^2
      //const double s_BiotOverL2Geo=s_BiotGeo/(s_CharLengthGeo *s_CharLengthGeo);   
      //const double s_BiotOverL2=s_BiotOverL2Geo;
      const double s_Boltzmann=1.380658E-23; 
      //const double s_Planck=6.6260755E-34;
      //const double SECONDS_PER_YEAR=1.0*60.0*60.0*24.0*365.0*1000000.0;
      //const double s_BoltzmannOverPlanck=s_Boltzmann*SECONDS_PER_YEAR/s_Planck;
      const double s_BoltzmannOverPlanck = 6.571073735576960e+23; 
      const double s_ConvGOR= 5.614584; //  was  5.614599999999

      const double s_CalF= 5.1;
      const double s_CalA = 0.0821;
      const double s_EcalB = 0.53526;
      //const double s_TCref = 105.0;
      const double s_LopDivisor = 10.0;

      //amazingly, this very small number makes a very big difference to expulsion at very high VRE
      //const double s_WboMin= 0.000000001;
      //const double s_TuningConst=50.0;

      //now put at top of temperature range of lab experiments with torbanite (200 to 300C)
      //const double Tlab = 573.0; 
      //--------------------diffusion state theory constants
      const double R = 8.314511;
      //const double Uj= 25000.0;
      //const double T0torbanite= 106.0;//pseudoactivation energy of free volume term for diffusion
      //                                //based on lab-measured apparent activation energy
      //const double T0aromatic= 240.0;
      const double VAN_KREVELEN_HC_CORRECTOR=0.5; 

      //const double s_HCmin= 0.7;
      //const double s_HCmax= 1.8;

      const double s_ActUMax = 1000000000000.0; 
      const double s_CminHC  = 0.9999999; 
      const double s_HminHC  = 0.0;
      const double s_DensMaxGas = 10.0;
      //const double s_OmaxHC  = 0.3;

      /// \brief Fraction of time-step that is used to determine closeness to a snapshot.
      ///
      /// If the next time-step is close to a snapshot then take the snapshot time 
      /// rather than perform another time-step with a possibly very small (O(1.0e-13)) time-step size.
      const double TimeStepFraction = 0.001;

      const double s_TCabs=273.15; //Kelvin transformation, input in C //

      /// \brief Standard temperature conditions.
      const double StandardTemperature = 15.5555556 + s_TCabs; //Kelvin

      /// \brief Standard pressure conditions.
      const double StandardPressure    = 101325.353; //Pa


      //const double s_BetaOverAlpha = 0.00000056;  //Dlubek et al 2004
      //new changes of activation entropy with coking, seems to much improve fit to lab data
      //const double s_dSperCoke = -150.0; 

      const double s_grav  = 9.807; 
      const double s_patmos = 100000.0;

      //provisional constants
      //constants for OC of preasphaltene as function of HC of preasphaltene
      //const double OCpreasphalt1= 0.083333;
      //const double OCpreasphalt2= -0.36667;
      //const double OCpreasphalt3= 0.4;

      //constants for HC of kerogen as function of HC of preasphaltene
      //const double HCkerogen1= 1.06;
      //const double HCkerogen2= 0.07;

      //constants for OC of kerogen as function of OC of preasphaltene
      //const double OCkerogen1= 1.68;
      //const double OCkerogen2= 0.03;

      //fractions of N in species relative to precursors
      //const double Nkerogen= 0.1;
      //const double Npreasphalt= 0.96;
      //const double Nasphaltene= 0.8;
      //const double Nresin= 0.9;
      //const double Nprecoke= 0.66667;
      //const double Nhetero1= 4; //1.6667
      //const double Nhetero2= 0.001;
      //const double NC15plusAro= 0.004;

      //---------------formula of asphaltene as f(preasphalt), (e.g. Durand-Souron '82)

      //const double HCAsphOverPreasphalt = 1.0;
      //const double OCAsphOverPreasphalt = 0.6;
      //const double OCasphMin = 0.001;

      //const double OCprecokeWhenHCpreasphaltZero = 0.09;
      //const double OCprecokePerPreasphalt = 0.008;

      //Adjust Activation energies for Preasphaltene, Asphaltene and Resin
      //Hardwired adjustment parameters + GUI parameter--> m_Emean;
      const double convert2Joule=1000.0;
      const double convertMpa2Pa = 1.0E+6;
      //use this set to e.g. 20 to effectively increase Ediff1 for S-rich kerogen
      //const double EdropForS = 0.0;               
      //const double EcrackingCC = 217.0*convert2Joule; //effective value for carbon-carbon bond cracking
      //Ediff1 should be increased to handle sulphur-rich kerogen(e.g. Type IIS)
      //const double Ediff1 = 10.0*convert2Joule;//differences in activation energies between preasphaltene and asphaltene
      //const double Ediff2 = 7.0*convert2Joule;

      //const double STABILITY_FACTOR=0.002;//Species::ComputeAromaticity has a branch that sets aromaticity=1.0!!!

      //const double s_TauInitial = 0.0000017185;

      const double UNDEFINEDVALUE = 99999.0;
      const double GOR_UPPERBOUND = 100000.0;
      //FunApi(DensOil)=APIC1/DensOil*APIC2-APIC3;
      const double APIC1=141.5;
      const double APIC2=1000.0; 
      const double APIC3=131.5; 

       const double ZERO=0.000001;
      // if(FlxvOil > 1E-39)
      const double FLXVOILZERO=1E-39;
      //If CumvOil > 9.999946E-41
      const double CUMVOILZERO=9.999946E-41;
      const double CONVERSIONTOOILZERO= 1E-41;

      //formula volume, Van Krevelen, eqn. XVI,6, p.317
      //FormVol = FormVol1 + FormVol2 * Atom(IatomH, L) + FormVol3 * Atom(IatomO, L) - VolRing * Rc
      const double  FormVol1  = 9.9;
      const double  FormVol2  = 3.1;
      const double  FormVol3  = 3.75;
      //volume of rings, Van Krevelen and Chermin '54, VK eqn. XVI,4, p.317
      //VolRing = VolRing1 + VolRing2 * Atom(IatomH, L)
      const double VolRing1  = 9.1;
      const double VolRing2  = -3.65;

      //minimum preasphaltene aromaticity now set to 0.2 rather than zero (max H/Ci that code can handle is now hcmax)
      //const double AromMin = 0.2;
      //const double AromMax = 0.9;
      //apparent reaction order of preasphaltene as a function of
      //its atomic H/C ratio.
      //N.b. no longer relative to kerogen H/C, a la Delvaux 1989.
      //const double Order0= 4.75;
      //const double OrderPerHoverC= -1.875;

      //const double ActCrit=0.0;

      const double VRE1=0.4;
      const double VRE2=0.5;
      const double VRE3=0.6;
      const double VRE4=0.7;
      #ifdef WIN32
         const std::string folder_divider = "\\";
      #else // !WIN32
         const std::string folder_divider = "/";
      #endif // WIN32

      /// \brief Conversion factor for cubic-metres to barrels.
      ///
      /// Barrels per cubic-metre.
      const double CubicMetresToBarrel = 6.2898107704; 


      /// \brief The volume of a mole of methane at surface conditions (mol/m^3).
      const double VolumeMoleMethaneAtSurfaceConditions = 42.306553;

      /// \brief Conversion factor for cubic-metres to cubic-feet.
      ///
      /// Cubic-feet per cubic-metre.
      const double CubicMetresToCubicFeet = 35.3146667;

      /// \brief Conversion factor for kg to US ton.
      const double KilogrammeToUSTon = 1.0 / 907.18474;

      /// \brief Conversion factor for km^2 to acres.
      const double AcresPerKilometreSquared = 247.10538147;

      /// \brief Conversion of gas-oil ratio to field units.
      ///
      /// Standard cubic-feet per barrel.
      const double GorConversionFactor = CubicMetresToCubicFeet / CubicMetresToBarrel;

      /// \brief Conversion of condensate-gas ratio to field units.
      ///
      /// Barrels per thousand cubic-feet.
      const double CgrConversionFactor = CubicMetresToBarrel / ( 1.0e-3 * CubicMetresToCubicFeet );



   }
   namespace CFG
   {
      const std::string EndOfTable = "[EndOfTable]";
      const std::string TableSourceRockProperties = "Table:[SourceRockProperties]";

      //Table:SimulatorProperties
      const std::string TableSimulatorProperties = "Table:[SimulatorProperties]";
      const std::string PreprocessSpeciesKinetics = "PreprocessSpeciesKinetics";
      const std::string PreprocessSpeciesComposition = "PreprocessSpeciesComposition";
      const std::string UseDefaultGeneralParameters = "UseDefaultGeneralParameters";
      const std::string NumberOfTimesteps = "NumberOfTimesteps";
      const std::string SimulateOpenConditions = "SimulateOpenConditions";
      const std::string MaximumTimeStepSize = "MaximumTimeStepSize";
      const std::string MassBalancePercentTolerance = "MassBalancePercentTolerance";

      //Table:Elements
      const std::string TableElements = "Table:[Elements]";
      const std::string TableSpecies = "Table:[Species]";
      const std::string SpeciesCompositionByName = "Table:[SpeciesCompositionByName]";
      const std::string SpeciesPropertiesByName = "Table:[SpeciesPropertiesByName]";
      const std::string ReactionsBySpeciesName = "Table:[ReactionsBySpeciesName]";
      const std::string ReactionRatiosBySpeciesName = "Table:[ReactionRatiosBySpeciesName]";
      const std::string GeneralParameters = "Table:[GeneralParameters]";
   }
}
#endif // CONSTANTS_H
