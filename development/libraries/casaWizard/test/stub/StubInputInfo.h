#pragma once

#include "model/TargetInputInfo.h"
#include "model/predictionTargetManager.h"

namespace casaWizard {
namespace ua {

class stubTargetInputInfo : public TargetInputInfo {
public:
   stubTargetInputInfo(QVector<bool> surfaceSelectionStates,
                       bool temperatureTargetsSelected,
                       bool vreTargetsSelected,
                       QString depthInput):
      TargetInputInfo(surfaceSelectionStates,
                      temperatureTargetsSelected,
                      vreTargetsSelected,
                      depthInput)
   {
   }
   stubTargetInputInfo(PredictionTargetManager& p):
      TargetInputInfo(QVector<bool>(p.validSurfaceNames().size(),true),
                      true,
                      true,
                      "80085")
   {
   }

   QVector<XYName> getTargetLocations() const override{
      //make up some targetLocations
      QVector<TargetInputInfo::XYName> targetLocations;
      targetLocations.push_back(XYName(1000,1000, "left bottom"));
      targetLocations.push_back(XYName(9000,1000, "right bottom"));
      targetLocations.push_back(XYName(1000,9000, "left top"));
      targetLocations.push_back(XYName(9000,9000, "right top"));
      return targetLocations;
   }
};

}
}

