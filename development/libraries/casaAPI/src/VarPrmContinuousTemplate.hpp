//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#pragma once
/// @file VarPrmContinuousTemplate.hpp
/// @brief This file keeps API definition for handling continuous parameters.

#include "VarPrmContinuousTemplate.h"

namespace casa
{

template<class Prm>
VarPrmContinuousTemplate<Prm>::VarPrmContinuousTemplate(const Prm& original,
                          const char* name,
                          double minValue,
                          double maxValue,
                          PDF rangeShape ) : VarPrmContinuous()
{
  m_name = name;
  m_pdf = rangeShape;
  m_baseValue.reset( new Prm(original));
  m_baseValue->setParent(this);
  m_minValue.reset( new Prm(this, original, minValue));
  m_maxValue.reset( new Prm(this, original, maxValue));
}

template<class Prm>
std::vector<std::string> VarPrmContinuousTemplate<Prm>::name() const
{
  return {Prm::varprmName()};
}

template<class Prm>
size_t VarPrmContinuousTemplate<Prm>::dimension() const
{
  return 1;
}

template<class Prm>
const char * VarPrmContinuousTemplate<Prm>::typeName() const
{
  return Prm::varprmName().c_str();
}

template<class Prm>
SharedParameterPtr VarPrmContinuousTemplate<Prm>::newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & /*vin*/ ) const
{
  SharedParameterPtr prm(new Prm(mdl, m_baseValue->parameters()));
  prm->setParent(this);
  return prm;
}

template<class Prm>
SharedParameterPtr VarPrmContinuousTemplate<Prm>::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
  double minV = dynamic_cast<Prm*>( m_minValue.get() )->asDoubleArray()[0];
  double maxV = dynamic_cast<Prm*>( m_maxValue.get() )->asDoubleArray()[0];

  double prmV = *vals++;

  if ( minV > prmV || prmV > maxV )
  {
    throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of " << Prm::key() <<
                                                                      ": " << prmV << " falls out of range: [" << minV << ":" << maxV << "]";
  }

  Prm * prmPtr = dynamic_cast<Prm*>(m_baseValue.get());
  SharedParameterPtr prm( new Prm( this, *prmPtr, prmV ) );

  return prm;
}

template<class Prm>
SharedParameterPtr VarPrmContinuousTemplate<Prm>::newParameterFromDoubles(std::vector<double> & vals, int & off) const
{
  std::vector<double>::const_iterator it = vals.begin() + off;
  std::vector<double>::const_iterator sit = it;
  SharedParameterPtr ret = newParameterFromDoubles( it );
  off += static_cast<unsigned int>(it - sit);
  return ret;
}

template<class Prm>
std::vector<double> VarPrmContinuousTemplate<Prm>::asDoubleArray( const SharedParameterPtr prm ) const
{
  return prm->asDoubleArray();
}

template<class Prm>
VarPrmContinuousTemplate<Prm>::VarPrmContinuousTemplate(CasaDeserializer & dz, const unsigned int objVer )
{
  bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer);

  Prm::loadVarPrm(dz, objVer);

  if ( !ok )
  {
    throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << Prm::varprmName() << " deserialization exception";
  }
}

template<class Prm>
void VarPrmContinuousTemplate<Prm>::onCategoryChosen( const Parameter * )
{
}

template<class Prm>
void VarPrmContinuousTemplate<Prm>::onSerialization( CasaSerializer::ObjRefID objSerID )
{
  m_dependsOn.insert( objSerID );
}

}
