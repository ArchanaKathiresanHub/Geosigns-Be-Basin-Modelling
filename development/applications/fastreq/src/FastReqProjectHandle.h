#ifndef FASTREQPROJECTHANDLE_H
#define FASTREQPROJECTHANDLE_H

#include <string>
#include <vector>
using namespace std;

namespace database
{
   class Database;
}
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"

using namespace DataAccess;

namespace FastReq
{
class FastReqProjectHandle : public Interface::ProjectHandle
{
public:

    FastReqProjectHandle (database::Database * database, const std::string & name, const std::string & accessMode);

    static FastReqProjectHandle *CreateFrom (const std::string & inputFileName);

    virtual ~FastReqProjectHandle (void);
    unsigned int getNumberOfPressureTemperatureNodes() const;
    unsigned int getNumberOfHighResDecompactionNodes() const;
    unsigned int getNumberOfGenexNodes() const;
    unsigned int getNumberOfMigrationNodes() const;
    unsigned int getNumberOfSnapShots() const;

protected:
   void loadRequiredRunOptions();
   void loadCrustVariables();
   void loadMantleThicknessPresentDay();

   void computeNodes();
   void computeGenexNodes(); 
   void computeMigrationNodes();
   void setNumberOfHighResGridNodes();
   void setNumberOfLowResGridNodes();
private:

   double m_BrickHeightSediment;
   double m_BrickHeightCrust;
   double m_BrickHeightMantle;
   double m_MaxCrustThickness;
   double m_MinCrustThickness;
   double m_MantleThicknessPresentDay;
   double m_CrustThinningRatio;

   unsigned int m_LowResGridNodes;
   unsigned int m_HighResGridNodes;

   unsigned int m_PressureTemperatureNodes;
   unsigned int m_HighResDecompationNodes;
   unsigned int m_GenexNodes;
   unsigned int m_MigrationNodes;
};

   inline unsigned int FastReqProjectHandle::getNumberOfPressureTemperatureNodes() const
   {
      return m_PressureTemperatureNodes;
   }
   inline unsigned int FastReqProjectHandle::getNumberOfHighResDecompactionNodes() const
   {
      return m_HighResDecompationNodes;
   }
   inline unsigned int FastReqProjectHandle::getNumberOfGenexNodes() const
   {
      return m_GenexNodes;
   }
   inline unsigned int FastReqProjectHandle::getNumberOfMigrationNodes() const
   {
      return m_MigrationNodes;
   }
}
#endif 
