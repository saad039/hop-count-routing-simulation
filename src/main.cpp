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
const int startingServerPort=5000;
class eventLoop;
class routingTable;
static Graph routingTable;
static std::atomic<int> clientCount={1};
//implement a server. Multiple servers can be run on multiple threads.
//Each will be listening on a udp port

void addClient(const std::string& server, const std::string&client);
std::vector<std::string> routingPath(const std::string& src, const std::string& dst);
void handleError(const char* error){
        printf("%s\n",error);
        exit(-1);
}
class routingServer{
    private:
    int getPort(const std::string name){
        for(auto& pair: clients){
            if(std::get<1>(std::get<1>(pair))==name){
                return std::get<0>(pair);
            }
        }
        return -1;
    }
public:
    routingServer(const routingServer&)=delete;
    routingServer& operator=(const routingServer&)=delete;

    routingServer(const std::string& name,const char* ip=nullptr)
    :name(name)
    {
        if((this->socketfd=socket(AF_INET,SOCK_DGRAM,0))<0)
            handleError("Socket could not be created");
        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family=AF_INET;
        if(!ip)
            servaddr.sin_addr.s_addr=INADDR_ANY;
        else
            servaddr.sin_addr.s_addr=inet_addr(ip);
        servaddr.sin_port=htons(port++);
       if(bind(socketfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
           handleError("Unable to bind socket");
        status=true;
        routingTable.addNode(name);
        routerPort=port-1;
    }
    void receive(const int bytes){
         auto data=std::make_unique<char[]>(bytes);
         fd_set rdset;
         FD_SET(this->socketfd,&rdset);
         int len=sizeof(cliaddr);
         std::cout<<"Router: "<<name<<" is now receiving at port: "<<routerPort<<std::endl;
         while(true){
            memset(data.get(),'\0',bytes);
            int fds=select(this->socketfd+1,&rdset,NULL,NULL,NULL);
            if(FD_ISSET(this->socketfd,&rdset)){
                int bytes_recv=recvfrom(this->socketfd,data.get(),bytes,0,
                                        (struct sockaddr*) &cliaddr,(socklen_t*)&len);
                if(bytes_recv<0)
                   handleError("Error in receiving data");
                int cport = (int) ntohs(cliaddr.sin_port); 
                char *ip = inet_ntoa(cliaddr.sin_addr);
                //std::cout<<"Sender's port: "<<cport<<std::endl;
                std::string message(data.get()); 
                message.erase(std::remove(std::begin(message),
                        std::end(message),'\n'),std::end(message));
                std::cout<<"Sender: "<<cport<<", Message: "<<message<<std::endl;  
                if(message =="HS"){ // a new client establishing connection
                   std::string alias="c";
                   alias+=std::to_string(clientCount++);
                   this->clients.insert({port,{ip,alias}});
                   addClient(name,alias); //add node and add edge to the graph
                   std::cout<<"Client Registered"<<std::endl;
                }  
                else{
                    //packet forwarding(routing)
                    int hopCount=std::stoi(message.substr(0,message.find('#')));
                    if(hopCount-1!=0){
                        message=message.substr(message.find('#')+1);//hop count trimmed
                        std::string dest=message.substr(0,message.find(':'));
                        std::vector<std::string> route=routingPath(name,dest);
                        if(route.size()==1){
                            std::cout<<"Path not available"<<std::endl;
                        }
                        else{
                            const std::string& nextDest=route[1];
                            int destPort;
                            if(nextDest[0]=='s'){
                                std::string index=nextDest.substr(1);
                                destPort=startingServerPort+(std::stoi(index)-1);
                            }
                            else{
                                destPort=getPort(nextDest);
                            }
                            if(destPort!=-1){
                                message=message.substr(message.find(':')+1);
                                if(message.find('$')==std::string::npos){
                                message+="$";
                                }
                                message+=name;
                                message+=',';
                                std::cout<<"Message forwarded: "<<message<<std::endl;
                                int bysent=send(message.c_str(),
                                    std::get<0>(clients[destPort]).c_str(),destPort);
                            }
                            else{
                                std::cout<<"Packet forwarding failed"<<std::endl;
                            }
                        }
                    }    
                }
            }
         }
        //  return std::make_pair(bytes,std::move(data));
    }
    int send(const char* data,const char* ip,int port){
            struct sockaddr_in destaddr;
            bzero(&destaddr,sizeof(destaddr));
            destaddr.sin_family=AF_INET;
            if(!ip)
                destaddr.sin_addr.s_addr=INADDR_ANY;
            else
                destaddr.sin_addr.s_addr=inet_addr(ip);
            destaddr.sin_port=htons(port);
            int len=sizeof(destaddr);
            int bytesSend = sendto(this->socketfd, data,256, 0, (struct sockaddr *) &destaddr, len);
            if (bytesSend < 0)
                handleError("Failed to send data");
            return bytesSend;
        }
    ~routingServer(){close(this->socketfd);       }
    bool getStatus() const{return this->status;   }
    int operator() ()const{return this->socketfd; }
private:
    static std::atomic<int> port;//(r)
    int socketfd;
    struct sockaddr_in servaddr,cliaddr;
    bool status=false;
    int routerPort;
    std::map<int,std::pair<std::string,std::string>> clients; //(r){port,{ip,name}}
    std::string name;//(r)
};
std::atomic<int> routingServer::port{startingServerPort};
void addClient(const std::string& server, const std::string&client){
    routingTable.addNode(client);
    routingTable.addDualEdge(server,client);
    //serialize table
}
std::vector<std::string> routingPath(const std::string& src, const std::string& dst){
    if(!routingTable.pathCheck(src,dst))
        return std::vector<std::string>();
    auto p1 =routingTable.getShortestPath(src,dst);
    auto p2 =routingTable.getShortestPath(dst,src);
    std::reverse(std::begin(p2),std::end(p2));
    return p1.size() > p2.size()?  p1   :   p2;
}
//Threading on receive will be invoked from outside.
//Server and client indexing will start from 1
std::unique_ptr<std::vector<std::thread>> 
    igniteNetwork(const std::vector<std::unique_ptr<routingServer>>& routers){
    const int receiveBufferSize=512;
    std::vector<std::thread> threads;
    for(int i=0;i<routers.size();i++){
        threads.push_back(std::thread([&](){
            routers[i]->receive(receiveBufferSize);
        }));
    }
    return std::make_unique<std::vector<std::thread>>(std::move(threads));
}
int main()
{
   
    auto s1=std::make_unique<routingServer>("s1");
    auto s2=std::make_unique<routingServer>("s2");
    routingTable.addDualEdge("s1","s2");
    auto t1=std::thread([&](){
        s1->receive(512);
    });

    auto t2=std::thread([&](){
        s2->receive(512);
    });
    t1.join();
    t2.join();
	return 0;	
}


class eventLoop{
private:
typedef std::size_t usint; 
    static std::unique_ptr<eventLoop> tpInstance; // instance of eventLoop 
    std::thread tpThread; // vector containing all the threads in the eventLoop
    std::queue<std::function<void()>>tasks; // vector containing tasks
    bool doKill=false; // a boolean flag to signal joining of threads
    std::mutex mutex; // mutex for providing mutual exclusion access to tasks vector
    std::condition_variable conVar; // condition variable to avoid busy waiting

    eventLoop()
        :tpThread(std::thread(&eventLoop::start, this))
    {

    }
	void start() {
		for (;;) {
			std::function<void()> currentTask;
			{
				std::unique_lock<std::mutex> lock(mutex);
				conVar.wait(lock, [&]() {return doKill || !tasks.empty(); });
				if (this->doKill && tasks.empty())
					break;
				currentTask = std::move(this->tasks.front());
				this->tasks.pop();
			}
			currentTask();
		}
	}
	void stop() {
		{
			std::unique_lock<std::mutex> lock(mutex);
			this->doKill = true;
			conVar.notify_one();
		}
		tpThread.join();
	}
public:

    static std::unique_ptr<eventLoop>& factory(){
        if(tpInstance==nullptr)
            tpInstance.reset(new eventLoop());
        return tpInstance;
    }
    template<typename _type>
    auto enqueue_task(_type Task) ->std::future<decltype(Task())>
    {
        auto lambda=std::make_shared<std::packaged_task<decltype(Task())()>>(Task);
        {
			std::unique_lock<std::mutex> lock(mutex); 
            this->tasks.emplace([=](){
                (*lambda)();
            });
            this->conVar.notify_one();
        }
        return lambda->get_future();
    }
    eventLoop(const eventLoop&)=delete;
    eventLoop(const eventLoop&&)=delete;
    eventLoop& operator=(const eventLoop&)=delete;
    eventLoop& operator=(const eventLoop&&)=delete;
    ~eventLoop(){
       stop();
    }
};