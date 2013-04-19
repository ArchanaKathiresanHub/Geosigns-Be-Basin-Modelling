#ifndef _MIGRATION_REQUESTHANDLING_H_
#define _MIGRATION_REQUESTHANDLING_H_

#include "mpi.h"

#include "RequestDefs.h"

#include <string>
using std::string;

namespace migration {
   class Reservoir;

   struct ColumnValueRequest;
   struct ColumnCompositionRequest;

   enum RequestMode { UNTILALLFINISHED, UNTILRESPONDED, UNTILOUTOFREQUESTS };

   enum RequestId { FINISHED, VALUERESPONSE, VALUEREQUEST, VALUEARRAYRESPONSE, VALUEARRAYREQUEST, COLUMNREQUEST, RESETREQUEST, CHARGESRESPONSE, CHARGESREQUEST, TRAPREQUEST, MIGRATIONREQUEST };

   extern void AllGatherFromAll (void * sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype);
   extern void RootGatherFromAll (void * sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype);

   extern int SumAll (int myValue);
   extern long SumAll (long myValue);
   extern double SumAll (double myValue);
   extern int MaximumAll (int myValue);
   extern double MaximumAll (double myValue);
   extern int AndAll (int myValue);

   class RequestHandling
   {
      public:
	 static void StartRequestHandling (Reservoir * requestHandler, const string& functionName);
	 static void FinishRequestHandling ();

	 static void SendRequest (ColumnValueRequest & valueRequest, ColumnValueRequest & valueResponse);
	 static void SendRequest (ColumnValueArrayRequest & valueArrayRequest, ColumnValueArrayRequest & valueArrayResponse);
	 static void SendRequest (ColumnColumnRequest & columnRequest);
	 static void SendRequest (TrapPropertiesRequest & tpRequest);
	 static void SendMigrationRequest (MigrationRequest & request);
	 static void SendRequest (ColumnCompositionRequest & chargesRequest, ColumnCompositionRequest & chargesResponse);
	 static void SendProxyReset (int rank, ColumnValueRequest & resetRequest);

	 static void HandleRequests (RequestMode mode);
	 static void HandleRequests (RequestMode mode, ColumnValueRequest * valueResponse);
	 static void HandleRequests (RequestMode mode, ColumnValueArrayRequest * valueArrayResponse);
	 static void HandleRequests (RequestMode mode, ColumnCompositionRequest * chargesResponse);
	 static void HandleRequests (RequestMode mode, ColumnValueRequest * valueResponse, ColumnValueArrayRequest * valueArrayResponse, ColumnCompositionRequest * chargesResponse);

	 static bool ProxyUseAllowed (void);

	 static RequestHandling & GetInstance ();

	 void startRequestHandling (Reservoir * requestHandler, const string& functionName);
	 void finishRequestHandling ();

	 inline void handleRequests (RequestMode mode);
	 inline void handleRequests (RequestMode mode, ColumnValueRequest * valueResponse);
	 inline void handleRequests (RequestMode mode, ColumnValueArrayRequest * valueArrayResponse);
	 inline void handleRequests (RequestMode mode, ColumnCompositionRequest * chargesResponse);

	 void handleRequests (RequestMode mode, ColumnValueRequest * valueResponse, ColumnValueArrayRequest * valueArrayResponse, ColumnCompositionRequest * chargesResponse);

      protected:
	 RequestHandling();
	 virtual ~RequestHandling();

      private:
	 static RequestHandling * s_instance;

	 char * m_mpi_buffer;
	 int m_mpi_buffer_size;
	 
	 Reservoir * m_requestHandler;
	 string m_functionName;
	 bool m_requestHandling;
	 int m_finished;
	 bool m_proxyUseAllowed;

	 void sendFinished ();

	 bool checkForIncomingMessages (bool blockUntilReceived, int & opcode, int & source);

	 bool handleFinishedMessage (const int & source);
	 void handleValueResponse (const int & source, ColumnValueRequest * valueResponse);
	 void handleValueArrayResponse (const int & source, ColumnValueArrayRequest * valueArrayResponse);
	 void handleChargesResponse (const int & source, ColumnCompositionRequest * chargesResponse);

	 void handleValueRequest (const int & source);
	 void handleValueArrayRequest (const int & source);
	 void handleResetRequest (const int & source);
	 void handleColumnRequest (const int & source);
	 void handleColumnCompositionRequest (const int & source);
	 void handleTrapRequest (const int & source);
	 void handleMigrationRequest (const int & source);


	 void allowProxyUse (void);
	 void disallowProxyUse (void);
      public:
	 bool proxyUseAllowed (void);
   };

   class RequestHandle
   {
      public:
	 RequestHandle(Reservoir * requestHandler, const string& functionName)
	 {
	    RequestHandling::StartRequestHandling (requestHandler, functionName);
	 }

	 ~RequestHandle()
	 {
	    RequestHandling::FinishRequestHandling ();
	 }
   };

   void RequestHandling::handleRequests (RequestMode mode)
   {
      return  handleRequests (mode, 0, 0, 0);
   }

   void RequestHandling::handleRequests (RequestMode mode, ColumnValueRequest * valueResponse)
   {
      return  handleRequests (mode, valueResponse, 0, 0);
   }

   void RequestHandling::handleRequests (RequestMode mode, ColumnValueArrayRequest * valueArrayResponse)
   {
      return handleRequests (mode, 0, valueArrayResponse, 0);
   }

   void RequestHandling::handleRequests (RequestMode mode, ColumnCompositionRequest * chargesResponse)
   {
      return handleRequests (mode, 0, 0, chargesResponse);
   }
} // namespace migration


#endif
