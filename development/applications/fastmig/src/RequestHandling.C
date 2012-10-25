#include "mpi.h"

#include <assert.h>

#include <iostream>
#include <vector>


using namespace std;

#include<sstream>
using std::ostringstream;
extern ostringstream cerrstrstr;

#include "rankings.h"
#include "RequestDefs.h"
#include "RequestHandling.h"
#include "Reservoir.h"


/// Initialize a new request handling phase.
namespace migration
{

void RequestHandling::StartRequestHandling (Reservoir * requestHandler, const string& functionName)
{
   RequestHandling::GetInstance ().startRequestHandling (requestHandler, functionName);
}

/// Complete a request handling phase.
/// Will finish after all processors have completed their phase.
void RequestHandling::FinishRequestHandling ()
{
   RequestHandling::GetInstance ().finishRequestHandling ();
}

/// send a value request to another processor
void RequestHandling::SendRequest (ColumnValueRequest & valueRequest, ColumnValueRequest & valueResponse)
{
   int rank = GetRank (valueRequest.i, valueRequest.j);

   assert (RequestHandling::ProxyUseAllowed ());

   MPI_Bsend (&valueRequest, 1, ColumnValueType, rank, VALUEREQUEST, PETSC_COMM_WORLD);
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

/// send a value request to another processor
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

/// send a column information update request to another processor
void RequestHandling::SendRequest (ColumnColumnRequest & columnRequest)
{
   assert (RequestHandling::ProxyUseAllowed ());

   MPI_Bsend (&columnRequest, 1, ColumnColumnType, GetRank (columnRequest.i, columnRequest.j), COLUMNREQUEST, PETSC_COMM_WORLD);
   assert (columnRequest.valueSpec > SET);
   
   HandleRequests (UNTILOUTOFREQUESTS);
   return;
}

/// send properties of a trap to processor 0 to be saved in the TrapIoTbl
void RequestHandling::SendRequest (TrapPropertiesRequest & tpRequest)
{
   assert (RequestHandling::ProxyUseAllowed ());

   MPI_Bsend (&tpRequest, 1, TrapPropertiesType, 0, TRAPREQUEST, PETSC_COMM_WORLD);
   assert (tpRequest.valueSpec > SET);
   
   HandleRequests (UNTILOUTOFREQUESTS);
   return;
}

/// send a migration to processor 0 to be ultimately saved in the MigrationIoTbl
void RequestHandling::SendMigrationRequest (MigrationRequest & request)
{
   assert (RequestHandling::ProxyUseAllowed ());

   MPI_Bsend (&request, 1, MigrationType, 0, MIGRATIONREQUEST, PETSC_COMM_WORLD);
   
   HandleRequests (UNTILOUTOFREQUESTS);
   return;
}

/// send a charge request to another processor
void RequestHandling::SendRequest (ColumnCompositionRequest & chargesRequest, ColumnCompositionRequest & chargesResponse)
{
   assert (RequestHandling::ProxyUseAllowed ());

   MPI_Bsend (&chargesRequest, 1, ColumnCompositionType, GetRank (chargesRequest.i, chargesRequest.j), CHARGESREQUEST, PETSC_COMM_WORLD);
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

/// send a request to reset a proxy column
void RequestHandling::SendProxyReset (int rank, ColumnValueRequest & resetRequest)
{
   assert (RequestHandling::ProxyUseAllowed ());

   MPI_Bsend (&resetRequest, 1, ColumnValueType, rank, RESETREQUEST, PETSC_COMM_WORLD);

   HandleRequests (UNTILOUTOFREQUESTS);
}

bool RequestHandling::ProxyUseAllowed ()
{
   return GetInstance ().proxyUseAllowed ();
}

void RequestHandling::HandleRequests (RequestMode mode)
{
   RequestHandling::GetInstance ().handleRequests (mode);
}

void RequestHandling::HandleRequests (RequestMode mode, ColumnValueRequest * valueResponse)
{
   RequestHandling::GetInstance ().handleRequests (mode, valueResponse);
}

void RequestHandling::HandleRequests (RequestMode mode, ColumnValueArrayRequest * valueArrayResponse)
{
   RequestHandling::GetInstance ().handleRequests (mode, valueArrayResponse);
}

void RequestHandling::HandleRequests (RequestMode mode, ColumnCompositionRequest * chargesResponse)
{
   RequestHandling::GetInstance ().handleRequests (mode, chargesResponse);
}

void RequestHandling::HandleRequests (RequestMode mode, ColumnValueRequest * valueResponse, ColumnValueArrayRequest * valueArrayResponse, ColumnCompositionRequest * chargesResponse)
{
   RequestHandling::GetInstance ().handleRequests (mode, valueResponse, valueArrayResponse, chargesResponse);
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


RequestHandling * RequestHandling::s_instance = 0;

/// start a request handling phase.
RequestHandling::RequestHandling() : m_requestHandler (), m_functionName (""), m_requestHandling (false), m_finished (0), m_proxyUseAllowed (true)
{
}

/// complete a request handling phase.
/// Will finish after all processors have completed their phase.
RequestHandling::~RequestHandling()
{
}

RequestHandling & RequestHandling::GetInstance ()
{
   if (!s_instance) s_instance = new RequestHandling;
   return * s_instance;
}


/// initialize a new request handling phase.
void RequestHandling::startRequestHandling (Reservoir * requestHandler, const string& functionName)
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
   m_mpi_buffer = new char[ m_mpi_buffer_size ];
   if ( MPI_Buffer_attach ( m_mpi_buffer, m_mpi_buffer_size ) != MPI_SUCCESS )
   {
      cout << " MPI_Buffer_attach failure " << endl;
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
   MPI_Buffer_detach ( m_mpi_buffer, &m_mpi_buffer_size );
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


/// This function handles all incoming requests until either a response to and outgoing request has arrived or until there are no more
/// incoming requests or until all processors have finished doing what they were doing
/// In responding to incoming request, a processor should not send requests to other processors as this function is not
/// reentrant.
void RequestHandling::handleRequests (RequestMode mode, ColumnValueRequest * valueResponse,ColumnValueArrayRequest * valueArrayResponse, ColumnCompositionRequest * chargesResponse)
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
	 case VALUERESPONSE:
	    // another processor responded to a request from me.
	    handleValueResponse (source, valueResponse);

	    // this is the response I was waiting for
	    checkForMoreMessages = false;
	    break;

	 case VALUEARRAYRESPONSE:
	    // another processor responded to a request from me.
	    handleValueArrayResponse (source, valueArrayResponse);

	    // this is the response I was waiting for
	    checkForMoreMessages = false;
	    break;

	 case CHARGESRESPONSE:
	    // another processor responded to a request from me.
	    handleChargesResponse (source, chargesResponse);

	    // this is the response I was waiting for
	    checkForMoreMessages = false;
	    break;

	 case VALUEREQUEST:
	    // another processor sent me a request, respond if necessary.
	    handleValueRequest (source);
	    break;

	 case VALUEARRAYREQUEST:
	    // another processor sent me a request, respond if necessary.
	    handleValueArrayRequest (source);
	    break;

	 case RESETREQUEST:
	    // another processor sent me a request, respond if necessary.
	    handleResetRequest (source);
	    break;

	 case COLUMNREQUEST:
	    // another processor sent me a request, respond if necessary.
	    handleColumnRequest (source);
	    break;

	 case CHARGESREQUEST:
	    // another processor sent me a request, respond if necessary.
	    handleColumnCompositionRequest (source);
	    break;

	 case TRAPREQUEST:
	    handleTrapRequest (source);
	    break;

	 case MIGRATIONREQUEST:
	    handleMigrationRequest (source);
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

/// Returns true if a finished message has been received from each processor
bool RequestHandling::handleFinishedMessage (const int & source)
{
   int processorFinished;
   MPI_Status recvStatus;
   MPI_Recv (&processorFinished, 1, MPI_INT, source, FINISHED, PETSC_COMM_WORLD, &recvStatus);
   return (++m_finished == NumProcessors ());
}

void RequestHandling::handleValueResponse (const int & source, ColumnValueRequest * valueResponse)
{
   assert (valueResponse);
   MPI_Status recvStatus;
   MPI_Recv (valueResponse, 1, ColumnValueType, source, VALUERESPONSE, PETSC_COMM_WORLD, &recvStatus);
}

void RequestHandling::handleValueArrayResponse (const int & source, ColumnValueArrayRequest * valueArrayResponse)
{
   assert (valueArrayResponse);
   MPI_Status recvStatus;
   MPI_Recv (valueArrayResponse, 1, ColumnValueArrayType, source, VALUEARRAYRESPONSE, PETSC_COMM_WORLD, &recvStatus);
}

void RequestHandling::handleChargesResponse (const int & source, ColumnCompositionRequest * chargesResponse)
{
   assert (chargesResponse);
   MPI_Status recvStatus;
   MPI_Recv (chargesResponse, 1, ColumnCompositionType, source, CHARGESRESPONSE, PETSC_COMM_WORLD, &recvStatus);
}

void RequestHandling::handleValueRequest (const int & source)
{
   ColumnValueRequest valueRequest;
   MPI_Status recvStatus;
   MPI_Recv (&valueRequest, 1, ColumnValueType, source, VALUEREQUEST, PETSC_COMM_WORLD, &recvStatus);

   if (valueRequest.valueSpec > SET)
   {
      m_requestHandler->setValue (valueRequest);
   }
   else
   {
      ColumnValueRequest valueResponse;
      m_requestHandler->getValue (valueRequest, valueResponse);

      MPI_Bsend (&valueResponse, 1, ColumnValueType, source, VALUERESPONSE, PETSC_COMM_WORLD);
   }
}

void RequestHandling::handleValueArrayRequest (const int & source)
{
   ColumnValueArrayRequest valueArrayRequest;
   MPI_Status recvStatus;
   MPI_Recv (&valueArrayRequest, 1, ColumnValueArrayType, source, VALUEARRAYREQUEST, PETSC_COMM_WORLD, &recvStatus);

   if (valueArrayRequest.valueSpec > SET)
   {
      m_requestHandler->setValue (valueArrayRequest);
   }
   else
   {
      ColumnValueArrayRequest valueArrayResponse;
      m_requestHandler->getValue (valueArrayRequest, valueArrayResponse);

      MPI_Bsend (&valueArrayResponse, 1, ColumnValueArrayType, source, VALUEARRAYRESPONSE, PETSC_COMM_WORLD);
   }
}

void RequestHandling::handleResetRequest (const int & source)
{
   ColumnValueRequest valueRequest;
   MPI_Status recvStatus;
   MPI_Recv (&valueRequest, 1, ColumnValueType, source, RESETREQUEST, PETSC_COMM_WORLD, &recvStatus);

   m_requestHandler->clearProxyProperties (valueRequest);
}

void RequestHandling::handleColumnRequest (const int & source)
{
   ColumnColumnRequest columnRequest;
   MPI_Status recvStatus;
   MPI_Recv (&columnRequest, 1, ColumnColumnType, source, COLUMNREQUEST, PETSC_COMM_WORLD, &recvStatus);

   m_requestHandler->manipulateColumn (columnRequest);
}

void RequestHandling::handleColumnCompositionRequest (const int & source)
{
   ColumnCompositionRequest columnCompositionRequest;

   MPI_Status recvStatus;
   MPI_Recv (&columnCompositionRequest, 1, ColumnCompositionType, source, CHARGESREQUEST, PETSC_COMM_WORLD, &recvStatus);

   assert (columnCompositionRequest.valueSpec > SET);

   m_requestHandler->manipulateColumnComposition (columnCompositionRequest);
}

void RequestHandling::handleTrapRequest (const int & source)
{
   TrapPropertiesRequest trapRequest;

   MPI_Status recvStatus;
   MPI_Recv (&trapRequest, 1, TrapPropertiesType, source, TRAPREQUEST, PETSC_COMM_WORLD, &recvStatus);

   assert (trapRequest.valueSpec > SET);

   m_requestHandler->processTrapProperties (trapRequest);
}

void RequestHandling::handleMigrationRequest (const int & source)
{
   MigrationRequest migrationRequest;

   MPI_Status recvStatus;
   MPI_Recv (&migrationRequest, 1, MigrationType, source, MIGRATIONREQUEST, PETSC_COMM_WORLD, &recvStatus);

   m_requestHandler->processMigration (migrationRequest);
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
