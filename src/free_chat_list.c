/*
 * free_chat_list.c
 *
 *  Created on: 2017/07/18
 *      Author: b5122025
 */
#include "mynet.h"
#include "free_chat.h"

client_info *add_client(int _sock,char _name[],client_info *ci){
	client_info *new = malloc(sizeof(client_info));
	if(new != NULL){
		new->sock = _sock;
		strcpy(new->name ,_name);
		new->next = ci;
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
		else printf("***Success set up list.\n");
	}
	head = ls;
}

void insert_info(client_list *head,int sock,char name[]){
	client_info *ci = head->top;
	while(ci->next != NULL)ci = ci->next;
	ci->next = add_client(sock,name,NULL);
}

void delete_info(int _sock){
	client_info *ci = head->top;
	while(ci!= NULL){
		if(_sock == ci->next->sock){
			free(ci->next);
			ci->next = ci->next->next;
			break;
		}
		ci = ci->next;
	}
}

int is_rest_info(int _sock){
	client_info *ci = head->top;
	while(ci!= NULL){
		if(_sock == ci->sock){
			return 1;
		}
		ci = ci->next;
	}
	return 0;
}

void show_list(){
	client_info *ci = head->top->next;//next
	printf("\n***Now List state.\n");
	while(ci!= NULL){
		printf("\t%d,%s\n",ci->sock,ci->name);
		ci = ci->next;
	}
	printf("***END of List.\n\n");
}
