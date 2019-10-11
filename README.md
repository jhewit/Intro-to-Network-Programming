# Intro-to-Network-Programming
**CSS432 - Homework Assignment 1**

## To Build and Run Locally through Linux CLI
### Server.cpp
1. Compile and name the executable.

    ```g++ Server.cpp -o Server -lpthread```
2. Run the Server with the correct number of arguments

    ```./Server 4000 20000```
### Client.cpp
1. Compile and name the executable.

    ```g++ Client.cpp -o Client```
2. Run the Client with the correct number of arguments:
   *Port#, #Repetitions, #data buffers, buffer size (in bytes), hostname, type of send process*

    ```./Client 4000 20000 15 100 localhost 1```
