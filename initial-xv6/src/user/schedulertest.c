#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define NFORK 10
#define IO 5

int main()
{
  int n, pid;
  int wtime, rtime;
  int twtime = 0, trtime = 0;
  for (n = 0; n < NFORK; n++)
  {
    pid = fork();
    if (pid < 0)
      break;
    if (pid == 0)
    {
      if (n < IO)
      {
        sleep(200); // IO bound processes
      }
      else
      {
        for (volatile int i = 0; i < 1000000000; i++)
        {
        } // CPU bound process
      }
      printf("Process %d finished\n", n);
      exit(0);
    }
  }
  for (; n > 0; n--)
  {
    if (waitx(0, &wtime, &rtime) >= 0)
    {
      trtime += rtime;
      twtime += wtime;
    }
  }
  printf("Average rtime %d,  wtime %d\n", trtime / NFORK, twtime / NFORK);
  exit(0);
}

// Enhanced Schedulertest
// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"
// #include "kernel/fcntl.h"

// #define NFORK 10
// #define IO 5

// int main()
// {
//   int n, pid;
//   int wtime, rtime, iotime;
//   int twtime = 0, trtime = 0, tiotime = 0;
//   for (n = 0; n < NFORK; n++)
//   {
//     pid = fork();
//     if (pid < 0)
//       break;
//     if (pid == 0)
//     {
//       if (n < IO)
//       {
//         sleep(200); // IO bound processes
//         printf("Process %d (PID %d) IO bound finished (Arrival Time: %d)\n", n, getpid(), uptime());
//       }
//       else
//       {
//         for (volatile int i = 0; i < 1000000000; i++)
//         {
//         } // CPU bound process
//         printf("Process %d (PID %d) CPU bound finished (Arrival Time: %d)\n", n, getpid(), uptime());
//       }
//       exit(0);
//     }
//   }
//   for (; n > 0; n--)
//   {
//     int child_pid = waitx(&wtime, &rtime, &iotime);
//     if (child_pid >= 0)
//     {
//       trtime += rtime;
//       twtime += wtime;
//       tiotime += iotime;
//       printf("Process (PID %d) - Arrival Time: %d, rtime: %d, wtime: %d, iotime: %d\n", child_pid, uptime(), rtime, wtime, iotime);
//     }
//   }
//   printf("Average rtime %d, wtime %d, iotime %d\n", trtime / NFORK, twtime / NFORK, tiotime / NFORK);
//   exit(0);
// }
