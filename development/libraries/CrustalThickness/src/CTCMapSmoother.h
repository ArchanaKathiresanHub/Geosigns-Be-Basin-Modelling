//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_MAPSMOOTHER_H_
#define _CRUSTALTHICKNESS_MAPSMOOTHER_H_

namespace DataAccess {
   namespace Interface {
      class GridMap;
   }
}

#include <boost/multi_array.hpp>

namespace CrustalThicknessInterface {

   /// @class Class used to smooth maps by averaging values on a specifed radius
   class MapSmoother  {

   public:

      /// @param smoothingRadius The smoothing radius
      /// @details Sets the error range epsilon to 1.0e-3
      MapSmoother( const unsigned int smoothingRadius );
      ~MapSmoother() {}

      /// @brief Smooth (by simle average) the given map according to the smoothing radius which has been defined in the constructor
      bool averageSmoothing( DataAccess::Interface::GridMap * mapToSmooth );

   private:

      /// @defgroup smooth_workflow
      /// @brief To be executed in this order
      /// @{
      /// @brief Initialize class attributes according to the map to smooth
      /// @details Also retreive maps data with gohst nodes
      bool initialize( DataAccess::Interface::GridMap * mapToSmooth );
      /// @brief Compute the first columns along the first row according to the map to smooth
      void computeFirstColumnsSums( DataAccess::Interface::GridMap * mapToSmooth );
      /// @brief Populate m_sumMap and m_numberMap according to the map to smooth
      void computeSmoothedValues( DataAccess::Interface::GridMap * mapToSmooth );
      /// @brief Compute the first columns (up to the smoothing radius) along the first row according to the map to smooth
      void setSmoothedValues( DataAccess::Interface::GridMap * mapToSmooth );
      /// @brief Destruct dynamic memory objects
      /// @details Also restore maps data
      void finalize( DataAccess::Interface::GridMap * mapToSmooth );
      /// @}


      bool m_mapToRestore; ///< Tell if we need to restore the map or not after the smoothing

      /// @defgroup TemporaryArrays
      /// @{
      boost::multi_array<double, 2> m_sumMap;    ///< Maps each (i,j) to the sum of the cell values included in the smoothing radius
      boost::multi_array<int,    2> m_numberMap; ///< Maps each (i,j) to the number of cells included in the smoothing radius
      /// @}

      /// @defgroup InputMapRange
      /// Loaded from map to smooth
      /// @{
      unsigned int m_firstI;
      unsigned int m_firstJ;
      unsigned int m_lastI;
      unsigned int m_lastJ;
      const bool   m_ghostNodes; ///< Set to true in constructor (for debug purpose only)
      /// @}

      /// @defgroup Smoothing
      /// @{
      const unsigned int m_smoothingRadius; ///< The smoothing radius
      unsigned int m_xSmoothingRadius;      ///< The smoothing radius in the i direction
      unsigned int m_ySmoothingRadius;      ///< The smoothing radius in the j direction
      unsigned int m_xLastSmoothedNode;     ///< The last smoothed radius in the i direction
      unsigned int m_yLastSmoothedNode;     ///< The last smoothed radius in the j direction
      /// @}

      /// @defgroup Maping
      /// @{
      double * m_columnMap[2];        ///< Maping between (i,j) and the value of its column
      int *    m_numberMapCollect[2]; ///< Maping between (i,j) and the number of cells of its column
      /// @}

      /// @defgroup Errors
      /// @{
      /// \brief The error range for post-smoothing correction
      /// \details If the smoothed value is between -epsilon and +epsilon
      ///    then the smoothed value is set to 0
      const double m_epsilon;
      /// @}

   };

}
#endif