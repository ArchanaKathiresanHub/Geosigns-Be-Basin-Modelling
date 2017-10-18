//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "MapSmoother.h"

// utilitites
#include "LogHandler.h"
#include "NumericFunctions.h"

// DataAccess library
#include "Interface/GridMap.h"
#include "Interface/Interface.h"

using namespace DataAccess;
using namespace Interface;
using namespace CrustalThicknessInterface;

MapSmoother::MapSmoother( const unsigned int smoothingRadius ) :
   m_ghostNodes(true),
   m_smoothingRadius( smoothingRadius ),
   m_epsilon(1.0e-3)
{}

//------------------------------------------------------------//
//
//
//            j, yr  ---->     ---> sum
//              _____  
//              | | | 
//              -----
//              | | | 
//      |       -----   |
//   xr | i     | | |   | col
//      V       -----   V
//              | | |
//              ----- 
//              | | |
//              -----
//
//
bool MapSmoother::averageSmoothing( GridMap * mapToSmooth ){
   bool status = initialize ( mapToSmooth );
   if (status){
      computeFirstColumnsSums( mapToSmooth );
      computeSmoothedValues  ( mapToSmooth );
      setSmoothedValues      ( mapToSmooth );
      finalize               ( mapToSmooth );
   }
   return status;
}

bool MapSmoother::initialize( GridMap * mapToSmooth ){
   //Check inputs
   if (mapToSmooth == nullptr) {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Trying to smooth a map which do not exist (null pointer)"
         << ", smoothing will be skiped for this map";
      return false;
   }

   //Check in which status the map is given
   //Will be returned with the same status in finalize()
   m_mapToRestore = not mapToSmooth->retrieved();

   //Be sure that the gohst nodes are retrieved
   if (mapToSmooth->retrieved() && m_ghostNodes) {
      mapToSmooth->restoreData();
      mapToSmooth->retrieveData( m_ghostNodes );
   }
   else{
      mapToSmooth->retrieveData();
   }

   //Get the grid size
   m_firstI = mapToSmooth->firstI( m_ghostNodes );
   m_firstJ = mapToSmooth->firstJ( m_ghostNodes );
   m_lastI  = mapToSmooth->lastI ( m_ghostNodes );
   m_lastJ  = mapToSmooth->lastJ ( m_ghostNodes );

   //Initialize the boost arrays
   m_sumMap   .resize(boost::extents[m_lastI - m_firstI + 1][m_lastJ - m_firstJ + 1]);
   m_numberMap.resize(boost::extents[m_lastI - m_firstI + 1][m_lastJ - m_firstJ + 1]);

   //Initialize the 2d arrays
   m_columnMap[0]        = new double[(m_lastJ - m_firstJ + 1)*2];
   m_columnMap[1]        = &m_columnMap[0][m_lastJ - m_firstJ + 1];
   m_numberMapCollect[0] = new int[(m_lastJ - m_firstJ + 1)*2];
   m_numberMapCollect[1] = &m_numberMapCollect[0][m_lastJ - m_firstJ + 1];

   //Define the smoothing radius in x and y
   m_xSmoothingRadius = m_lastI - m_firstI;
   if (m_xSmoothingRadius > m_smoothingRadius) m_xSmoothingRadius = m_smoothingRadius;
   m_ySmoothingRadius = m_lastJ - m_firstJ;
   if (m_ySmoothingRadius > m_smoothingRadius) m_ySmoothingRadius = m_smoothingRadius;

   //Set smoothing boundaries
   m_xLastSmoothedNode = (m_lastI - m_xSmoothingRadius > 0 ? m_lastI - m_xSmoothingRadius : 0);
   m_yLastSmoothedNode = (m_lastJ - m_ySmoothingRadius > 0 ? m_lastJ - m_ySmoothingRadius : 0);

   return true;
}

