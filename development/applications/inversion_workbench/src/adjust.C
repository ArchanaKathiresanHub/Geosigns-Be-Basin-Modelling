#include <iostream>
#include <vector>
#include <typeinfo>
#include <iterator>
#include <algorithm>
#include <cmath>

#include "project.h"
#include "optionparser.h"
#include "Utils.h"

#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"


const std::string basementParameters[] 
  = { "TopAsthenoTemp", "Temperature at the top of the asthenospheric mantle "
                        "(bottom of lithospheric mantle) i.e. melting "
                        "temperature."
    , "TopCrustHeatProd", "The surface Radiogenic heat production of the basement"
    , "CrustHeatPDecayConst", "The decay constant of the surface Radiogenic heat "
                              "production of the basement"
    , "LithoMantleThickness", "Lithospheric Mantle thickness"
    , "InitialLthMntThickns", "Initial Litho Mantle thickness"
    , "FixedCrustThickness", "Fixed Crust Thickness"
    } ;


const std::string lithotypes[] 
  = {"*", "All lithogies" 
    ,"Std. Sandstone" ,     "Standard Sandstone"
    ,"SM. Sandstone",       "Soil Mechanics Sandstone"
    ,"Std. Shale",          "Standard Shale"
    ,"SM.Mudst.40%Clay",    "Soil Mechanics Mudstone 40% Clay"
    ,"SM.Mudst.50%Clay",    "Soil Mechanics Mudstone 50% Clay"
    ,"SM.Mudst.60%Clay",    "Soil Mechanics Mudstone 60% Clay"
    ,"Std. Siltstone",      "Standard Siltstone"
    ,"Std.Grainstone",      "Standard Grainstone (Limestone)"
    ,"Std.Dolo.Grainstone", "Standard Domlomitic Grainstone (Dolostone)"
    ,"Std.Lime Mudstone",   "Standard Calcareous Mudstone (Limestone)"
    ,"Std.Dolo.Mudstone",   "Standard Dolomitic Mudstone (Dolostone)"
    ,"Std. Chalk",          "Standard Chalk"
    ,"Std. Marl",           "Standard Marl"
    ,"Std. Anhydrite",      "Standard Anhydrite"
    ,"Std. Salt",           "Standard Salt"
    ,"Sylvite",             "Sylvite"
    ,"Std. Coal",           "Standard Coal"
    ,"Std. Basalt",         "Standard Basalt"
    ,"Crust",               "Standard Crust"
    ,"Litho. Mantle",       "Lithospheric Mantle"
    ,"Astheno. Mantle",     "Asthenospheric Mantle"
    ,"HEAT Sandstone",      "Sandstone as used by IBS-HEAT"
    ,"HEAT Shale",          "Shale as used by IBS-HEAT"
    ,"HEAT Limestone",      "Limestone as used by IBS-HEAT"
    ,"HEAT Dolostone",      "Dolostone as used by IBS-HEAT"
    ,"HEAT Chalk",          "Chalk as used by IBS-HEAT"
    } ;

const std::string lithotypeProperties[]
  = { "CompacCoefES", "Vertical Effective Stress compaction coefficient"
    , "CompacCoefSC", "Sclater & Christie compaction coefficient"
    , "CompacCoefFM", "Falvey & Middleton compaction coefficient"
    , "StpThCond", "Thermal conductivity at standard temperature and pressure"
    , "DepoPerm", "The depositional permeability (K0) in milli Darcy"
    , "PermIncrRelaxCoef", "User defined dimensionless coefficient (q*) controlling the increase in permeability on unloading"
    , "SurfacePorosity", "volumetric fraction of pore space in the rock/fluid mixture at the surface for a lithotype or a lithology"
  };

