//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mpi.h"

#include <assert.h>

#include <iostream>
#include <vector>


using namespace std;

#include<sstream>
using std::ostringstream;
extern std::ostringstream cerrstrstr;

#include "rankings.h"
#include "RequestDefs.h"
#include "RequestHandling.h"
#include "MigrationReservoir.h"
#include "Migrator.h"

int messageId = 0;
int requestHandlingCycle = 0;

/// Initialize a new request handling phase.
namespace migration
{

   void RequestHandling::StartRequestHandling (Migrator * requestHandler, const std::string& functionName)
   {
      RequestHandling::GetInstance ()->startRequestHandling (requestHandler, functionName);
   }

   /// Complete a request handling phase.
   /// Will finish after all processors have completed their phase.
   void RequestHandling::FinishRequestHandling ()
   {
      RequestHandling::GetInstance ()->finishRequestHandling ();
   }
   //-------------------------------------------------------------------------------------------------------------//
   /// 1. send a ColumnValueRequest request to another processor
   void RequestHandling::SendRequest (ColumnValueRequest & valueRequest, ColumnValueRequest & valueResponse)
   {
      int rank = GetRank (valueRequest.i, valueRequest.j);

      assert (RequestHandling::ProxyUseAllowed ());

      MPI_Bsend (&valueRequest, 1, ColumnValueType, rank, COLUMNVALUEREQUEST, PETSC_COMM_WORLD);

      if (valueRequest.valueSpec > SET)
      {
         HandleRequests (UNTILOUTOFREQUESTS);
         return;
      }
      else
      {
         HandleRequests (UNTILRESPONDED, &valueResponse);
      }
   }

   void RequestHandling::handleColumnValueRequest (const int & source)
   {
      ColumnValueRequest valueRequest;
      MPI_Status recvStatus;
      MPI_Recv (&valueRequest, 1, ColumnValueType, source, COLUMNVALUEREQUEST, PETSC_COMM_WORLD, &recvStatus);

      if (valueRequest.valueSpec > SET)
      {
         m_requestHandler->getReservoir (valueRequest.reservoirIndex)->setValue (valueRequest);
      }
      else
      {
         ColumnValueRequest columnValueResponse;
         m_requestHandler->getReservoir (valueRequest.reservoirIndex)->getValue (valueRequest, columnValueResponse);

         MPI_Bsend (&columnValueResponse, 1, ColumnValueType, source, COLUMNVALUERESPONSE, PETSC_COMM_WORLD);
      }
   }

   void RequestHandling::handleColumnValueResponse (const int & source, ColumnValueRequest * columnValueResponse)
   {
      MPI_Status recvStatus;

      assert (columnValueResponse);
      MPI_Recv (columnValueResponse, 1, ColumnValueType, source, COLUMNVALUERESPONSE, PETSC_COMM_WORLD, &recvStatus);
   }

   //-------------------------------------------------------------------------------------------------------------//
   // 2. send a ColumnColumnRequest request to another processor
   void RequestHandling::SendRequest (ColumnColumnRequest & columnRequest)
   {
      assert (RequestHandling::ProxyUseAllowed ());

      MPI_Bsend (&columnRequest, 1, ColumnColumnType, GetRank (columnRequest.i, columnRequest.j), COLUMNREQUEST, PETSC_COMM_WORLD);
      assert (columnRequest.valueSpec > SET);

      HandleRequests (UNTILOUTOFREQUESTS);
      return;
   }

   void RequestHandling::handleColumnRequest (const int & source)
   {
      ColumnColumnRequest columnRequest;
      MPI_Status recvStatus;
      MPI_Recv (&columnRequest, 1, ColumnColumnType, source, COLUMNREQUEST, PETSC_COMM_WORLD, &recvStatus);

      m_requestHandler->getReservoir (columnRequest.reservoirIndex)->manipulateColumn (columnRequest);
   }

   //-------------------------------------------------------------------------------------------------------------//
   // 3. send  properties of a trap to processor 0 to be saved in the TrapIoTbl
   void RequestHandling::SendRequest (TrapPropertiesRequest & tpRequest)
   {
      assert (RequestHandling::ProxyUseAllowed ());

      MPI_Bsend (&tpRequest, 1, TrapPropertiesType, 0, TRAPPROPERTIESREQUEST, PETSC_COMM_WORLD);
      assert (tpRequest.valueSpec > SET);

      HandleRequests (UNTILOUTOFREQUESTS);
      return;
   }

   void RequestHandling::handleTrapPropertiesRequest (const int & source)
   {
      TrapPropertiesRequest trapRequest;
      MPI_Status recvStatus;

      MPI_Recv (&trapRequest, 1, TrapPropertiesType, source, TRAPPROPERTIESREQUEST, PETSC_COMM_WORLD, &recvStatus);

      assert (trapRequest.valueSpec > SET);

      m_requestHandler->getReservoir (trapRequest.reservoirIndex)->processTrapProperties (trapRequest);
   }
   //-------------------------------------------------------------------------------------------------------------//
   // 4. send a migration to processor 0 to be ultimately saved in the MigrationIoTbl
   void RequestHandling::SendMigrationRequest (MigrationRequest & request)
   {
      assert (RequestHandling::ProxyUseAllowed ());

      MPI_Bsend (&request, 1, MigrationType, 0, MIGRATIONREQUEST, PETSC_COMM_WORLD);

      HandleRequests (UNTILOUTOFREQUESTS);
      return;
   }

