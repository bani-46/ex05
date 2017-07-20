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

#define BUFSIZE 488
#define USER_LENGTH 15

/*Header*/
enum{
	HELO=1,
	HERE,
	JOIN,
	POST,
	MESG,
	QUIT
};
/*Send Target*/
enum{
	ALL=1,
	EACH
};

/*Structures*/
typedef struct{
	char header[4];
	char separator;
	char msg[];
}packet;

typedef struct Client_Info{
	int  sock;
	char name[USER_LENGTH];
	struct Client_Info *next;
}client_info;

typedef struct{
	client_info *top;
}client_list;
client_list *head;

/*main func*/
void free_chat_server(int _port_number,char username[]);
void free_chat_client(int _port_number,char username[],
							struct sockaddr_in *from_adrs);
/*server func*/
void udp_monitor(int _sock_udp_listen,char _username[],int _sock_listen);
void tcp_monitor(int _sock_listen);
void * echo_thread(void *arg);

/*wrapper func*/
int accept_client(int s, struct sockaddr *addr, socklen_t *addrlen);
int send_message(int s, void *buf, size_t len, int flags);
int recv_message(int s, void *buf, size_t len, int flags);

/*msg managers*/
char *create_packet(int type,char *message);
int analyze_packet(char *_header);
void msg_processor(char *_r_buf,int _sock);
char * get_cname(int _sock);
void send_MESG(char *_buf,int _sock,int mode);
char * format_MESG(char *name,char *msg);

/*list func*/
client_info *add_client(int _sock,char _name[],client_info *ci);
void make_list(void);
void insert_info(client_list *head,int sock,char name[]);
void delete_info(int _sock);
void show_list();
int is_rest_info(int _sock);

#endif /* FREE_CHAT_H_ */
