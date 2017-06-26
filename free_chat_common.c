/*
 * free_chat_common.c
 *
 *  Created on: 2017/06/21
 *      Author: b5122025
 */

#include "mynet.h"

int accept_client(int s, struct sockaddr *addr, socklen_t *addrlen)
{
  int r;
  if((r=accept(s,addr,addrlen))== -1){
    exit_errmesg("accept()");
  }
  return(r);
}

int send_message(int s, void *buf, size_t len, int flags)
{
  int r;
  if((r=send(s,buf,len,flags))== -1){
    exit_errmesg("send()");
  }
  return(r);
}

int recv_message(int s, void *buf, size_t len, int flags)
{
  int r;
  if((r=recv(s,buf,len,flags))== -1){
    exit_errmesg("recv()");
  }
  return(r);
}
