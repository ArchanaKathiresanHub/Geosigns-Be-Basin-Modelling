//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_TABLEFUNCTORSIMPLE_H
#define INTERFACE_TABLEFUNCTORSIMPLE_H

// std library
#include <memory>

// DataAccess library
#include "ProjectHandle.h"
#include "database.h"
using database::Record;

namespace DataAccess
{

   namespace Interface
   {
      /// @class TableFunctorSimple Generic template class which contains the method to read one line of an IoTbl
      ///    To load multiple lines see TableFunctorCompound class
      template< class T >
      class TableFunctorSimple{

      public:
         // pointer to member function of object factory
         typedef std::shared_ptr<const T>( ObjectFactory::*produceData )(ProjectHandle* projectHandle, Record* record) const;

         /// @brief Construct a functor wich can load one data accessors (for the unique line) of an IoTbl and store it in m_data
         /// @param[in] produceFunction The member function of the object factory which will produce the data accessors
         /// @param[out] data The placeholder where will be stored the data accessor
         TableFunctorSimple( ProjectHandle& projectHandle, const produceData produceFunction, std::shared_ptr<const T>& data ) :
            m_projectHandle( projectHandle ),
            m_produceData( produceFunction ),
            m_data( data ){}

         /// @brief The method which produces the data accessor from the object factory specified by m_produceData
         ///    and for the given record
         /// @details The accessor is stored in m_data
         /// @param[in] record Record corresponding to one line of the IoTbl
         void operator() ( Record* record );

      private:
         ProjectHandle& m_projectHandle;
         const produceData m_produceData;  ///< Pointer to the member function of the object factory which will produce the data accessor
         std::shared_ptr<const T>& m_data; ///< The data accessors produced by m_produceData
      };
   }
}

template< class T >
void DataAccess::Interface::TableFunctorSimple<T>::operator()( Record* record ){
   m_data = (m_projectHandle.getFactory()->*m_produceData)(&m_projectHandle, record);
}

#endif