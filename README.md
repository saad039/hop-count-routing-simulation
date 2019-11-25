<h2>hop-count-routing-simulation</h2>>
<h6> A hop-count based routing simulation using UDP sockets that using Dijksta's algorithm to calculate the shortest path </h6></br>
<br><h6> between source and destination.</h6>
<h2>Dependencies</h2>
<ul>
<li>clang v9.0</li>
<li>c++17</li>
<li>Boost libaries </li>
</ul>
<h2>Steps</h2>
<ul>
<li>Install boost. </li>
</ul>
<br>```sudo apt-get install libboost-all-dev```</br>
<ul>
<li>To build the project, give write permission to the script "build.sh" . </li>
</ul>
<br>```chmod a+x build.sh```</br>
</ul>
<li>To build the project, run the script "build.sh" . To generate debug build use the flag debug and use release for
    generating release builds.This will generate the server side executable in the bin folder.</li>
</ul>
<br>```./build.sh debug```</br>
<ul>
<li>The first router(server) is passed only one argument which is the port number on which it listens. When you add new routers you pass two arguments, first being the the port number on which they listen for incomming request and the second 
being the router to which they are connected.</li>
<li>Compile the file client.cpp. The client executable is passed a single argument which is the port number of the router <br>to which the client is connected. </li>
<li> After creating a topology of your network, the syntax to route a message is</li>  
     <br>```hopCount#destinationClientPort:Message```</br>
     <br>Entering this command on any client will deliver the message to the destination router </br>
</ul>
