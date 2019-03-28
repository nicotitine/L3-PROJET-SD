#include "socket_library.h"


// Liste des variables globales
// !!! A DEPLACER DANS CLIENT.H
int main_tcp_socket, udp_socket;
static struct sockaddr_in this_address;
static struct sockaddr_in udp_server_address;
struct Client this_client;
struct Client other_clients[MAX_CLIENTS];
int tcp_sockets[MAX_CLIENTS];
int tcp_rcv[MAX_CLIENTS];
int my_socket_index;
static clock_t _current_time = 0;
pthread_t tcp_listen_thread, udp_listen_thread, chat_thread, write_thread, timer_thread;
int still_alive_timeout = 10;
char *address = "226.1.2.3";
long port = 8888;
char *interface = "eth0";

// Liste des signatures des fonctions :
// !!! A DEPLACER DANS CLIENT.H !!!
void *listen_new_tcp_connection();
int create_new_tcp_socket(int);
int join_chat();
void *listen_chat();
void *handle_udp();
void close_chat(int);
void *write_message();
void *start_timer(void *);



// Pas utilisé pour le moment
// L'idée était de quitter le chat si le client ne recoit pas de requete udp STILL_ALIVE pendant 20 sec
// (le serveur les envoie toutes les 5 secondes)
void *start_timer(void *seconds) {
    // int *sec = (int *)seconds;
    // _current_time = clock() + (*sec) * CLOCKS_PER_SEC;
    // while(clock() < _current_time){
    //
    //
    // }

    //printf("Server is not responding\n");
    //close_chat(2);
}

int main(int argc , char *argv[]) {
    // Partie permettant de récupérer les arguments de lancement (vu en TP de SE)
    // Permet de récupérer  :
    //      - l'interface réseau sur laquelle on veut lancer le client (-i)
    //      - l'addresse ipv4 du serveur (-s)
    //      - le port d'écoute du serveur (-p)
    int opt = 0;
    char *optstring = "hs:p:i:";
    static struct option long_opts[] = {
        {"help",                    no_argument,            0,          'h'},
        {"server-address",          required_argument,      0,          's'},
        {"port",                    required_argument,      0,          'p'},
        {"interface",               required_argument,      0,          'i'}
    };
    int long_index = 0;
    char *endptr = NULL;

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
            case 'i':
                if(optarg != NULL) {
                    interface = optarg;
                }
            break;
            default:
                fprintf(stderr, "%s:%s:%d: getopt_long function has failed : unknown option -- '%c'.\n", argv[0], __FILE__, __LINE__, optopt);
                exit(EXIT_FAILURE);
        }
    }
    // Initialisation des variables globales
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        tcp_sockets[i] = 0;
        tcp_rcv[i] = 0;
        other_clients[i].pseudo[0] = 0;
        other_clients[i].address[0] = 0;
        other_clients[i].port = 0;
        other_clients[i].banned = 0;
    }


    // Ignore le signal SIGPIPE
    signal (SIGPIPE, SIG_IGN);
    // Appel la fonction close_chat en cas de ctrl+c
    signal(SIGINT, close_chat);

    // printf("sizeof requete_t : %ld\n", sizeof(enum udp_requete_t));
    // printf("sizeof taille_donnees : %ld\n",sizeof(int));
    // printf("sizeof sender : %ld\n", sizeof(struct Client));
    // printf("\tsizeof pseudo : %ld\n", sizeof(char[50]));
    // printf("\tsizeof address : %ld\n", sizeof(char[15]));
    // printf("\tsizeof port : %ld\n", sizeof(int));
    // printf("\tsizeof banned : %ld\n", sizeof(int));
    // printf("\tsizeof struct requete : %ld\n", sizeof(struct udp_requete));

    // Lancement du processus d'écoute tcp principal (permet la connexion tcp entre clients)
    // Si erreur, quitte le programme
    pthread_create(&tcp_listen_thread, NULL, listen_new_tcp_connection, NULL);
    pthread_join(tcp_listen_thread, NULL);

    // Lancement du thread d'écoute udp et le thread d'écoute tcp
    pthread_create(&udp_listen_thread, NULL, handle_udp, NULL);
    pthread_create(&chat_thread, NULL, listen_chat, NULL);

    // Demande le pseudo de l'utilisateur
	printf("Veuillez entez votre pseudo : ");
    fgets(this_client.pseudo, sizeof(this_client.pseudo), stdin);
    strtok(this_client.pseudo, "\n");
    fflush(stdin);


    // Rejoint le chat
    join_chat();

    //pthread_create(&timer_thread, NULL, start_timer, &still_alive_timeout);

    // Lance le thread qui permet de lire les entrées clavier
    pthread_create(&write_thread, NULL, write_message, NULL);

    // Attend la fin de l'execution des thread pour quitter le programme
    pthread_join(udp_listen_thread, NULL);
    pthread_join(chat_thread, NULL);
    pthread_join(write_thread, NULL);
    return 0;
}

