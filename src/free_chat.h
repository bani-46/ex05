/*
 * free_chat.h
 *
 *  Created on: 2017/06/19
 *      Author: b5122025
 */
#ifndef FREE_CHAT_H_
#define FREE_CHAT_H_

#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>
#include "mynet.h"

#define BUFSIZE 512
//#define R_BUFSIZE 512
#define NAMELENGTH 15

/*Header*/
enum{
	HELO=1,
	HERE,
	JOIN,
	POST,
	MESG,
	QUIT
};

typedef struct{
	char header[4];
	char separator;
	char msg[];
}packet;

typedef struct Client_Info{
	int  sock;
	char name[NAMELENGTH];
	struct Client_Info *next;
}client_info;

typedef struct{
	client_info *top;
}client_list;


void free_chat_server(int _port_number);
void free_chat_client(int _port_number,char username[],
							struct sockaddr_in *from_adrs);

void udp_monitor(int _sock_udp_listen);
void tcp_monitor(int _sock_listen);
void * echo_thread(void *arg);

int accept_client(int s, struct sockaddr *addr, socklen_t *addrlen);
int send_message(int s, void *buf, size_t len, int flags);
int recv_message(int s, void *buf, size_t len, int flags);

char *create_packet(int type,char *message);
int analyze_packet(char *_header);
void msg_processor(char *_r_buf,int _sock);

client_info *add_client(int _sock,char _name[],client_info *ci);
client_list *make_list(void);


#endif /* FREE_CHAT_H_ */
