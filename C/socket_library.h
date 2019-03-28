#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <net/if.h>
#include <sys/signal.h>
#include <getopt.h>

#define MAX_CLIENTS 10

enum udp_requete_t {
    ADD_CLIENT = 1,
    REFRESH_TABLE,
    REMOVE_CLIENT,
    BAN_CLIENT,
    UNBAN_CLIENT,
    TOO_MUCH_CLIENT,
    STILL_ALIVE,
    SERVER_CLOSED,
    PSEUDO_ALREADY_TAKEN
};

struct Client {
    char pseudo[50];
    char address[15];
    int port;
    int banned;
} __attribute__((packed));

struct udp_requete {
    enum udp_requete_t requete_type;
    int taille_donnees;
    struct Client sender;
    struct Client tab[MAX_CLIENTS];
} __attribute__((packed));;


/*
 * Create a multicast UDP socket and return the socket descriptor
 * The first parameter is the address to use
 * The second parameter is the port to use
 */
int create_udp_socket(char *, long);

/*
 * Refresh dest by copying every field from src to dest
 */
 int refresh_client_table(struct Client[], struct Client[]);

 /*
  * Copy every field from src to dest
  */
 int cpy_request_sender(struct Client, struct Client);

 /*
  * Get the IPv4 of the machine. We need to send socket and
  * the interface on which one we want to get the IP
  */
  char * get_ipv4_address(int, char *);

  int find_user(struct Client[], struct Client);

  int insert_client(struct Client[], struct Client, int[]);


int create_client_udp_socket();
