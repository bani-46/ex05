/*
 * free_chat_server.c
 *
 *  Created on: 2017/06/19
 *      Author: b5122025
 */
#include "mynet.h"
#include "free_chat.h"

void free_chat_server(int _port_number,char username[]){
	int sock_udp,sock_listen;

	sock_udp = init_udpserver(_port_number);
	sock_listen = init_tcpserver(_port_number,5);
	printf("***Monitoring start.\n");
	make_list();
	while(1){
		udp_monitor(sock_udp,username,sock_listen);
	}
}
