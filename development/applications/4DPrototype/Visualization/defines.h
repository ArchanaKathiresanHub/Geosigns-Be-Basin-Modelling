//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

#ifdef _WIN64
#ifdef VISUALIZATIONDLL_EXPORTS
#define VISUALIZATIONDLL_API __declspec(dllexport) 
#else
#define VISUALIZATIONDLL_API __declspec(dllimport) 
#endif
#else
#define VISUALIZATIONDLL_API
#endif
#endif
