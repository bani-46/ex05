/*
 * free_chat_util.c
 *
 *  Created on: 2017/06/19
 *      Author: b5122025
 */
#include "mynet.h"
#include "free_chat.h"
#include <sys/wait.h>
#include <pthread.h>

int n_clinet = 0;
int sock_udp;
struct sockaddr_in from_adrs;
#define NAMELENGTH 15

char Buffer[S_BUFSIZE];

static char *chop_nl(char *s);

typedef struct Client_Info{
	int  sock;
	char name[NAMELENGTH];
	struct Client_Info *next;
} *client_info;

void udp_monitor(int _sock_udp){
	socklen_t from_len;
	char r_buf[R_BUFSIZE];
	int strsize;
	fd_set mask, readfds;

	sock_udp = _sock_udp;//todo
	FD_ZERO(&mask);
	FD_SET(sock_udp, &mask);
	readfds = mask;
	select( sock_udp+1, &readfds, NULL, NULL, NULL );
	/*recv data*/
	if(FD_ISSET(sock_udp,&readfds)){
		from_len = sizeof(from_adrs);
		strsize = Recvfrom(sock_udp, r_buf, R_BUFSIZE-1, 0,
				(struct sockaddr *)&from_adrs, &from_len);
		r_buf[strsize] = '\0';
		msg_processor(r_buf);
	}
}

void tcp_monitor(int _sock_listen){
	pthread_t tid;
	int sock_accepted;
	int *tharg;

	/*connection accept*/
	printf("[INFO]tcp success connect\n");

	sock_accepted = accept(_sock_listen, NULL, NULL);
	/* スレッド関数の引数を用意する */
	if( (tharg = (int *)malloc(sizeof(int)))==NULL ){
		exit_errmesg("malloc()");
	}
	*tharg = sock_accepted;

	if( pthread_create(&tid, NULL, echo_thread, (void *)tharg) != 0 ){
		exit_errmesg("pthread_create()");
	}
}
void * echo_thread(void *arg){
	int sock_accepted;
	int strsize;
	char r_buf[R_BUFSIZE];
	fd_set mask, readfds;

	sock_accepted = *((int *)arg);

	pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */
	printf("[THEARD:%p]start\n",(void *)pthread_self());
	while(1){
		/* set bit_mask */
		FD_ZERO(&mask);
		FD_SET(sock_accepted, &mask);

		for(;;){
			/* check recv data */
			readfds = mask;
			select( sock_accepted + 1, &readfds, NULL, NULL, NULL );

			/* recv message */
			if( FD_ISSET(sock_accepted, &readfds) ){
				strsize = recv_message(sock_accepted, r_buf, R_BUFSIZE-1, 0);
				if(strsize == 0){
					printf("[THEARD:%p]close\n",(void *)pthread_self());
					close(sock_accepted);
					break;
				}
				r_buf[strsize] = '\0';
				msg_processor(r_buf);
				printf("[RECV:%p]%s",(void *)pthread_self(),r_buf);
			}
		}
		break;
	}
	return(NULL);
}

void create_packet(int type,char *message){
	switch(type){
	case HELO:
		snprintf(Buffer,S_BUFSIZE,"HELO");
		break;
	case HERE:
		snprintf(Buffer,S_BUFSIZE,"HERE");
		break;
	case JOIN:
		snprintf(Buffer,S_BUFSIZE,"JOIN %s",message);
		break;
	case POST:
		snprintf(Buffer,S_BUFSIZE,"POST %s",message);
		break;
	case MESSAGE:
		snprintf(Buffer,S_BUFSIZE,"MESSAGE %s",message);
		break;
	case QUIT:
		snprintf(Buffer,S_BUFSIZE,"QUIT");
		break;
	}
}

int analyze_packet(char *_header){
	if( strncmp( _header, "HELO", 4 )==0 ) return(HELO);
	if( strncmp( _header, "HERE", 4 )==0 ) return(HERE);
	if( strncmp( _header, "JOIN", 4 )==0 ) return(JOIN);
	if( strncmp( _header, "POST", 4 )==0 ) return(POST);
	if( strncmp( _header, "MESG", 4 )==0 ) return(MESSAGE);
	if( strncmp( _header, "QUIT", 4 )==0 ) return(QUIT);
	return 0;
}

void msg_processor(char *_r_buf){
	printf("[DEBUG]%s\n",_r_buf);
	packet *recv_data = (packet *)_r_buf;
	switch(analyze_packet(recv_data->header)){
	case HELO:
		create_packet(HERE,"");
		Sendto(sock_udp,Buffer,strlen(Buffer),0,
				(struct sockaddr *)&from_adrs, sizeof(from_adrs));
		printf("[INFO]Appear New Client.\n");
		n_clinet++;
		break;
	case JOIN:
		chop_nl(recv_data->msg);



		break;
	case POST:

		break;
	case MESSAGE:

		break;
	case QUIT:

		break;
	}
}

static char *chop_nl(char *s){
	int len;
	len = strlen(s);
	if( s[len-1] == '\n' ){
		s[len-1] = '\0';
	}
	return(s);
}