   void RequestHandling::handleMigrationRequest (const int & source)
   {
      MigrationRequest migrationRequest;

      MPI_Status recvStatus;
      MPI_Recv (&migrationRequest, 1, MigrationType, source, MIGRATIONREQUEST, PETSC_COMM_WORLD, &recvStatus);

      m_requestHandler->getReservoir (migrationRequest.reservoirIndex)->processMigration (migrationRequest);
   }
   //-------------------------------------------------------------------------------------------------------------//
   // 5 a. send a charge request to another processor
   void RequestHandling::SendRequest (ColumnCompositionRequest & chargesRequest, ColumnCompositionRequest & chargesResponse)
   {
      assert (RequestHandling::ProxyUseAllowed ());

      MPI_Bsend (&chargesRequest, 1, ColumnCompositionType, GetRank (chargesRequest.i, chargesRequest.j), COLUMNCOMPOSITIONREQUEST, PETSC_COMM_WORLD);
      if (chargesRequest.valueSpec > SET)
      {
         HandleRequests (UNTILOUTOFREQUESTS);
         return;
      }
      else
      {
         HandleRequests (UNTILRESPONDED, &chargesResponse);
      }
   }

   void RequestHandling::handleColumnCompositionRequest (const int & source)
   {
      ColumnCompositionRequest columnCompositionRequest;
      ColumnCompositionRequest columnCompositionResponse;

      MPI_Status recvStatus;
      MPI_Recv (&columnCompositionRequest, 1, ColumnCompositionType, source, COLUMNCOMPOSITIONREQUEST, PETSC_COMM_WORLD, &recvStatus);

      if (columnCompositionRequest.valueSpec > SET)
      {
         m_requestHandler->getReservoir (columnCompositionRequest.reservoirIndex)->manipulateColumnComposition (columnCompositionRequest);
      }
      else
      {
         ColumnCompositionRequest columnCompositionResponse;
         m_requestHandler->getReservoir (columnCompositionRequest.reservoirIndex)->getColumnComposition (columnCompositionRequest, columnCompositionResponse);

         MPI_Bsend (&columnCompositionResponse, 1, ColumnCompositionType, source, COLUMNCOMPOSITIONRESPONSE, PETSC_COMM_WORLD);
      }
   }

   void RequestHandling::handleColumnCompositionResponse (const int & source, ColumnCompositionRequest * columnCompositionResponse)
   {
      MPI_Status recvStatus;

      assert (columnCompositionResponse);
      MPI_Recv (columnCompositionResponse, 1, ColumnCompositionType, source, COLUMNCOMPOSITIONRESPONSE, PETSC_COMM_WORLD, &recvStatus);
   }

   //-------------------------------------------------------------------------------------------------------------//

   /// 5 b. send a request for an addition to a local buffer
   void RequestHandling::SendRequest(ColumnCompositionPositionRequest & chargesRequest, ColumnCompositionPositionRequest & chargesResponse)
   {
	   assert(RequestHandling::ProxyUseAllowed());

	   MPI_Bsend(&chargesRequest, 1, ColumnCompositionPositionType, GetRank(chargesRequest.i, chargesRequest.j), COLUMNCOMPOSITIONPOSITIONREQUEST, PETSC_COMM_WORLD);
	   
	   HandleRequests(UNTILOUTOFREQUESTS);
   }

   void RequestHandling::handleColumnCompositionPositionRequest(const int & source)
   {
	   ColumnCompositionPositionRequest valueRequest;
	   MPI_Status recvStatus;
	   MPI_Recv(&valueRequest, 1, ColumnCompositionPositionType, source, COLUMNCOMPOSITIONPOSITIONREQUEST, PETSC_COMM_WORLD, &recvStatus);

	   m_requestHandler->getReservoir(valueRequest.reservoirIndex)->manipulateColumnCompositionPosition(valueRequest);
   }

   //-------------------------------------------------------------------------------------------------------------//

   /// 6. send a request to reset a proxy column
   void RequestHandling::SendProxyReset (int rank, ColumnValueRequest & resetRequest)
   {
      assert (RequestHandling::ProxyUseAllowed ());

      MPI_Bsend (&resetRequest, 1, ColumnValueType, rank, COLUMNRESETREQUEST, PETSC_COMM_WORLD);

      HandleRequests (UNTILOUTOFREQUESTS);
   }

   void RequestHandling::handleColumnResetRequest (const int & source)
   {
      ColumnValueRequest valueRequest;
      MPI_Status recvStatus;
      MPI_Recv (&valueRequest, 1, ColumnValueType, source, COLUMNRESETREQUEST, PETSC_COMM_WORLD, &recvStatus);

      m_requestHandler->getReservoir (valueRequest.reservoirIndex)->clearProxyProperties (valueRequest);
   }

   //-------------------------------------------------------------------------------------------------------------//

