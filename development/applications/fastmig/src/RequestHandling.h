//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_REQUESTHANDLING_H_
#define _MIGRATION_REQUESTHANDLING_H_

#include "mpi.h"

#include "RequestDefs.h"

#include <string>
using std::string;

namespace migration
{
   class Reservoir;
   class Migrator;

   struct ColumnValueRequest;
   struct ColumnCompositionRequest;
   struct FormationNodeCompositionRequest;

   enum RequestMode { UNTILALLFINISHED, UNTILRESPONDED, UNTILOUTOFREQUESTS };

   enum RequestId
   {
      FINISHED, VALUEARRAYRESPONSE, VALUEARRAYREQUEST, COLUMNREQUEST,
      COLUMNVALUERESPONSE, COLUMNVALUEREQUEST, COLUMNRESETREQUEST, COLUMNCOMPOSITIONRESPONSE,
      COLUMNCOMPOSITIONREQUEST, TRAPPROPERTIESREQUEST, MIGRATIONREQUEST, CHARGESREQUEST,
      FORMATIONNODEVALUERESPONSE, FORMATIONNODEVALUEREQUEST, FORMATIONNODECOMPOSITIONRESPONSE,
      FORMATIONNODECOMPOSITIONREQUEST, FORMATIONNODETHREEVECTORRESPONSE,
      FORMATIONNODETHREEVECTORREQUEST, FORMATIONNODETHREEVECTORVALUEREQUEST, FORMATIONNODETHREEVECTORVALUERESPONSE
   };


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
      static void StartRequestHandling (Migrator * requestHandler, const string& functionName);

      static void FinishRequestHandling ();

      void startRequestHandling (Migrator * requestHandler, const string& functionName);

      void finishRequestHandling ();

      static RequestHandling * GetInstance ();

      static bool ProxyUseAllowed (void);

      inline static bool AllProcessorsFinished (bool finished);

      // these functions call HandleRequests

      static void SendRequest (ColumnValueRequest & valueRequest, ColumnValueRequest & valueResponse);

      static void SendRequest (ColumnValueArrayRequest & valueArrayRequest, ColumnValueArrayRequest & valueArrayResponse);

      static void SendRequest (ColumnColumnRequest & columnRequest);

      static void SendRequest (TrapPropertiesRequest & tpRequest);

      static void SendMigrationRequest (MigrationRequest & request);

      static void SendFormationNodeThreeVectorValueRequest (FormationNodeThreeVectorValueRequest & formationNodeThreeVectorValueRequest, FormationNodeThreeVectorValueRequest & formationNodeThreeVectorValueResponse);

      static void SendRequest (ColumnCompositionRequest & chargesRequest, ColumnCompositionRequest & chargesResponse);

      static void SendProxyReset (int rank, ColumnValueRequest & resetRequest);

      static void SendFormationNodeValueRequest (FormationNodeValueRequest & valueRequest, FormationNodeValueRequest & valueResponse);

      static void SendFormationNodeCompositionRequest (FormationNodeCompositionRequest & chargesRequest, FormationNodeCompositionRequest & valueResponse);

      static void SendFormationNodeThreeVectorRequest (FormationNodeThreeVectorRequest & vectorRequest, FormationNodeThreeVectorRequest & vectorResponse);

      // HandleRequests
      static void HandleRequests (RequestMode mode);

      static void HandleRequests (RequestMode mode, ColumnValueRequest * valueResponse);

      static void HandleRequests (RequestMode mode, ColumnValueArrayRequest * valueArrayResponse);

      static void HandleRequests (RequestMode mode, ColumnCompositionRequest * chargesResponse);

      static void HandleRequests (RequestMode mode, FormationNodeCompositionRequest * chargesResponse);

      static void HandleRequests (RequestMode mode, FormationNodeValueRequest * formationNodeValueResponse);

      static void HandleRequests (RequestMode mode, FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse);

      static void HandleRequests (RequestMode mode, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse);

      static void HandleRequests (RequestMode mode, ColumnValueRequest * columnValueResponse, ColumnValueArrayRequest * valueArrayResponse, ColumnCompositionRequest * columnCompositionResponse,
         FormationNodeValueRequest * formationNodeValueResponse, FormationNodeCompositionRequest * formationNodeCompositionResponse,
         FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse);

      inline void handleRequests (RequestMode mode);

      inline void handleRequests (RequestMode mode, ColumnValueRequest * valueResponse);

      inline void handleRequests (RequestMode mode, ColumnValueArrayRequest * valueArrayResponse);

      inline void handleRequests (RequestMode mode, ColumnCompositionRequest * chargesResponse);

