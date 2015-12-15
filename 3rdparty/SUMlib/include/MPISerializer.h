// Copyright 2014, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MPISERIALIZER_H
#define SUMLIB_MPISERIALIZER_H


#include "SUMlib.h"
#include "StreamSerializer.h"
#include "mpi.h"

namespace SUMlib {

// This class implements the StreamDeserializer and StreamSerializer interface.
class MPISerializer
   : public StreamDeserializer, public StreamSerializer
{
public:
   MPISerializer( MPI_Comm, int );
   virtual ~MPISerializer();

   virtual bool loadBytes( void*, unsigned int );

   virtual bool saveBytes( const void*, unsigned int );

private:    // Data

   MPI_Comm m_comm;
   int      m_rank;
}; // class MPISerializer

}

#endif // SUMLIB_MPISERIALIZER_H

