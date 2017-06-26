/* File : BasinModeling.CasaAPI.i */
 
%module EosPackAPI

%{
// Interface for CBMGenerics library
#include "../../CBMGenerics/src/ComponentManager.h"
using namespace CBMGenerics;

// Interface for EosPack library
#include "EosPackCAPI.h"
#include "EosPack.h"
#include "PTDiagramCalculator.h"
%}

%include "../../swig-common/StdHelper.i"
%include <csharp.swg>
%include <enums.swg>
%include <arrays_csharp.i>

%include <carrays.i>
%array_functions(double, doubleArray);
%rename(PhaseId2) pvtFlash::PhaseId;

// EosPackCAPI.h:EosPackComputeWithLumpingArr()
%apply double INPUT[]  { double * compMasses }
%apply double INPUT[]  { double * in_compMasses }
%apply double OUTPUT[] { double * phaseCompMasses }
%apply double OUTPUT[] { double * phaseDensity }
%apply double OUTPUT[] { double * phaseViscosity }

// EosPackCAPI.h:BuildPTDiagram()
%apply double INPUT[]  { double * comp }
%apply double OUTPUT[] { double * points }
%apply int    INOUT[]  { int    * szIso }
%apply double OUTPUT[] { double * isolines }
%apply double OUTPUT[] { double * critPt }
%apply double OUTPUT[] { double * bubbleP }

// EosPack.h::compute()
%apply double INPUT[]  { double   compMasses[] }
%apply double INPUT[]  { double   in_compMasses[] }
%apply double INPUT[]  { double   in_compMasses[ComponentId::NUMBER_OF_SPECIES] }
%apply double OUTPUT[] { double   phaseDensity[] }
%apply double OUTPUT[] { double   phaseViscosity[] }
%apply double INPUT[]  { double * kValues }
%apply double INPUT[]  { double * pKValues }
%apply double OUTPUT[] { double unlump_fraction[] }

// EosPack.h::computeWithLumping()
%apply double INPUT[]  { double   in_compMasses[] }

// Interface for CBMGenerics library
%include "../../CBMGenerics/src/ComponentManager.h"
%include "EosPackCAPI.h"
%include "EosPack.h"
%include "PTDiagramCalculator.h"

