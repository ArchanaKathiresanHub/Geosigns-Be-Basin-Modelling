//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Reader for data extracted using Track1D from a 3D run
#pragma once

#include <QStringList>
#include <QVector>

namespace casaWizard
{

namespace sac
{

class Case3DTrajectoryReader
{
public:
   Case3DTrajectoryReader(const QString filename);
   void read();

   QStringList properties() const;
   QVector<double> x() const;
   QVector<double> y() const;
   QVector<double> depth() const;
   QVector<double> readProperty(const QString property) const;

private:
   void extractProperties(const QString header);

   QString m_filename;
   QStringList m_properties;

   QVector<double> m_x;
   QVector<double> m_y;
   QVector<double> m_depth;
   QVector<QVector<double>> m_propertyValues;
};

}  // namespace sac

}  // namespace casaWizard
