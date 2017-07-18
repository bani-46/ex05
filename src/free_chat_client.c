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
	char ip_adrs[20];
	char *s_buf,r_buf[BUFSIZE],input_msg[BUFSIZE];
	//	char *p;

	strncpy(ip_adrs,inet_ntoa(from_adrs->sin_addr),20);
	sock= init_tcpclient(ip_adrs,_port_number);

	printf("***Connect server.\n");

	/*login process*/
	s_buf = create_packet(JOIN,username);
	strsize = strlen(s_buf);
	send_message(sock, s_buf, strsize, MSG_NOSIGNAL);
	fflush(stdout);

	printf("***Success login.\n");
	/* set bit mask */
	FD_ZERO(&mask);
	FD_SET(0, &mask);
	FD_SET(sock, &mask);

	while(1){
		/* check recv data */
		readfds = mask;
		select( sock+1, &readfds, NULL, NULL, NULL );

		if( FD_ISSET(0, &readfds) ){
			fgets(input_msg, BUFSIZE, stdin);
			if(strstr(input_msg,"QUIT")!=NULL){
				s_buf = create_packet(QUIT,"");
				send_message(sock, s_buf,strlen(s_buf), MSG_NOSIGNAL);
				printf("***Program finish.\n");
				break;
			}
			else{
				s_buf = create_packet(POST,input_msg);
				send_message(sock, s_buf,strlen(s_buf), MSG_NOSIGNAL);
			}
		}

		if( FD_ISSET(sock, &readfds) ){
			/* recv message */
			strsize = recv_message(sock, r_buf, BUFSIZE-1, 0);
			if(strsize == 0){
				printf("***Server is down.\n");
				break;
			}
			r_buf[strsize] = '\0';
			msg_processor(r_buf,sock);
			fflush(stdout);
		}

	}
}
