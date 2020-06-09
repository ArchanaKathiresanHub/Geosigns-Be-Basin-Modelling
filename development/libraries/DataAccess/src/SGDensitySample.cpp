#include "SGDensitySample.h"

#include "cauldronschemafuncs.h"
#include "database.h"

DataAccess::Interface::SGDensitySample::SGDensitySample ( ProjectHandle& projectHandle,
                                                          database::Record* record ) : DAObject ( projectHandle, record ) {
   m_density = database::getSGMeanBulkDensity ( record );
}

DataAccess::Interface::SGDensitySample::~SGDensitySample () {
}
