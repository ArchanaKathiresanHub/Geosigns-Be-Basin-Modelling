//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DATA_MODEL__ABSTRACT_GRID_H_
#define _DATA_MODEL__ABSTRACT_GRID_H_

namespace DataModel {

   /// \brief Describes a regular grid structure.
   ///
   /// This global grid structure may be defined across multiple processors
   /// an object of this type defines a smaller sub-structre.
   class AbstractGrid {

   public :

      virtual ~AbstractGrid () = default;

      /// return the first local horizontal index
      virtual int firstI ( bool includeGhosts ) const = 0;

      /// return the first local vertical index
      virtual int firstJ ( bool includeGhosts ) const = 0;

      /// return the last local horizontal index
      virtual int lastI ( bool includeGhosts ) const = 0;

      /// return the last local vertical index
      virtual int lastJ ( bool includeGhosts ) const = 0;

      /// return the distance between two vertical gridlines
      virtual double deltaI () const = 0;

      /// return the distance between two horizontal gridlines
      virtual double deltaJ () const = 0;

      /// return the leftmost grid coordinate value
      virtual double minI () const = 0;

      /// return the bottommost grid coordinate value
      virtual double minJ () const = 0;

      /// return the rightmost grid coordinate value
      virtual double maxI () const = 0;

      /// return the topmost grid coordinate value
      virtual double maxJ () const = 0;

      /// return the rightmost grid coordinate value
      virtual int numIGlobal (void) const = 0;

      /// return the topmost grid coordinate value
      virtual int numJGlobal (void) const = 0;

      /// return the leftmost grid coordinate value
      virtual double minIGlobal (void) const = 0;

      /// return the bottommost grid coordinate value
      virtual double minJGlobal (void) const = 0;

      /// return the rightmost grid coordinate value
      virtual double maxIGlobal (void) const = 0;

      /// return the topmost grid coordinate value
      virtual double maxJGlobal (void) const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_GRID_H_
