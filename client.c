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
#include <dirent.h>
#include <limits.h>

struct paquet{
    int id;
    int type; //0 message 1 fichier
    char str[1000]; //données
    char nameFile[50];//nom du fichier
};

struct nameFile_socket{
    int socket;
    char nameFile[50];//nom du fichier
};

int get_last_tty() {
  FILE *fp;
  char path[1035];
  fp = popen("/bin/ls /dev/pts", "r");
  if (fp == NULL) {
    printf("can't execute the command\n" );
    exit(1);
  }
  int i = INT_MIN;
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    if(strcmp(path,"ptmx")!=0){
      int tty = atoi(path);
      if(tty > i) i = tty;
    }
  }

  pclose(fp);
  return i;
}

FILE* new_tty() {
  pthread_mutex_t the_mutex;  
  pthread_mutex_init(&the_mutex,0);
  pthread_mutex_lock(&the_mutex);
  system("gnome-terminal"); sleep(1);
  char *tty_name = ttyname(STDIN_FILENO);
  int ltty = get_last_tty();
  char str[2];
  sprintf(str,"%d",ltty);
  int i;
  for(i = strlen(tty_name)-1; i >= 0; i--) {
    if(tty_name[i] == '/') break;
  }
  tty_name[i+1] = '\0';  
  strcat(tty_name,str);  
  FILE *fp = fopen(tty_name,"wb+");
  pthread_mutex_unlock(&the_mutex);
  pthread_mutex_destroy(&the_mutex);
  return fp;
}

void* displayDirectory() {
    FILE* fp1 = new_tty();
    fprintf(fp1,"%s\n","Only for display\n");

    DIR *dp;
    struct dirent *ep;
    dp = opendir ("./transfer");
    if (dp != NULL) {
        fprintf(fp1,"Files\n-------------\n");
        while (ep = readdir(dp)) {
            if(strcmp(ep->d_name,".")!=0 && strcmp(ep->d_name,"..")!=0) 
                fprintf(fp1,"%s\n",ep->d_name);
            }    
            (void)closedir(dp);
        }
    else {
        perror ("Can't open the directory");
    }
}

char* selectFileName() {
  displayDirectory();
  
  FILE *fps = NULL;
  char* fileName = malloc(900);
  char * dirName = "./transfer/";
  char filePath[1023];

  while(fps == NULL) {
    strcpy(filePath, "");

    printf("Select a file : ");
    fgets(fileName,900,stdin);
    fileName[strlen(fileName)-1]='\0';

    strcat(filePath, dirName);
    strcat(filePath, fileName);

    fps = fopen(filePath, "r");
    if (fps == NULL){
        printf("no such file : %s\n",fileName);
    }

  }
  fclose(fps);
  return fileName;
}

//function send file to the server
void* sendFile(void* argument){
    struct nameFile_socket * arg =(struct nameFile_socket*) argument;
    char msg[1000];
    struct paquet p;
    p.type=1;

    strcpy(p.nameFile,arg->nameFile);
    int sock = arg->socket;
    char path[70] = "./transfer/";
    strcat(path, arg->nameFile);

    FILE* file=fopen(path,"r");

    if(file == NULL){
      perror("Error opening file");
      exit(0);
    }

    printf("Uploadind...\n");
    while(fgets(msg, 1000, file) != NULL){
        strcpy(p.str,msg);
        send(sock,&p,sizeof(p),0);;
    }
    printf("Uploaded !\n");
    close(file);
}

//function send message to the server until the client writes end
void* sendToServer(void* socket){
    char msg[1000];
    struct paquet p;
    int sock = (int) socket;
    printf("You can write your messages \n");
    while(1){
        fgets(msg,200,stdin); 

        if(strcmp(msg, "file\n") == 0) {//send file

            //nouvelle fonction pour l'envoi 

            char* fileName = selectFileName();

            struct nameFile_socket arg;
            arg.socket = sock;
            strcpy(arg.nameFile, fileName);

            printf("File selected : %s\n",fileName);

            pthread_t PTh_send_file;
            //creating pthread
            pthread_create(&PTh_send_file,NULL, sendFile,(void*) &arg);
        }
        else {//send message
            p.type=0;
            strcpy(p.str,msg);
            send(sock,&p,sizeof(p),0);
            if(strcmp(msg,"end\n") == 0){
                break;
            }
        }
    }
    close(sock);
    printf("Disconected ! \n");
    exit(0);
}

//function load file from the server
void loadFile(struct paquet p){
    struct paquet f=p;
    char path[70] = "./transfered/";
    strcat(path, f.nameFile);
    FILE* file=fopen(path, "a+");
    fputs(f.str,file);
    fclose(file);
}


//function receive message from the server until the client receives end
void* receiveFromServer(void * socket){
    char answ[1000];
    struct paquet p;
    int sock = (int) socket;

    while(1){
        int rcv = recv(sock,&p,sizeof(p),0);
        if(rcv<0){
            perror("error receive");
        }
        if(p.type==0){
            if(strcmp(p.str,"end\n") == 0){
                break;
            }
            printf("Other client said: %s",p.str);
        }else{
           //load file
            loadFile(p);
        }
    }
    close(sock);
    printf("Disconected ! \n");
    exit(0);
}



int main(int argc, char const *argv[]) {
	if(argc != 3) {
		perror("invalid argument number : \n1 : Ip\n2 : Port");
		return -1;
	}

    //IP address and port number
	char* ip;
    ip = argv[1];
	char* port; 
    port = argv[2];
    
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
    
    //pthread uses for send and receive
    pthread_t PTh_send;
    pthread_t PTh_receive;
    //creating pthread
    pthread_create(&PTh_send,NULL, sendToServer,(void*) dSock);
    pthread_create(&PTh_receive,NULL, receiveFromServer,(void*) dSock);
    //wait until the threads are finished to close the client
    pthread_join(PTh_send, NULL);
    pthread_join(PTh_receive, NULL);

    return 0;
}
