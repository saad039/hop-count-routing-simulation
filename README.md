# hop-count-routing-simulation #
<br><h6> A hop-count based routing simulation using UDP sockets that using Dijksta's algorithm to calculate the shortest path </h6></br>
<br><h6> between source and destination.</br>
<br># Dependencies #</br>
<ol>
<li>clang v9.0</li>
<li>c++17</li>
<li>Boost libaries </li>
</ol>

<ol>
<li>Steps
<li>Install boost. </li>
<br>```sudo apt-get install libboost-all-dev```</br>
<li>To build the project, give write permission to the script ```build.sh``` . </li>
<br>```chmod a+x build.sh```</br>
<li>To build the project, run the script ```build.sh``` . To generate debug build use the flag debug and use release for  </li>
     <br>generating release builds.This will generate the server side executable in the bin folder.</br>
<br>```./build.sh debug```</br>
<li>The first router(server) is passed only one argument which is the port number on which it listens. When you add new routers you pass two arguments, first being the the port number on which they listen for incomming request and the second 
being the router to which they are connected.</li>
<li>Compile the file client.cpp. The client executable is passed a single argument which is the port number of the router <br>to which the client is connected. </li>
<li> After creating a topology of your network, the syntax to route a message is</li>  
     <br>```hopCount#destinationClientPort:Message```</br>
     <br>Entering this command on any client will deliver the message to the destination router </br>
</ol>
