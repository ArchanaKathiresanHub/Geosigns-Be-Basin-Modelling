// 
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef _DERIVED_PROPERTIES__MOCK_GRID_H_
#define _DERIVED_PROPERTIES__MOCK_GRID_H_

#include "AbstractGrid.h"

namespace DataModel {

   /// \brief X-coordinate of default origin of grid.
   static double OriginX = 0.0;

   /// \brief Y-coordinate of default origin of grid.
   static double OriginY = 0.0;

   /// \brief Spacing between nodes in x-direction.
   static double DeltaX = 100.0;

   /// \brief Spacing between nodes in y-direction.
   static double DeltaY = 100.0;

   /// \brief Describes a regular grid structure.
   ///
   /// This global grid structure may be defined across multiple processors
   /// an object of this type defines a smaller sub-structre.
   class MockGrid final: public DataModel::AbstractGrid {

   public :

      MockGrid ( const int startI, const int ghostStartI,
                 const int startJ, const int ghostStartJ,
                 const int endI,   const int ghostEndI,
                 const int endJ,   const int ghostEndJ );

      /// return the first local horizontal index
      virtual int firstI ( bool includeGhosts ) const;

      /// return the first local vertical index
      virtual int firstJ ( bool includeGhosts ) const;

      /// return the last local horizontal index
      virtual int lastI ( bool includeGhosts ) const;

      /// return the last local vertical index
      virtual int lastJ ( bool includeGhosts ) const;

      /// return the distance between two vertical gridlines
      virtual double deltaI () const;

      /// return the distance between two horizontal gridlines
      virtual double deltaJ () const;

      /// return the leftmost grid coordinate value
      virtual double minI () const;

      /// return the bottommost grid coordinate value
      virtual double minJ () const;

      /// return the rightmost grid coordinate value
      virtual double maxI () const;

      /// return the topmost grid coordinate value
      virtual double maxJ () const;

      /// return the rightmost grid coordinate value
      virtual int numIGlobal (void) const;

      /// return the topmost grid coordinate value
      virtual int numJGlobal (void) const;

      /// return the leftmost grid coordinate value
      virtual double minIGlobal (void) const;

      /// return the bottommost grid coordinate value
      virtual double minJGlobal (void) const;

      /// return the rightmost grid coordinate value
      virtual double maxIGlobal (void) const;

      /// return the topmost grid coordinate value
      virtual double maxJGlobal (void) const;
 private :

      /// \brief The non-ghost index is in position 0 and the ghost index is in position 1.
      int m_firstI[2];
      int m_lastI[2];

      int m_firstJ[2];
      int m_lastJ[2];

   };

} // namespace DataModel


inline DataModel::MockGrid::MockGrid ( const int startI, const int ghostStartI,
                                       const int startJ, const int ghostStartJ,
                                       const int endI,   const int ghostEndI,
                                       const int endJ,   const int ghostEndJ ) {

   m_firstI [ 0 ] = startI;
   m_firstI [ 1 ] = ghostStartI;

   m_firstJ [ 0 ] = startJ;
   m_firstJ [ 1 ] = ghostStartJ;

   m_lastI [ 0 ] = endI;
   m_lastI [ 1 ] = ghostEndI;

   m_lastJ [ 0 ] = endJ;
   m_lastJ [ 1 ] = ghostEndJ;
}

inline int DataModel::MockGrid::firstI ( const bool includeGhosts ) const {
   return m_firstI [(includeGhosts ? 1 : 0 )];
}

inline int DataModel::MockGrid::firstJ ( const bool includeGhosts ) const {
   return m_firstJ [(includeGhosts ? 1 : 0 )];
}

inline int DataModel::MockGrid::lastI ( const bool includeGhosts ) const {
   return m_lastI [(includeGhosts ? 1 : 0 )];
}

inline int DataModel::MockGrid::lastJ ( const bool includeGhosts ) const {
   return m_lastJ [(includeGhosts ? 1 : 0 )];
}


inline double DataModel::MockGrid::deltaI () const {
   return DeltaX;
}

inline double DataModel::MockGrid::deltaJ () const {
   return DeltaY;
}

inline double DataModel::MockGrid::minI () const {
   return OriginX;
}

inline double DataModel::MockGrid::minJ () const {
   return OriginY;
}

inline double DataModel::MockGrid::maxI () const {
   return OriginX + ( m_lastI [ 1 ] - m_firstI [ 1 ]) * DeltaX;
}

inline double DataModel::MockGrid::maxJ () const {
   return OriginY + ( m_lastJ [ 1 ] - m_firstJ [ 1 ]) * DeltaY;
}

inline int DataModel::MockGrid::numIGlobal () const {
   return lastI( false ) - firstI( false ) + 1;
}

inline int DataModel::MockGrid::numJGlobal () const {
   return lastJ( false ) - firstJ( false ) + 1;
}

inline double DataModel::MockGrid::maxIGlobal () const {
   return maxI();
}

inline double DataModel::MockGrid::minIGlobal () const {
   return minI();
}

inline double DataModel::MockGrid::maxJGlobal () const {
   return maxJ();
}

inline double DataModel::MockGrid::minJGlobal () const {
   return minJ();
}




#endif // _DERIVED_PROPERTIES__MOCK_GRID_H_
