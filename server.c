#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>


int main(int argc, char const *argv[]){

	if(argc != 2) {
		perror("Invalid argument number : \n1 : Port");
		return -1;
	}

	char* port = argv[1];
    
    struct sockaddr_in addrClient;

    struct sockaddr_in addrServ;
    addrServ.sin_family=AF_INET;
    addrServ.sin_addr.s_addr = INADDR_ANY;
    addrServ.sin_port = htons((short) atoi(argv[1]));
    socklen_t lg = sizeof(struct sockaddr_in);

    
    int ID_CLIENT_1 = 1;
    int ID_CLIENT_2 = 2;
    char msg[200];

    int dSock = socket(PF_INET,SOCK_STREAM,0);

    if (dSock < 0){
        perror("Error while creating the server socket");
        exit(0);
    }

    int rBind = bind(dSock,(struct sockaddr *) &addrServ,sizeof(addrServ));

    if (rBind < 0){
            perror("Error while binding");
            exit(0);
        }

    int rListen = listen(dSock,10);

    if(rListen < 0){
        perror("Error while listen");
        exit(0);
    }

    printf("Server launched !\n");
    
    while(1){
        int socketClient1 = accept(dSock,(struct sockaddr*) &addrClient,&lg);
        if(socketClient1 < 0) {
		    perror("Error while accepting first connection");
		    exit(0);
	    }

        send(socketClient1,&ID_CLIENT_1,sizeof(int),0);

        int socketClient2 = accept(dSock,(struct sockaddr*) &addrClient,&lg);
        if(socketClient2 < 0) {
		    perror("Error while accepting second connection");
		    exit(0);
	    }

        send(socketClient2,&ID_CLIENT_2,sizeof(int),0);

        while(1){
            recv(socketClient1,&msg,sizeof(msg),0);
            send(socketClient2,&msg,strlen(msg)+1,0);
            if(strcmp(msg,"end\n") == 0){
                break;
            }
            recv(socketClient2,&msg,sizeof(msg),0);
            send(socketClient1,&msg,strlen(msg)+1,0);
            if(strcmp(msg,"end\n") == 0){
                break;
            }
        }
        close(socketClient1);
        close(socketClient2);
        
    } 
    
    close(dSock);
    return 0;
}
