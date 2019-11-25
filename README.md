# hop-count-routing-simulation
<br>## A hop-count based routing simulation using UDP sockets that using Dijksta's algorithm to calculate the shortest path between
<br>## source and destination.</br>
<br># Dependencies</br>
<br>## 1. clang v9.0</br>
<br>## 2. c++17</br>
<br>## 3. Boost libaries.</br>
<br># Steps</br>
<br>## 1. Install boost. </br>
<br>```sudo apt-get install libboost-all-dev```</br>
<br>## 2. To build the project, give write permission to the script ```build.sh``` . </br>
<br>```chmod a+x build.sh```</br>
<br>## 3. To build the project, run the script ```build.sh``` . To generate debug build use the flag debug and use release for </br>
     <br>generating release builds.This will generate the server side executable in the bin folder.</br>
<br>```./build.sh debug```</br>
<br>## 4. The first router(server) is passed only one argument which is the port number on which it listens. When you add new                                                routers you pass two arguments, first being the the port number on which they listen for incomming request and the second 
being the router to which they are connected.</br>
<br>## 5. Compile the file client.cpp. The client executable is passed a single argument which is the port number of the router to 
         <br>which the client is connected. </br>
<br>## 6. After creating a topology of your network, the syntax to route a message is </br>
     <br>```hopCount#destinationClientPort:Message```</br>
     <br>Entering this command on any client will deliver the message to the destination router </br>
