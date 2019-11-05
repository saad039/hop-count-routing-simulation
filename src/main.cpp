#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <functional>
#include <future>
#include <queue>
#include <vector>
#include <condition_variable>
#include <mutex>
#include<map>
#include<sstream>
#include<algorithm>
#include<chrono>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include "graph-library/Graph.h"
extern "C"{
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/wait.h>
}

#define BUFFER_SIZE 512
//implement a server. Multiple servers can be run on multiple threads.
//Each will be listening on a udp port

std::vector<std::string> tokenizer(const std::string& str, const char delimeter);
void handleError(const char* error){
        printf("%s\n",error);
        exit(-1);
}
class RoutingTable{
    Graph netwgit push -u origin masterork;
    const std::string fileName="routing-table.txt";
    std::mutex mutex;
    std::thread th;
    //{port,ip}
    std::map <int,std::string> nodes;
    //{srcPort,destPort}
    std::vector<std::pair<int,int>> edges;
    public:
    RoutingTable(bool shouldClear){
        if(shouldClear){ //clears the file in case of first server
            std::ofstream out(fileName,std::ofstream::out | std::ofstream::trunc);
            out.close();
        }
        deserialize();
        th=std::move(std::thread([&](){
                while(true){
                    deserialize();
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }
        ));
    }
    std::string getIPFromPort(const int port){
        std::string ip;
        for(auto& pair:nodes){
            if(std::get<0>(pair)==port){
                ip=std::get<1>(pair);
            }
        }
        return ip;
    }
    void addNode(const int port,const std::string& ip){
        {
            std::unique_lock<std::mutex> lock(mutex);
            std::string name=std::to_string(port);
            if(!this->network.nodeExists(name)){
                network.addNode(name);
                this->nodes.insert({port,ip});
            }
        }
        serialize();
    }
    private:
        void insertEdgeHelper(int srcPort,int destPort){
            auto it=std::find_if(std::begin(edges),
                                 std::end(edges),
                                 [&](const std::pair<int,int>& p){
                                     return std::get<0>(p)==srcPort &&
                                            std::get<1>(p)==destPort;
                                 });
            if(it==std::end(edges))
                this->edges.push_back({srcPort,destPort});
        }
    public:

    void addEdge(int srcPort,int destPort){
        {
            std::unique_lock<std::mutex> lock(mutex);
            this->network.addDualEdge(std::to_string(srcPort),std::to_string(destPort));
            insertEdgeHelper(srcPort,destPort);
        }
        std::cout<<"Total edges: "<<this->edges.size()<<std::endl;
        for(auto& edge: edges){
            std::cout<<std::get<0>(edge)<<"->"<<std::get<1>(edge)<<std::endl;
        }
        std::cout<<"----------\n";
        serialize();
    }   
    void serialize(){
        std::ofstream out(fileName);
        boost::interprocess::file_lock ol(fileName.c_str());
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> sol(ol); 
        if(out){
            out<<nodes.size()<<std::endl;
            for(auto& pair: nodes){
                out <<std::get<0>(pair)<<','
                    <<std::get<1>(pair)<<','
                    <<std::endl;
            }
            out<<edges.size()<<std::endl;
            for(auto& pair:edges){
                out<<std::get<0>(pair)<<','<<std::get<1>(pair)<<std::endl;
            }
        }
    }
    std::vector<std::string> routingPath(const std::string& src, const std::string& dst){
        return network.getCustomShortestPath(src,dst);
    }
    void deserialize(){
        std::ifstream infile(fileName);
        boost::interprocess::file_lock ol(fileName.c_str());
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> sol(ol); 
        if(infile){
            std::unique_lock<std::mutex> lock(mutex);
            std::string line;
            getline(infile,line);
            if(line.size()==0)  return;
            int numNodes=std::stoi(line);
            for(int i=0;i<numNodes;i++){
                getline(infile,line);
                std::vector<std::string> tokens=tokenizer(line,',');
                if(!this->network.nodeExists(tokens[0])){
                    network.addNode(tokens[0]);
                    this->nodes.insert({std::stoi(tokens[0]),tokens[1]});
                }
            }
            getline(infile,line);
            int numEdges=std::stoi(line);
            for(int i=0;i<numEdges;i++){
                getline(infile,line);
                std::vector<std::string> tokens=tokenizer(line,',');
                this->network.addDualEdge(tokens[0],tokens[1]);
                int srcPort=std::stoi(tokens[0]);
                int destPort=std::stoi(tokens[1]);
                insertEdgeHelper(srcPort,destPort);                
            }
        }
    }
    ~RoutingTable(){
        th.join();
    }
};
class routingServer{
    int socketfd;
    struct sockaddr_in servaddr,cliaddr;
    bool status=false;
    int routerPort;
    std::map<int,std::pair<std::string,std::string>> clients; //(r){port,{ip,name}}
    RoutingTable table;
public:
    routingServer(const int port,bool clear,const char* ip=nullptr)
    :routerPort(port),table(clear) {
        if((this->socketfd=socket(AF_INET,SOCK_DGRAM,0))<0)
            handleError("Socket could not be created");
        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family=AF_INET;
        if(!ip)
            servaddr.sin_addr.s_addr=INADDR_ANY;
        else
            servaddr.sin_addr.s_addr=inet_addr(ip);
        servaddr.sin_port=htons(port);
       if(bind(socketfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
           handleError("Unable to bind socket");
        status=true;
    }
    void receive(){
        char data[BUFFER_SIZE];
        int len = sizeof(cliaddr);
        std::cout << "Router: is now receiving at port: " << routerPort << std::endl;
        while (true)
        {
            memset(data, '\0', BUFFER_SIZE);
            int bytes_recv = recvfrom(this->socketfd, data, BUFFER_SIZE, 0,
                                      (struct sockaddr *)&cliaddr, (socklen_t *)&len);
            if (bytes_recv < 0)
                handleError("Error in receiving data");
            int cport = (int)ntohs(cliaddr.sin_port);
            char *ip = inet_ntoa(cliaddr.sin_addr);
            std::string message(data);
            message.erase(std::remove(std::begin(message),std::end(message), '\n'),std::end(message));
            if (message == "HS"){
                // a new client establishing connection
                //add node and add edge to the graph
                table.addNode(cport, ip);
                table.addEdge(this->routerPort, cport);
                std::cout << "Client Registered" << std::endl;
            }
            else{   //packet forwarding(routing)
                std::cout << "Sender: " << cport << ", Message: " << message << std::endl;
                int hopCount = std::stoi(message.substr(0, message.find('#')));
                if (hopCount - 1 != 0){
                    message = message.substr(message.find('#') + 1);         //hop count trimmed
                    std::string dest = message.substr(0, message.find(':')); //dest is port
                    std::cout<<"Dest: "<<dest<<std::endl;
                    std::vector<std::string> route = table.routingPath(
                        std::to_string(this->routerPort),
                        dest);
                    std::cout<<"----------Printing Route----------"<<std::endl;
                    for(auto&r: route){
                        std::cout<<r<<"->";
                    }
                    std::cout<<"\n----------Printing  Done----------"<<std::endl;
                    if (route.size() == 0){
                        std::cout << "Path not available" << std::endl;
                    }
                    else{
                        int destPort = std::stoi(route[1]);
                        message = message.substr(message.find(':') + 1);
                        if (message.find('$') == std::string::npos){
                            message += "$";
                        }
                        message += std::to_string(this->routerPort);
                        message += ',';
                        std::string toInsert=std::to_string(hopCount-1);
                        toInsert+="#";
                        toInsert+=dest;
                        toInsert+=':';
                        message.insert(0,toInsert);
                        int bysent = send(message,
                                      table.getIPFromPort(destPort).c_str(),
                                      destPort);
                        if (bysent < 0)
                            std::cout << "Failed to forward message" << std::endl;
                        else{
                            std::cout << "message (" << message 
                            <<") forwarted to destination port: "<<destPort<< std::endl;
                        }
                    }
                }
                else{
                    std::cout<<"Packet Dropped"<<std::endl;
                }
                
            }
        }
    }
    int send(const std::string& data,const std::string& ip,int port){
            struct sockaddr_in destaddr;
            bzero(&destaddr,sizeof(destaddr));
            destaddr.sin_family=AF_INET;
            destaddr.sin_addr.s_addr=inet_addr(ip.c_str());
            destaddr.sin_port=htons(port);
            int len=sizeof(destaddr);
            int bytesSend = sendto(this->socketfd, data.c_str(),data.size()+1, 0, (struct sockaddr *) &destaddr, len);
            return bytesSend;
        }
    ~routingServer(){close(this->socketfd);       }
    bool getStatus() const{return this->status;   }
    int operator() ()const{return this->socketfd; }
    RoutingTable& getTable(){return this->table;  }
    routingServer(const routingServer&)=delete;
    routingServer& operator=(const routingServer&)=delete;
};

//port is same as name
//number of servers
//port,ip
//number of clients
//port,ip
//number of edges
//---------------
//serverPort,serverIP,edgeOfServerPort (future)

////serverPort,edgeOfServerPort

int main(int argc, char const *argv[]){
    bool shouldClear=argc==3?false:true;
    const char* argv1=argv[1];
    routingServer rs(std::stoi(argv1),shouldClear); //rs is on on port
    rs.getTable().addNode(std::stoi(argv1),std::string("127.0.0.1"));
    if(argc==3){
        rs.getTable().addEdge(std::stoi(argv1),std::stoi(argv[2]));
    }
    rs.receive();
	return 0;	
}
std::vector<std::string> tokenizer(const std::string& str, const char delimeter) {
    std::vector<std::string> tokens;
	std::string temp;
	temp.reserve(50);
	for (int i = 0, size = str.length(); i < size; i++) {
		if (str[i] != delimeter) {
			temp += str[i];
		}
		else {
			if (temp.length()) {
				tokens.push_back(temp);
				temp.clear();
				temp.reserve(50);
			}
		}
	}
	if (temp.length())
		tokens.push_back(temp);
    return std::move(tokens);
}


