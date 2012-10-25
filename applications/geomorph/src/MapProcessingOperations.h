//------------------------------------------------------------//

#ifndef __MapProcessingOperations__HH__
#define __MapProcessingOperations__HH__

//------------------------------------------------------------//

#include <iostream>
#include "GenericTwoDArray.h"

//------------------------------------------------------------//

namespace AllochMod {


  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file MapProcessingOperations.h
  /// \brief Functions operating on a distribution map.

  /// \namespace MapProcessing
  /// \brief Provides class and operations to enable the processing
  /// of binary maps.

  /// \typedef BinaryMap
  /// \brief Define the binary map.
  ///
  /// A binary map is defined as containing only the values false or true.
  typedef GenericTwoDArray<bool> BinaryMap;

  /// \brief Detect the edge on objects in the binary map.
  ///
  /// The edge is stored in the dual of the input map.
  /// i.e. There is a node defined, in the dual map, at the
  /// centre of every element in the primal map.
  void detectEdge ( const BinaryMap& primalMap,
                          BinaryMap& map );

  /// \brief Perform dilation on inputmap and store result in dilated.
  void dilateMap ( const BinaryMap& input,
                         BinaryMap& dilated );

  /// \brief Perform erosion on inputmap and store result in eroded.
  void erodeMap  ( const BinaryMap& input,
                         BinaryMap& eroded );

  /// \brief Determines whether or not a particular position in the map
  /// should be dilated.
  bool dilateEntry ( const BinaryMap& copy,
                     const int        i,
                     const int        j );

  /// \brief Determines whether or not a particular position in the map
  /// should be eroded.
  bool erodeEntry ( const BinaryMap& copy,
                    const int        i,
                    const int        j );

  /// \brief Output the binary map to the output stream.
  ///
  /// The default output stream is standard out.
  void print ( const BinaryMap& map,
                     std::ostream& o = std::cout,
               const char falseChar = '.',
               const char trueChar  = '#' );


  /** @} */

}

//------------------------------------------------------------//

#endif // __MapProcessingOperations__HH__

//------------------------------------------------------------//
