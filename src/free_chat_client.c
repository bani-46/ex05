/*
 * free_chat_client.c
 *
 *  Created on: 2017/06/19
 *      Author: b5122025
 */
#include "free_chat.h"

void free_chat_client(int _port_number,char username[],struct sockaddr_in *from_adrs){
	int sock,strsize;
	fd_set mask, readfds;
	char ip_adrs[20],s_buf[S_BUFSIZE],r_buf[R_BUFSIZE];

	strncpy(ip_adrs,inet_ntoa(from_adrs->sin_addr),20);
	sock= init_tcpclient(ip_adrs,_port_number);

	printf("[INFO]Connect server.\n");

	/* set bit mask */
	FD_ZERO(&mask);
	FD_SET(0, &mask);
	FD_SET(sock, &mask);

	for(;;){
		/* check recv data */
		readfds = mask;
		select( sock+1, &readfds, NULL, NULL, NULL );

		if( FD_ISSET(0, &readfds) ){
			/* todo */
			fgets(s_buf, S_BUFSIZE, stdin);
			strsize = strlen(s_buf);
			send_message(sock, s_buf, strsize, 0);
		}

		if( FD_ISSET(sock, &readfds) ){
			/* recv message */
			strsize = recv_message(sock, r_buf, R_BUFSIZE-1, 0);
			r_buf[strsize] = '\0';
			printf("%s",r_buf);
			fflush(stdout);
		}

	}
}
