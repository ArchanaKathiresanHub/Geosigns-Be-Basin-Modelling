//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_TABLEFUNCTORCOMPOUND_H
#define INTERFACE_TABLEFUNCTORCOMPOUND_H

// std library
#include <vector>

// DataAccess library
#include "ProjectHandle.h"
#include "database.h"
using database::Record;

namespace DataAccess
{

   namespace Interface
   {
      /// @class TableFunctorCompound Generic template class which contains the method to read multiple lines of an IoTbl
      ///    To load only one line see TableFunctorSimple class
      template< class T >
      class TableFunctorCompound{

      public:
         // pointer to member function of object factory
         typedef std::shared_ptr<const T>( ObjectFactory::*produceData )(ProjectHandle* projectHandle, Record* record) const;

         /// @brief Construct a functor wich can load multiple data accessors (one per line) of an IoTbl and store them in m_data
         /// @param[in] produceFunction The member function of the object factory which will produce the data accessors
         /// @param[out] vector The list where will be stored the data accessors (one per line of the IoTbl)
         TableFunctorCompound( ProjectHandle& projectHandle, const produceData produceFunction, std::vector<std::shared_ptr<const T>>& vector ) :
            m_projectHandle( projectHandle ),
            m_produceData( produceFunction ),
            m_data( vector ){}

         /// @brief The method which produces the data accessor from the object factory specified by m_produceData
         ///    and for the given record
         /// @details The accessors are stored in m_data
         /// @param[in] record Record corresponding to one line of the IoTbl
         void operator() ( Record* record );

      private:
         ProjectHandle&  m_projectHandle;
         const produceData m_produceData;               ///< Pointer to the member function of the object factory which will produce the data accessors
         std::vector<std::shared_ptr<const T>>& m_data; ///< List of the data accessors produced by m_produceData
      };
   }
}

template< class T >
void DataAccess::Interface::TableFunctorCompound<T>::operator()( Record* record ){
   m_data.push_back( (m_projectHandle.getFactory()->*m_produceData)(&m_projectHandle, record) );
}

#endif