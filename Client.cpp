//--------------------------------Client.cpp------------------------------------
//Description: This program is the client application for a client-server
//             homework assignment for CSS432 at the University of Washington.
//             It's a Linux-based client application that creates a socket,
//             connects to a server application, and sends the incoming
//             arguments to the server in an array. It then receives the
//             the reply and reads the contents and outputs the time statistics.
//------------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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

//----------------------------------main----------------------------------------
//Description: The primary driver for the client. It creates the initial socket
//             via the client port number being passed in as an argument, as
//             well as the data for how often to run the connections, the buffer
//             size, the hostname of the server, and the type of data to send.
//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  // Checking to ensure the correct number of arguments are being received
  if (argc < 7 || atoi(argv[6]) > 3 || atoi(argv[6]) < 1)
  {
    perror("Too few arguments passed in, expected 6.");
    exit(1);
  }

  // Assign variables from arguments
  int port        = atoi(argv[1]);
  int repitition  = atoi(argv[2]);
  int nbufs       = atoi(argv[3]);
  int bufsize     = atoi(argv[4]);
  char* serverIp  = argv[5];
  int type        = atoi(argv[6]);

  // Check that nbufs * bufsize = 1500
  if (nbufs * bufsize != 1500)
  {
    cout << "Error -- Data Buffer Size does not equal 1,500." << endl;
    exit(1);
  }

  // LOGS -- REMOVE LATER
  cout << "Port: " << port << "\nRepitition: " << repitition
       << "\nNumber of Data Buffers: " << nbufs
       << "\nBuffer Size: " << bufsize << "\nServer IP Name: "
       << serverIp << "\nScenario Type: " << type << endl;

  // Receive a hostent structure corresponding to the IP - check if host resolves
  struct hostent* host = gethostbyname(serverIp);

  if (host == NULL)
  {
    perror("Unable to resolve host.");
  }

  int clientSd;             // Declare a client socket descriptor
  sockaddr_in sendSockAddr; // Declare a sockaddr_in

  // Zero initialize it
  bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
  // Set its data members
  sendSockAddr.sin_family       = AF_INET; //Address Family internet
  sendSockAddr.sin_addr.s_addr  = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
  sendSockAddr.sin_port         = htons(port);

  // Open a stream-oriented socket with the internet address Family - check for failure
  if ((clientSd = socket(AF_INET, SOCK_STREAM, 0)) <= -1)
  {
    perror("Unable to open a client socket.");
  }

  // Connect to server socket - check for failed connection
  int connectStatus = connect(clientSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
  if (connectStatus < 0)
  {
    perror("Failed to connect to the server.");
  }

  // Create array, initialize loop and timer variables
  char databuf[nbufs][bufsize];
  struct timeval startClock, stopClock, lapTime;

  gettimeofday(&startClock, NULL); // Start clock

  for (int i = 0; i < repitition; i++)
  {
    if (type == 1) // Multiple writes case - Writes for each buffer
    {
      for (int j = 0; j < nbufs; j++)
      {
        write(clientSd, databuf[j], bufsize);
      }
    }
    else if (type == 2) // WriteV case - Sends all buffers of an array of data structures
    {
      struct iovec vector[nbufs];
      for (int j = 0; j < nbufs; j++)
      {
        vector[j].iov_base  = databuf[j];
        vector[j].iov_len   = bufsize;
      }
      writev(clientSd, vector, nbufs);
    }
    else // Single write case - Sends an nbufs sized array of all data buffers at once
    {
      write(clientSd, databuf, (nbufs * bufsize));
    }
  }

  gettimeofday(&lapTime, NULL); // Exit loop of repititions and get the lap time

  int count;
  read(clientSd, &count, sizeof(count)); // Read incoming data and stop the clock
  gettimeofday(&stopClock, NULL);

  // Calculate time
  long lap = (lapTime.tv_sec - startClock.tv_sec) * 1000000;
  lap += (lapTime.tv_usec - startClock.tv_usec);
  long totalTime = (stopClock.tv_sec - startClock.tv_sec) * 1000000;
  totalTime += (stopClock.tv_usec - startClock.tv_usec);

  cout << "Test " << type << ": data-sending time " << lap << " usec, round-trip time = "
       << totalTime << " usec, #read = " << count << endl;

  // Close the socket
  close(clientSd);

  return 0;
} // end of main
