#include "ConstantGridInterpolator.h"

Genex6::ConstantGridInterpolator::ConstantGridInterpolator ( const DataAccess::Interface::GridMap*  data ) : 
   m_data ( data )
{
}
   
Genex6::ConstantGridInterpolator::~ConstantGridInterpolator() {
   m_data = 0;
}

double Genex6::ConstantGridInterpolator::evaluateProperty ( const int &I, 
                                                            const int &J ) const {
   return m_data->getValue ( (unsigned int)I, (unsigned int)J );
}

