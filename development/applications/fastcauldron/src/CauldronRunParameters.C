#include "CauldronRunParameters.h"

#include "FastcauldronSimulator.h"

CauldronRunParameters::CauldronRunParameters ( Interface::ProjectHandle * projectHandle, database::Record * record) : Interface::RunParameters ( projectHandle, record ) {
}

bool CauldronRunParameters::getChemicalCompaction () const {
   return Interface::RunParameters::getChemicalCompaction () and
          FastcauldronSimulator::getInstance ().getCalculationMode () == PRESSURE_AND_TEMPERATURE_MODE;
}
