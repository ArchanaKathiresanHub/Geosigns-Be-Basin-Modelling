//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROPERTY_H_INCLUDED
#define PROPERTY_H_INCLUDED

#include "GridMapCollection.h"

#include <MeshVizInterface/mesh/data/MiDataSetI.h>
#include <MeshVizInterface/mesh/data/MiDataSetIjk.h>

#include <vector>

namespace DataAccess
{
  namespace Interface
  {
    class GridMap;
  }
}

/**
 *
 */
class FormationProperty : public MiDataSetIjk<double>
{
  GridMapCollection m_values;

  std::string m_name;

  MiDataSet::DataBinding m_binding;

  size_t m_timestamp;

public:

  FormationProperty(const std::string& name, const std::vector<const DataAccess::Interface::GridMap*>& values);

  FormationProperty(const FormationProperty&) = delete;

  FormationProperty& operator=(const FormationProperty&) = delete;

  virtual double get(size_t i, size_t j, size_t k) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;
};

/**
 *
 */
class Formation2DProperty : public MiDataSetIjk<double>
{
  std::vector<const DataAccess::Interface::GridMap*> m_values;

  std::string m_name;

  MiDataSet::DataBinding m_binding;

  size_t m_timestamp;

  double m_minVal;
  double m_maxVal;

public:

  Formation2DProperty(const std::string& name, const std::vector<const DataAccess::Interface::GridMap*>& values);

  Formation2DProperty(const Formation2DProperty&) = delete;

  Formation2DProperty& operator=(const Formation2DProperty&) = delete;

  virtual ~Formation2DProperty();

  virtual double get(size_t i, size_t j, size_t k) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;
};

/**
*
*/
class SurfaceProperty: public MiDataSetI<double>
{
  const DataAccess::Interface::GridMap* m_values;
  unsigned int m_numI;
  unsigned int m_numJ;

  std::string m_name;

  MiDataSet::DataBinding m_binding;

  size_t m_timestamp;

  double m_minVal;
  double m_maxVal;

public:

  SurfaceProperty(const std::string& name, const DataAccess::Interface::GridMap* values);

  virtual ~SurfaceProperty();

  SurfaceProperty(const SurfaceProperty&) = delete;

  SurfaceProperty& operator=(const SurfaceProperty&) = delete;

  virtual double get(size_t i) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;
};

/**
*
*/
class ReservoirProperty: public MiDataSetIjk<double>
{
  const DataAccess::Interface::GridMap* m_values;

  std::string m_name;

  MiDataSet::DataBinding m_binding;

  size_t m_timestamp;

  double m_minVal;
  double m_maxVal;

public:

  ReservoirProperty(const std::string& name, const DataAccess::Interface::GridMap* values);

  virtual ~ReservoirProperty();

  ReservoirProperty(const ReservoirProperty&) = delete;

  ReservoirProperty& operator=(const ReservoirProperty) = delete;

  virtual double get(size_t i, size_t j, size_t k) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;
};

/**
*
*/
class VectorProperty : public MiDataSetIjk<MbVec3d>
{
  const DataAccess::Interface::GridMap* m_values[3];

  std::string m_name;

  MiDataSet::DataBinding m_binding;

  //size_t m_numI;
  //size_t m_numJ;
  //size_t m_numK;

  size_t m_timestamp;

public:

  VectorProperty(const std::string& name, const DataAccess::Interface::GridMap* values[3]);

  virtual ~VectorProperty();

  VectorProperty(const VectorProperty&) = delete;

  VectorProperty& operator=(const VectorProperty&) = delete;

  virtual MbVec3d get(size_t i, size_t j, size_t k) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual MbVec3d getMin() const;

  virtual MbVec3d getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;
};

/**
*
*/
class FormationIdProperty : public MiDataSetIjk<double>
{
  std::vector<double> m_ids;

  double m_minVal;
  double m_maxVal;

  size_t m_timeStamp;

public:

  explicit FormationIdProperty(const std::vector<double>& ids);

  virtual double get(size_t i, size_t j, size_t k) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;
};

/**
 *
 */
class PersistentTrapIdProperty : public MiDataSetIjk<double>
{
  const DataAccess::Interface::GridMap* m_trapIds;
  std::vector<unsigned int> m_translationTable;
  unsigned int m_minId;

  size_t m_timeStamp;

  double m_minVal;
  double m_maxVal;

  double translateId(double id) const;

public:

  PersistentTrapIdProperty(
    const DataAccess::Interface::GridMap* trapIds, 
    const std::vector<unsigned int>& translationTable, 
    unsigned int minId);

  virtual ~PersistentTrapIdProperty();

  PersistentTrapIdProperty(const PersistentTrapIdProperty&) = delete;

  PersistentTrapIdProperty& operator=(const PersistentTrapIdProperty&) = delete;

  virtual double get(size_t i, size_t j, size_t k) const;

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual std::string getName() const;

  virtual size_t getTimeStamp() const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;
};

#endif
