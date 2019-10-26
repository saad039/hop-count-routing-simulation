#include <iostream>
#include <fstream>
#include <string>
#include<memory>
#include<atomic>

#include "graph-library/Graph.h"
extern "C"{
#include<stdio.h>
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
#include<sys/wait.h>
}

//implement a server. Multiple servers can be run on multiple threads.
//Each will be listening on a udp port
void handleError(const char* error){
        printf("%s\n",error);
        exit(-1);
}
class routingServer{
private:
    class UdpSocket{
    private:
        int socketfd;
        struct sockaddr_in servaddr,cliaddr;
        bool status=false;
    public:
        UdpSocket(const UdpSocket&)=delete;
        UdpSocket& operator=(const UdpSocket&)=delete;

        UdpSocket(const int port,const char* ip=nullptr){
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
        bool getStatus() const{return this->status;}
        std::pair<int,std::unique_ptr<char[]>> receive(const int bytes){
             auto data=std::make_unique<char[]>(bytes);
             memset(data.get(),'\0',bytes);
             int len=sizeof(cliaddr);
             int bytes_recv=recvfrom(this->socketfd,data.get(),bytes,0,(struct sockaddr*) &cliaddr,(socklen_t*)&len);
             if(bytes_recv<0)
                handleError("Error in receiving data");
             return std::make_pair(bytes,std::move(data));
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
        ~UdpSocket(){
            close(this->socketfd);
        }
        int operator() ()const{return this->socketfd;}
    };
    
public:
    routingServer(const char* ip=nullptr)
    :udpsock(port++,ip)
    {
        if(this->udpsock.getStatus())
            std::cout<<"Router is up at : "<<port-1<<std::endl;
        else
            std::cout<<"Router failed to get up at : "<<port-1<<std::endl;
    }
    ~routingServer(){};
    int send(const char* data,const char* ip,int port){
        return this->udpsock.send(data,ip,port);
    }
    std::pair<int,std::unique_ptr<char[]>> receive(const int bytes){
        return udpsock.receive(bytes);
    }

private:
    static std::atomic<int> port;
    UdpSocket udpsock;
};
std::atomic<int> routingServer::port{5030};
int main()
{
/*
    FD_SET(int fd, fd_set *set); Add fd to the set .
    FD_CLR(int fd, fd_set *set); Remove fd from the set .
    FD_ISSET(int fd, fd_set *set); Return true if fd is in the set .
    FD_ZERO(fd_set *set); Clear all entries from the set .
    int select(int numfds, fd_set *readfds, fd_set *writefds,
        fd_set *exceptfds, struct timeval *timeout);
*/
    routingServer s1;
    const char* data="Hello World from Saad.essaere";
    int bytes=s1.send(data,nullptr,5040);
    std::cout<<"Bytes send: "<<bytes<<std::endl;
    
    // int listenfd,maxfd,n,clients[1024];
    // socklen_t len;
    // char ip[INET_ADDRSTRLEN]; 
    // char msg[1024],sending[1024];
    // char clientAddr[1024];
   
    // printf("\n\n Binded");
    
	// fd_set tset,rset;
	// for( int i = 0 ; i < 1023 ; i++ ){
	// 	clients[i]=-1;
	// }
	
	// FD_ZERO(&rset);
	// FD_SET(listenfd,&rset);
	// maxfd=listenfd;
	// while(1){
	// 	tset=rset;
	// 	int fd_count=select(maxfd+1,&tset,NULL,NULL,NULL);
	// 	if(FD_ISSET(listenfd,&tset)){
    //             char msg[256];
               
    //             printf("Client said: %s\n",msg);
	// 		}
	// 	}
	// close(listenfd);
	return 0;	
}

std::string edgeString(std::tuple<std::string, std::string, int> edge)
{
    std::string str = std::get<0>(edge) + std::get<1>(edge) + std::to_string(std::get<2>(edge));
    return str;
}

//TODO
/*
1)Whenever a client/routing-server is attached to any router/server it will register its IP and 
update its routing table(routing file.txt).                                                    (10)
    a) client/routing-server and routing-server communicate via udp
    b) clients/routing-server tells the routing-server its ip address and port
    c) routing server updates the routing_file.txt with the record 
    {clientIP/routing-server,clientPort/routing-server,routing-server}

2)This routing table will be shared with other routers so each router should have
information about the connected networks/clients with other routers.                (20)
    a)when ever a change is made to the routing_file.txt, each routing-server should 
    update its record.
3)Routers should also mention the convergence state it means that when each router
have updated information about the connected clients of all other routers it is called
to be converged. So if new clients comes in a network our network should come to
converged state within the next 5 seconds.                                          (10)
    a)all routing-server should update their records within 5-seconds
4)As the routing table sharing is a broadcast msg in our case but every router should
have one routing table, it should not have multiple copies.                         (10)
    a)shoud read/write information to the same file. No global data
    5)In order to efficiently deliver data towards destination each router should know
    which path to be followed to deliver this packet. We are considering the simplest
    approach of RIP that works on the hop count. So there is a need to keep only that
path in routing table that is most efficient, all other paths should be declined.   (20)
    a)shortest path
*/