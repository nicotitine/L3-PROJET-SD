
#include "socket_library.h"

int create_udp_socket(char *address, long port) {
    int udp_socket, reuse = 1;
	struct ip_mreq mreq;
	static struct sockaddr_in multicast_address;

	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

	mreq.imr_multiaddr.s_addr = inet_addr(address);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(udp_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) < 0) {
        return -1;
	}

	if (setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (int *)&reuse, sizeof(reuse)) < 0) {
		return -1;
	}

	bzero((char *)&multicast_address, sizeof(multicast_address));
	multicast_address.sin_family = AF_INET;
	multicast_address.sin_addr.s_addr = htonl(INADDR_ANY);
	multicast_address.sin_port = htons(port);

	if(bind(udp_socket, (struct sockaddr *)&multicast_address, sizeof(struct sockaddr_in)) == -1) {
		return -1;
	}

	return udp_socket;
}

int cpy_request_sender(struct Client dest, struct Client src) {
    strcpy(dest.pseudo, src.pseudo);
    strcpy(dest.address, src.address);
    dest.port = src.port;
    dest.banned = src.banned;
}

int refresh_client_table(struct Client dest[MAX_CLIENTS], struct Client src[MAX_CLIENTS]) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        strcpy(dest[i].pseudo, src[i].pseudo);
        strcpy(dest[i].address, src[i].address);
        dest[i].port = src[i].port;
        dest[i].banned = src[i].banned;
    }
    return 0;
}

int find_user(struct Client tab[MAX_CLIENTS], struct Client client) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(strcmp(tab[i].pseudo, client.pseudo) == 0) {
            return i;
        }
    }
    return -1;
}

int insert_client(struct Client tab[MAX_CLIENTS], struct Client client, int tcp_sockets[MAX_CLIENTS]) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(tab[i].pseudo[0] == 0 && tcp_sockets[i] == 0) {
            tab[i] = client;
            //tcp_sockets[i] = sock;
            return i;
        }
    }
    return -1;
}

char * get_ipv4_address(int sock, char *interface) {
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    ioctl(sock, SIOCGIFADDR, &ifr);
    return inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);
}