void *write_message() {
    char entry[150], message[250];
    fd_set readfds;
    int nb_octets, read_octets, i, fd_stdin;
    struct timeval tv;
    fd_stdin = fileno(stdin);

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);

        // Vérifie toutes les 50 ms si une entrée est disponible sur stdin
        tv.tv_sec = 0;
        tv.tv_usec = 50000;
        nb_octets = select(fd_stdin+1, &readfds, NULL, NULL, &tv);

        // Si une entrée est disponible
        if(nb_octets > 0) {
            // Lis l'entrée
            read_octets = read(fd_stdin, entry, 1024);
            if(read_octets > 0 && read_octets < 150) {
                // Compose le message tcp a envoyé de type "pseudo : message"
                strcat(message, this_client.pseudo);
                strcat(message, " : ");
                strcat(message, entry);
                strtok(message, "\n");

                // Supprime l'entrée clavier pour afficher un message plus joli
                printf("\033[1A");

                // Vérifie que le client actuel n'est pas banni
                if(this_client.banned == 1) {
                    printf("Tu es banni, tu ne peux pas parler pour le moment...\n");
                } else {
                    printf("\r%s\n", message);
                    // Envoi le message à tous les clients, sauf moi même
                    for(i = 0; i < MAX_CLIENTS; i++) {
                        if(i != my_socket_index && tcp_sockets[i] != 0) {
                            send(tcp_sockets[i], &message, sizeof(message), 0);
                        }
                    }
                }
            } else {
                printf("Your %d message size is greater than 150\n", read_octets);
            }
            // Réinitialise les tableaux de caractères
            message[0] = 0;
            entry[0] = 0;
        }
    }
}

void close_chat(int id) {
    // Fonction permettant de fermer toutes les sockets créées ainsi que tous les threads lancés
    struct udp_requete requete;
    requete.requete_type = REMOVE_CLIENT;
    requete.sender = this_client;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    if(id != 1) {
        sendto(udp_socket, &requete, sizeof(struct udp_requete), 0, (struct sockaddr *) &udp_server_address, sizeof(struct sockaddr_in));
    }
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(tcp_sockets[i] != 0) {
            close(tcp_sockets[i]);
        }
        if(tcp_rcv[i] != 0) {
            close(tcp_rcv[i]);
        }
    }
    close(main_tcp_socket);
    close(udp_socket);
    pthread_kill(tcp_listen_thread, 0);
    pthread_kill(udp_listen_thread, 0);
    pthread_kill(chat_thread, 0);
    exit(EXIT_SUCCESS);
}

int create_new_tcp_socket(int index) {
    // Créé une socket tcp avec le client référencé dans le tableau other_clients à la position index
    int sock;
    static struct sockaddr_in new_client_address;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero((char *) &new_client_address, sizeof(struct sockaddr_in));
    new_client_address.sin_family = AF_INET;
    new_client_address.sin_port = htons(other_clients[index].port);
    new_client_address.sin_addr.s_addr = inet_addr(other_clients[index].address);
    connect(sock, (struct sockaddr *) &new_client_address, sizeof(struct sockaddr_in));

    return sock;
}

