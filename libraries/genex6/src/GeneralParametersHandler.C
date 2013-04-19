#include "GeneralParametersHandler.h"

namespace Genex6
{

GeneralParametersHandler::GeneralParametersHandler()
{
   m_ParameterValues[asphalteneRatio] = 2.5;  
   m_ParameterValues[BiotGeo]         = 10.0; 
   m_ParameterValues[CharLengthGeo]   = 0.5;  
   m_ParameterValues[BiotOverL2Geo]   = m_ParameterValues[BiotGeo] / (m_ParameterValues[CharLengthGeo] * 
                                                                      m_ParameterValues[CharLengthGeo]); 
   m_ParameterValues[BiotOverL2]      = m_ParameterValues[BiotOverL2Geo]; 
   m_ParameterValues[TCref]           = 105.0; 
   m_ParameterValues[WboMin]          = 0.000000001; 
   m_ParameterValues[TuningConst]     = 50.0; 
   m_ParameterValues[Tlab]            = 573.0; 
   m_ParameterValues[Uj]              = 25000.0; 
   m_ParameterValues[T0torbanite]     = 106.0;
   m_ParameterValues[T0aromatic]      = 240.0;
   m_ParameterValues[HCmin]           = 0.7; 
   m_ParameterValues[HCmax]           = 1.8; 
   m_ParameterValues[OmaxHC]          = 0.3; 
   m_ParameterValues[BetaOverAlpha]   = 0.00000056; 
   m_ParameterValues[dSperCoke]       = -150.0;  
   m_ParameterValues[OCpreasphalt1]   = 0.083333; 
   m_ParameterValues[OCpreasphalt2]   = -0.36667; 
   m_ParameterValues[OCpreasphalt3]   = 0.4; 
   m_ParameterValues[HCkerogen1]      = 1.06; 
   m_ParameterValues[HCkerogen2]      = 0.07;
   m_ParameterValues[OCkerogen1]      = 1.68; 
   m_ParameterValues[OCkerogen2]      = 0.03; 
   m_ParameterValues[Nkerogen]        = 0.1; 
   m_ParameterValues[Npreasphalt]     = 0.96; 
   m_ParameterValues[Nasphaltene]     = 0.8; 
   m_ParameterValues[Nresin]          = 0.9;
   m_ParameterValues[Nprecoke]        = 0.66667;
   m_ParameterValues[Nhetero1]        = 6.0;  // was 4.0
   m_ParameterValues[Nhetero2]        = 0.001; 
   m_ParameterValues[HCAsphOverPreasphalt] = 0.9; // in GX5 = 1.0; in VBA6 = 0.9
   m_ParameterValues[OCAsphOverPreasphalt] = 0.3; // was 0.6
   m_ParameterValues[OCasphMin]            = 0.001; 
   m_ParameterValues[OCprecokeWhenHCpreasphaltZero] = 0.06;
   m_ParameterValues[OCprecokePerPreasphalt]        = 0.005; 
   m_ParameterValues[EdropForS]            = 0.0;              
   m_ParameterValues[EdropPerS]            = 300000;              
   m_ParameterValues[EcrackingCC]          = 217000.0;   // 217   
   m_ParameterValues[SCratio]              = 0.0; //0.05
   m_ParameterValues[Sasphaltene]          = 0.6667;
   m_ParameterValues[Sprecoke]             = 0.4;
   m_ParameterValues[Scoke1]               = 0.5;
   m_ParameterValues[Shetero]              = 0.1;
   m_ParameterValues[SO4massfract]         = 0.0;
   
   m_ParameterValues[PreasphalteneAromMin] = 0.2; 
   m_ParameterValues[PreasphalteneAromMax] = 0.9;      
   m_ParameterValues[Order0]               = 4.75; 
   m_ParameterValues[OrderPerHoverC]       = -1.875;
   m_ParameterValues[ActCrit]              = 0.0;

   // Parameters for compatibility with GX5 (simulation like Genex5)
   m_ParameterValues[Nhetero1GX5]                      = 4.0; 
   m_ParameterValues[HCAsphOverPreasphaltGX5]          = 1.0;
   m_ParameterValues[OCAsphOverPreasphaltGX5]          = 0.6;
   m_ParameterValues[OCprecokeWhenHCpreasphaltZeroGX5] = 0.09;
   m_ParameterValues[OCprecokePerPreasphaltGX5]        = 0.008; 
   m_ParameterValues[NC15plusAroGX5]                   = 0.004; 
   m_ParameterValues[Ediff1GX5]                        = 10000.0;
   m_ParameterValues[Ediff2GX5]                        = 7000.0; 

   // Parameters for compatibility with OTGC5 (simulation like OTGC5)
   m_ParameterValues[OCprecokeWhenHCpreasphaltZeroOTGC5] = 0.08;
   m_ParameterValues[OCprecokePerPreasphaltOTGC5]        = 0.01; 

   const std::string tempParameterNames[] =
   {
      "asphalteneRatio", "BiotGeo", "CharLengthGeo", "BiotOverL2Geo", "BiotOverL2", "TCref", "WboMin", 
      "TuningConst", "Tlab", "Uj", "T0torbanite", "T0aromatic", "HCmin", "HCmax", "OmaxHC", "BetaOverAlpha",
      "dSperCoke", "OCpreasphalt1", "OCpreasphalt2", "OCpreasphalt3", "HCkerogen1", "HCkerogen2", 
      "OCkerogen1", "OCkerogen2", "Nkerogen", "Npreasphalt", "Nasphaltene", "Nresin", "Nprecoke", "Nhetero1", "Nhetero2", 
      "HCAsphOverPreasphalt", "OCAsphOverPreasphalt", "OCasphMin", "OCprecokeWhenHCpreasphaltZero", "OCprecokePerPreasphalt", 
      "EdropForS", "EdropPerS", "EcrackingCC", 
      "SCratio", "Sasphaltene", "Sprecoke", "Scoke1", "Shetero", "SO4massfract",
      "PreasphalteneAromMin", "PreasphalteneAromMax", "Order0", "OrderPerHoverC", "ActCrit", 
      "Nhetero1GX5", "HCAsphOverPreasphaltGX5", "OCAsphOverPreasphaltGX5", 
      "OCprecokeWhenHCpreasphaltZeroGX5", "OCprecokePerPreasphaltGX5", "NC15plusAroGX5", "Ediff1GX5", "Ediff2GX5", 
      "OCprecokeWhenHCpreasphaltZeroOTGC5", "OCprecokePerPreasphaltOTGC5", ""
   };
   for(int i = 0; tempParameterNames[i] != ""; ++i)  
   {
      m_ParameterNames[i] =  tempParameterNames[i];
      m_ParameterIdByName[tempParameterNames[i]] = i;
   }
}
void GeneralParametersHandler::SetParameterById(const int &ParameterId , const double &value) 
{
   if(ParameterId >= 0 && ParameterId < NumberOfParameters) {
      m_ParameterValues[ParameterId] = value;
   }
}
void GeneralParametersHandler::SetParameterByName(const std::string &ParameterName, const double &value) 
{
   std::map<std::string, int>::const_iterator it = m_ParameterIdByName.find(ParameterName);
   if(it != m_ParameterIdByName.end()) {
      m_ParameterValues[it->second] = value;

      if( it->second == BiotGeo || it->second == CharLengthGeo ) {
         m_ParameterValues[BiotOverL2Geo] = m_ParameterValues[BiotGeo] / (m_ParameterValues[CharLengthGeo] *
                                                                          m_ParameterValues[CharLengthGeo]);
         m_ParameterValues[BiotOverL2] = m_ParameterValues[BiotOverL2Geo];
      }
   } else {
       //throw
   }
}
double GeneralParametersHandler::GetParameterById(const int &ParameterId) 
{
   double ret = -1.0;
   if(ParameterId >= 0 && ParameterId < NumberOfParameters) {
      ret = m_ParameterValues[ParameterId];
   }
   return ret;

}
void GeneralParametersHandler::PrintConfigurationFileGeneralParameters(std::ofstream &outfile)
{
   std::map<std::string, int>::const_iterator it = m_ParameterIdByName.begin();
   while(it != m_ParameterIdByName.end()) {
      outfile << it->first << "," << m_ParameterValues[it->second] << std::endl;
      ++ it;
   }
}
GeneralParametersHandler::~GeneralParametersHandler() 
{
    m_ParameterIdByName.clear();
}
GeneralParametersHandler & GeneralParametersHandler::getInstance() 
{
   static GeneralParametersHandler theHandler;
   return theHandler;
}

}
