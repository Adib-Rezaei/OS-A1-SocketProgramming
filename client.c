#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>

#define CLIENT_NUM 3
#define TALK_NUM 3

int connect_server(int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }

    return fd;
}

void broadcast_message(int sock, char buff[], struct sockaddr_in* bc_address){
    memset(buff, 0, 1024);
    read(0, buff, 1024);
    int a = sendto(sock, buff, strlen(buff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));        
    recv(sock, buff, 1024, 0);
}

int can_client_talk(int client_id, int message_count, int schedule[3][3]){
    if(message_count == schedule[client_id][0]
        || message_count == schedule[client_id][1]
        || message_count == schedule[client_id][2]){
            return 1;
        }
    return 0;
}

void alarm_handler(int sig){
    printf("You ran out of time\n");
}

int main(int argc, char const *argv[]) {
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    int server_fd;
    char buff[1024] = {0};
    char temp[1024] = {0};
    char choice[1024] = {0};

    server_fd = connect_server(atoi(argv[1]));
    
    /* recv category list */
    recv(server_fd, buff, 1024, 0);    
    printf("%s\n", buff);
    memset(buff, 0, 1024);

    /* get room choice from stdin */
    read(0, choice, 1024);        
    send(server_fd, choice, 1024, 0);
    choice[strlen(choice)-1] = '\0';
    /* recv room port number */
    recv(server_fd, buff, 1024, 0);    
    int room_port = atoi(buff); 
    memset(buff, 0, 1024);

    /* recv client id corresponding to room */
    recv(server_fd, buff, 1024, 0);    
    int client_id = atoi(buff);
    memset(buff, 0, 1024);
    printf("client id = %d\n", client_id);

    /* connect broadcast */
    int sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(room_port); 
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255"); 

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));
  



    int schedule[CLIENT_NUM][TALK_NUM] = {
        {1,5,8},
        {2,4,9},
        {3,6,7}
    };

    int choose_best_answer_time[3] = {3, 6, 9};

    memset(buff, 0, 1024);

    char log[1024];
    for(int message_count = 1 ; message_count <= 9 ; message_count++)
    {
        
        printf("message count = %d\n", message_count);
        
        if(message_count == schedule[client_id][client_id])
        {
            printf("Ask your question: \n");
            memset(buff, 0, 1024);
            read(0, buff, 1024);
            sprintf(log, "Category: %s - Client id: %d - Question: %s", choice, client_id, buff);
            int a = sendto(sock, buff, strlen(buff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));        
            recv(sock, buff, 1024, 0);
        }
        else if (can_client_talk(client_id, message_count, schedule)){
            printf("Answer: \n");
            memset(buff, 0, 1024);
            alarm(60);
            int n = read(0, buff, 1024);
            alarm(0);
            if(n < 0){
                strcpy(buff, "pass");
            }
            int a = sendto(sock, buff, strlen(buff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));        
            recv(sock, buff, 1024, 0);
        }
        else
        {
            printf("listening... \n");
            memset(buff, 0, 1024);
            recv(sock, buff, 1024, 0);
            if(message_count > choose_best_answer_time[client_id] - 3
            && message_count <= choose_best_answer_time[client_id]){
                int client_id_ans = -1;
                for(int i=0; i< 3; i++){
                    for(int j=0; j< 3; j++){
                        if(schedule[i][j] == message_count){
                            client_id_ans = i;
                            break;
                        }
                    }
                }
                sprintf(temp, "Category: %s - Client id: %d - Answer: %s", choice, client_id_ans, buff);
                strcat(log, temp);
            }
            printf("%s\n", buff);
        }

        if(message_count == choose_best_answer_time[client_id]){
            memset(buff, 0, 1024);
            printf("choose the best answer[1-2]: \n");
            read(0, buff, 1024);

            sprintf(temp, "The best answer is number %s", buff);
            strcat(log, temp);
            send(server_fd, log, 1024, 0);
            memset(log, 0, 1024);
            int a = sendto(sock, buff, strlen(buff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));        
            recv(sock, buff, 1024, 0);
        }
        else {
            for(int i=0; i< 3; i++){
                if(choose_best_answer_time[i] == message_count){
                    recv(sock, buff, 1024, 0);
                    printf("%s\n", buff);
                    break;
                }
            }
        }
    }
}