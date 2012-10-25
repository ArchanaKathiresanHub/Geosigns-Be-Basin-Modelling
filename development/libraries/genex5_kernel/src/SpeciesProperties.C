#include "SpeciesProperties.h"
#include "Species.h"
#include "Constants.h"
#include "GeneralParametersHandler.h"

namespace Genex5
{
SpeciesProperties::SpeciesProperties(Species *const in_Species,
                      const double &in_activationEnergy1,
                      const double &in_activationEnergy2,
                      const double &in_entropy,
                      const double &in_volume,
                      const double &in_reactionOrder,
                      const double &in_diffusionEnergy1,
                      const double &in_diffusionEnergy2,
                      const double &in_jumpLength,
                      const double &in_density,
                      const double &in_B0,
                      const double &in_Aromaticity
                      ):
                      m_theSpecies(in_Species),
                      m_activationEnergy1(in_activationEnergy1),
                      m_activationEnergy2(in_activationEnergy2),
                      m_entropy(in_entropy),
                      m_volume(in_volume),
                      m_reactionOrder(in_reactionOrder),
                      m_diffusionEnergy1(in_diffusionEnergy1),
                      m_diffusionEnergy2(in_diffusionEnergy2),
                      m_jumpLength(in_jumpLength),
                      m_density(in_density),
                      m_B0(in_B0),
                      m_aromaticity(in_Aromaticity)
{
   m_molWeight   = 0.0;   //computed based on composition
   m_mobile      = false;    //derived
   m_reactive    = false;  //derived
   m_HC          = false;        //derived
   m_Oil         = false;       //derived
   m_HCgas       = false;     //derived
}
void SpeciesProperties::CheckValueBounds()
{

}
void SpeciesProperties::OutputOnScreen() const
{ 
   cout   <<","<<m_molWeight 
                    <<","<<m_density
                    <<","<<m_activationEnergy1
                    <<","<<m_activationEnergy2
                    <<","<<m_entropy
                    <<","<<m_volume
                    <<","<<m_reactionOrder
                    <<","<<m_diffusionEnergy1
                    <<","<<m_diffusionEnergy2
                    <<","<<m_jumpLength
                    <<","<<m_B0
                    <<","<<m_aromaticity
            <<endl;
}
void SpeciesProperties::OutputOnFile(ofstream &outfile) const
{
    outfile       <<","<<m_molWeight
                    <<","<<m_density
                    <<","<<m_activationEnergy1
                    <<","<<m_activationEnergy2
                    <<","<<m_entropy
                    <<","<<m_volume
                    <<","<<m_reactionOrder
                    <<","<<m_diffusionEnergy1
                    <<","<<m_diffusionEnergy2
                    <<","<<m_jumpLength
                    <<","<<m_B0
                    <<","<<m_aromaticity
            <<endl;
}
//check for Bounds
//update changes due to change in composition
void SpeciesProperties::Update()
{
   if(m_diffusionEnergy2 < 1E-30)
   {
      m_diffusionEnergy2 = m_diffusionEnergy1;
   }
   if(m_activationEnergy2 < 1E-30)
   {
      m_activationEnergy2 = m_activationEnergy1;
   }
   //IsMobile
   m_mobile = m_diffusionEnergy1 < Genex5::Constants::s_ActUMax ? true:false;
   //IsReactive
   m_reactive = m_activationEnergy1 < Genex5::Constants::s_ActUMax ? true:false;
   //Is HC
   double AtomC = m_theSpecies->GetCompositionByElement("C");
   double AtomH = m_theSpecies->GetCompositionByElement("H");
   double AtomO = m_theSpecies->GetCompositionByElement("O");

   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   static double OmaxHC = theHandler.GetParameterById(GeneralParametersHandler::OmaxHC);

   //(Atom(IatomC, L) > CminHC And Atom(IatomH, L) > HminHC And Atom(IatomO, L) < OmaxHC)
   if(AtomC > Genex5::Constants::s_CminHC && AtomH > Genex5::Constants::s_HminHC && AtomO < OmaxHC)
   {
      m_HC = true;
   }
   else
   {
      m_HC = false;
   }
   //Is OIL
   //FunCritOil = (Act(6, L) < ActUMax And Atom(IatomC, L) > CminHC And Atom(IatomH, L) > HminHC _
   //                And Atom(IatomO, L) < OmaxHC And Dens(L) > DensMaxGas)
   if(m_diffusionEnergy1 < Genex5::Constants::s_ActUMax 
      && AtomC > Genex5::Constants::s_CminHC 
      && AtomH > Genex5::Constants::s_HminHC 
      && AtomO < OmaxHC 
      && m_density > Genex5::Constants::s_DensMaxGas)
   {
      m_Oil = true;
   }
   else
   {
      m_Oil = false;
   }
   //Is m_HCGas
   //FunCritHCgas = (Act(6, L) < ActUMax And Atom(IatomC, L) > CminHC And Atom(IatomH, L) > HminHC _
   //                And Atom(IatomO, L) < OmaxHC And Dens(L) < DensMaxGas)
   if(m_diffusionEnergy1 < Genex5::Constants::s_ActUMax 
      && AtomC > Genex5::Constants::s_CminHC 
      && AtomH > Genex5::Constants::s_HminHC 
      && AtomO < OmaxHC 
      && m_density < Genex5::Constants::s_DensMaxGas)
   {
      m_HCgas = true;
   }
   else
   {
      m_HCgas = false;
   }
   m_molWeight = m_theSpecies->ComputeMolWeight();   //computed based on composition
   m_aromaticity = m_theSpecies->ComputeAromaticity();  //computed

   //B0(L) = FunB0(Act(6, L))
   m_B0 = m_theSpecies->ComputeB0();
   if(m_mobile == false)
   {
      m_density = m_theSpecies->ComputeDensity();
   }
}
void SpeciesProperties::PrintBenchmarkProperties(ofstream &outfile) const
{
   /*    <<"Dens"<<","
         <<"FW"<<","
         <<"Arom"<<","
         <<"Er1"<<","
         <<"Er2"<<","
         <<"Ed1"<<","
         <<"B0"<<","
         <<"IsMobile"<<","
         <<"IsReactive"<<","
         <<"IsHC"<<","
         <<"IsOil"<<","
         <<"IsHCgas"<<std::endl;*/

   outfile.setf(ios::fixed);
   outfile   <<m_density<<","
               <<m_molWeight<<","
               <<m_aromaticity<<","
               <<m_activationEnergy1<<","
               <<m_activationEnergy2<<",";

   outfile.setf(ios::scientific);

   outfile   <<m_diffusionEnergy1<<","
               <<m_B0<<",";

   outfile.setf(ios::fixed);
   
   std::string value;
   value=m_mobile?"TRUE":"FALSE";
   outfile<<value<<",";
   value=m_reactive?"TRUE":"FALSE";
   outfile<<value<<",";
   value=m_HC?"TRUE":"FALSE";
   outfile<<value<<",";
   value=m_Oil?"TRUE":"FALSE";
   outfile<<value<<",";
   value=m_HCgas?"TRUE":"FALSE";
   outfile<<value<<endl;
         
}

}//end of Genex5 namespace
