# hop-count-routing-simulation
##A hop-count based routing simulation using UDP sockets that using Dijksta's algorithm to calculate the shortest path between
##source and destination.
#Dependencies
##1. clang v9.0
##2. c++17
##3. Boost libaries.
#Steps
##1. Install boost. 
```sudo apt-get install libboost-all-dev```
##2. To build the project, give write permission to the script ```build.sh``` . 
```chmod a+x build.sh```
##3. To build the project, run the script ```build.sh``` . To generate debug build use the flag debug and use release for 
     generating release builds.This will generate the server side executable in the bin folder.
```./build.sh debug```
##4. The first router(server) is passed only one argument which is the port number on which it listens. When you add new routers
     you pass two arguments, first being the the port number on which they listen for incomming request and the second being the 
     router to which they are connected.
##5. Compile the file client.cpp. The client executable is passed a single argument which is the port number of the router to 
     which the client is connected. 
##6. After creating a topology of your network, the syntax to route a message is 
     ```hopCount#destinationClientPort:Message```
     Entering this command on any client will deliver the message to the destination router 