   /// 7. send a FormationNodeValueRequest request  
   void RequestHandling::SendFormationNodeValueRequest (FormationNodeValueRequest & formationNodeValueRequest, FormationNodeValueRequest & formationNodeValueResponse)
   {
      int rank = GetRank (formationNodeValueRequest.i, formationNodeValueRequest.j);

      assert (RequestHandling::ProxyUseAllowed ());

      formationNodeValueRequest.messageId = ++messageId;

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "SendFormationNodeValueRequest (" << rank << ", " <<
         formationNodeValueRequest.messageId << ", " << ValueSpecNames[formationNodeValueRequest.valueSpec] << ", " << formationNodeValueRequest.formationIndex << ", " <<
         formationNodeValueRequest.i << ", " << formationNodeValueRequest.j << ", " << formationNodeValueRequest.k << ")" << endl;
      logstream.flush ();
#endif

      MPI_Bsend (&formationNodeValueRequest, 1, FormationNodeValueType, rank, FORMATIONNODEVALUEREQUEST, PETSC_COMM_WORLD);
      if (formationNodeValueRequest.valueSpec > SET)
      {
         HandleRequests (UNTILOUTOFREQUESTS);
         return;
      }
      else
      {
         HandleRequests (UNTILRESPONDED, &formationNodeValueResponse);
      }
   }

   void RequestHandling::handleFormationNodeValueRequest (const int & source)
   {
      MPI_Status recvStatus;
      FormationNodeValueRequest valueRequest;
      MPI_Recv (&valueRequest, 1, FormationNodeValueType, source, FORMATIONNODEVALUEREQUEST, PETSC_COMM_WORLD, &recvStatus);

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "handleFormationNodeValueRequest (" << source << ", " << valueRequest.messageId << ", " <<
         ValueSpecNames[valueRequest.valueSpec] <<
         ", " << valueRequest.formationIndex << ", " <<
         valueRequest.i << ", " << valueRequest.j << ", " << valueRequest.k << ")" << endl;
      logstream.flush ();
#endif

      if (valueRequest.valueSpec > SET)
      {

         m_requestHandler->getFormation (valueRequest.formationIndex)->setValue (valueRequest);
      }
      else
      {
         FormationNodeValueRequest formationNodeValueResponse;

#if 0
         cerr << GetRankString () << ": " << m_requestHandler->getFormation (valueRequest.formationIndex)->getName () << " (" << valueRequest.formationIndex << ")"
            "->handleFormationNodeValueRequest (" << ValueSpecNames[valueRequest.valueSpec] << ")" << endl;
#endif

         m_requestHandler->getFormation (valueRequest.formationIndex)->getValue (valueRequest, formationNodeValueResponse);

         formationNodeValueResponse.messageId = ++messageId;

#ifdef USELOGSTREAM
         logstream << GetRankString () << ": " << "sendFormationNodeValueResponse (" << source << ", " << formationNodeValueResponse.messageId << ", " <<
            ValueSpecNames[formationNodeValueResponse.valueSpec] <<
            ", " << formationNodeValueResponse.formationIndex << ", " <<
            formationNodeValueResponse.i << ", " << formationNodeValueResponse.j << ", " << formationNodeValueResponse.k << ")" << endl;
         logstream.flush ();
#endif
         //here is sending and calculating the setFinite element values
         MPI_Bsend (&formationNodeValueResponse, 1, FormationNodeValueType, source, FORMATIONNODEVALUERESPONSE, PETSC_COMM_WORLD);
      }
   }

   void RequestHandling::handleFormationNodeValueResponse (const int & source, FormationNodeValueRequest * formationNodeValueResponse)
   {
      MPI_Status recvStatus;

      assert (formationNodeValueResponse);
      MPI_Recv (formationNodeValueResponse, 1, FormationNodeValueType, source, FORMATIONNODEVALUERESPONSE, PETSC_COMM_WORLD, &recvStatus);

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "handleFormationNodeValueResponse (" << source << ", " << formationNodeValueResponse->messageId << ", " <<
         ValueSpecNames[formationNodeValueResponse->valueSpec] <<
         ", " << formationNodeValueResponse->formationIndex << ", " <<
         formationNodeValueResponse->i << ", " << formationNodeValueResponse->j << ", " << formationNodeValueResponse->k << ")" << endl;
      logstream.flush ();
#endif
   }
   //-------------------------------------------------------------------------------------------------------------//

   /// 8. send a FormationNodeCompositionRequest request  
   void RequestHandling::SendFormationNodeCompositionRequest (FormationNodeCompositionRequest & formationNodeCompositionRequest, FormationNodeCompositionRequest & formationNodeCompositionResponse)
   {
      int rank = GetRank (formationNodeCompositionRequest.i, formationNodeCompositionRequest.j);

      assert (RequestHandling::ProxyUseAllowed ());

      formationNodeCompositionRequest.messageId = ++messageId;

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "SendFormationNodeCompositionRequest (" << rank << ", " <<
         formationNodeCompositionRequest.messageId << ", " << ValueSpecNames[formationNodeCompositionRequest.valueSpec] << ", " << formationNodeCompositionRequest.formationIndex << ", " <<
         formationNodeCompositionRequest.i << ", " << formationNodeCompositionRequest.j << ", " << formationNodeCompositionRequest.k << ")" << endl;
      logstream.flush ();
#endif

      MPI_Bsend (&formationNodeCompositionRequest, 1, FormationNodeCompositionType, rank, FORMATIONNODECOMPOSITIONREQUEST, PETSC_COMM_WORLD);
      if (formationNodeCompositionRequest.valueSpec > SET)
      {
         HandleRequests (UNTILOUTOFREQUESTS);
         return;
      }
      else
      {
         HandleRequests (UNTILRESPONDED, &formationNodeCompositionResponse);
      }
   }

