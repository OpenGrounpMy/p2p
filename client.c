#include"public.h"

pthread_t tid1;//接收线程的标识符
char g_name[NAME_LEN];//客户端用户的名称
int  g_locate;//客户端在在线客户列表中的位置
int  g_total;//在线客户的总数

void flush();
int CheckExist();
void  ShowList();
int MakeTempList(int *tmp);
void *RecvMsg(void *fd);
void SendMsg(int fd);

void flush()
{
    char c;
    do{
	c=getc(stdin);
    }while(c!='\n'&&c!=EOF);
}

int CheckExist()
{
    int i;
    for(i=0;i<MAX_CLIENT;i++)
    {
        if(!strcmp(g_name,clientList[i].name))
            break;

    }
    if(i<MAX_CLIENT)
    {
        printf("this name: %s is already exist!!\n",g_name);
        return 1;
    }
    else
        return 0;
    
}
void  ShowList()
{
    int i;
    g_total=0;

    printf("\t _____________________________ \n");
    printf("\t|         CLIENT LIST         |\n");
    printf("\t|_____________________________|\n");
    printf("\t|\e[4m  sort   |  name   | ifchat  \e[24m|\n ");

    for(i=0;i<MAX_CLIENT;i++)
    {
        if(clientList[i].socketFd!=0)
        {
            if(i==g_locate)
            {
                printf("\t|\e[4;31m *%-4d   |   %-5s |   %-5d \e[0m|\n",++g_total,clientList[i].name,clientList[i].chatnow);
                
            }
            else
            {
                printf("\t|\e[4m   %-4d  |   %-5s |   %-5d \e[24m|\n",++g_total,clientList[i].name,clientList[i].chatnow);
                
            }
            
        }
        
    }
    printf("\t|\e[4m  Total:%-3d     \e[24m|\n",g_total);
}

int MakeTempList(int *tmp)
{
    int i,n=0;
    for(i=0;i<MAX_CLIENT;i++)
    {
        if(clientList[i].socketFd!=0)
        { 
	    tmp[n]=i;
	    n++; 
	}
        
    }
    ShowList();
    int select;
    printf("please select the user \n");
    if(1!=scanf("%d",&select))
    {
        flush();
        printf("bad select \n");
        return -1;
        
    }
    if(select<=g_total)
    {
        if(tmp[select-1]==g_locate)
        {
            printf("\e[33m#SYSTEM:YOU CAN NOT SELECT YOURSELF\e[0m\n");
            return -1;
            
        }
        else
            return tmp[select-1];
        
    }
    else
    {
        printf("bad select \n");
        return -1;
        
    }
}

void *RecvMsg(void *fd)
{
    int sockfd=*(int *)fd;
    MESSAGE msg;
    while(1)
    {
        bzero(&msg,sizeof(msg));
        msg.type=ERROR;
        read(sockfd,&msg,sizeof(msg));
        
        if(msg.type==ERROR)
            break;
        switch(msg.type)
        {
            case LOGIN:
                if(msg.fromUserLocate==g_locate)
                    printf("\e[34m######  >  loing succeed\e[0m\n");
                else
                    printf("\e[33m#LOGIN  > 上线提醒:%-10s上线\n",msg.fromUser);
                break;
            case EXIT:
                printf("\e[33m#EXIT  > 下线提醒:%-10s下线\n",clientList[msg.fromUserLocate].name);
                break;
            /*case PUBLIC:
                printf("\e[32m#PUBLIC > From:%-10s Msg:%s\e[0m\n",msg.fromUser,msg.message);
                break;*/
            case PRIVATE:
                if(clientList[g_locate].chatnow==1){
                  //  printf("sorry, the user don't want chat with you.");
                    break;
                }else{
                    printf("\e[31m#PRIVATE> From:%-10s Msg:%s\e[0m\n",msg.fromUser,msg.message);
                    break;
                }
            default:
                break;
            
        }
        memcpy(&clientList,&msg.clientList,sizeof(clientList));
        
    }
    printf("server is breakdown \n");
    exit(1);
    
}

