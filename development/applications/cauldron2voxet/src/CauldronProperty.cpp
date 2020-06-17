#include "CauldronProperty.h"
#include "voxetschemafuncs.h"

#include "ProjectHandle.h"

CauldronProperty::CauldronProperty ( const DataAccess::Interface::ProjectHandle& cauldronProjectHandle,
                                     database::Record*         record ) :
   m_cauldronProjectHandle ( cauldronProjectHandle ),
   m_record ( record ) {
}

const std::string& CauldronProperty::getCauldronName () const {
   return database::getCauldronPropertyName ( m_record );
}

const std::string& CauldronProperty::getVoxetName () const {
   return database::getVoxetPropertyName ( m_record );
}

const DataAccess::Interface::Property* CauldronProperty::getProperty () const {
   return m_cauldronProjectHandle.findProperty ( getCauldronName ());
}

bool CauldronProperty::getVoxetOutput () const {
   return database::getVoxetOutput ( m_record );
}

float CauldronProperty::getConversionFactor () const {
   return database::getConversionFactor ( m_record );
}

const std::string& CauldronProperty::getUnits () const {
   return database::getOutputPropertyUnits ( m_record );
}