void MapSmoother::computeFirstColumnsSums( GridMap * mapToSmooth ){
   //
   //  1.  Init the col sums: go along the first row and collect all column sums up to xr    
   //                j  ---->     
   //              _____  
   //         i    |x|x|  col[0][j], m_numberMapCollect[0][j]
   //          |   -----
   //          |   |x|x| 
   //          |   -----   
   //          V   |x|x|   
   //        xr    -----   
   //              | | |
   //              ----- 
   //              | | |
   //              -----
   //
   for (unsigned int j = m_firstJ, jj = 0; j <= m_lastJ; ++j, ++jj) {
      int    num = 0;
      double val = 0;
      bool undefValue = true;
      for (unsigned int i = m_firstI; i <= m_firstI + m_xSmoothingRadius; ++i) {
         if (mapToSmooth->getValue( i, j ) != Interface::DefaultUndefinedMapValue) {
            val += mapToSmooth->getValue( i, j );
            ++num;
            undefValue = false;
         }
      }
      if (undefValue) {
         m_columnMap[0][jj]        = Interface::DefaultUndefinedMapValue;
         m_numberMapCollect[0][jj] = Interface::DefaultUndefinedMapValueInteger;
      }
      else {
         m_columnMap[0][jj]        = val;
         m_numberMapCollect[0][jj] = num;
      }
   }
}

void MapSmoother::computeSmoothedValues( GridMap * mapToSmooth ){
   //
   //  2. For each row the first step is to init the first sum: sum up column sums of first yr columns.
   //            j ----- yr    
   //              _____  
   //              |x| | --> collect sum 
   //              -----
   //              | | | 
   //      |       -----   
   //      | i     | | |   
   //      V       -----   
   //              | | |
   //              ----- 
   //              | | |
   //              -----
   //
   unsigned int ii = 0;
   unsigned int ii1 = 1;

   for (unsigned int i = m_firstI; i <= m_lastI; ++i) {
      //compute the first value
      int    num = 0;
      double val = 0;
      bool undefValue = true;
      for (unsigned int j = m_firstJ, jj = 0; j <= m_firstJ + m_ySmoothingRadius; ++j, ++jj) {
         if (m_columnMap[ii][jj] != Interface::DefaultUndefinedMapValue) {
            val += m_columnMap       [ii][jj];
            num += m_numberMapCollect[ii][jj];
            undefValue = false;
         }
      }
      if (!undefValue) {
         m_sumMap   [i - m_firstI][0] = val;
         m_numberMap[i - m_firstI][0] = num;
      }
      else {
         m_sumMap   [i - m_firstI][0] = Interface::DefaultUndefinedMapValue;
         m_numberMap[i - m_firstI][0] = Interface::DefaultUndefinedMapValueInteger;
      }
      for (unsigned int j = m_firstJ, jj = 0; j <= m_lastJ; ++j, ++jj) {
         val = 0;
         num = 0;
         undefValue = true;
         if (j < m_lastJ) {
            // moving the window from left to right, update the sums 
            if (m_sumMap[i - m_firstI][j - m_firstJ] != Interface::DefaultUndefinedMapValue) {
               // collect the sum for sum[j+1] (next in the row)
               val = m_sumMap   [i - m_firstI][j - m_firstJ];
               num = m_numberMap[i - m_firstI][j - m_firstJ];
               undefValue = false;
            }
            if (j < m_yLastSmoothedNode && m_yLastSmoothedNode != 0) {
               if (m_columnMap[ii][jj + m_ySmoothingRadius + 1] != Interface::DefaultUndefinedMapValue) {
                  // update the sum: add the next column from the right
                  val += m_columnMap       [ii][jj + m_ySmoothingRadius + 1];
                  num += m_numberMapCollect[ii][jj + m_ySmoothingRadius + 1];
                  undefValue = false;
               }
            }
            if (j >= m_firstJ + m_ySmoothingRadius) {
               if (m_columnMap[ii][jj - m_ySmoothingRadius] != Interface::DefaultUndefinedMapValue) {
                  // update the sum: substruct the column from the left
                  val -= m_columnMap       [ii][jj - m_ySmoothingRadius];
                  num -= m_numberMapCollect[ii][jj - m_ySmoothingRadius];
                  undefValue = false;
               }
            }
            if (!undefValue) {
               m_sumMap   [i - m_firstI][j - m_firstJ + 1] = val;
               m_numberMap[i - m_firstI][j - m_firstJ + 1] = num;
            }
            else {
               m_sumMap   [i - m_firstI][j - m_firstJ + 1] = Interface::DefaultUndefinedMapValue;
               m_numberMap[i - m_firstI][j - m_firstJ + 1] = Interface::DefaultUndefinedMapValueInteger;
            }
         }
         if (i < m_lastI) {
            // update the column sum in the row below
            undefValue = true;
            val = 0;
            num = 0;
            if (m_columnMap[ii][jj] != Interface::DefaultUndefinedMapValue) {
               val = m_columnMap       [ii][jj];
               num = m_numberMapCollect[ii][jj];
               undefValue = false;
            }
            if (i < m_xLastSmoothedNode && m_xLastSmoothedNode != 0) {
               if (mapToSmooth->getValue( i + m_xSmoothingRadius + 1, j ) != Interface::DefaultUndefinedMapValue) {
                  // add value under
                  val += mapToSmooth->getValue( i + m_xSmoothingRadius + 1, j );
                  ++num;
                  undefValue = false;
               }
            }
            if (i >= m_firstI + m_xSmoothingRadius) {
               if (mapToSmooth->getValue( i - m_xSmoothingRadius, j ) != Interface::DefaultUndefinedMapValue) {
                  // substract value above
                  val -= mapToSmooth->getValue( i - m_xSmoothingRadius, j );
                  --num;
                  undefValue = false;
               }
            }
            if (undefValue) {
               m_columnMap       [ii1][jj] = Interface::DefaultUndefinedMapValue;
               m_numberMapCollect[ii1][jj] = Interface::DefaultUndefinedMapValueInteger;
            }
            else {
               m_columnMap       [ii1][jj] = val;
               m_numberMapCollect[ii1][jj] = num;
            }
         }
      }
      if (ii == 0){
         ii  = 1;
         ii1 = 0;
      }
      else{
         ii  = 0;
         ii1 = 1;
      }
   }
}

