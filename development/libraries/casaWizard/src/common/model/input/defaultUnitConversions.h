#pragma once

#include <QMap>
#include <QString>

namespace casaWizard {

QMap<QString, double> defaultUnitConversions =
{
    {"F", 0.3048},
    {"fT", 0.3048},
    {"f", 0.3048},
    {"Ft", 0.3048},
    {"ft", 0.3048},
    {"FT", 0.3048},
    {"GR/CC3", 1000},
    {"g/cm3", 1000},
    {"g/cc", 1000},
    {"G/CM3", 1000},
    {"G/C3", 1000},
    {"API", 1.0},
    {"api", 1.0},
    {"APIG", 1.0},
    {"gAPI", 1.0},
    {"GAPI", 1.0},
    {"US/M", 1.0},
    {"us/m", 1.0},
    {"m", 1.0},
    {"M", 1.0},
    {"M/S", 1.0},
    {"us/ft", 3.28083989501},
    {"U/FT", 3.28083989501},
    {"US/F", 3.28083989501},
    {"US/FT", 3.28083989501},
    {"us/m", 1.0},
};

} // namespace casaWizard