void *handle_udp() {
    // Fonction qui gère tous les types de requetes udp
    // Le client peut traiter les requetes REFRESH_TABLE, TOO_MUCH_CLIENT, STILL_ALIVE, SERVER_CLOSED, BAN_CLIENT, UNBAN_CLIENT, PSEUDO_ALREADY_TAKEN
    // REFRESH_TABLE :
    //      Reçoit la nouvelle table des clients qui remplace celle en local
    //      Si un nouveau client est détecté, le client actuel tente de se connecter en tcp avec le nouveau
    //      Si un client quitte le chat, il met a jour sa table des clients
    //          Dans ce cas, la fermeture des sockets liées à la connexion des deux clients se fait dans la fonction listen_chat
    // TOO_MUCH_CLIENT :
    //      Si un client demande de se connecter et qu'il y a déjà MAX_CLIENTS de connectés, le serveur met la structure du client demandeur dans
    //      requete.sender pour que les autres clients ne soient pas affectés. Le client demandeur se ferme en affichant pourquoi.
    // STILL_ALIVE :
    //      Ne marche pas pour le moment
    // SERVER_CLOSED :
    //      A la fermeture du serveur (maintenance, crash, ...), il envoie une requete vide de type SERVER_CLOSED pour prévenir tous les clients.
    //      Tous les clients se ferment alors en affichant le message adéquat
    // BAN_CLIENT :
    //      Le serveur envoie une requete de type BAN_CLIENT avec le client a bannir dans requete.sender. Tous les clients vérifient qu'ils ne correspondent pas à requete.sender
    //      Si le client est égal à requete.sender, il met à jour la structure this_client pour mettre banned à 1.
    //      Dans ce cas, l'envoie de message tcp est annulé.
    // UNBAN_CLIENT :
    //      Même fonctionnement que BAN_CLIENT dans le sens inverse
    // PSEUDO_ALREADY_TAKEN :
    //      Quand un nouveau client demande de rejoindre le chat, il envoie son pseudo. Le serveur vérifie dans sa table de clients que ce pseudo n'est pas déjà pris. Dans le cas contraire,
    //      le serveur renvoie une requete de type PSEUDO_ALREADY_TAKEN. Le client quite en affichant le message adéquat.
    int nb_octets, i;
    while (1) {
        struct udp_requete *requete = malloc(sizeof(struct udp_requete));
        if ((nb_octets = recv(udp_socket, requete, sizeof(struct udp_requete), 0)) > 0) {

            int requete_type = requete->requete_type;
            static struct sockaddr_in other_client;

            switch(requete_type) {
                case REFRESH_TABLE:
                    for(i = 0; i < MAX_CLIENTS; i++) {
                        if((requete->tab[i].pseudo[0] != 0) && (other_clients[i].pseudo[0] == 0)) {
                            if(strcmp(requete->tab[i].pseudo, this_client.pseudo) == 0) {
                                my_socket_index = i;
                            }
                            other_clients[i] = requete->tab[i];
                            tcp_sockets[i] = create_new_tcp_socket(i);
                            printf("%s a rejoint le chat !\n", other_clients[i].pseudo);
                        }

                        if(requete->tab[i].pseudo[0] == 0 && other_clients[i].pseudo[0] != 0) {
                            printf("%s a quitté le chat\n", other_clients[i].pseudo);
                            other_clients[i].pseudo[0] = 0;
                            other_clients[i].address[0] = 0;
                            other_clients[i].port = 0;
                            other_clients[i].banned = 0;
                        }
                    }
                break;
                case TOO_MUCH_CLIENT:
                    if(strcmp(requete->sender.pseudo, this_client.pseudo) == 0) {
                        printf("Il y a trop de clients connectés ... réessaye plus tard\n");
                        close_chat(1);
                    }
                break;
                case STILL_ALIVE:
                    //  printf("Server is still alive ! wouaw !!!\n");
                    // if(pthread_kill(timer_thread, 0) != 0) {
                    //     //perror("unale to kill timer_thread");
                    // }
                    // pthread_create(&timer_thread, NULL, start_timer, &still_alive_timeout);
                break;
                case SERVER_CLOSED:
                    printf("Le serveur a été déconnecté ...\n");
                    close_chat(0);
                break;
                case BAN_CLIENT:
                    for(i = 0; i < MAX_CLIENTS; i++) {
                        if(strcmp(requete->sender.pseudo, other_clients[i].pseudo) == 0) {
                            other_clients[i].banned = 1;
                        }
                    }
                    if(strcmp(requete->sender.pseudo, this_client.pseudo) == 0) {
                        printf("Tu as été banni ...\n");
                        this_client.banned = 1;
                    }
                break;
                case UNBAN_CLIENT:
                    for(i = 0; i < MAX_CLIENTS; i++) {
                        if(strcmp(requete->sender.pseudo, other_clients[i].pseudo) == 0) {
                            other_clients[i].banned = 0;
                        }
                    }
                    if(strcmp(requete->sender.pseudo, this_client.pseudo) == 0) {
                        printf("Tu n'est plus banni !\n");
                        this_client.banned = 0;
                    }
                break;
                case PSEUDO_ALREADY_TAKEN:
                    if(strcmp(requete->sender.pseudo, this_client.pseudo) == 0 && strcmp(requete->sender.address, this_client.address) == 0 && requete->sender.port == this_client.port) {
                        printf("Le pseudo \"%s\" est déjà pris...\n", this_client.pseudo);
                        close_chat(1);
                    }
                break;
            }
            int j;
        }
        free(requete);
        usleep(50000);
    }
}

