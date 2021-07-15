#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<memory>
#include <netdb.h>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <fcntl.h>
#include <pthread.h>
#include<iostream>
#include<vector>
#include<sys/epoll.h>
#include "../http.hpp"
using namespace std;
/*
fd_set rd_list,fd_list;
static int fd_max = 0;
pthread_mutex_t m;
*/
/*epoll*/
#define MAX_EPOLL_NUM 500
void setnonblocking(int fd){
    int flag = fcntl(fd,F_GETFL);
    fcntl(fd,F_SETFL,flag|O_NONBLOCK);
}
int main(){
    char buffer[2000];
    int fd = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in ser ;
    ser.sin_addr.s_addr = inet_addr("192.168.1.107");
    ser.sin_family = AF_INET;
    ser.sin_port = htons(8000);
    http_requests s(1);
    bind(fd,(sockaddr*)&ser,sizeof(ser));
    listen(fd,200);
    int epfd = epoll_create(200);
    epoll_event ev,events[MAX_EPOLL_NUM];
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
    while(true){
        int nfds = epoll_wait(epfd,events,MAX_EPOLL_NUM,-1);
        if(nfds<=0) continue;
        for(int i=0;i<nfds;i++){
            int temp_fd = events[i].data.fd;
            if(temp_fd==fd){//new connection
                if(events[i].events&EPOLLIN){//new connection
                    int c_fd = accept(fd,NULL,NULL);
                    setnonblocking(c_fd);
                    ev.data.fd  = c_fd;
                    ev.events = EPOLLIN|EPOLLOUT|EPOLLRDHUP;
                    epoll_ctl(epfd,EPOLL_CTL_ADD,c_fd,&ev);
                    printf("a client connects\n");
                }
            }
            else{//client changes
                if(events[i].events&EPOLLRDHUP){//close
                    ev.data.fd = temp_fd;
                    ev.events = EPOLLIN|EPOLLOUT|EPOLLRDHUP;
                    epoll_ctl(epfd,EPOLL_CTL_DEL,temp_fd,&ev);
                    close(temp_fd);
                    printf("a client left\n");
                    continue;
                }
                if(events[i].events&EPOLLIN){
                    while(true){
                        memset(buffer,0,2000);
                        int len = recv(temp_fd,buffer,2000,0);
                        if(len>0){
                            puts(buffer);
                        } 
                        
                        else{
                            if(events[i].events&EPOLLOUT){
                                write(temp_fd,s.render_request().c_str(),s.length());
                            }
                            if(errno = EAGAIN) break;//try again,keep connection
                            else{
                                perror("ERROR");
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    close(fd);
    return 0;
}

/*select*/
// int main(){
//     int fd = socket(AF_INET,SOCK_STREAM,0);
//     sockaddr_in ser;
//     ser.sin_addr.s_addr = inet_addr("127.0.0.1");
//     ser.sin_family = AF_INET;
//     ser.sin_port = htons(8000);
//     bind(fd,(sockaddr*)&ser,sizeof(ser));listen(fd,5);
//     int max_fd = fd;
//     fd_set rdlist;
//     char buf[200];
//     while(true){
//         for(int i=0;i<=max_fd;i++) FD_SET(i,&rdlist);
//         int flag = select(max_fd+1,&rdlist,NULL,NULL,NULL);
//         if(flag<0) break;
//         for(int i=0;i<=max_fd;i++){
//             if(FD_ISSET(i,&rdlist)){
//                 if(i==fd){//new connection
//                     int c_fd = accept(fd,NULL,NULL);
//                     int f = fcntl(c_fd,F_GETFL);
//                     fcntl(c_fd,F_SETFL,f|O_NONBLOCK);
//                     max_fd = c_fd > max_fd ? c_fd : max_fd;
//                 }
//                 else{
//                     while(true){
//                         memset(buf,0,200);
//                         int len = read(i,buf,200);
//                         if(len>0) puts(buf);
//                         else if(len==0){
//                             printf("closed");
//                             break;
//                         }
//                         else{
//                             if(errno==EAGAIN) break;
//                             perror("ERROR");
//                             break;
//                         }
//                     }
//                 }
//             }
//         }
//     }
//     close(fd);
//     return 0;
// }
