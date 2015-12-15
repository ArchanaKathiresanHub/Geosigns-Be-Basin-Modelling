
#include "mpi.h"
#include <iostream>
#include <cmath>
#include <thread>
#include <string>
#include <fstream>

using std::thread;
using std::endl;
using std::cout;
using std::cin;
using std::string;
using std::ifstream;

#include "fibonacci.h"
#include "write_fun.h"
#include "parameters.h"

// the main function

int main()
{
     // init mpi

     MPI::Init();
     int my_rank;
     my_rank = MPI::COMM_WORLD.Get_rank();

     // local parameters

     const string BASE_NAME = "test_file_";
     const long int DIMEN = 1*static_cast<long int>(pow(10.0, 6.0));
     const char DATA_REPRESENTATION[128] = "native"; // "internal", "external32"
     const int I_DO_MAX = 11;
     const int WR_T = 2;
     const int STEP = 3;
     const unsigned long int CPU_LOAD = 10UL;
     const int IO_MODE = MPI::MODE_CREATE |
                         MPI::MODE_RDWR |
                         MPI::MODE_DELETE_ON_CLOSE;

     // local variables

     MPI::File * arr_files = new MPI::File [I_DO_MAX];
     thread * thWR = new thread [I_DO_MAX];
     thread * thCALC = new thread [I_DO_MAX];
     ifstream in_file;
     int counter = -1;
     int status_last = -1;
     string tmp_str;

     MPI::COMM_WORLD.Barrier();

     // give job here

     for(int i = 0; i < I_DO_MAX; i++) {

          // local to the loop variables

          MPI::Offset disp;
          MPI::Status status;

          // counter

          if (my_rank == 0) {
               cout << "----------------------------------------------------------------->> "
                    << i << endl;
          }

          // open the file

          string tmp_name = BASE_NAME + std::to_string(static_cast<long long>(i));

          arr_files[i] = MPI::File::Open(MPI::COMM_WORLD,
                                         tmp_name.c_str(),
                                         IO_MODE,
                                         MPI::INFO_NULL);

          MPI::COMM_WORLD.Barrier();

          // set the MPI file view

          disp = my_rank * DIMEN * 8; // displacement in bytes

          arr_files[i].Set_view(disp,
                                MPI::DOUBLE,
                                MPI::DOUBLE,
                                DATA_REPRESENTATION,
                                MPI::INFO_NULL);

          MPI::COMM_WORLD.Barrier();

          // write to the file in parallel

          thWR[i] = thread(write_fun,
                           arr_files[i],
                           DIMEN,
                           WR_T,
                           my_rank);

          MPI::COMM_WORLD.Barrier();

          // do calc

          thCALC[i] = thread(fibonacci, CPU_LOAD);

          MPI::COMM_WORLD.Barrier();

          thCALC[i].join();

          MPI::COMM_WORLD.Barrier();

          // do synch

          if (i%STEP == 0) {
               thWR[i].detach();
               while(true) {
                    sleep(2); // sleep for few seconds every check-time
                    tmp_str = F_SENTINEL + std::to_string(static_cast<long long>(my_rank));
                    in_file.open(tmp_str.c_str());
                    in_file >> counter;
                    in_file.close();
                    if (counter >= i) break;
               }
          } else {
               thWR[i].detach();
          }
     }

     MPI::COMM_WORLD.Barrier();

     // wait all the mpi procs to finish writing

     while(true) {
          sleep(2); // sleep for few seconds every check-time
          in_file.open(tmp_str.c_str());
          in_file >> counter;
          in_file.close();
          if (counter == I_DO_MAX) break;
     }

     MPI::COMM_WORLD.Barrier();

     // close the unit files

     for(int i = 0; i < I_DO_MAX; i++) {
          arr_files[i].Close();
     }

     // remove sentinel file

     status_last = system((RM+tmp_str).c_str());

     // free up the RAM and finalize MPI

     delete [] arr_files;

     MPI::Finalize();

     return status_last;
}

// end