void *listen_new_tcp_connection() {
    // Fonction d'écoute de nouvelle connexion tcp. Permet d'accepter les demandes de connexion des autres clients
    int opt = 1;

    if( (main_tcp_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if( setsockopt(main_tcp_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    this_address.sin_family = AF_INET;
    this_address.sin_addr.s_addr = inet_addr(get_ipv4_address(main_tcp_socket, interface));
    this_address.sin_port = 0;

    if (bind(main_tcp_socket, (struct sockaddr *)&this_address, sizeof(this_address))<0) {
        printf("Unable to launch the tcp listener ... try to change the launching option -i (\"eth0\" by default.).\nRefeer to ifconfig command to see which interface you are connected with...\n");
        exit(EXIT_FAILURE);
    }

    if (listen(main_tcp_socket, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int join_chat() {
    // Fonction permettant de rejoindre le chat en se connectant au serveur
    // et en lui envoyant une requete de type ADD_CLIENT
    struct udp_requete requete;
    struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
    int nb_octets, i;

	getsockname(main_tcp_socket, (struct sockaddr *) &sin, &len);
	//printf("TCP listening on  %s:%d\n", inet_ntoa(this_address.sin_addr), ntohs(sin.sin_port));
    strncpy(this_client.address, get_ipv4_address(main_tcp_socket, interface), sizeof(this_client.address));
	this_client.port = ntohs(sin.sin_port);
	this_client.banned = 0;

	requete.requete_type = ADD_CLIENT;
	requete.sender = this_client;
	for(i = 0; i < MAX_CLIENTS; i++) {
		requete.tab[i] = other_clients[i];
	}

    int udp_server_address_size = sizeof(struct sockaddr_in);

	udp_socket = create_udp_socket(address, port);

	bzero(&udp_server_address, sizeof(struct sockaddr_in));
	udp_server_address.sin_family = AF_INET;
    udp_server_address.sin_port = htons(port);
    udp_server_address.sin_addr.s_addr = inet_addr(address);

	nb_octets = sendto(udp_socket, &requete, (sizeof(requete)), 0, (struct sockaddr *) &udp_server_address, sizeof(struct sockaddr_in));
    return 0;
}

void *listen_chat() {
	fd_set readfds;
	int max_sd, nb_octets, i, activity, new_socket;
	struct timeval tv;
	char buffer[1025];
  	socklen_t addrlen = sizeof(struct sockaddr_in);

    tv.tv_sec = 0;
	tv.tv_usec = 50000;

	while (1) {
        FD_ZERO(&readfds);
        FD_SET(main_tcp_socket, &readfds);
        max_sd = main_tcp_socket;

        for (i = 0; i < MAX_CLIENTS; i++) {
            if( tcp_rcv[i] > 0) {
                FD_SET(tcp_rcv[i], &readfds);
            }
            if(tcp_rcv[i] > max_sd) {
                max_sd = tcp_rcv[i];
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
        if ((activity < 0) && (errno!=EINTR)) {
            printf("select error");
        }

        if (FD_ISSET(main_tcp_socket, &readfds)) {
            if ((new_socket = accept(main_tcp_socket, (struct sockaddr *) &this_address, &addrlen))<0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            for (i = 0; i < MAX_CLIENTS; i++) {
                if( tcp_rcv[i] == 0 ) {
                    tcp_rcv[i] = new_socket;
                    break;
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
        	if (FD_ISSET(tcp_rcv[i], &readfds)) {
                if ((nb_octets = read(tcp_rcv[i], buffer, 1024)) == 0) {
                   close(tcp_rcv[i]);
                   close(tcp_sockets[i]);
                   tcp_rcv[i] = 0;
                   tcp_sockets[i] = 0;
               } else {
                   printf("%c[2K\r", 27);
                   fflush(stdout);
                   fflush(stdin);
                   printf("%s\n", buffer);
                   fflush(stdout);
                   fflush(stdin);
               }
            }
        }
        usleep(5000);
    }
}