void showUsage()
{
   std::cout << "NAME\n"
      << "\tadjust - Let's you change input parameters in a Cauldron Project3d file from the command line"
      << '\n'
      << "SYNOPSIS\n"
      << "\tUsage: adjust [OPTION] ... \n"
	   << '\n'
	   << "OPTIONS\n"
           << "\t--project <cauldron-project-file>   Specify input file. (in which case you do not need to give a name as the last parameter)\n" 
           << "\t--output <cauldron-project-file>    Specify output file (by default is equal to input file).\n" 
	   << "\t--set-basement-property <parameter-name>=<new-value>\n"
           << "\t                                    Set a specified property in the BasementIoTbl. See below for a list of parameter names\n"
      << "\t--set-source-rock-lithology <layer-name>,<property>,<new value>\n"
           << "\t                                    Set a specified property in the SourceRockLithoIoTbl for given layer name\n"
	   << "\t--set-crust-thickness <series of ages, series of thickness >\n"
	   << "\t                                    Specify the thicnkess of the crust at a specified moment in history, e.g.:\n"
	   << "\t                                      $ adjust --set-crust-thickness 0 100 200 300, 30000 20000 25000 35000 MyProject.project3d\n"
	   << "\t                                    Note: As a (necessary) side effect, it will also clear the snapshot table\n"
	   << "\t--adjust-thermal-conductivity <lithotype>=<multiplication factor>\n"
	   << "\t                                    Adjust the thermal conductity of one or all lithogies in them model. See below for a list of lithotype names.\n"
	   << "\t--set-lithology-property <property>,<lithotype>=<offset>,<multiplication factor>\n"
	   << "\t--show-erosion-formations           Outputs the names of the formations that are erosions.\n"
	   << "\t--set-erosion <formation-name>=<thickness>[,<t0>,<t1>,<t2>]\n"
	   << "\t                                    Specify the thickness and age of an erosion and coinciding eroded layer. t0 marks the beginning of\n"
	   << "\t                                    the eroded formation, t1 the end of the eroded formation and the beginning of the erosion, and t2\n"
	   << "\t                                    the end of the erosion.\n"
         << "\t                                    Note: As a (necessary) side effect, it will also clear the snapshot table\n"
	   << "\t--add-erosion <thickness>,<erosion start>,<erosion length>\n"
	   << "\t--help                              to print this message.\n"
         << std::endl;

   std::cout << "BASEMENT PARAMETERS\n";
   for ( size_t i = 0 ; i < sizeof(basementParameters)/sizeof(basementParameters[0]) / 2; ++i )
   {
     std::cout << "\t" << i << ". '" << basementParameters[2*i] << "' -> '" << basementParameters[2*i+1] << "'\n";
   }
   std::cout << std::endl;

   std::cout << "LITHOTYPE NAMES\n";
   for (size_t i = 0 ; i < sizeof( lithotypes ) / sizeof( lithotypes[0] ) / 2; ++i )
   {
     std::cout << "\t" << i << ". '" << lithotypes[2*i] << "' -> '" << lithotypes[2*i + 1] << "'\n";
   }
   
   std::cout << "\n";
   std::cout << "LITHOLOGY PROPERTIES\n";
   for ( size_t i = 0 ; i < sizeof(lithotypeProperties) / sizeof(lithotypeProperties[0]) / 2; ++i )
   {
     std::cout << "\t" << i << ". '" << lithotypeProperties[2*i] << "' -> '" << lithotypeProperties[2*i + 1] << "'\n";
   }

   std::cout << std::endl;
}


void terminateHandler()
{
   try
   {
      throw;
   }
   catch( std::exception & e )
   {
      std::cerr << '\n';

      std::cerr << "ERROR: "  << e.what() << endl;
      std::exit(1);
   }
   catch(...)
   {
      std::cerr << "UNKNOWN ERROR" << endl;
      std::exit(1);
   }
}


