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
struct sockaddr_in from_adrs;

char Buffer[S_BUFSIZE];

static char *chop_nl(char *s);

client_info *add_client(int _sock,char _name[],client_info *ci){
	client_info *new = malloc(sizeof(client_info));
	if(new != NULL){
		new->sock = _sock;
		strcpy(new->name ,_name);
		new->next = ci;
	}
	return new;
}

client_list *make_list(void){
  client_list *ls = malloc(sizeof(client_list));
  if (ls != NULL) {
    ls->top = add_client(0,NULL,NULL);
    if (ls->top == NULL) {
      free(ls);
      return NULL;
    }
  }
  return ls;
}

void insert_info(client_list *ls,int sock,char name[]){
	client_info *ci = ls->top;
	while(ci->next != NULL)ci = ci->next;
	ci->next = add_client(sock,name,NULL);
}

int delete_info(){

}

void udp_monitor(int _sock_udp){
	socklen_t from_len;
	char r_buf[R_BUFSIZE];
	int strsize;
	fd_set mask, readfds;

	FD_ZERO(&mask);
	FD_SET(_sock_udp, &mask);
	readfds = mask;
	select( _sock_udp+1, &readfds, NULL, NULL, NULL );
	/*recv data*/
	if(FD_ISSET(_sock_udp,&readfds)){
		from_len = sizeof(from_adrs);
		strsize = Recvfrom(_sock_udp, r_buf, R_BUFSIZE-1, 0,
				(struct sockaddr *)&from_adrs, &from_len);
		r_buf[strsize] = '\0';
		msg_processor(r_buf,_sock_udp);
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
				msg_processor(r_buf,sock_accepted);
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
	case MESG:
		snprintf(Buffer,S_BUFSIZE,"MESG %s",message);
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
	if( strncmp( _header, "MESG", 4 )==0 ) return(MESG);
	if( strncmp( _header, "QUIT", 4 )==0 ) return(QUIT);
	return 0;
}

void msg_processor(char *_r_buf,int _sock){
	packet *recv_data = (packet *)_r_buf;
	switch(analyze_packet(recv_data->header)){
	case HELO:
		create_packet(HERE,"");
		Sendto(_sock,Buffer,strlen(Buffer),0,
				(struct sockaddr *)&from_adrs, sizeof(from_adrs));
		printf("[INFO]Appear New Client.\n");
		n_clinet++;
		break;
	case JOIN:
		chop_nl(recv_data->msg);
//		insert_info(ls,_sock,recv_data->msg);todo
		break;
	case POST:
		create_packet(MESG,recv_data->msg);
		send_message(_sock,Buffer,strlen(Buffer),0);
		break;
	case MESG:

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