void MapSmoother::setSmoothedValues( GridMap * mapToSmooth ){
   int footPrintI = (m_ghostNodes ? (m_lastI - m_firstI > 4 ? 2 : 0) : 0);
   int footPrintJ = (m_ghostNodes ? (m_lastJ - m_firstJ > 4 ? 2 : 0) : 0);
   for (unsigned int i = m_firstI + footPrintI; i <= m_lastI - footPrintI; ++i) {

      for (unsigned int j = m_firstJ + footPrintJ; j <= m_lastJ - footPrintJ; ++j) {
         double val = m_sumMap   [i - m_firstI][j - m_firstJ];
         int    num = m_numberMap[i - m_firstI][j - m_firstJ];

         if (val != Interface::DefaultUndefinedMapValue && mapToSmooth->getValue( i, j ) != Interface::DefaultUndefinedMapValue) {
            if (num == Interface::DefaultUndefinedMapValueInteger) {
               // This can happen when outside the AOI
               num = 1;
            }
            else if (num == 0) {
               // If this happen then something is wrong in the smoothing algorithm
               LogHandler( LogHandler::ERROR_SEVERITY ) << "Zero numerical value in the smoothing algorithm! Will be set to 1.";
               num = 1;
            }
            else  if (num < 0) {
               // If this happen then something is wrong in the smoothing algorithm
               LogHandler( LogHandler::ERROR_SEVERITY ) << "Negative numerical value in the smoothing algorithm! Will be set to 1.";
               num = 1;
            }
            double multVal = 1.0 / static_cast<double>(num);
            double smoothedValue = val  * multVal;
            // Correct from numerical errors during summation and subtraction of columns
            // If the result is of the order of one millimeter or less, then we assume this value is due
            // to numerical errors durring smoothing and we set it to zero
            if (NumericFunctions::inRange( smoothedValue, -m_epsilon, m_epsilon )){
               smoothedValue = 0.0;
            }
            mapToSmooth->setValue( i, j, smoothedValue );
         }
      }
   }
}

void MapSmoother::finalize( GridMap * mapToSmooth ){

   delete[] m_columnMap[0];
   delete[] m_numberMapCollect[0];
   if (m_mapToRestore){
      mapToSmooth->restoreData();
   }
}