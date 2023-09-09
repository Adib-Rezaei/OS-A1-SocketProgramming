#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <ctype.h>


int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    if(listen(server_fd, 10) < 0){
        printf("an error occured in listening on server (num of connections maybe)...");
    }

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}

void create_room(int *waiting_room, int room_port){
    char room_port_str[1024];
    sprintf(room_port_str, "%d", room_port);
    for(int i=0; i< sizeof(waiting_room); i++){
        char client_id[1024];
        sprintf(client_id, "%d", i);
        send(waiting_room[i], room_port_str, 1024, 0);
        send(waiting_room[i], client_id, 1024, 0);
    }
}


int main(int argc, char const *argv[]) {
    setbuf(stdout, NULL);
    int room_port = atoi(argv[1]);
    int server_fd, new_socket, max_sd;
    char buffer[1024] = {0};
    fd_set master_set, working_set;
       
    server_fd = setupServer(8080);

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server is running\n", 18);

    int computer_waiting_room[3] = {0};
    int computer_queue_length = 0;

    int mechanic_waiting_room[3] = {0};
    int mechanic_queue_length = 0;

    int civil_waiting_room[3] = {0};
    int civil_queue_length = 0;

    int electronic_waiting_room[3] = {0};
    int electronic_queue_length = 0;

    char QA_results[2048] = {0};

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int current_fd = 0; current_fd <= max_sd; current_fd++) {
            memset(buffer, 0, 1024);

            if (FD_ISSET(current_fd, &working_set)) {
                
                if (current_fd == server_fd) {      // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. fd = %d\n", new_socket);
                    char intro[] = "choose from following categories\n1.CE\n2.EE\n3.CVE\n4.ME\n";
                    send(new_socket, intro, strlen(intro), 0);
                }
                
                else {                              // client sending msg
                    int bytes_received;
                    bytes_received = recv(current_fd , buffer, 1024, 0);
                    
                    if (bytes_received == 0) {      // EOF
                        printf("client fd = %d closed\n", current_fd);
                        close(current_fd);
                        FD_CLR(current_fd, &master_set);
                        continue;
                    }
                    
                    if(!isdigit(buffer[0])){
                        strcat(QA_results, buffer);
                        printf("%s", buffer);
                        printf("\n");
                        continue;  
                    }

                    int category = atoi(buffer);
                    switch (category)
                    {
                    case 1:
                        computer_waiting_room[computer_queue_length++] = current_fd;
                        if(computer_queue_length == 3){
                            create_room(computer_waiting_room, room_port);
                            memset(computer_waiting_room, 0, sizeof(computer_waiting_room));
                            room_port++;
                        }
                        break;

                    case 2:
                        electronic_waiting_room[electronic_queue_length++] = current_fd;
                        if(electronic_queue_length == 3){
                            create_room(electronic_waiting_room, room_port);
                            memset(electronic_waiting_room, 0, sizeof(electronic_waiting_room));
                            room_port++;
                        }
                        break;

                    case 3:
                        civil_waiting_room[civil_queue_length++] = current_fd;
                        if(civil_queue_length == 3){
                            create_room(civil_waiting_room, room_port);
                            memset(civil_waiting_room, 0, sizeof(computer_waiting_room));
                            room_port++;
                        }
                        break;

                    case 4:
                        mechanic_waiting_room[mechanic_queue_length++] = current_fd;
                        if(mechanic_queue_length == 3){
                            create_room(mechanic_waiting_room, room_port);
                            memset(mechanic_waiting_room, 0, sizeof(computer_waiting_room));
                            room_port++;
                        }
                        break;

                    default:
                        printf("Invalid category");
                        break;
                    }
                    
                }
            }
        }

    }

    return 0;
}