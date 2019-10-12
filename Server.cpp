//-----------------------------Server.cpp---------------------------------------
//Description: This program is the server application for a client-server
//             homework assignment for CSS432 at the University of Washington.
//             It's a Linux-based server application that accepts several
//             incoming parallel socket connections by using the pthreads
//             library and records the time needed to read the data in each
//             packet, and presents it to the console while sending a count
//             back to the client application.
//------------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <strings.h>      // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev

using namespace std;
const int BUFSIZE = 1500;      // Buffer size, hw requirement

// Structure for holding data for an individual thread's repetition and socket descriptor
struct thread_data
{
  int repetition;
  int sd;
};

//-------------------------------writeStats-------------------------------------
//Description: This function is called by each thread and counts the number of
//             reads, as well as records the time in which it takes to do so.
//             Then it sends that data back to the client.
//------------------------------------------------------------------------------
void *writeStats(void *data)
{
  // Initialize variables
  int count;
  char databuf[BUFSIZE];
  struct timeval startClock, stopClock;
  gettimeofday(&startClock, NULL);      // Start clock

  // Loop over thread and read contents
  for (int i = 0; i < ((thread_data *)data)->repetition; i++)
  {
    for (int nRead = 0; (nRead += read(((thread_data *)data)->sd, databuf + nRead, BUFSIZE - nRead)) < BUFSIZE; ++count);
    count++;
  }

  gettimeofday(&stopClock, NULL);     // Stop clock


  // Calculate time
  long deltaTime = (stopClock.tv_sec - startClock.tv_sec) * 1000000;
  deltaTime += (stopClock.tv_usec - startClock.tv_usec);

  cout << "data-receiving time = " << deltaTime << " usec" << endl; //Display time

  write(((thread_data *)data)->sd, &count, sizeof(count)); // Send count and socket data
} //end of writeStats

//----------------------------------main----------------------------------------
//Description: The primary driver for the server. It creates the initial socket
//             via the server port number being passed in as an argument,
//             then listens for a connecting host and establishes threads for
//             each connection.
//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  int port        = atoi(argv[1]);    // Client port number
  int repetition  = atoi(argv[2]);    // Number of repititions on the read

  int serverSd;                  // Declare a server socket descriptor
  sockaddr_in acceptSockAddr;    // Declare a sockaddr_in structure

  bzero((char*)&acceptSockAddr, sizeof(acceptSockAddr)); // Zero initialize it
  // Set its data members
  acceptSockAddr.sin_family       = AF_INET; // Address Family Internet
  acceptSockAddr.sin_addr.s_addr  = htonl(INADDR_ANY);
  acceptSockAddr.sin_port         = htons(port);

  // Open a stream-oriented socket with the internet address Family
  if ((serverSd = socket(AF_INET, SOCK_STREAM, 0)) <= -1)
  {
    perror("Unable to open a server socket.");
  }

  // Set the SO_REUSEADDR option
  const int on = 1;
  if (setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) <= -1)
  {
    perror("Unable to execute setsockopt SO_REUSEADDR.");
  }

  // Bind this socket to its local Address
  if (bind(serverSd, (sockaddr*)&acceptSockAddr, sizeof(acceptSockAddr)) <= -1)
  {
    perror("Unable to bind the server socket to the local address.");
  }

  // Instruct the operating system to listen to up to n connection requests
  listen(serverSd, 10);

  // Receive a request from a client
  int newSd; // Declare new serverSd
  sockaddr_in newSockAddr;
  socklen_t newSockAddrSize = sizeof(newSockAddr);

  struct thread_data *data; // Declare thread_data struct pointer
  while (1)
  {
    if ((newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize)) <= -1)
    {
      perror("Unable to accept incoming socket connection.");
    }

    // Create new thread
    pthread_t new_thread;
    data              = new thread_data;
    data->repetition  = repetition;
    data->sd          = newSd;
    int iret1         = pthread_create(&new_thread, NULL, writeStats, (void*) data);
  }

  // Close socket and free memory
  close(data->sd);

  return 0;
} //end of main