   void RequestHandling::handleFormationNodeCompositionRequest (const int & source)
   {
      FormationNodeCompositionRequest formationNodeCompositionRequest;
      FormationNodeCompositionRequest formationNodeCompositionResponse;

      MPI_Status recvStatus;
      MPI_Recv (&formationNodeCompositionRequest, 1, FormationNodeCompositionType, source, FORMATIONNODECOMPOSITIONREQUEST, PETSC_COMM_WORLD, &recvStatus);

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "handleFormationNodeCompositionRequest (" << source << ", " << formationNodeCompositionRequest.messageId << ", " <<
         ValueSpecNames[formationNodeCompositionRequest.valueSpec] <<
         ", " << formationNodeCompositionRequest.formationIndex << ", " <<
         formationNodeCompositionRequest.i << ", " << formationNodeCompositionRequest.j << ", " << formationNodeCompositionRequest.k << ")" << endl;
      logstream.flush ();
#endif

      if (formationNodeCompositionRequest.valueSpec > SET)
      {
         m_requestHandler->getFormation (formationNodeCompositionRequest.formationIndex)->manipulateFormationNodeComposition (formationNodeCompositionRequest);
      }
      else
      {
         FormationNodeCompositionRequest formationNodeCompositionResponse;
#if 0
         cerr << GetRankString () << ": " << m_requestHandler->getFormation (formationNodeCompositionRequest.formationIndex)->getName () << " (" << formationNodeCompositionRequest.formationIndex << ")"
            "->handleFormationNodeCompositionRequest (" << ValueSpecNames[formationNodeCompositionRequest.valueSpec] << ")" << endl;
#endif
         m_requestHandler->getFormation (formationNodeCompositionRequest.formationIndex)->getFormationNodeComposition (formationNodeCompositionRequest, formationNodeCompositionResponse);

         formationNodeCompositionResponse.messageId = ++messageId;

#ifdef USELOGSTREAM
         logstream << GetRankString () << ": " << "sendFormationNodeCompositionResponse (" << source << ", " << formationNodeCompositionResponse.messageId << ", " <<
            ValueSpecNames[formationNodeCompositionResponse.valueSpec] <<
            ", " << formationNodeCompositionResponse.formationIndex << ", " <<
            formationNodeCompositionResponse.i << ", " << formationNodeCompositionResponse.j << ", " << formationNodeCompositionResponse.k << ")" << endl;
         logstream.flush ();
#endif

         MPI_Bsend (&formationNodeCompositionResponse, 1, FormationNodeCompositionType, source, FORMATIONNODECOMPOSITIONRESPONSE, PETSC_COMM_WORLD);
      }
   }

   void RequestHandling::handleFormationNodeCompositionResponse (const int & source, FormationNodeCompositionRequest * formationNodeCompositionResponse)
   {
      MPI_Status recvStatus;

      assert (formationNodeCompositionResponse);
      MPI_Recv (formationNodeCompositionResponse, 1, FormationNodeCompositionType, source, FORMATIONNODECOMPOSITIONRESPONSE, PETSC_COMM_WORLD, &recvStatus);

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "handleFormationNodeCompositionResponse (" << source << ", " << formationNodeCompositionResponse->messageId << ", " <<
         ValueSpecNames[formationNodeValueResponse->valueSpec] <<
         ", " << formationNodeValueResponse->formationIndex << ", " <<
         formationNodeCompositionResponse->i << ", " << formationNodeCompositionResponse->j << ", " << formationNodeCompositionResponse->k << ")" << endl;
      logstream.flush ();
#endif
   }
   //-------------------------------------------------------------------------------------------------------------//

   /// 9. send a FormationNodeThreeVectorRequest request
   void RequestHandling::SendFormationNodeThreeVectorRequest (FormationNodeThreeVectorRequest & formationNodeThreeVectorRequest, FormationNodeThreeVectorRequest & formationNodeThreeVectorResponse)
   {
      int rank = GetRank (formationNodeThreeVectorRequest.i, formationNodeThreeVectorRequest.j);

      assert (RequestHandling::ProxyUseAllowed ());

      formationNodeThreeVectorRequest.messageId = ++messageId;

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "SendFormationNodeThreeVectorRequest (" << rank << ", " << formationNodeThreeVectorRequest.messageId << ", " << ValueSpecNames[formationNodeThreeVectorRequest.valueSpec] << ")" << endl;
      logstream.flush ();
#endif
      MPI_Bsend (&formationNodeThreeVectorRequest, 1, FormationNodeThreeVectorType, rank, FORMATIONNODETHREEVECTORREQUEST, PETSC_COMM_WORLD);
      if (formationNodeThreeVectorRequest.valueSpec > SET)
      {
         HandleRequests (UNTILOUTOFREQUESTS);
         return;
      }
      else
      {
         HandleRequests (UNTILRESPONDED, &formationNodeThreeVectorResponse);
      }
   }

