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
#include <pthread.h>



void* sendToServer(void* socket){
    char msg[200];
    int sock = (int) socket;
    printf("You can write your messages \n");
    while(1){
        fgets(msg,200,stdin); 
        send(sock,&msg,strlen(msg)+1,0);
        if(strcmp(msg,"end\n") == 0){
            break;
        }
    }
    printf("Disconected ! \n");
    close(sock);
    exit(0);
}

void* receiveFromServer(void * socket){
    char answ[200];
    int sock = (int) socket;
    while(1){
        recv(sock,&answ,sizeof(answ),0);
        if(strcmp(answ,"end\n") == 0){
            break;
        }
        printf("Other client said: %s",answ);
    }
    printf("Disconected ! \n");
    close(sock);
    exit(0);
}



int main(int argc, char const *argv[]) {
	if(argc != 3) {
		perror("invalid argument number : \n1 : Ip\n2 : Port");
		return -1;
	}

    //IP address and port number
	char* ip = argv[1];
	char* port = argv[2];
    
    //struct uses for server connexion
    struct sockaddr_in addrServ;
    addrServ.sin_family=AF_INET;
    inet_pton(AF_INET, ip, &(addrServ.sin_addr));
    addrServ.sin_port = htons((short) atoi(port));
    socklen_t igA = sizeof(struct sockaddr_in);

    //Client ID
    int MY_ID;

    //Open Connexion
    int dSock = socket(PF_INET,SOCK_STREAM,0);
    int connexion = connect(dSock, (struct sockaddr*)&addrServ, igA);
    if (connexion < 0){
        perror("Connexion Error : ");
        exit(0);
    }

    printf("Connected ! \n");
    
    //ID reception from the server
    recv(dSock,&MY_ID,sizeof(int),0);
    printf("You are the client %d \n",MY_ID);
    
    pthread_t PTh_send;
    pthread_t PTh_receive;

    pthread_create(&PTh_send,NULL, sendToServer,(void*) dSock);
    pthread_create(&PTh_receive,NULL, receiveFromServer,(void*) dSock);
    pthread_join(PTh_send, NULL);
    pthread_join(PTh_receive, NULL);

    return 0;
}
