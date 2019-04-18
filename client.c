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


int main(int argc, char const *argv[]) {
	if(argc != 3) {
		perror("invalid argument number : \n1 : Ip\n2 : Port");
		return -1;
	}

	char* ip = argv[1];
	char* port = argv[2];
    
    struct sockaddr_in addrServ;

    addrServ.sin_family=AF_INET;
    inet_pton(AF_INET, ip, &(addrServ.sin_addr));
    addrServ.sin_port = htons((short) atoi(port));
    socklen_t igA = sizeof(struct sockaddr_in);
    
    char msg[200];
    char answ[200];
    int MY_ID;

    int dSock = socket(PF_INET,SOCK_STREAM,0);
    
    int connexion = connect(dSock, (struct sockaddr*)&addrServ, igA);

    if (connexion < 0){
        perror("Error : ");
        exit(0);
    }

    printf("Connected ! \n");
    
    recv(dSock,&MY_ID,sizeof(int),0);
    printf("You are the client %d \n",MY_ID);
    
    while(1){
        if (MY_ID == 1){
            printf("Enter your message: \n");
            fgets(msg,200,stdin); 
            send(dSock,&msg,strlen(msg)+1,0);
            if(strcmp(msg,"end\n") == 0){
                break;
            }
            printf("Waiting for answer: \n");
            recv(dSock,&answ,sizeof(answ),0);
            if(strcmp(answ,"end\n") == 0){
                break;
            }
            printf("Client 2: %s",answ);
        }

        if(MY_ID == 2){
            printf("Waiting for answer: \n");
            recv(dSock,&answ,sizeof(answ),0);
            if(strcmp(answ,"end\n") == 0){
                break;
            }
            printf("Client 1: %s",answ);
            printf("Enter your message: \n");
            fgets(msg,200,stdin);
            send(dSock,&msg,strlen(msg)+1,0);
            if(strcmp(msg,"end\n") == 0){
                break;
            }
        }
    } 
    printf("Disconected ! \n");
    close(dSock);
    return 0;
}
