#include<iostream>
#include<memory>
#include<string>
#include<thread>
#include<mutex>
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
#define BUFFER_SIZE 256
std::mutex logMutex;
std::mutex sockMutex;
void safeLog(const char* msg){
    std::unique_lock<std::mutex> lock(logMutex);
    std::cout<<msg<<std::endl;
}
void handleError(const char* error){
    perror(error);
    exit(1);
}
class UdpSocket{
    private:
        int socketfd;
        struct sockaddr_in servaddr;
    public:
        UdpSocket(const UdpSocket&)=delete;
        UdpSocket()=delete;
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
            connect(socketfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
        }
        int receive(char* receiveBuffer){
             memset(receiveBuffer,'\0',BUFFER_SIZE);
             int len=sizeof(servaddr);
             int size=recvfrom(this->socketfd,receiveBuffer,BUFFER_SIZE,0,(struct sockaddr*) &servaddr,(socklen_t*)&len);
             if(size<0)
                handleError("Error in receiving data");
             return size;
        }
        int send(const std::string& data){
            int len=sizeof(servaddr);
            int size = sendto(this->socketfd, data.c_str(),data.length()+1, 0, (struct sockaddr *) &servaddr, len);
            if (size < 0) 
                handleError("Failed to send data");
            return size;
        }
        ~UdpSocket(){close(this->socketfd);}
        int operator() ()const{return this->socketfd;}
    };
    
int main(int argc, char const *argv[]){
    const char* handshake="HS";
    bool status=false;
    int serverPort=std::stoi(argv[1]);
    UdpSocket sock(serverPort,nullptr);
    int bysent=sock.send(handshake);
    if(bysent<0){
        handleError("Failed to handShake");
    }
    if(status)
        safeLog("Client is up and running.'~' to exit");
    char buffer[BUFFER_SIZE]={0};
    std::thread th([&](){
        while(true){
            char buffer[BUFFER_SIZE];
            int bytes=sock.receive(buffer);
            std::string msg="Data received: ";
            msg+=std::string(buffer);
            safeLog(msg.c_str());
        }
    });
    std::string input;
    while(true){
            std::getline(std::cin,input);
            if(input.find('~')!=std::string::npos)    
                return 0;
            bysent=sock.send(input);
            std::string msg="Bytes read: ";
            msg+=std::to_string(input.size());
            safeLog(msg.c_str());
        }
        
    return 0;
}
