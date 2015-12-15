#include "RunParameters.h"

#include "FastcauldronSimulator.h"

RunParameters::RunParameters ( Interface::ProjectHandle * projectHandle, database::Record * record) : Interface::RunParameters ( projectHandle, record ) {
}


bool RunParameters::getChemicalCompaction () const {
   return Interface::RunParameters::getChemicalCompaction () and
          FastcauldronSimulator::getInstance ().getCalculationMode () == PRESSURE_AND_TEMPERATURE_MODE;
}
