#include "crustThinningOneEvent.h"

namespace casaWizard
{

namespace ua
{

CrustThinningOneEvent::CrustThinningOneEvent() :
  InfluentialParameter
  (
    "Crust Thinning One Event",
    "\"CrustThinningOneEvent\"",
    "CrustThinningOneEvent",
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
  arguments().addArgument("Crust thickness factor - minimal range value",              0.5  );
  arguments().addArgument("Crust thickness factor - maximal range value",               1.5  );
  addPDFArgument();
}

void CrustThinningOneEvent::fillArguments(const ProjectReader& /*projectReader*/)
{
}

} // namespace ua

}  // namespace casaWizard