      inline void handleRequests (RequestMode mode, FormationNodeCompositionRequest * chargesResponse);

      inline void handleRequests (RequestMode mode, FormationNodeValueRequest * formationNodeValueResponse);

      inline void handleRequests (RequestMode mode, FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse);

      inline void handleRequests (RequestMode mode, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse);

      void handleRequests (RequestMode mode, ColumnValueRequest * columnValueResponse, ColumnValueArrayRequest * valueArrayResponse, ColumnCompositionRequest * columnCompositionResponse,
         FormationNodeValueRequest * formationNodeValueResponse, FormationNodeCompositionRequest * formationNodeCompositionResponse,
         FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse);

   protected:
      RequestHandling ();
      virtual ~RequestHandling ();

   private:
      static RequestHandling * s_instance;

      Migrator * m_requestHandler;

      char * m_mpi_buffer;
      int m_mpi_buffer_size;

      string m_functionName;
      bool m_requestHandling;
      int m_finished;
      bool m_proxyUseAllowed;

      void sendFinished ();

      bool checkForIncomingMessages (bool blockUntilReceived, int & opcode, int & source);

      void handleColumnValueResponse (const int & source, ColumnValueRequest * columnValueResponse);
      void handleColumnCompositionResponse (const int & source, ColumnCompositionRequest * columnCompositionResponse);

      bool handleFinishedMessage (const int & source);
      void handleValueArrayResponse (const int & source, ColumnValueArrayRequest * valueArrayResponse);

      void handleColumnValueRequest (const int & source);
      void handleColumnResetRequest (const int & source);
      void handleTrapPropertiesRequest (const int & source);

      void handleValueArrayRequest (const int & source);
      void handleColumnRequest (const int & source);
      void handleColumnCompositionRequest (const int & source);
      void handleMigrationRequest (const int & source);

      void handleFormationNodeValueResponse (const int & source, FormationNodeValueRequest * formationNodeValueResponse);
      void handleFormationNodeValueRequest (const int & source);

      void handleFormationNodeCompositionResponse (const int & source, FormationNodeCompositionRequest * formationNodeCompositionResponse);
      void handleFormationNodeCompositionRequest (const int & source);

      void handleFormationNodeThreeVectorResponse (const int & source, FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse);
      void handleFormationNodeThreeVectorRequest (const int & source);

      void handleFormationNodeThreeVectorValueRequest (const int & source);
      void handleFormationNodeThreeVectorValueResponse (const int & source, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse);

      void allowProxyUse (void);
      void disallowProxyUse (void);
   public:
      bool proxyUseAllowed (void);
   };

   class RequestHandle
   {
   public:
      RequestHandle (Migrator * requestHandler, const string& functionName)
      {
         RequestHandling::StartRequestHandling (requestHandler, functionName);
      }

      ~RequestHandle ()
      {
         RequestHandling::FinishRequestHandling ();
      }
   };

   bool RequestHandling::AllProcessorsFinished (bool finished)
   {
      return  (AndAll ((int)finished) != 0);
   }

   void RequestHandling::handleRequests (RequestMode mode)
   {
      return  handleRequests (mode, 0, 0, 0, 0, 0, 0, 0);
   }

   void RequestHandling::handleRequests (RequestMode mode, ColumnValueRequest * columnValueResponse)
   {
      return  handleRequests (mode, columnValueResponse, 0, 0, 0, 0, 0, 0);
   }

   void RequestHandling::handleRequests (RequestMode mode, ColumnCompositionRequest * chargesResponse)
   {
      return handleRequests (mode, 0, 0, chargesResponse, 0, 0, 0, 0);
   }

   void RequestHandling::handleRequests (RequestMode mode, FormationNodeValueRequest * formationNodeValueResponse)
   {
      return  handleRequests (mode, 0, 0, 0, formationNodeValueResponse, 0, 0, 0);
   }

   void RequestHandling::handleRequests (RequestMode mode, FormationNodeCompositionRequest * chargesResponse)
   {
      return handleRequests (mode, 0, 0, 0, 0, chargesResponse, 0, 0);
   }

   void RequestHandling::handleRequests (RequestMode mode, FormationNodeThreeVectorRequest * formationNodeThreeVectorResponse)
   {
      return  handleRequests (mode, 0, 0, 0, 0, 0, formationNodeThreeVectorResponse, 0);
   }

   void RequestHandling::handleRequests (RequestMode mode, FormationNodeThreeVectorValueRequest * formationNodeThreeVectorValueResponse)
   {
      return  handleRequests (mode, 0, 0, 0, 0, 0, 0, formationNodeThreeVectorValueResponse);
   }

} // namespace migration


#endif