   void RequestHandling::handleFormationNodeThreeVectorRequest (const int & source)
   {
      MPI_Status recvStatus;
      FormationNodeThreeVectorRequest threeVectorRequest;

      MPI_Recv (&threeVectorRequest, 1, FormationNodeThreeVectorType, source, FORMATIONNODETHREEVECTORREQUEST, PETSC_COMM_WORLD, &recvStatus);

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "handleFormationNodeThreeVectorRequest (" << source << ", " << threeVectorRequest.messageId << ", " << ValueSpecNames[threeVectorRequest.valueSpec] << ")" << endl;
      logstream.flush ();
#endif
      FormationNodeThreeVectorRequest formationNodeThreeVectorResponse;

#if 0
      cerr << GetRankString () << ": " << m_requestHandler->getFormation (threeVectorRequest.formationIndex)->getName () << " (" << threeVectorRequest.formationIndex << ")"
         "->handleFormationNodeThreeVectorRequest (" << threeVectorRequest.threeVectorSpec << ")" << endl;
#endif
      m_requestHandler->getFormation (threeVectorRequest.formationIndex)->getThreeVector (threeVectorRequest, formationNodeThreeVectorResponse);

      formationNodeThreeVectorResponse.messageId = ++messageId;

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "sendFormationNodeThreeVectorResponse (" << source << ", " << formationNodeThreeVectorResponse.messageId << ", " << ValueSpecNames[formationNodeThreeVectorResponse.valueSpec] << ")" << endl;
      logstream.flush ();
#endif

      MPI_Bsend (&formationNodeThreeVectorResponse, 1, FormationNodeThreeVectorType, source, FORMATIONNODETHREEVECTORRESPONSE, PETSC_COMM_WORLD);
   }

   void RequestHandling::handleFormationNodeThreeVectorResponse (const int & source, FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse)
   {
      MPI_Status recvStatus;

      assert (formationNodeThreeVectorResponse);
      MPI_Recv (formationNodeThreeVectorResponse, 1, FormationNodeThreeVectorType, source, FORMATIONNODETHREEVECTORRESPONSE, PETSC_COMM_WORLD, &recvStatus);

#ifdef USELOGSTREAM
      logstream << GetRankString () << ": " << "handleFormationNodeThreeVectorResponse (" << source << ", " << formationNodeThreeVectorResponse->messageId << ", " << ValueSpecNames[formationNodeThreeVectorResponse->valueSpec] << ")" << endl;
      logstream.flush ();
#endif
   }

   //-------------------------------------------------------------------------------------------------------------//

   /// 10. Send a FormationNodeThreeVectorValueRequest: from a three vector (e.g. a set of quadrature points) returns a value (e.g. the depth at that particular set of quadrature points)

   void RequestHandling::SendFormationNodeThreeVectorValueRequest (FormationNodeThreeVectorValueRequest & formationNodeThreeVectorValueRequest, FormationNodeThreeVectorValueRequest & formationNodeThreeVectorValueResponse)
   {
      int rank = GetRank (formationNodeThreeVectorValueRequest.i, formationNodeThreeVectorValueRequest.j);

      assert (RequestHandling::ProxyUseAllowed ());

      formationNodeThreeVectorValueRequest.messageId = ++messageId;

      MPI_Bsend (&formationNodeThreeVectorValueRequest, 1, FormationNodeThreeVectorValueType, rank, FORMATIONNODETHREEVECTORVALUEREQUEST, PETSC_COMM_WORLD);

      HandleRequests (UNTILRESPONDED, &formationNodeThreeVectorValueResponse);
   }


   void RequestHandling::handleFormationNodeThreeVectorValueRequest (const int & source)
   {
      MPI_Status recvStatus;
      FormationNodeThreeVectorValueRequest threeVectorValueRequest;

      MPI_Recv (&threeVectorValueRequest, 1, FormationNodeThreeVectorValueType, source, FORMATIONNODETHREEVECTORVALUEREQUEST, PETSC_COMM_WORLD, &recvStatus);

      FormationNodeThreeVectorValueRequest threeVectorValueResponse;

      threeVectorValueResponse.messageId = ++messageId;

      m_requestHandler->getFormation (threeVectorValueRequest.formationIndex)->getThreeVectorValue (threeVectorValueRequest, threeVectorValueResponse);

      //send the response 
      MPI_Bsend (&threeVectorValueResponse, 1, FormationNodeThreeVectorValueType, source, FORMATIONNODETHREEVECTORVALUERESPONSE, PETSC_COMM_WORLD);
   }

   void RequestHandling::handleFormationNodeThreeVectorValueResponse (const int & source, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse)
   {
      MPI_Status recvStatus;

      assert (formationNodeThreeVectorValueResponse);
      MPI_Recv (formationNodeThreeVectorValueResponse, 1, FormationNodeThreeVectorValueType, source, FORMATIONNODETHREEVECTORVALUERESPONSE, PETSC_COMM_WORLD, &recvStatus);
   }

   //-------------------------------------------------------------------------------------------------------------//

   /// 11. send a value request to another processor
   void RequestHandling::SendRequest (ColumnValueArrayRequest & valueArrayRequest, ColumnValueArrayRequest & valueArrayResponse)
   {
      int rank = GetRank (valueArrayRequest.i, valueArrayRequest.j);

      assert (RequestHandling::ProxyUseAllowed ());

      MPI_Bsend (&valueArrayRequest, 1, ColumnValueArrayType, rank, VALUEARRAYREQUEST, PETSC_COMM_WORLD);
      if (valueArrayRequest.valueSpec > SET)
      {
         HandleRequests (UNTILOUTOFREQUESTS);
         return;
      }
      else
      {
         HandleRequests (UNTILRESPONDED, &valueArrayResponse);
      }
   }

