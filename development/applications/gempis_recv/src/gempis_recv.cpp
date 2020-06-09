/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <iostream>
#include <iomanip>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

long SecondsAtStart = 0;
long SecondsAtLapTime = 0;

void startTime (void)
{
   timespec tp;
   clock_gettime(CLOCK_REALTIME, &tp);
   SecondsAtStart = SecondsAtLapTime = tp.tv_sec;
}

long getElapsedTime (void)
{
   timespec tp;
   clock_gettime(CLOCK_REALTIME, &tp);
   return tp.tv_sec - SecondsAtStart;
}


void* SocketHandler(void*);

int main(int argc, char *argv[])
{
   pid_t cpid;

   int sockfd, newsockfd, portno;
   socklen_t clilen;
   char buf;
   struct sockaddr_in serv_addr, cli_addr;
   int n;
   pthread_t thread_id=0;
   FILE * portfile;

   if (argc < 2)
   {
      fprintf (stderr, "ERROR, no port provided\n");
      exit (1);
   }
   sockfd = socket (AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0)
      error ("ERROR opening socket");
   bzero ((char *) &serv_addr, sizeof (serv_addr));
   portno = atoi (argv[1]);

   // find an unused port to bind to starting from the command line supplied port number
   while (1)
   {
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = INADDR_ANY;
      serv_addr.sin_port = htons (portno);
      if (bind (sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) == 0)
	 break;
      portno++;
   }


   // output the port number
   if (argc == 3)
      portfile = fopen(argv[2], "w");
   else
      portfile = stdout;

   fprintf (portfile, "%d\n", portno);
   fflush (portfile);
   if (argc == 3)
      fclose (portfile);

   // fork to make the parent process exit.
   // this way the used port number can be picked up after gempis_recv exits.
   // work will continue in the child process.
   cpid = fork();
   if (cpid != 0) exit(0);

   listen(sockfd,5);

   // we want to handle only one connection.
#ifdef MULTIPLECONNECTIONS
   while (1)
   {
#endif
      int * csock;
      csock = (int *) malloc (sizeof (int));
      clilen = sizeof (struct sockaddr);
      if ((*csock = accept (sockfd, (struct sockaddr *) & cli_addr, &clilen)) != -1)
      {
	 /*
         printf ("---------------------\nReceived connection from %s\n", inet_ntoa (cli_addr.sin_addr));
	 */
         pthread_create (&thread_id, 0, &SocketHandler, (void *) csock);
#ifdef MULTIPLECONNECTIONS
         pthread_detach (thread_id);
#else
         pthread_join (thread_id, NULL);
#endif
      }
      else
      {
	 fprintf (stderr, "gempis_recv: accept attempt failed\n");
         perror ("accept");
      }

#ifdef MULTIPLECONNECTIONS
   }
#endif

   return 0;
}

void* SocketHandler(void* lp)
{
   int *csock = (int *) lp;

   // fprintf (stderr, "Connection %d opened\n", *csock);

   int first = 1;

   startTime();

   char buf;
   // write to stdout what is received on the socket, prepending each line with time elapsed
   while (read (*csock, &buf, 1) > 0)
   {
      if (first == 1)
      {
	 long elapsed = getElapsedTime();
	 cout << setw (4) << setfill (' ') << elapsed / 60 << ":" << setw (2) << setfill ('0') << elapsed % 60;
	 cout.flush ();
	 write (1, ":: ", 3);
	 first = 0;
      }

      write (1, &buf, 1);
      if (buf == '\n') first = 1;
   }

   close (*csock);
   free (csock);

   return 0;
}
