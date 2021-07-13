//
// Copyright ( C ) 2013-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed by CGI Information Systems & Management Consultants Pvt Ltd, India under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
using namespace std;

#include <gtest/gtest.h>

#include "PVTCalculator.h"
#include "NumericFunctions.h"

TEST( PVTCalculator, GORMcalcTest)
{
	Genex6::PVTComponents vapour;
	Genex6::PVTComponents liquid;

	// Component masses in Vapour form
	vapour.m_components[ComponentId::ASPHALTENE] = 0.123;
	vapour.m_components[ComponentId::RESIN] = 0.1456;
	vapour.m_components[ComponentId::C15_PLUS_ARO] = 0.1736;
	vapour.m_components[ComponentId::C15_PLUS_SAT] = 0.1764;
	vapour.m_components[ComponentId::C6_MINUS_14ARO] = 0.244;
	vapour.m_components[ComponentId::C6_MINUS_14SAT] = 0.387;

	vapour.m_components[ComponentId::C5] = 0.34634;
	vapour.m_components[ComponentId::C4] = 0.5783;
	vapour.m_components[ComponentId::C3] = 0.863;
	vapour.m_components[ComponentId::C2] = 0.923;
	vapour.m_components[ComponentId::C1] = 0.5263;

	vapour.m_components[ComponentId::COX] = 0.1;
	vapour.m_components[ComponentId::N2] = 0.1;

	vapour.m_components[ComponentId::H2S] = 0.345;

	vapour.m_components[ComponentId::LSC] = 0.434;
	vapour.m_components[ComponentId::C15_PLUS_AT] = 0.2323;
	vapour.m_components[ComponentId::C6_MINUS_14BT] = 0.4343;
	vapour.m_components[ComponentId::C6_MINUS_14DBT] = 0.234;
	vapour.m_components[ComponentId::C6_MINUS_14BP] = 0.44;
	vapour.m_components[ComponentId::C15_PLUS_ARO_S] = 0.2345;
	vapour.m_components[ComponentId::C15_PLUS_SAT_S] = 0.3432;
	vapour.m_components[ComponentId::C6_MINUS_14SAT_S] = 0.4323;
	vapour.m_components[ComponentId::C6_MINUS_14ARO_S] = 0.2332;
	
	// Component masses in Liquid form
	liquid.m_components[ComponentId::ASPHALTENE] = 0.2345;
	liquid.m_components[ComponentId::RESIN] = 0.3452;
	liquid.m_components[ComponentId::C15_PLUS_ARO] = 0.12342;
	liquid.m_components[ComponentId::C15_PLUS_SAT] = 0.34342;
	liquid.m_components[ComponentId::C6_MINUS_14ARO] = 0.5542;
	liquid.m_components[ComponentId::C6_MINUS_14SAT] = 0.5662;

	liquid.m_components[ComponentId::C5] = 0.43542;
	liquid.m_components[ComponentId::C4] = 0.5642;
	liquid.m_components[ComponentId::C3] = 0.532;
	liquid.m_components[ComponentId::C2] = 0.3532;
	liquid.m_components[ComponentId::C1] = 0.2354;

	liquid.m_components[ComponentId::COX] = 0.4532;
	liquid.m_components[ComponentId::N2] = 0.35342;

	liquid.m_components[ComponentId::H2S] = 0.54542;

	liquid.m_components[ComponentId::LSC] = 0.35342;
	liquid.m_components[ComponentId::C15_PLUS_AT] = 0.53532;
	liquid.m_components[ComponentId::C6_MINUS_14BT] = 0.353452;
	liquid.m_components[ComponentId::C6_MINUS_14DBT] = 0.6662;
	liquid.m_components[ComponentId::C6_MINUS_14BP] = 0.34532;
	liquid.m_components[ComponentId::C15_PLUS_ARO_S] = 0.3532;
	liquid.m_components[ComponentId::C15_PLUS_SAT_S] = 0.53452;
	liquid.m_components[ComponentId::C6_MINUS_14SAT_S] = 0.5342;
	liquid.m_components[ComponentId::C6_MINUS_14ARO_S] = 0.53532;

	double gormVal = Genex6::PVTCalc::getInstance().computeGorm(vapour,liquid);

	bool equal = NumericFunctions::isEqual(gormVal, 0.58688667253091786, 1e-15);

	EXPECT_EQ(true, equal);
		
}