   void RequestHandling::handleValueArrayRequest (const int & source)
   {
      ColumnValueArrayRequest valueArrayRequest;
      MPI_Status recvStatus;
      MPI_Recv (&valueArrayRequest, 1, ColumnValueArrayType, source, VALUEARRAYREQUEST, PETSC_COMM_WORLD, &recvStatus);

      if (valueArrayRequest.valueSpec > SET)
      {
         m_requestHandler->getReservoir (valueArrayRequest.reservoirIndex)->setValue (valueArrayRequest);
      }
      else
      {
         ColumnValueArrayRequest valueArrayResponse;
         m_requestHandler->getReservoir (valueArrayRequest.reservoirIndex)->getValue (valueArrayRequest, valueArrayResponse);

         MPI_Bsend (&valueArrayResponse, 1, ColumnValueArrayType, source, VALUEARRAYRESPONSE, PETSC_COMM_WORLD);
      }
   }

   void RequestHandling::handleValueArrayResponse (const int & source, ColumnValueArrayRequest * valueArrayResponse)
   {
      assert (valueArrayResponse);
      MPI_Status recvStatus;
      MPI_Recv (valueArrayResponse, 1, ColumnValueArrayType, source, VALUEARRAYRESPONSE, PETSC_COMM_WORLD, &recvStatus);
   }

   //-------------------------------------------------------------------------------------------------------------//

   /// 12. send Finished
   /// inform other processors that it has completed its request handling phase.
   void RequestHandling::sendFinished (void)
   {
      assert (m_requestHandling);
      int rank = GetRank ();
      for (int i = 0; i < NumProcessors (); ++i)
      {
         MPI_Bsend (&rank, 1, MPI_INT, i, FINISHED, PETSC_COMM_WORLD);
      }
   }

   /// Returns true if a finished message has been received from each processor
   bool RequestHandling::handleFinishedMessage (const int & source)
   {
      int processorFinished;
      MPI_Status recvStatus;
      MPI_Recv (&processorFinished, 1, MPI_INT, source, FINISHED, PETSC_COMM_WORLD, &recvStatus);
      return (++m_finished == NumProcessors ());
   }

   //-------------------------------------------------------------------------------------------------------------//

   bool RequestHandling::ProxyUseAllowed ()
   {
      return GetInstance ()->proxyUseAllowed ();
   }

   int SumAll (int myValue)
   {
      int result;
      MPI_Allreduce (&myValue, &result, 1, MPI_INT, MPI_SUM, PETSC_COMM_WORLD);
      return result;
   }

   long SumAll (long myValue)
   {
      long result;
      MPI_Allreduce (&myValue, &result, 1, MPI_LONG, MPI_SUM, PETSC_COMM_WORLD);
      return result;
   }

   double SumAll (double myValue)
   {
      double result;
      MPI_Allreduce (&myValue, &result, 1, MPI_DOUBLE, MPI_SUM, PETSC_COMM_WORLD);
      return result;
   }

   int MaximumAll (int myValue)
   {
      int result;
      MPI_Allreduce (&myValue, &result, 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD);
      return result;
   }

   void AllGatherFromAll (void * sendbuf, int sendcount, MPI_Datatype sendtype,
      void *recvbuf, int recvcount, MPI_Datatype recvtype)
   {
      MPI_Allgather (sendbuf, sendcount, sendtype,
         recvbuf, recvcount, recvtype, PETSC_COMM_WORLD);
   }

   void RootGatherFromAll (void * sendbuf, int sendcount, MPI_Datatype sendtype,
      void *recvbuf, int recvcount, MPI_Datatype recvtype)
   {
      MPI_Gather (sendbuf, sendcount, sendtype,
         recvbuf, recvcount, recvtype, 0, PETSC_COMM_WORLD);
   }

   double MaximumAll (double myValue)
   {
      double result;
      MPI_Allreduce (&myValue, &result, 1, MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD);
      return result;
   }

   int AndAll (int myValue)
   {
      int result;
      MPI_Allreduce (&myValue, &result, 1, MPI_INT, MPI_LAND, PETSC_COMM_WORLD);
      return result;
   }

   //Start with request handling	

   RequestHandling * RequestHandling::s_instance = 0;

   /// start a request handling phase.
   RequestHandling::RequestHandling () : m_requestHandler (0), m_functionName (""), m_requestHandling (false), m_finished (0), m_proxyUseAllowed (true)
   {

   }

   /// complete a request handling phase.
   /// Will finish after all processors have completed their phase.
   RequestHandling::~RequestHandling ()
   {
      if (s_instance)
      {
         delete s_instance;
         s_instance = 0;
      }
   }

   RequestHandling * RequestHandling::GetInstance ()
   {
      if (!s_instance)
      {
         s_instance = new RequestHandling;
      }
      return s_instance;
   }

   void RequestHandling::HandleRequests (RequestMode mode)
   {
      RequestHandling::GetInstance ()->handleRequests (mode);
   }

   void RequestHandling::HandleRequests (RequestMode mode, ColumnValueRequest * columnValueResponse)
   {
      RequestHandling::GetInstance ()->handleRequests (mode, columnValueResponse, 0, 0, 0, 0, 0, 0);
   }

