#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>

void error(const char *msg)
{
   perror(msg);
   exit(0);
}

int pfd[2];
int sockfd;

int main(int argc, char *argv[])
{
   // child process id
   pid_t cpid;

   if (argc < 3)
   {
      fprintf (stderr, "usage %s hostname port ....\n", argv[0]);
      exit (0);
   }

   // create a pipe for the child process to fill and for the parent process to empty.
   if (pipe (pfd) == -1)
   {
      perror ("pipe");
      exit (-1);
   }

   cpid = fork();

   if (cpid != 0) // parent
   {
      int portno;
      struct sockaddr_in serv_addr;
      struct hostent *server;

      char buf;

      // close the input of the pipe as we will only read from it in this (parent) process
      close (pfd[1]);

      portno = atoi (argv[2]);

      // connect to the server (mpi_recv) process to pass data to that was read from the pipe
      sockfd = socket (AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0)
         error ("ERROR opening socket");
      server = gethostbyname (argv[1]);
      if (server == NULL)
      {
         fprintf (stderr, "ERROR, no such host\n");
         exit (0);
      }
      bzero ((char *) &serv_addr, sizeof (serv_addr));
      serv_addr.sin_family = AF_INET;
      bcopy ((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
      serv_addr.sin_port = htons (portno);
      if (connect (sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
      {
         error ("ERROR while connecting");
      }

      // pass on the output from the target command (mpirun ...) to the server (mpi_recv).
      int read_status, write_status;
      while ((read_status = read(pfd[0], &buf, 1)) > 0 && (write_status = write(sockfd, &buf, 1)) > 0);
      // kill processes after the read or the write fails.

      // probably only necessary after the write fails as this would mean that mpi_recv was killed
      // Let's also try to kill the lsf job
      if (write_status <= 0)
      {
         char * jobid = getenv ("LSB_JOBID");
         if (jobid != 0)
         {
            char kill_cmd[128];
            snprintf (kill_cmd, sizeof (kill_cmd), "bkill %s", jobid);
            system (kill_cmd);
            sleep (5);
         }

         // kill the child process after the read or the write fails.
         kill (cpid, SIGTERM);
      }

   }
   else // child
   {
      char **args = (char **) malloc (argc * sizeof(char *));

      // close the output of the pipe as we do not use it in the child
      close(pfd[0]);

      // connect stdout/stderr to the input of the pipe
      dup2(pfd[1], 1);
      dup2(pfd[1], 2);

      // execute the target command (mprun ...)
      int i;
      for (i = 3; i < argc; ++i)
      {
         args[i-3] = argv[i];
      }
      args[i-3] = NULL;

      execv (args[0], args);

      // if we get here, execv failed
      fprintf (stderr, "Exec of %s failed]n", args[0]);
      perror ("execv");
   }

   return 0;
}
