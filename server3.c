#include"public.h"
#include"sys/stat.h"
#include"fcntl.h"
#define ADD  7
#define DEL  8
#define FIFO  "FIFO"

struct ARG
{
    int locate;
    int fifoFd;
};

int SearchLocate()
{
    int i;
    for(i=0;i<MAX_CLIENT;i++)
    {
        if(clientList[i].socketFd==0)
            break;
        
    }
    if(i<MAX_CLIENT) 
        return i;
    else  
        return -1;
}


void TransmitMsg(int cmd,int locate,MESSAGE msg)
{
    
    memcpy(&msg.clientList,&clientList,sizeof(clientList));//拷贝字节
    if(cmd==PRIVATE)
    {
    
        write(clientList[msg.sendUserLocate].socketFd,&msg,sizeof(msg));
        printf("\e[31m#聊天 >  发件人:%-5s  收件人:  %-5s  消息内容:%s\e[0m\n",clientList[locate].name,clientList[msg.sendUserLocate].name,msg.message);
    }else{
        int i;
        for (i=0;i<MAX_CLIENT;i++)
        {
            if(clientList[i].socketFd!=0 && i!=locate)
            {
                write(clientList[i].socketFd,&msg,sizeof(msg));
                printf("\e[32m#公告  >  发件人:%-5s  收件人:  %-5s 消息:%s\e[0m\n",clientList[locate].name,clientList[i].name,msg.message);
            }
        }
        if(cmd==LOGIN)
        {
            write(clientList[locate].socketFd,&msg,sizeof(msg));
        }
    }
}

void UpdateList(int cmd , char *name,int locate)
{
    if(cmd==ADD)
    {
        strcpy(clientList[locate].name,name);
        printf("\e[33m*用户上线> 名字:%-5s  \e[0m\n",clientList[locate].name);
    }
    else if(cmd==DEL)
    {
        printf("\e[33m*用户下线> 名字:%-5s \e[0m\n",clientList[locate].name);
        clientList[locate].socketFd=0;
        bzero(clientList[locate].name,NAME_LEN);
    }
}

void *RecvMsg(void *arg_t)//接受消息
{
    struct ARG arg=*(struct ARG *)arg_t;
    MESSAGE msg;
    while(1)
    {
        int flag;
        bzero(&msg,sizeof(msg));  
        msg.type=ERROR;
        read(clientList[arg.locate].socketFd,&msg,sizeof(msg));
        msg.fromUserLocate=arg.locate;
        if(msg.type==EXIT||msg.type==ERROR)
        {
            if(msg.type==ERROR)
            {
                strcpy(msg.message,"breakdown");
                printf("\e[33m*CLIENT:%s HAD BREAKDOWN\e[0m\n",clientList[msg.fromUserLocate].name);
                msg.type=EXIT;
            }
            if(-1==(flag=write(arg.fifoFd,&msg,sizeof(msg))))
            {
                perror("write fifo error");
                exit(1);
            }
            break;
        }
        if(-1==(flag=write(arg.fifoFd,&msg,sizeof(msg))))
        {
            perror("write fifo error");
            exit(1);
        }
    }
    return NULL;
}

void *SendMsg(void *fd)//发送消息
{
    int fifoFd;
    if(-1==(fifoFd=open(FIFO,O_RDONLY)))
    {
        perror("open fifo error");
        exit(1);
    }
    int flag;
    MESSAGE msg;
    while(1)
    {
        if(-1==(flag=read(fifoFd,&msg,sizeof(msg))))
        {
            perror("read fifo error");
            exit(2);
        } 
        int exit_fd;
        switch(msg.type)
        {
            case LOGIN:
                UpdateList(ADD,msg.fromUser,msg.fromUserLocate);
                TransmitMsg(LOGIN,msg.fromUserLocate,msg);
                break;
            case PUBLIC:
                TransmitMsg(PUBLIC,msg.fromUserLocate,msg);
                break;
            case PRIVATE:
                TransmitMsg(PRIVATE,msg.fromUserLocate,msg);
                break;
            case EXIT:
                exit_fd=clientList[msg.fromUserLocate].socketFd;
                UpdateList(DEL,msg.fromUser,msg.fromUserLocate);
                TransmitMsg(EXIT,msg.fromUserLocate,msg);
                close(exit_fd);
                break;
            default:
        //printf("bad data %d\n",msg.type);
                break;    
        }
    }
    return NULL;
}

int main()
{
    printf("\n\tservice is start.....\n");
    pthread_t tid1,tid2;//创建线程
    int fd,clientfd,wr_fifo;//套接字文件描述符
    socklen_t  sock_len;//建立套接字关键字
    sock_len= sizeof(struct sockaddr_in);//地址长度

    mkfifo(FIFO,O_CREAT|O_EXCL);//建立实名管道
    pthread_create(&tid1,NULL,SendMsg,NULL);//创建接收消息的线程

    struct  sockaddr_in server,client;//本地地址
    server.sin_port=htons(PORT);//服务器端口
    server.sin_family=AF_INET;////AF_INET协议族
    server.sin_addr.s_addr=INADDR_ANY;//任意本地地址
    if(-1== (fd=socket(AF_INET,SOCK_STREAM,0)))//建立套接字
    {
        perror("socket error ");
        exit(1);
    }

    //绑定套接口
    if(-1==bind(fd,(struct sockaddr*)&server,sock_len))//将&server结构体强制转换成struct sockaddr*类型
    {
        perror("bind error");
        exit(2);
    }

    //侦听
    if(-1==(listen(fd,MAX_CLIENT+1)))
    {
        perror("listen error");
        exit(3);
    }
    
    //打开管道fifo
    if(-1==(wr_fifo=open(FIFO,O_WRONLY)))
    {
        perror("open fifo error");
        exit(1);
    }
    while(1)
    {
        if(-1==(clientfd=(accept(fd,(struct sockaddr*)&client,&sock_len))))//接收客户端连接
        {
            perror("accept error");
            exit(4);
        }
        int locate=-1;//自身定位为-1
        MESSAGE msg;//创建信息结构体
        if(-1==(locate=SearchLocate()))//找到最新的还未定位的客户端结构体，产生定位
        {
            printf("\e[33m*RECEIVE A APPLY BUT CANNOT ALLOW CONNECT\e[0m\n");
            msg.type=EXIT;
            write(clientfd,&msg,sizeof(msg));
        }
        else
        {
            struct ARG arg;
            arg.fifoFd=wr_fifo;
            arg.locate=locate;
            msg.type=OK;
            memcpy(&msg.clientList,&clientList,sizeof(clientList));
            msg.fromUserLocate=locate;//发件人的定位
            write(clientfd,&msg,sizeof(msg));//把文件写到客户端接口
            clientList[locate].socketFd=clientfd;//客户端链表标记为接口信息
            pthread_create(&tid1,NULL,RecvMsg,(void *)&arg);//创建接受消息的线程
        }
    }
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);//结束两个线程
    unlink("FIFO");//关闭fifo管道
    return 0;
}
