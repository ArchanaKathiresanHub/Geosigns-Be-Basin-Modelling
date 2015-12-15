#include "VitriniteReflectance.h"
#include "VreArrheniusMethod.h"
#include "VreTtiMethod.h"

namespace GeoPhysics {

VitriniteReflectance * VitriniteReflectance::create( const std::string & algorithmName )
{
   if (algorithmName == "Burnham & Sweeney")
   {
      return new SweeneyBurnham();
   }
   else if (algorithmName == "Larter") 
   {
      return new Larter();
   }
   else if (algorithmName == "Modified Lopatin")
   {
      return new LopatinModified();
   }
   else if (algorithmName == "Lopatin & Dykstra")
   {
      return new LopatinDykstra();
   }
   else if (algorithmName == "Lopatin & Wapples")
   {
      return new LopatinWaples();
   }
   else if (algorithmName == "Lopatin & Royden")
   {
      return new LopatinRoyden();
   }
   else if (algorithmName == "Lopatin & Issler")
   {
      return new LopatinIssler();
   }
   else if (algorithmName == "Lopatin & Hood")
   {
      return new LopatinHood();
   }
   else if (algorithmName == "Lopatin & Goff")
   {
     return new LopatinGoff();
   }
   else
   {
     std::cout << "Invalid Vitrinite calculation algorithm. Previous used" << std::endl;
     return 0;
   }
}

} // end namespace GeoPhysics
