#include "mpi.h"
#include "overburden.h"
#include "Interface/ProjectHandle.h"
#include "Interface/PropertyValue.h"
#include "Interface/GridMap.h"
#include "Formation.h"

#include <limits>

using namespace DataAccess;

using Interface::ProjectHandle;

namespace migration {
namespace overburden {


template <typename PRED>
vector<const Formation*> getDownwardOverburdenFormationsIf(const 
   Formation* formation, PRED pred)
{
   const ProjectHandle* projectHandle = formation->getProjectHandle();
   Interface::FormationList* formations = projectHandle->getFormations();

   vector<const Formation*> overburdenFormationsFromTop;
   for (Interface::FormationList::const_iterator it = formations->begin();
      it != formations->end(); ++it)
   {
      const Formation * iteratorFormation = dynamic_cast <const Formation *> (*it);
      if (iteratorFormation == formation) break;
      else {
         if (pred(iteratorFormation))  
            overburdenFormationsFromTop.push_back(iteratorFormation);
      }
   }
   delete formations;

   return overburdenFormationsFromTop;
}

template <typename PRED>
vector<const Formation*> getUpwardOverburdenFormationsIf(const 
   Formation* formation, PRED pred)
{
   vector<const Formation*> overburdenFormationsFromTop = getDownwardOverburdenFormationsIf(
      formation, pred);

   vector<const Formation*> overburdenFormationsFromBase;
   overburdenFormationsFromBase.reserve(overburdenFormationsFromTop.size());
   for (vector<const Formation*>::reverse_iterator it = overburdenFormationsFromTop.rbegin(); 
      it != overburdenFormationsFromTop.rend(); ++it)
      overburdenFormationsFromBase.push_back(*it);

   return overburdenFormationsFromBase;
}

struct AlwaysTrue {
   bool operator()(const Formation* formation) { return true; }
};

OverburdenFormations getOverburdenFormations(const Formation* formation,
   bool upward)
{
   if (upward)
      return OverburdenFormations(getUpwardOverburdenFormationsIf(formation, AlwaysTrue()),true);
   else
      return OverburdenFormations(getDownwardOverburdenFormationsIf(formation, AlwaysTrue()), false);
}

template <typename PRED>
vector<const Formation*> getOverburdenFormationsIf(const Formation* formation, PRED pred, 
   bool upward)
{
   if (upward)
      return getUpwardOverburdenFormationsIf(formation, pred);
   else
      return getDownwardOverburdenFormationsIf(formation, pred);
}

} } // namespace migration::overburden

// Some templates from overburden.[Ch] must be instantiated:

// #include "SelectIfThicknessIsLargerThanZero.h"

// namespace migration { namespace overburden {

// using namespace migration;

// template
// vector<const Formation*> getOverburdenFormationsIf<SelectIfThicknessIsLargerThanZero>(
//    const Formation*,SelectIfThicknessIsLargerThanZero, bool);

// } }