   void RequestHandling::HandleRequests (RequestMode mode, ColumnCompositionRequest * columnCompositionResponse)
   {
      RequestHandling::GetInstance ()->handleRequests (mode, 0, 0, columnCompositionResponse, 0, 0, 0, 0);
   }

   void RequestHandling::HandleRequests (RequestMode mode, ColumnValueArrayRequest * columnValueArrayResponse)
   {
      RequestHandling::GetInstance ()->handleRequests (mode, 0, columnValueArrayResponse, 0, 0, 0, 0, 0);
   }

   void RequestHandling::HandleRequests (RequestMode mode, FormationNodeValueRequest * formationNodeValueResponse)
   {
      RequestHandling::GetInstance ()->handleRequests (mode, 0, 0, 0, formationNodeValueResponse, 0, 0, 0);
   }

   void RequestHandling::HandleRequests (RequestMode mode, FormationNodeCompositionRequest * formationNodeCompositionResponse)
   {
      RequestHandling::GetInstance ()->handleRequests (mode, 0, 0, 0, 0, formationNodeCompositionResponse, 0, 0);
   }

   void RequestHandling::HandleRequests (RequestMode mode, FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse)
   {
      RequestHandling::GetInstance ()->handleRequests (mode, 0, 0, 0, 0, 0, formationNodeThreeVectorResponse, 0);
   }

   void RequestHandling::HandleRequests (RequestMode mode, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse)
   {
      RequestHandling::GetInstance ()->handleRequests (mode, 0, 0, 0, 0, 0, 0, formationNodeThreeVectorValueResponse);
   }

   void RequestHandling::HandleRequests (RequestMode mode, ColumnValueRequest * columnValueResponse, ColumnValueArrayRequest * valueArrayResponse, ColumnCompositionRequest * columnCompositionResponse,
      FormationNodeValueRequest * formationNodeValueResponse, FormationNodeCompositionRequest * formationNodeCompositionResponse,
      FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse)
   {
      RequestHandling::GetInstance ()->handleRequests (mode, columnValueResponse, valueArrayResponse, columnCompositionResponse, formationNodeValueResponse,
         formationNodeCompositionResponse, formationNodeThreeVectorResponse, formationNodeThreeVectorValueResponse);
   }

   /// initialize a new request handling phase.
   void RequestHandling::startRequestHandling (Migrator * requestHandler, const std::string& functionName)
   {
#if 0
      ReportProgress ("starting RequestHandling: ", functionName);
#endif 

      assert (!m_requestHandler);
      m_requestHandler = requestHandler;

      assert (!m_requestHandling);
      m_finished = 0;
      m_functionName = functionName;

      // Allocate memory to MPI so it can buffer sends. Assume one to each MPI process plus 1 extra.
      // Assume that the maximum size of all data types is not larger than 1k bytes.
      m_mpi_buffer_size = (1 << 18) + MPI_BSEND_OVERHEAD;
      m_mpi_buffer = new char[m_mpi_buffer_size];
      if (MPI_Buffer_attach (m_mpi_buffer, m_mpi_buffer_size) != MPI_SUCCESS)
      {
         cout << "Basin_Warning:  MPI_Buffer_attach failure " << endl;
      }

      /// required as this processor might get and handle a finish request before s_finished was reset, otherwise.
      MPI_Barrier (PETSC_COMM_WORLD);

      m_requestHandling = true;

   }

   /// complete a request handling phase.
   /// Will finish after all processors have completed their phase.
   void RequestHandling::finishRequestHandling ()
   {
      assert (m_requestHandling);
      sendFinished ();

      handleRequests (UNTILALLFINISHED);
#if 0
      ReportProgress ("finished RequestHandling: ", m_functionName);
#endif 

      // release MPI buffer
      MPI_Buffer_detach (m_mpi_buffer, &m_mpi_buffer_size);
      delete[] m_mpi_buffer;
      m_mpi_buffer = 0;

      MPI_Barrier (PETSC_COMM_WORLD);

      m_requestHandling = false;

      assert (m_requestHandler);
      m_requestHandler = 0;


#if 0
      Serialize ();
      cerr << cerrstrstr.str ();
      cerr.flush ();
      cerrstrstr.str ("");
      Deserialize ();
#endif

   }

