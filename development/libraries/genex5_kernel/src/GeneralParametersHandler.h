#ifndef GENERALPARAMETERSHANDLER_H
#define GENERALPARAMETERSHANDLER_H

#include <string>
#include <map>


namespace Genex5
{

class GeneralParametersHandler
{
public:
   enum ParameterId
   {
      asphalteneRatio = 0, BiotGeo, CharLengthGeo, BiotOverL2Geo, BiotOverL2, TCref, WboMin, TuningConst, Tlab, Uj, T0torbanite,
      T0aromatic, HCmin, HCmax, OmaxHC, BetaOverAlpha, dSperCoke, OCpreasphalt1, OCpreasphalt2, OCpreasphalt3, HCkerogen1, HCkerogen2, 
      OCkerogen1, OCkerogen2, Nkerogen, Npreasphalt, Nasphaltene, Nresin, Nprecoke, Nhetero1, Nhetero2, NC15plusAro, HCAsphOverPreasphalt,
      OCAsphOverPreasphalt, OCasphMin, OCprecokeWhenHCpreasphaltZero, OCprecokePerPreasphalt, EdropForS, EcrackingCC, Ediff1, Ediff2,
      PreasphalteneAromMin, PreasphalteneAromMax, Order0, OrderPerHoverC, ActCrit,
      NumberOfParameters
   };
   
   void SetParameterById(const int &ParameterId , const double &value);
   void SetParameterByName(const std::string &ParameterName, const double &value);
   double GetParameterById(const int &ParameterId);

   static GeneralParametersHandler &getInstance();
   ~GeneralParametersHandler();

private:
  
   std::string m_ParameterNames   [NumberOfParameters];
   double m_ParameterValues  [NumberOfParameters];
   std::map<std::string, int> m_ParameterIdByName;

   
   GeneralParametersHandler();
   GeneralParametersHandler(const GeneralParametersHandler &);
   GeneralParametersHandler & operator=(const GeneralParametersHandler &);
   
};

}
#endif
