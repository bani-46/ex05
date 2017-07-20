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

struct sockaddr_in from_adrs;

static char *chop_nl(char *s);

void udp_monitor(int _sock_udp,char _username[],int _sock_listen){
	socklen_t from_len;
	char r_buf[BUFSIZE];
	int strsize;
	fd_set mask, readfds;
	char *s_buf,input_msg[BUFSIZE];

	FD_ZERO(&mask);
	FD_SET(0, &mask);
	FD_SET(_sock_udp, &mask);
	readfds = mask;
	select( _sock_udp+1, &readfds, NULL, NULL, NULL );
	if( FD_ISSET(0, &readfds) ){
		fgets(input_msg, BUFSIZE, stdin);
		chop_nl(input_msg);
		s_buf = format_MESG(_username,input_msg);
		s_buf = create_packet(MESG,s_buf);
		send_MESG(s_buf,0,ALL);//todo
	}
	/*recv data*/
	if(FD_ISSET(_sock_udp,&readfds)){
		from_len = sizeof(from_adrs);
		strsize = Recvfrom(_sock_udp, r_buf, BUFSIZE-1, 0,(struct sockaddr *)&from_adrs, &from_len);
		msg_processor(r_buf,_sock_udp);
		r_buf[strsize] = '\0';
		/*tcp monitoring*/
		tcp_monitor(_sock_listen);
	}
}

void tcp_monitor(int _sock_listen){
	pthread_t tid;
	int sock_accepted;
	int *tharg;

	/*connection accept*/
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
	char r_buf[BUFSIZE],*cname;
	fd_set mask, readfds;

	sock_accepted = *((int *)arg);

	pthread_detach(pthread_self());
	while(1){
		/* set bit_mask */
		FD_ZERO(&mask);
		FD_SET(sock_accepted, &mask);

		while(1){
			/* check recv data */
			readfds = mask;
			select( sock_accepted + 1, &readfds, NULL, NULL, NULL );

			/* recv message */
			if( FD_ISSET(sock_accepted, &readfds) ){
				strsize = recv_message(sock_accepted, r_buf, BUFSIZE-1, 0);
				if(strsize == 0){
					//when client logouted without QUIT
					if(is_rest_info(sock_accepted)){
						cname = get_cname(sock_accepted);
						printf("***_%s_ was logout.\n",cname);
						delete_info(sock_accepted);
					}
					close(sock_accepted);
//					show_list();
					break;
				}
				r_buf[strsize] = '\0';
				msg_processor(r_buf,sock_accepted);
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
	char *buf,*cname;
	buf = malloc(BUFSIZE);
	switch(analyze_packet(recv_data->header)){
	case HELO://use server(connect client to server)
		buf = create_packet(HERE,"");
		Sendto(_sock,buf,strlen(buf),0,(struct sockaddr *)&from_adrs, sizeof(from_adrs));
		break;
	case JOIN://use server(add new cleint info)
		chop_nl(recv_data->msg);
		printf("***_%s_ is login.\n",recv_data->msg);
		insert_info(head,_sock,recv_data->msg);
		break;
	case POST://use server(format recv message to "[name]msg" and send message each client)
		cname = get_cname(_sock);
		chop_nl(recv_data->msg);
		buf = format_MESG(cname,recv_data->msg);
		printf("%s\n",buf);
		buf = create_packet(MESG,buf);
		send_MESG(buf,_sock,EACH);
		break;
	case MESG://use client(show recv message)
		chop_nl(recv_data->msg);
		printf("%s\n",recv_data->msg);
		break;
	case QUIT://use server(delete client info)
		cname = get_cname(_sock);
		printf("***_%s_ was logout.\n",cname);
		delete_info(_sock);
		break;
	}
	buf = NULL;
}

char * get_cname(int _sock){
	client_info *ci = head->top->next;
	while(ci!= NULL){
		if(ci->sock == _sock)break;
		ci = ci->next;
	}
	return ci->name;
}

char * format_MESG(char *name,char *msg){
	static char fmsg[BUFSIZE];
	snprintf(fmsg,BUFSIZE,"[%s]%s",name,msg);
	return fmsg;
}

void send_MESG(char *_buf,int _sock,int mode){
	client_info *ci = head->top->next;
	switch(mode){
	case ALL:
		while(ci!= NULL){
			send_message(ci->sock, _buf, strlen(_buf), MSG_NOSIGNAL);
			ci = ci->next;
		}
		break;
	case EACH:
		while(ci!= NULL){
			if(ci->sock != _sock){
				send_message(ci->sock, _buf, strlen(_buf), MSG_NOSIGNAL);
			}
			ci = ci->next;
		}
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