   /// This function handles all incoming requests until either a response to and outgoing request has arrived or until there are no more
   /// incoming requests or until all processors have finished doing what they were doing
   /// In responding to incoming request, a processor should not send requests to other processors as this function is not
   /// reentrant.
   void RequestHandling::handleRequests (RequestMode mode, ColumnValueRequest * columnValueResponse, ColumnValueArrayRequest * valueArrayResponse, ColumnCompositionRequest * columnCompositionResponse,
      FormationNodeValueRequest * formationNodeValueResponse, FormationNodeCompositionRequest * formationNodeCompositionResponse,
      FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse)
   {
      assert (m_requestHandling);
      assert (proxyUseAllowed ());

      if (mode == UNTILALLFINISHED && m_finished >= NumProcessors ()) return;

      // Disallow sending requests to other processors
      disallowProxyUse ();

      bool checkForMoreMessages = true;
      while (checkForMoreMessages)
      {
         bool blockUntilReceived = (mode == UNTILRESPONDED || mode == UNTILALLFINISHED);

         int opcode = -1;
         int source = -1;
         bool received = checkForIncomingMessages (blockUntilReceived, opcode, source);
         if (!received)
         {
            checkForMoreMessages = false;
            continue;
         }

         // received one of these messages
         switch (opcode)
         {

         case FINISHED:
            // a processor has finished.
            if (handleFinishedMessage (source))
            {
               assert (mode != UNTILRESPONDED); // in that case finished messages cannot have been received from all
               checkForMoreMessages = false;
            }
            break;

            //----------------------------------------------//
            // responses
            //----------------------------------------------//

         case COLUMNVALUERESPONSE:
            // another processor responded to a request from me.
            handleColumnValueResponse (source, columnValueResponse);

            // this is the response I was waiting for
            checkForMoreMessages = false;
            break;

         case FORMATIONNODEVALUERESPONSE:
            // another processor responded to a request from me.
            handleFormationNodeValueResponse (source, formationNodeValueResponse);

            // this is the response I was waiting for
            checkForMoreMessages = false;
            break;

         case FORMATIONNODETHREEVECTORRESPONSE:
            // another processor responded to a request from me.
            handleFormationNodeThreeVectorResponse (source, formationNodeThreeVectorResponse);

            // this is the response I was waiting for
            checkForMoreMessages = false;
            break;

         case FORMATIONNODECOMPOSITIONRESPONSE:
            // another processor responded to a request from me.
            handleFormationNodeCompositionResponse (source, formationNodeCompositionResponse);

            // this is the response I was waiting for
            checkForMoreMessages = false;
            break;

         case COLUMNCOMPOSITIONRESPONSE:
            // another processor responded to a request from me.
            handleColumnCompositionResponse (source, columnCompositionResponse);

            // this is the response I was waiting for
            checkForMoreMessages = false;
            break;

         case FORMATIONNODETHREEVECTORVALUERESPONSE:
            handleFormationNodeThreeVectorValueResponse (source, formationNodeThreeVectorValueResponse);

            // this is the response I was waiting for
            checkForMoreMessages = false;
            break;

         case VALUEARRAYRESPONSE:
            // another processor responded to a request from me.
            handleValueArrayResponse (source, valueArrayResponse);

            // this is the response I was waiting for
            checkForMoreMessages = false;
            break;

            //----------------------------------------------//
            // requests
            //----------------------------------------------//

         case COLUMNVALUEREQUEST:
            // another processor sent me a request, respond if necessary.
            handleColumnValueRequest (source);
            break;

         case FORMATIONNODEVALUEREQUEST:
            // another processor sent me a request, respond if necessary.
            handleFormationNodeValueRequest (source);
            break;

         case FORMATIONNODECOMPOSITIONREQUEST:
            // another processor sent me a request, respond if necessary.
            handleFormationNodeCompositionRequest (source);
            break;

         case FORMATIONNODETHREEVECTORREQUEST:
            // another processor sent me a request, respond if necessary.
            handleFormationNodeThreeVectorRequest (source);
            break;

         case FORMATIONNODETHREEVECTORVALUEREQUEST:
            // another processor sent me a request, respond if necessary.
            handleFormationNodeThreeVectorValueRequest (source);
            break;

         case COLUMNRESETREQUEST:
            // another processor sent me a request, respond if necessary.
            handleColumnResetRequest (source);
            break;

         case COLUMNCOMPOSITIONREQUEST:
            // another processor sent me a request, respond if necessary.
            handleColumnCompositionRequest (source);
            break;

		 case COLUMNCOMPOSITIONPOSITIONREQUEST:
			 // another processor sent me a request, respond if necessary.
			 handleColumnCompositionPositionRequest(source);
			 break;

         case TRAPPROPERTIESREQUEST:
            // another processor sent me a request, respond if necessary.
            handleTrapPropertiesRequest (source);
            break;

         case MIGRATIONREQUEST:
            // another processor sent me a request, respond if necessary.
            handleMigrationRequest (source);
            break;

         case VALUEARRAYREQUEST:
            // another processor sent me a request, respond if necessary.
            handleValueArrayRequest (source);
            break;

         case COLUMNREQUEST:
            // another processor sent me a request, respond if necessary.
            handleColumnRequest (source);
            break;

         }
      }

      // Allow sending requests to other processors
      allowProxyUse ();
      return;
   }

   bool RequestHandling::checkForIncomingMessages (bool blockUntilReceived, int & opcode, int & source)
   {
      MPI_Status probeStatus;

      if (blockUntilReceived)
      {
         MPI_Probe (MPI_ANY_SOURCE, MPI_ANY_TAG, PETSC_COMM_WORLD, &probeStatus);
         opcode = probeStatus.MPI_TAG;
         source = probeStatus.MPI_SOURCE;
         return true;
      }
      else
      {
         int flag;                 // whether a message is pending

         MPI_Iprobe (MPI_ANY_SOURCE, MPI_ANY_TAG, PETSC_COMM_WORLD, &flag, &probeStatus);
         opcode = probeStatus.MPI_TAG;
         source = probeStatus.MPI_SOURCE;
         return (flag != 0);
      }
   }

   /// Handle a finished message
   void RequestHandling::allowProxyUse (void)
   {
      m_proxyUseAllowed = true;
   }

   void RequestHandling::disallowProxyUse (void)
   {
      m_proxyUseAllowed = false;
   }

   bool RequestHandling::proxyUseAllowed (void)
   {
      return m_proxyUseAllowed;
   }
}


