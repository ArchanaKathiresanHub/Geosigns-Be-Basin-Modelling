#include "crustThinning.h"

#include "model/input/projectReader.h"

namespace casaWizard
{

namespace ua
{

CrustThinning::CrustThinning() :
  InfluentialParameter
  (
    "Crust Thinning",
    "\"CrustThinning\"",
    "Crust Thinning",
    "[m]",
    4
  )
{
  arguments().addArgument("Initial crust thickness - minimal range value",              0    );
  arguments().addArgument("Initial crust thickness - maximal range value",              1000 );
  arguments().addArgument("Crust thinning event start time [Ma] - minimal range value", 0    );
  arguments().addArgument("Crust thinning event start time [Ma] - maximal range value", 10   );
  arguments().addArgument("Crust thinning event duration [MY] - minimal range value",   0    );
  arguments().addArgument("Crust thinning event duration [MY] - maximal range value",   10   );
  arguments().addArgument("Crust thickness factor - minimal range value",               0.5  );
  arguments().addArgument("Crust thickness factor - maximal range value",               1.5  );
  arguments().addArgument("Thickness map name", QStringList("Select project3d file"));
  addPDFArgument();
}

void CrustThinning::fillArguments(const ProjectReader& projectReader)
{
  const QStringList mapNames = projectReader.mapNames();
  arguments().setListOptions(0, mapNames);
}

} // namespace ua

}  // namespace casaWizard
