#ifndef GENERALPARAMETERSHANDLER_H
#define GENERALPARAMETERSHANDLER_H

#include <string>
#include <map>
#include <fstream>

namespace Genex6
{

class GeneralParametersHandler
{
public:
   enum ParameterId
   {
      asphalteneRatio = 0, BiotGeo, CharLengthGeo, BiotOverL2Geo, BiotOverL2, TCref, WboMin, 
      TuningConst, Tlab, Uj, T0torbanite, T0aromatic, HCmin, HCmax, OmaxHC, BetaOverAlpha, 
      dSperCoke, OCpreasphalt1, OCpreasphalt2, OCpreasphalt3, HCkerogen1, HCkerogen2, 
      OCkerogen1, OCkerogen2, Nkerogen, Npreasphalt, Nasphaltene, Nresin, Nprecoke, Nhetero1, Nhetero2, 
      HCAsphOverPreasphalt, OCAsphOverPreasphalt, OCasphMin, OCprecokeWhenHCpreasphaltZero, OCprecokePerPreasphalt,
      EdropForS, EdropPerS, EcrackingCC, 
      SCratio, Sasphaltene, Sprecoke, Scoke1, Shetero, SO4massfract,
      PreasphalteneAromMin, PreasphalteneAromMax, Order0, OrderPerHoverC, ActCrit,
      Nhetero1GX5, HCAsphOverPreasphaltGX5, OCAsphOverPreasphaltGX5, 
      OCprecokeWhenHCpreasphaltZeroGX5, OCprecokePerPreasphaltGX5, NC15plusAroGX5, Ediff1GX5, Ediff2GX5,
      OCprecokeWhenHCpreasphaltZeroOTGC5, OCprecokePerPreasphaltOTGC5, WboMinGx7, NumberOfParameters
   };
   
   void SetParameterById(const int &ParameterId , const double &value);
   void SetParameterByName(const std::string &ParameterName, const double &value);
   double GetParameterById(const int &ParameterId);

   static GeneralParametersHandler &getInstance();
   ~GeneralParametersHandler();
   void PrintConfigurationFileGeneralParameters(std::ofstream &outfile);

private:
  
   std::string m_ParameterNames [NumberOfParameters];
   double m_ParameterValues [NumberOfParameters];
   std::map<std::string, int> m_ParameterIdByName;
   
   GeneralParametersHandler();
   GeneralParametersHandler(const GeneralParametersHandler &);
   GeneralParametersHandler & operator=(const GeneralParametersHandler &);
   
};

}
#endif