void SendMsg(int fd)
{
    MESSAGE msg;
    msg.type=LOGIN;
    msg.fromUserLocate=g_locate;
    strcpy(msg.fromUser,g_name);
    strcpy(msg.message,g_name);
    write(fd,&msg,sizeof(msg));
    
    int tmp[MAX_CLIENT];
    int  key;
    while(1)
    {
        printf(" 1 chat record  2 private chat 3 EXIT 4 client list\n");
        if(1!= scanf("%d",&key))
        {
            key=0;
            flush();
        }
        bzero(&msg,sizeof(msg));
        strcpy(msg.fromUser,g_name);
        msg.fromUserLocate=g_locate;
        if(key==1){
            
        }else if(key==2){
            msg.clientList[g_locate].chatnow=1;
            clientList[g_locate].chatnow=1;
            if(-1!=(msg.sendUserLocate=MakeTempList(tmp)))
            {
                msg.clientList[msg.sendUserLocate].chatnow=1;
                clientList[msg.sendUserLocate].chatnow=1;
                //strcpy(msg.message,"Can you chat with me?")
                //write(fd,&msg,sizeof(msg));
                printf("\nprivate: please input content \nif put q, that you will cut chat\nif put l, you can now the list\n");
                flush();
                while(1){
                    bzero(tmp,sizeof(tmp));
                    msg.type=PRIVATE;
                //msg.clientList[g_locate].chatnow=1;
               // if(-1!=(msg.sendUserLocate=MakeTempList(tmp)))
                //{
                  //  msg.clientList[msg.sendUserLocate].chatnow=1;
                    //printf("\nprivate: please input content \n");
                    //flush();
                    fgets(msg.message,sizeof(msg.message),stdin);
                    msg.message[strlen(msg.message)-1]='\0';
                    if(strcmp(msg.message,"l")==0)
                        ShowList();
                    if(strcmp(msg.message,"q")==0){
                        clientList[g_locate].chatnow=0;
                        clientList[msg.sendUserLocate].chatnow=0;
                        msg.clientList[g_locate].chatnow=0;
                        msg.clientList[msg.sendUserLocate].chatnow=0;
                        break;
                    }
                    write(fd,&msg,sizeof(msg));
                    //pthread_cancel(tid1);
                }
            }
        }else if(key==3){
            printf("EXIT \n");
            msg.type=EXIT;
            strcpy(msg.message,"bye-bye");
            write(fd,&msg,sizeof(msg));
        }else if(key==4){
            ShowList();
        }else{
            printf("bad select  \n");
            msg.type=0;
        }
        /*switch(key)
        {
            case 1:
                msg.type=PUBLIC;
                printf("\npublic: please input content \n");
                flush();
                fgets(msg.message,sizeof(msg.message),stdin);
                msg.message[strlen(msg.message)-1]='\0';
                write(fd,&msg,sizeof(msg));
                break;
            case 2:
                    bzero(tmp,sizeof(tmp));
                    msg.type=PRIVATE;
                    if(-1!=(msg.sendUserLocate=MakeTempList(tmp)))
                    {
                        printf("\nprivate: please input content \n");
                        flush();
                        fgets(msg.message,sizeof(msg.message),stdin);
                        msg.message[strlen(msg.message)-1]='\0';
                        write(fd,&msg,sizeof(msg));
                    }
                break;
            case 3:
                printf("EXIT \n");
                msg.type=EXIT;
                strcpy(msg.message,"bye-bye");
                write(fd,&msg,sizeof(msg));
                break;
            case 4:
                ShowList();
                break;
            default:
                printf("bad select  \n");
                msg.type=0;
                break;
        }*/
        if(msg.type==EXIT)
        {
            break;
        }
        
    }
    pthread_cancel(tid1);
}
int main()
{
    int fd;
    char ip[20];//="192.168.43.31";
    printf("please input the ip \n");scanf("%s",ip);
    struct sockaddr_in addr;
    addr.sin_port=htons(PORT);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(ip);
    if(-1==(fd=socket(AF_INET,SOCK_STREAM,0)))
    {
        perror("socket error");
        exit(1);
        
    }
    if(-1==(connect(fd,(struct sockaddr*)&addr,sizeof(struct
        sockaddr))))
    {
        perror("connect error");
        exit(2);
        
    }
    MESSAGE msg;

    read(fd,&msg,sizeof(msg));
    if(msg.type==EXIT)
    {
        printf("service refuse connect \n");
        exit(1);
        
    }
    else
    {
        memcpy(&clientList,&msg.clientList,sizeof(clientList));
        g_locate=msg.fromUserLocate;
        pthread_create(&tid1,NULL,RecvMsg,(void *)&fd);
        do{
            printf("please input your name\n");scanf("%s",g_name);
            
        }while(CheckExist());
        SendMsg(fd);
        pthread_join(tid1,NULL);
        
    }
    return 0;
}


