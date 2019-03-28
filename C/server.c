#include "socket_library.h"

static struct sockaddr_in udp_server_address;
char *address = "226.1.2.3";
long port = 8888;
struct Client client_array[MAX_CLIENTS];
int client_nb = 0, udp_socket;
pthread_t still_alive_thread, udp_handle_thread, write_thread;

void *udp_handle();
void *still_alive_sender();
void *write_message();
void close_server(int);

void *write_message() {
    char entry[150];
    char pseudo[50];
    fd_set readfds;
    int nb_octets, read_octets, i;
    struct timeval tv;
    int fd_stdin;
    fd_stdin = fileno(stdin);
    struct udp_requete requete;

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);

        tv.tv_sec = 0;
        tv.tv_usec = 50000;
        nb_octets = select(fd_stdin+1, &readfds, NULL, NULL, &tv);

        if(nb_octets > 0) {
            read_octets = read(fd_stdin, entry, 1024);
            if(read_octets > 0 && read_octets < 150) {
                if(entry[0] == '/' && entry[1] == 'b' && entry[2] == 'a' && entry[3] == 'n') {
                    printf("Pseudo à bannir : ");
                    fgets(pseudo, sizeof(pseudo), stdin);
                    strtok(pseudo, "\n");
                    for(i = 0; i < MAX_CLIENTS; i++) {
                        if(strcmp(pseudo, client_array[i].pseudo) == 0) {
                            requete.requete_type = BAN_CLIENT;
                            requete.sender = client_array[i];
                            sendto(udp_socket, &requete, sizeof(struct udp_requete), 0, (struct sockaddr *)&udp_server_address, sizeof(struct sockaddr_in));
                            client_array[i].banned = 1;
                        }
                    }
                } else if (entry[0] == '/' && entry[1] == 'u' && entry[2] == 'n' && entry[3] == 'b' && entry[4] == 'a' && entry[5] == 'n'){
                    printf("Pseudo à débannir : ");
                    fgets(pseudo, sizeof(pseudo), stdin);
                    strtok(pseudo, "\n");
                    for(i = 0; i < MAX_CLIENTS; i++) {
                        if(strcmp(pseudo, client_array[i].pseudo) == 0) {
                            requete.requete_type = UNBAN_CLIENT;
                            requete.sender = client_array[i];
                            sendto(udp_socket, &requete, sizeof(struct udp_requete), 0, (struct sockaddr *)&udp_server_address, sizeof(struct sockaddr_in));
                            client_array[i].banned = 0;
                        }
                    }
                } else if(entry[0] == '/' && entry[1] == 'c' && entry[2] == 'l' && entry[3] == 'i' && entry[4] == 'e' && entry[5] == 'n' && entry[6] == 't' && entry[7] == 's') {
                    printf("Liste des clients : \n");
                    for(i = 0; i < MAX_CLIENTS; i++) {
                        if(client_array[i].pseudo[0] != 0) {
                            printf("\t- %s connecté sur %s:%d\n", client_array[i].pseudo, client_array[i].address, client_array[i].port);
                        } else {
                            printf("\t- non connecté\n");
                        }
                    }
                }
            }
            pseudo[0] = 0;
            entry[0] = 0;
        }
    }
}

void close_server(int id) {
    struct udp_requete requete;
    requete.requete_type = SERVER_CLOSED;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    if(id != 1) {
        sendto(udp_socket, &requete, sizeof(struct udp_requete), 0, (struct sockaddr *) &udp_server_address, sizeof(struct sockaddr_in));
    }
    close(udp_socket);
    exit(EXIT_SUCCESS);
}

void *still_alive_sender() {
    struct udp_requete requete;
    requete.requete_type = STILL_ALIVE;
    int nb_octets;

    while(1) {
        if((nb_octets = sendto(udp_socket, &requete, sizeof(struct udp_requete), 0, (struct sockaddr *) &udp_server_address, sizeof(struct sockaddr_in))) < 0) {

        }
        sleep(5);
    }

}

