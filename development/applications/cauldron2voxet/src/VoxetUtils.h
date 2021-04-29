//
// Copyright (C) 2021-2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#pragma once
#ifndef CAULDRON2VOXET_H
#define CAULDRON2VOXET_H

#include <vector>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#include <process.h>
#include "values_win.h"
#else
#include <unistd.h>
#include <values.h>
#endif // WIN_32

#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

// DataAccess API includes
#include "GridMap.h"
#include "Grid.h"
#include "Snapshot.h"
#include "Surface.h"
#include "Formation.h"
#include "Reservoir.h"
#include "LithoType.h"
#include "Property.h"
#include "PropertyValue.h"
#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "SimulationDetails.h"

// GeoPhysics library
#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"
#include "SurfaceProperty.h"
#include "PrimarySurfaceProperty.h"
#include "FormationPropertyAtSurface.h"

#include "DepthInterpolator.h"
#include "LayerInterpolator.h"
#include "GridDescription.h"
#include "VoxetCalculator.h"
#include "VoxetPropertyGrid.h"
#include "array.h"

#include "CauldronProperty.h"
#include "VoxetProjectHandle.h"
#include "voxetschema.h"
#include "voxetschemafuncs.h"

#include <string>
#include <vector>

#define Max(a,b)        (a > b ? a : b)
#define Min(a,b)        (a < b ? a : b)

#define UNDEFINED_VALUE_AT_GRID 99999

namespace VoxetUtils
{
    /// Write the values to the specified file.
    void write(const std::string& name,
        const VoxetPropertyGrid& values);


    /// Correct the endian-ness of the array. The voxet format requires that the binary data is written
    /// in big-endian format. After this call the numbers will be un-usable in the code.
    void correctEndian(VoxetPropertyGrid& values);

    /// Correct the endian-ness of a float
    float correctEndian(const float x);

    bool splitString(char* string, char separator, char*& firstPart, char*& secondPart, char*& thirdPart);

    double selectDefined(double undefinedValue, double preferred, double alternative);

    /// Write the ascii voxet header (vo-file)
    void writeVOheader(ofstream& file,
        const GridDescription& gridDescription,
        const string& outputFileName);

    /// Write the property information into the voxet header (vo-file)
    void writeVOproperty(ofstream& file,
        const int& propertyCount,
        const CauldronProperty* cauldronProperty,
        const string& propertyFileName,
        const float& nullValue);

    /// Write the tail of the  ascii voxet header (vo-file)
    void writeVOtail(ofstream& file);

    /// Test if the property is computable at basement
    bool isBasementProperty(const std::string&);

    /// Roundoff a decimal number to certain decimal places
    double roundoff(double number, int decimalPlaces);
}

#endif
