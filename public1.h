#ifndef PUBLIC_H_
#define PUBLIC_H_

#include"stdio.h"
#include"stdlib.h"
#include"string.h"
#include"sys/socket.h"
#include"sys/types.h"
#include"arpa/inet.h"
#include"netinet/in.h"
#include"unistd.h"
#include"pthread.h"

#define  MAX_CLIENT 10
#define  NAME_LEN   20
#define  MSG_LEN    100
#define  PORT       12345
#define LOGIN   1
#define EXIT    2
#define PUBLIC  3
#define PRIVATE 4
#define OK      5
#define ERROR   -6
typedef struct ClientList
{
char name[NAME_LEN];
int socketFd;//服务器为客户端分配的通信套接字标识
int chatnow;//聊天状态
}CLIENTLIST;

typedef struct Message
{
char fromUser[NAME_LEN];//客户端的名称
int  fromUserLocate;//客户端在列表中的位置
int  type;//私聊、登录、退出的消息类型
int  sendUserLocate;//私聊对象在客户端的位置
char message[MSG_LEN];//发送的消息内容
CLIENTLIST clientList[MAX_CLIENT];//在线客户的人数
}MESSAGE;

CLIENTLIST  clientList[MAX_CLIENT];
#endif