/// Retrieve and adjust the value of the parameter in the project file.
int main( int argc, char ** argv)
{
   std::set_terminate( & terminateHandler );

   std::vector< std::pair< std::string, double > > basementParameters;
   std::vector< std::pair< double, double > > crustThicnkessSeries;
   double value = 0.0;

   OptionParser options(argc, argv, "--", "=," );

   if (options.defined("help"))
   {
      showUsage();
      return EXIT_SUCCESS;
   }


   // Determining Input and Output file
   std::string inputProject, outputProject;

   outputProject = inputProject = options["project"][0];
   options.erase( "project" );

   try
   { 
      outputProject = options["output"][0];
      options.erase("output");
   }
   catch (OptionException & e)
   {
      /* ignore */
   }


   Project project( inputProject, outputProject );

   // Set basement property
   if (options.defined("set-basement-property"))
   {
      OptionParser::OptionValues values = options["set-basement-property"];
      if (values.size() % 3 != 0 )
      {
         throw OptionException() << "The parameter of --set-basement-property must be of the form NAME=VALUE";
      }

      for (int i = 0; i < values.size(); i+= 3)
      {
         if (values[i+1] != "=")
         {
            throw OptionException() << "The parameter of --set-basement-property must be of the form NAME=VALUE";
         }

         double x2 = 0.0;

         try
         {
            x2 = fromString<double>(values[i+2]);
         }
         catch (ConversionException & e)
         {
            throw OptionException() << "The value in the NAME=VALUE parameter of the --set-basement-property option must be real number.";
         }

         double x1 = project.setBasementProperty(values[i], x2);
      }

      options.erase("set-basement-property");
   }

   if ( options.defined( "set-source-rock-lithology" ) )
   {
      const char * sourceRockLithErrMsg = "The parameter of --set-source-rock-lithology must be of the form LAYER_NAME,PROPERTY,VALUE";

      OptionParser::OptionValues values = options["set-source-rock-lithology"];

      if (values.size() % 5 != 0 )
      {
         throw OptionException() << sourceRockLithErrMsg;
      }

      for ( int i = 0; i < values.size(); i+= 5 )
      {
         const std::string & layerName = values[i];
         if (values[i+1] != ",") throw OptionException() << sourceRockLithErrMsg;

         const std::string & propName = values[i+2];
         if (values[i+3] != ",") throw OptionException() << sourceRockLithErrMsg;

         const std::string & propValue = values[i+4];
         
         project.setSourceRockLithology( layerName, propName, propValue );
      }

      options.erase("set-source-rock-lithology");
   }

   if ( options.defined( "set-crust-thickness" ) )
   {
      OptionParser::OptionValues values = options["set-crust-thickness"];

      std::vector< double > timeSeries;

      size_t N = 0;
      for (; values[N] != ","; ++N)
      {
         timeSeries.push_back( fromString<double>( values[N] ) );
         if (N > 0 && timeSeries[N-1] >= timeSeries[N])
            throw OptionException() << "The age series in the --set-crust-thickness parameter must be strictly increasing: "
               << "age[" << N-2 << "] = " << timeSeries[N-1] << " should come after age[" << N << "] = " << timeSeries[N];

      }

      std::vector< double > thicknessSeries;
      for (size_t i = 0; i < N; ++i)
      {
         thicknessSeries.push_back( fromString<double>( values[N+i+1] ));
      }


      std::vector< std::pair<double, double > > series(N);
      for (size_t i = 0 ; i < N; ++i)
      {
         series[i] = std::make_pair( timeSeries[i], thicknessSeries[i]);
      }

      project.setCrustThickness( series );
      project.clearSnapshotTable();

      options.erase("set-crust-thickness");
   }

   if (options.defined("adjust-thermal-conductivity"))
   {
      OptionParser::OptionValues values = options["adjust-thermal-conductivity"];
      if (values.size() % 3 != 0 )
      {
         throw OptionException() << "The parameter of --adjust-thermal-conductivity must be of the form LITHOTYPE=CORRECTION";
      }

      for (int i = 0; i < values.size(); i+= 3)
      {
         if (values[i+1] != "=")
         {
            throw OptionException() << "The parameter of --adjust-thermal-conductivity must be of the form LITHOTYPE=CORRECTION";
         }

         double x2 = 0.0;

         try
         {
            x2 = fromString<double>(values[i+2]);
         }
         catch (ConversionException & e)
         {
            throw OptionException() << "The correction in the LITHOTYPE=CORRECTION parameter of the --adjust-thermal-conductivity option must be real number.";
         }

         project.adjustThermalConductivity(values[i], x2);
      }

      options.erase("adjust-thermal-conductivity");
   }

   if (options.defined("set-lithology-property"))
   {
      OptionParser::OptionValues values = options["set-lithology-property"];
      if (values.size() % 7 != 0 )
      {
         throw OptionException() << "The parameter of --set-lithology-property must be of the form PROPERTY,LITHOTYPE>=OFFSET,MULTIPLICATION_FACTOR";
      }

      for (int i = 0; i < values.size(); i+= 7)
      {
         if (values[i+1] != "," || values[i+3]!= "=" || values[i+5] != ",")
         {
            throw OptionException() << "The parameter of --set-lithology-property must be of the form PROPERTY,LITHOTYPE>=OFFSET,MULTIPLICATION_FACTOR";
         }

         std::string property = values[i];
         std::string lithotype = values[i+2];

         double offset = 0.0;

         try
         {
            offset = fromString<double>(values[i+4]);
         }
         catch (ConversionException & e)
         {
            throw OptionException() << "The offset in the --set-lithology-property parameter  must be a real number.";
         }

         double correctFactor = 0.0;
         try
         {
            correctFactor = fromString<double>(values[i+6]);
         }
         catch (ConversionException & e)
         {
            throw OptionException() << "The multiplication factor in the --set-lithology-property parameter  must be a real number.";
         }

         project.setLithotypeProperty(property, lithotype, offset, correctFactor);
      }

      options.erase("set-lithology-property");
   }

   if (options.defined("show-erosion-formations"))
   {
      typedef std::map< Project::Formation, std::vector< Project::Formation > > Map;
      Map formations = project.getErosionFormations();

      std::cout << "Erosion formations are:\n";
      for (Map::const_iterator formation = formations.begin(); formation != formations.end(); ++formation)
      {
         const Project::Formation & erosion = formation->first;
         const std::vector< Project::Formation > & eroded = formation->second;

         std::cout << '\'' << erosion << "' [" << erosion.m_minAge << " Ma - " << erosion.m_maxAge << " Ma] erodes ";
         if (erosion.m_constant)
            std::cout << erosion.m_minThickness;
         else
            std::cout << "between " << erosion.m_minThickness << " and " << erosion.m_maxThickness;

         std::cout << " meters from the following formation"
            << (eroded.size() > 1 ? "s" : "") << ": " ;

         for (size_t i = 0; i < eroded.size(); ++i)
         {
            if (i > 0)
            {
               std::cout << ", ";

               if (i < eroded.size() - 1)
                  std::cout << "and ";
            }

            std::cout << "'" << eroded[i] << "' [" << eroded[i].m_minAge << " Ma - " << eroded[i].m_maxAge 
               << " Ma] with thickness ";

            if (eroded[i].m_constant)
               std::cout << "of " << eroded[i].m_minThickness ;
            else
               std::cout << "between " << eroded[i].m_minThickness << " and  " << eroded[i].m_maxThickness ;

            std::cout << " meters";
         }

         // Mark all erosion formations that can be tweaked with a '*'
         if (eroded.size() == 1 && erosion.m_constant && eroded[0].m_minThickness && erosion.m_minThickness == 0.0 - eroded[0].m_minThickness)
            std::cout << " (*)";

         std::cout << "\n";
      }
      std::cout << "Note: entries marked with a (*) can be changed with the --set-erosion parameter\n";
      std::cout << std::endl;

      options.erase("show-erosion-formations");
   }

   if (options.defined("set-erosion"))
   {
      OptionParser::OptionValues values = options["set-erosion"];

      int i = 0;
      while( i < values.size() )
      {
         std::string formation = values[i];

         if (i +2 >= values.size() || values[i+1] !=  "=" )
            throw OptionException() << "The parameter of --set-erosion must be of the form NAME=THICKNESS[,T0,T1,T2]";

         i+=2;

         double thickness = fromString<double>(values[i]);
         if (thickness < 0.0)
            throw OptionException() << "The erosion thickness must be a positive real number.";

         i+=1;

         double t0 = NAN, t1 = NAN, t2 = NAN;

         if (i < values.size() && values[i] == ",")
         {
            ++i;
            if (i + 5 > values.size() || values[i+1] != "," || values[i+3] != ",")
               throw OptionException() << "The parameter of --set-erosion must be of the form NAME=THICKNESS[,T0,T1,T2]";


            t0 = fromString<double>(values[i]);
            t1 = fromString<double>(values[i+2]);
            t2 = fromString<double>(values[i+4]);

            if (t0 <= t1 || t1 <= t2)
               throw OptionException() << "T0, T1, and T2 must be in Ma and in chronological order. In practice it means that T0 > T1 > T2 must be true.";

            i+=5;
         }

         project.setErosionThickness(formation, thickness, t0, t1, t2);
      }

      // clear the snapshot table, because time of certain events change.
      project.clearSnapshotTable();
      options.erase("set-erosion");
   }

   // --add-erosion <thickness>,<erosion start>,<erosion length>
   if (options.defined("add-erosion"))
   {
      OptionParser :: OptionValues values = options["add-erosion"];

      if (values.size() % 5 != 0 )
      {
         throw OptionException() << "The parameter of --add-erosion must a triples of <thickness>, <erosion-start>, <erosion duration>";
      }

      for (int i = 0; i < values.size(); i+= 5)
      {
         if (values[i+1] != "," || values[i+3] != "," )
         {
            throw OptionException() << "The parameter of --add-erosion must a triples of <thickness>, <erosion-start>, <erosion duration>";
         }

         double thickness = 0.0;

         try
         {
            thickness = fromString<double>(values[i]);
         }
         catch (ConversionException & e)
         {
            throw OptionException() << "Thickness must be a positive real number";
         }

         if (thickness < 0.0)
            throw OptionException() << "Thickness must be a positive real number";

         double t0 = 0.0, duration = 0.0;
         try
         {
            t0 = fromString<double>(values[i+2]);
            duration = fromString<double>(values[i+4]);
         }
         catch(ConversionException & e)
         {
            throw OptionException() << "Start time and duration of erosion event must be real numbers";
         }

         if (duration < 0.0)
            throw OptionException() << "Duration of erosion event must be a positive real number.";

         project.addErosion(thickness, t0, duration);
      }

      project.clearSnapshotTable();
      options.erase("add-erosion");
   }

   project.close();

   if (! options.empty() )
   {
      std::vector< std::string > unusedParams = options.definedNames();
      std::cout << "WARNING: there " << (unusedParams.size() > 1 ? "are " : "is ") 
         << unusedParams.size() << " parameter" << (unusedParams.size() > 1? "s" : "") << " ignored:\n";

      for (size_t i = 0; i < unusedParams.size(); ++i)
         std::cout << "WARNING: --" << unusedParams[i] << '\n';

      std::cout << std::endl;
   }

   return EXIT_SUCCESS;
}
