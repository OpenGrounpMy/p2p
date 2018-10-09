#include"public.h"

pthread_t tid1;
char g_name[NAME_LEN];
int  g_locate;
int  g_total;
FILE *stream;
char buf[100];
char file[150];
char lose[50];


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
        printf("这个名字：%s 已经用过了。\n",g_name);
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
    printf("\t|\e[4m  序号   |       名字        \e[24m|\n ");

    for(i=0;i<MAX_CLIENT;i++)
    {
        if(clientList[i].socketFd!=0)
        {
            if(i==g_locate)
            {
                printf("\t|\e[4;31m *%-4d   |     %-10s    \e[0m|\n",++g_total,clientList[i].name,clientList[i].chatnow);
                
            }
            else
            {
                printf("\t|\e[4m   %-4d  |     %-10s    \e[24m|\n",++g_total,clientList[i].name,clientList[i].chatnow);
                
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
    printf("请选择你要发送文件的对象 \n");
    if(1!=scanf("%d",&select))
    {
        flush();
        printf("没有这个人 \n");
        return -1;
        
    }
    if(select<=g_total)
    {
        if(tmp[select-1]==g_locate)
        {
            printf("\e[33m#你不能发消息给你自己；\e[0m\n");
            return -1;
            
        }
        else
            return tmp[select-1];
        
    }
    else
    {
        printf("没有这个选项 \n");
        return -1;
        
    }
}

void *RecvMsg(void *fd)
{
    int sockfd=*(int *)fd;
    int fd1;
    MESSAGE msg;
    while(1)
    {
        bzero(&msg,sizeof(msg));
        msg.type=ERROR;
        read(sockfd,&msg,sizeof(msg));
        
        if(msg.type==ERROR)
            break;
        fd1=msg.fromUserLocate;
        switch(msg.type)
        {
            case LOGIN://成功连接与成员连接公告
                if(msg.fromUserLocate==g_locate)
                    printf("\e[34m######  >  成功进入聊天\e[0m\n");
                else
                    printf("\e[33m#加入> 名字：%s\e[0m\n",msg.fromUser);
                break;
            case EXIT://退出
                printf("\e[33m#退出> 名字：%s\e[0m\n",clientList[msg.fromUserLocate].name);
                break;
            /*case PUBLIC:
                printf("\e[32m#PUBLIC > From:%-10s Msg:%s\e[0m\n",msg.fromUser,msg.message);
                break;*/
            case PRIVATE://接收消息
                if(clientList[g_locate].chatnow==0){
                    printf("\e[31m#聊天> 发件人:%-10s \n信息内容:%s\e[0m\n",msg.fromUser,msg.message);
                    strcpy(file,"文件来自：");
                    strcat(file,msg.fromUser);
                    strcat(file,"   信息内容：");
                    strcat(file,msg.message);
                    strcat(file,"\n");
                    if((stream = fopen("recond","a"))==NULL)
                    {
                        perror("书写文件失败");
                        exit(3);
                    }
                    fputs(file,stream);
                    fclose(stream);
                    break;
                  //  printf("sorry, the user don't want chat with you.");
                    break;
                }else{
                    if(msg.sendUserLocate==g_locate)
                    {
                        printf("\e[31m#聊天> 发件人:%-10s \n信息内容:%s\e[0m\n",msg.fromUser,msg.message);
                        strcpy(file,"文件来自：");
                        strcat(file,msg.fromUser);
                        strcat(file,"   信息内容：");
                        strcat(file,msg.message);
                        strcat(file,"\n");
                        if((stream = fopen("recond","a"))==NULL)
                        {
                            perror("书写文件失败");
                            exit(3);
                        }
                        fputs(file,stream);
                        fclose(stream);
                        break;
                    }else{
                        msg.type=PRIVATE;//消息类型为进入服务器类型
                        msg.fromUserLocate=g_locate;//消息发件人为自己
                        strcpy(msg.fromUser,g_name);//把发件人的名字设置成自己
                        strcpy(msg.message,lose);//把发件人的信息也改成自己的名字
                        write(clientList[fd1].socketFd,&msg,sizeof(msg));
                    }
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
    MESSAGE msg;//创建一个信息的结构体
    msg.type=LOGIN;//消息类型为进入服务器类型
    msg.fromUserLocate=g_locate;//消息发件人为自己
    strcpy(msg.fromUser,g_name);//把发件人的名字设置成自己
    strcpy(msg.message,g_name);//把发件人的信息也改成自己的名字
    write(fd,&msg,sizeof(msg));//传送给服务器，让服务器通知各个客户端我已经进入聊天室
    
    int tmp[MAX_CLIENT];//建立int类型的数组
    int  key;//建立int类型的key来存放选项
    int m;
    while(1)
    {
        printf("请选择你要使用的功能：\n");
        printf(" \t1 进行聊天\n  \t2 查看聊天记录\n \t3 退出聊天\n \t4 用户列表\n");
        if(1!= scanf("%d",&key))
        {
            key=0;
            flush();
        }//输入选择
        bzero(&msg,sizeof(msg));//把信息清空
        strcpy(msg.fromUser,g_name);//信息发件人是自己
        msg.fromUserLocate=g_locate;//信息发件人定位是自己的定位
        if(key==2){
            int a;
            if((stream = fopen("recond","r"))==NULL)
            {
                perror("读取文件失败");
                exit(3);
            }
            while(1){
                if(fgets(buf,100,stream) == 0){
                    if(feof(stream)){
                        printf("文件结束了。\n");
                        break;
                    }else{
                        perror("读取失败");
                        break;
                    }
                }
                fputs(buf,stdout);
                printf("是否继续读取？ 1继续 2停止");
                scanf("%d",&a);
                if(a==2) break;
                else if(a==1) continue;
                else printf("抱歉，没有这个选项。\n");
            }
            fclose(stream);
            
        }else if(key==1){//进行聊天
            m=MakeTempList(tmp);
            if(m!=-1)
            {
                msg.clientList[g_locate].chatnow=m+1;//把我的聊天状态改为忙
                clientList[g_locate].chatnow=m+1;//我的状态忙碌
                //msg.clientList[m].chatnow=g_locate;//把我的聊天状态改为忙
                //clientList[m].chatnow=g_locate;
                msg.sendUserLocate=m;//发送信息对象定位确定
                msg.type=PRIVATE;//信息类型确定
                
                printf("\n可以开始聊天了\n输入 q 就代表着你要结束这次的聊天\n\n");
                flush();
                
                while(1){//输入消息内容
                    fgets(msg.message,sizeof(msg.message),stdin);
                    msg.message[strlen(msg.message)-1]='\0';
                    if(strcmp(msg.message,"q")==0){//如果输入q则退出聊天
                        clientList[g_locate].chatnow=0;
                        //msg.clientList[m].chatnow=0;//把我的聊天状态改为忙
                        //clientList[m].chatnow=0;
                        msg.clientList[g_locate].chatnow=0;
                        strcpy(file,"\n");
                        if((stream = fopen("recond","a"))==NULL)
                        {
                            perror("书写文件失败");
                            exit(3);
                        }
                        fputs(file,stream);
                        fclose(stream);
                        break;
                    }
                    strcpy(file,"文件发给：");
                    strcat(file,msg.fromUser);
                    strcat(file,"   信息内容：");
                    strcat(file,msg.message);
                    strcat(file,"\n");
                    if((stream = fopen("recond","a"))==NULL)
                    {
                        perror("书写文件失败");
                        exit(3);
                    }
                    fputs(file,stream);
                    fclose(stream);
                    write(fd,&msg,sizeof(msg));//把消息写入fd接口
                }
                bzero(tmp,sizeof(tmp));
            }
        }else if(key==3){
            printf("EXIT \n");
            msg.type=EXIT;
            strcpy(msg.message,"再见");
            write(fd,&msg,sizeof(msg));
        }else if(key==4){
            ShowList();
        }else{
            printf("这个选项错误 \n");
            msg.type=0;
        }

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
    strcpy(lose,"抱歉，现在该用户正在忙。\n");
    char ip[20];//="192.168.43.31";
    printf("请输入你要连接服务器的ip地址：\n");scanf("%s",ip);
    struct sockaddr_in addr;
    //初始化addr的信息
    addr.sin_port=htons(PORT);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(ip);
    if(-1==(fd=socket(AF_INET,SOCK_STREAM,0)))//fd为套接字的接口
    {
        perror("socket error/建立套接字失败");
        exit(1);
        
    }
    if(-1==(connect(fd,(struct sockaddr*)&addr,sizeof(struct
        sockaddr))))//把套接字的接口与地址相连接
    {
        perror("connect error/连接失败");
        exit(2);
        
    }
    if((stream = fopen("recond","w"))==NULL)
    {
        perror("创建文件失败");
        exit(3);
    }
    fclose(stream);
    MESSAGE msg;//创建一个信息的结构体

    read(fd,&msg,sizeof(msg));//从fd接口处读取信息保存到msg中
    if(msg.type==EXIT)//如果msg.type为EXIT类型，则表明连接失败
    {
        printf("service refuse connect/服务连接失败 \n");
        exit(1);
        
    }
    else
    {//如果连接成功，则把msg。clientList中的信息复制到clientList中
        memcpy(&clientList,&msg.clientList,sizeof(clientList));
        g_locate=msg.fromUserLocate;
        pthread_create(&tid1,NULL,RecvMsg,(void *)&fd);//运行接收消息的线程
        do{
            printf("请输入你的名字：\n");scanf("%s",g_name);
            
        }while(CheckExist());
        SendMsg(fd);//发送消息
        pthread_join(tid1,NULL);//结束接收消息线程
        
    }
    return 0;
}