void *udp_handle() {
    struct udp_requete response;
    int nb_octets, i, position_new;
    struct sockaddr_in client;
    int client_addr_size = sizeof(client);

    struct Client client_buf;

    bzero(&udp_server_address, sizeof(struct sockaddr_in));
	udp_server_address.sin_family = AF_INET;
    udp_server_address.sin_port = htons(port);
    udp_server_address.sin_addr.s_addr = inet_addr(address);
    while(1) {
        struct udp_requete *requete = malloc(sizeof(struct udp_requete));
        nb_octets = recvfrom(udp_socket, requete, sizeof(struct udp_requete), MSG_DONTWAIT, (struct sockaddr *)&client, &client_addr_size);
        if(nb_octets > 0) {
            int nb, nb_octets, taille, attendu, index, index_new_client;
            printf("requeste received \n");
            int requete_type = requete->requete_type;
            printf("requete type : %d\n", requete_type);
            switch(requete_type) {
                case ADD_CLIENT:
                    client_nb = client_nb +1;
                    if(client_nb > MAX_CLIENTS) {
                        response.requete_type = TOO_MUCH_CLIENT;
                        response.sender = requete->sender;
                        client_nb = client_nb -1;
                        sendto(udp_socket, &response, sizeof(struct udp_requete), 0, (struct sockaddr *) &udp_server_address, client_addr_size);
                    } else {
                        index_new_client = find_user(client_array, requete->sender);
                        if(index_new_client == -1) {
                            for(i = 0; i < MAX_CLIENTS; i++) {
                                if(client_array[i].pseudo[0] == 0) {
                                    strcpy(client_array[i].pseudo, requete->sender.pseudo);
                                    strcpy(client_array[i].address, requete->sender.address);
                                    client_array[i].port = requete->sender.port;
                                    client_array[i].banned = requete->sender.banned;
                                    free(requete);
                                    position_new = i;
                                    break;
                                }
                            }
                            printf("Nouveau client \"%s\" (banned : %d) en attente de connexion tcp sur : %s:%d.\n", client_array[position_new].pseudo, client_array[position_new].banned,client_array[position_new].address, client_array[position_new].port);

                            response.requete_type = REFRESH_TABLE;
                            for(i = 0; i < MAX_CLIENTS; i++) {
                                response.tab[i] = client_array[i];
                            }
                            if(sendto(udp_socket, &response, sizeof(struct udp_requete), 0, (struct sockaddr *)&udp_server_address, client_addr_size) < 0) {
                                perror("sendto");
                            }
                        } else {
                            response.requete_type = PSEUDO_ALREADY_TAKEN;
                            response.sender = requete->sender;
                            client_nb -= 1;
                            sendto(udp_socket, &response, sizeof(struct udp_requete), 0, (struct sockaddr *) &udp_server_address, client_addr_size);
                        }

                    }
                break;
                case REMOVE_CLIENT:
                    for(i = 0; i < MAX_CLIENTS; i++) {
                        if(strcmp(requete->sender.pseudo, client_array[i].pseudo) == 0) {
                            printf("%s disconnected\n", client_array[i].pseudo);
                            client_array[i].pseudo[0] = 0;
                            client_array[i].address[0] = 0;
                            client_array[i].port = 0;
                            client_array[i].banned = 0;
                        }
                    }

                    response.requete_type = REFRESH_TABLE;
                    for(i = 0; i < MAX_CLIENTS; i++) {
                        response.tab[i] = client_array[i];
                    }
                    if(sendto(udp_socket, &response, sizeof(struct udp_requete), 0, (struct sockaddr *) &udp_server_address, client_addr_size) < 0) {
                        perror("sendto");
                    }
                break;

            }
        } else {
            free(requete);
        }
        usleep(50000);
    }
}



int main(int argc, char **argv) {
    int opt = 0;
    char *optstring = "hs:p:";
    static struct option long_opts[] = {
        {"help",                    no_argument,            0,          'h'},
        {"server-address",          required_argument,      0,          's'},
        {"port",                    required_argument,      0,          'p'}
    };
    int long_index = 0;
    char *endptr = NULL;

    int i;


    opterr = 0;
    while ((opt = getopt_long(argc, argv, optstring, long_opts, &long_index)) != -1) {
        switch (opt) {
            case 'h':
            printf("Help : \n");
                exit(EXIT_SUCCESS);
            break;
            case 's':
                if (optarg != NULL) {
                    address = optarg;
                }
            break;
            case 'p':
                if(optarg != NULL) {
                    port = strtol(optarg, &endptr, 10);
                }
            break;
            default:
                fprintf(stderr, "%s:%s:%d: getopt_long function has failed : unknown option -- '%c'.\n", argv[0], __FILE__, __LINE__, optopt);
                exit(EXIT_FAILURE);
        }
    }

    for(i = 0; i < MAX_CLIENTS; i++) {
        client_array[i].pseudo[0] = 0;
        client_array[i].address[0] = 0;
        client_array[i].port = 0;
        client_array[i].banned = 0;
    }

	udp_socket = create_udp_socket(address, port);
    if(udp_socket > 0) {
        printf("Server launched on %s:%ld ...\n", address, port);
    } else {
        printf("Unable to launch the server for %s address and %ld port...\nTry with options -s and/or -p\nAddress must be between 225.0.0.0 and 239.0.0.0 and port between 1024 and 52164\n", address, port);
        exit(EXIT_FAILURE);
    }


    pthread_create(&udp_handle_thread, NULL, udp_handle, NULL);
    pthread_create(&still_alive_thread, NULL, still_alive_sender, NULL);
    pthread_create(&write_thread, NULL, write_message, NULL);

    signal(SIGINT, close_server);

    while(1) {
        sleep(5);
    }

    close(udp_socket);
	return 0;
}
