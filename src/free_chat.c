/*
 * free_chat.c
 *
 *  Created on: 2017/06/19
 *      Author: b5122025
 */

#include "mynet.h"
#include "free_chat.h"

#define TIMEOUT_SEC 1
#define DEFAULT_PORT 50001
#define USER_LENGTH 15   /* username */

char branch_mode(int _port_number);
struct sockaddr_in from_adrs;

int main(int argc, char *argv[])
{
	int port_number = DEFAULT_PORT;
	char username[USER_LENGTH];
	char mode;

	int option_num;
	opterr = 0;
	while(1){
		option_num = getopt(argc, argv, "u:p:h");
		if( option_num == -1 ) break;
		switch( option_num ){
		case 'u' :  /* set User name */
			snprintf(username, USER_LENGTH, "%s", optarg);
			break;
		case 'p':  /* set Port number */
			port_number = atoi(optarg);
			break;
		case '?' :
			fprintf(stderr,"Unknown option '%c'\n", optopt );
		case 'h' :
			fprintf(stderr,"Usage: %s -u username(within 15) -p port_number\n",argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}

	mode = branch_mode(port_number);
	switch(mode){
	case 'S':
		printf("[INFO]Set Server mode.\n");
		free_chat_server(port_number);
		break;
	case 'C':
		printf("[INFO]Set Client mode.\n");
		free_chat_client(port_number,username,&from_adrs);
		break;
	}
	exit(EXIT_SUCCESS);
}

char branch_mode(int _port_number){
	struct sockaddr_in broadcast_adrs;

	socklen_t from_len;

	int sock_broadcast;
	int broadcast_sw=1;
	fd_set mask, readfds;
	struct timeval timeout;
	int timeout_count = 0;

	char *s_buf;
	char r_buf[BUFSIZE];
	int strsize;

    /*set socket to use broadcast*/
	set_sockaddr_in_broadcast(&broadcast_adrs,_port_number);
	sock_broadcast = init_udpclient();
	if(setsockopt(sock_broadcast, SOL_SOCKET, SO_BROADCAST, (void *)&broadcast_sw, sizeof(broadcast_sw)) == -1){
		exit_errmesg("setsockopt()");
	}

	/* set bit mask */
	FD_ZERO(&mask);
	FD_SET(sock_broadcast, &mask);

	printf("[INFO]Packet sending...\n");
	while(1){
		readfds = mask;
		timeout.tv_sec = TIMEOUT_SEC;
		timeout.tv_usec = 0;
		/*[HELO] broadcasting*/
		s_buf = create_packet(HELO,"");
		Sendto(sock_broadcast, s_buf, strlen(s_buf), 0,(struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs) );
		/*no recv_data*/
		if( select( sock_broadcast+1, &readfds, NULL, NULL, &timeout )==0 ){
			timeout_count++;
			printf("[INFO]Time out %d.\n",timeout_count);
			/*set Server mode*/
			if(timeout_count == 3)break;
		}
		/*some message recv*/
		else{
			from_len = sizeof(from_adrs);
			strsize = Recvfrom(sock_broadcast, r_buf, BUFSIZE-1, 0,(struct sockaddr *)&from_adrs, &from_len);
			r_buf[strsize] = '\0';
			printf("[DEBUG]recv:%s\n",r_buf);
			/*set Client mode*/
			if(strcmp("HERE",r_buf) == 0){
				show_adrsinfo(&from_adrs);
				close(sock_broadcast);
				return 'C';
			}
		}
	}
	close(sock_broadcast);
	return 'S';
}
