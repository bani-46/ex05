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
client_list *head;

static char *chop_nl(char *s);

//make_listの返り値を常に保持(head)
//headをコピー、走査
client_info *add_client(int _sock,char _name[],client_info *ci){
	client_info *new = malloc(sizeof(client_info));
	if(new != NULL){
		new->sock = _sock;
		strcpy(new->name ,_name);
		new->next = ci;
//		printf("[INFO]%d,%s\n",new->sock,new->name);
	}
	return new;
}

void make_list(){
  client_list *ls;

  ls = malloc(sizeof(client_list));
  if (ls != NULL) {
    ls->top = add_client(0,"",NULL);
    if (ls->top == NULL) {
      free(ls);
    }
    else printf("[INFO]Success set up list.\n");
  }
  head = ls;
  printf("[DEBUG]head: %d %s %p",head->top->sock,head->top->name,head->top->next);
}

void insert_info(client_list *head,int sock,char name[]){
	client_info *ci = head->top;
	while(ci->next != NULL)ci = ci->next;
	ci->next = add_client(sock,name,NULL);
	printf("[INSERT]%d,%s\n",ci->next->sock,ci->next->name);
}

int delete_info(){
	return 0;
}

void show_list(){
	client_info *ci = head->top->next;
	printf("[DEBUG]head:%d:%s:%p\n",ci->sock,ci->name,ci->next);
	while(ci!= NULL){
		printf("[LIST]%d,%s\n",ci->sock,ci->name);
		ci = ci->next;
	}
	printf("[INFO]END of List.\n");
}

void udp_monitor(int _sock_udp){
	socklen_t from_len;
	char r_buf[BUFSIZE];
	int strsize;
	fd_set mask, readfds;

	FD_ZERO(&mask);
	FD_SET(_sock_udp, &mask);
	readfds = mask;
	select( _sock_udp+1, &readfds, NULL, NULL, NULL );
	/*recv data*/
	if(FD_ISSET(_sock_udp,&readfds)){
		from_len = sizeof(from_adrs);
		strsize = Recvfrom(_sock_udp, r_buf, BUFSIZE-1, 0,(struct sockaddr *)&from_adrs, &from_len);
		msg_processor(r_buf,_sock_udp);
		r_buf[strsize] = '\0';
	}
}

void tcp_monitor(int _sock_listen){
	pthread_t tid;
	int sock_accepted;
	int *tharg;

	/*connection accept*/
	printf("[INFO]tcp success connect\n");

	sock_accepted = accept(_sock_listen, NULL, NULL);
	/* set up arg for create thread */
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
	char r_buf[BUFSIZE];
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
				strsize = recv_message(sock_accepted, r_buf, BUFSIZE-1, 0);
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

char *create_packet(int type,char *message){
	static char buf[BUFSIZE];
	switch(type){
	case HELO:
		snprintf(buf,BUFSIZE,"HELO");
		break;
	case HERE:
		snprintf(buf,BUFSIZE,"HERE");
		break;
	case JOIN:
		snprintf(buf,BUFSIZE,"JOIN %s",message);
		break;
	case POST:
		snprintf(buf,BUFSIZE,"POST %s",message);
		break;
	case MESG:
		snprintf(buf,BUFSIZE,"MESG %s",message);
		break;
	case QUIT:
		snprintf(buf,BUFSIZE,"QUIT");
		break;
	}
	printf("[Packet]%s",buf);
	return buf;
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
	char *buf;
	buf = malloc(BUFSIZE);
	switch(analyze_packet(recv_data->header)){
	case HELO:
		buf = create_packet(HERE,"");
		Sendto(_sock,buf,strlen(buf),0,(struct sockaddr *)&from_adrs, sizeof(from_adrs));
		printf("[INFO]Appear New Client.\n");
		n_clinet++;
		break;
	case JOIN:
		chop_nl(recv_data->msg);
		printf("[INFO]No:%d,Name:%s is login.\n",_sock,recv_data->msg);
		insert_info(head,_sock,recv_data->msg);
		show_list();
		break;
	case POST:
		create_packet(MESG,recv_data->msg);
		//ソケットからnameを得る
		//msgに付与
		//得られたname以外にsend
		send_message(_sock,buf,strlen(buf),0);
		break;
	case MESG:
		printf("[MESG]%s.\n",recv_data->msg);
		break;
	case QUIT:

		break;
	}
//	free(buf);
}

static char *chop_nl(char *s){
	int len;
	len = strlen(s);
	if( s[len-1] == '\n' ){
		s[len-1] = '\0';
	}
	return(s);
}

