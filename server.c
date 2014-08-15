/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:chat tool  
 *
 *        Version:  1.0
 *        Created:  08/07/2014 03:43:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Shreck
 *      
 *
 * =====================================================================================
 */

#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <memory.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define  SERV_PORT             3333
#define  LISTEN_LEN            10
#define  INVALID_USERINFO      'n'
#define  VALID_USERINFO        'y'


struct info
{
char  user_name[20];
char  user_password[20];
int   flag;
int   conn_fp;     //保存新登陆用户的套接字       
}USER[50];

struct system_info
{
	char happen[50];
	char time[20];
};



struct system_info    msg1;

char                  MSG[128];
int                   k = 0;
int                   FLAG=0;
//自定义错误处理函数
void my_err(const char *err_string,int line)
{
	fprintf(stderr,"line:%d",line);
	perror(err_string);
	exit(1);
}

//验证登陆用户信息
int check_user(char *name,char *password,int conn_fd)
{
	int i = 0;
	
	while(1)
	{
		
		if(strcmp(USER[i].user_name,name) == 0)
		{
			if(strcmp(USER[i].user_password,password) == 0)
			{
			     USER[i].conn_fp = conn_fd;
			      return 1;
			}
		}
		i++;
	}

	return 0;
}

//申请帐号
void apply_account(int conn_fd)
{
	char recv_buf[20];
	static int  k=0;
	int  ret;
	int  fd;
	char s1[20]="^";
	time_t now;
	struct tm *timenow;
	time(&now);
	timenow = localtime(&now);
	
	ret = recv(conn_fd,recv_buf,sizeof(recv_buf),0);
	recv_buf[ret] = '\0';
	strcpy(USER[k].user_name,recv_buf);
	memset(recv_buf,0,sizeof(recv_buf));
	ret = recv(conn_fd,recv_buf,sizeof(recv_buf),0);
	recv_buf[ret] = '\0';
	strcpy(USER[k].user_password,recv_buf);
	send(conn_fd,"y",1,0);
	
	printf("%s申请了帐号\n",USER[k].user_name);
	strcat(s1,USER[k].user_name);
	strcpy(msg1.happen,s1);
	strcpy(msg1.time,asctime(timenow));
	
	fd = open("/home/shreck/c/system_record.txt",O_WRONLY | O_APPEND);    //将申请信息写入日志
	write(fd,&msg1,sizeof(msg1));
	memset(&msg1,0,sizeof(msg1));
	close(fd);
	printf("%s\n",asctime(timenow));
	USER[k].flag = 1;
	k++;
}



void check_userinfo(int conn_fd)             //验证用户信息
{
	char        recv_buf[128];
	char        name[20];
	char        password[20];
	int         ret;
	time_t      now;
	struct tm * timenow;
	int         fd;
	char        s2[20]="&";
	static int  i=0;
	time(&now);
	timenow = localtime(&now);
	

	while(1)
	{
		printf("aaa\n");
		ret = recv(conn_fd,recv_buf,sizeof(recv_buf),0);
		printf("bbb\n");
		recv_buf[ret]= '\0';
		strcpy(name,recv_buf);
		
		memset(recv_buf,0,sizeof(recv_buf));
		ret = recv(conn_fd,recv_buf,sizeof(recv_buf),0);
		recv_buf[ret] = '\0';
		strcpy(password,recv_buf);
		USER[i].flag = 1;
		i++;
		if(check_user(name,password,conn_fd) == 1)     //验证成功
		{
		
			send(conn_fd,"y",1,0);
			printf("%s链接到了服务器\n",name);
			printf("%s\n",asctime(timenow));
			strcat(s2,name);
			strcpy(msg1.happen,s2);
			strcpy(msg1.time,asctime(timenow));
			
			fd = open("/home/shreck/c/system_record.txt",O_WRONLY | O_APPEND);   //存入日志
			write(fd,&msg1,sizeof(msg1));
			memset(&msg1,0,sizeof(msg1));
			close(fd);
			break;
		}
		else
		{
			send(conn_fd,"n",1,0);
		}
	}
		
}


void *send_massage(int conn_fd)
{
	int          ret;
	int          i,j;
	int          conn;               //用来保存待接收信息人的套接字
	char         name[20];
	char         name_recv[200];    //存接收信息的那个人的名字
	char         send_msg[200];
	time_t       now;
	char         s3[20]="*";
	struct tm    *timenow;
	int          fd;
	time(&now);
	timenow = localtime(&now); 
	
	
	for(i=0;USER[i].flag != 0;i++)          //向客户端发送在线用户
	{
		if(USER[i].conn_fp == conn_fd)
		{
			strcpy(name,USER[i].user_name);
			continue;
		}
		if(USER[i].conn_fp < 0);
		else
		{
			
			send(conn_fd,USER[i].user_name,strlen(USER[i].user_name),0);
			usleep(10);
		}
	}
	usleep(10);
	send(conn_fd,"over",strlen("over"),0);             //over代表用户已发完
	
	while(1)
	{
		ret = recv(conn_fd,MSG,sizeof(MSG),0);    //接收客户端消息
		if(ret < 0)
		{
			my_err("recv",__LINE__);
		}
	
		MSG[ret] = '\0';
		if(strcmp(MSG,"exit") == 0)
		{
			printf("用户%s结束本次会话\n",name);
			printf("%s\n",asctime(timenow));
			strcat(s3,name);
			strcpy(msg1.happen,s3);
			strcpy(msg1.time,asctime(timenow));
			fd = open("/home/shreck/c/system_record.txt",O_WRONLY,O_APPEND);
			write(fd,&msg1,sizeof(msg1));
			memset(&msg1,0,sizeof(msg1));
			close(fd);
			close(conn_fd);
			exit(0);
			
		}
		if(MSG[0] == '@')                                          //判断是否为私聊
		{
			for(j=1,i=0;MSG[j] != ':';j++,i++)
			{
				name_recv[i] = MSG[j];                     //将接收人的姓名提取出来
			}
			for(i=0;USER[i].flag != 0;i++)
			{
	
				if(strcmp(USER[i].user_name,name_recv) == 0)
				{
					conn = USER[i].conn_fp;                     //将结构体中带发送人的名字所对应的套接字赋给conn
					break;
				}
		
			}
			for(i=0;USER[i].flag != 0;i++)
			{
				if(conn_fd == USER[i].conn_fp)
				{
					break;
				}
			}

			strcpy(send_msg,USER[i].user_name);
			strcat(send_msg,":");
			strcat(send_msg,MSG);
			send(conn,send_msg,strlen(send_msg),0);
			memset(MSG,0,sizeof(MSG));
			memset(send_msg,0,sizeof(send_msg));
		}
		else                                                               //群聊
		{
				for(j=0;USER[j].flag != 0;j++)
				{
					if(conn_fd == USER[j].conn_fp)
					{
						
						break;                            //检索当前套接字所对应的结构体，以获得他的用户名
					}
				}

				memset(name_recv,0,sizeof(name_recv));
				strcat(name_recv,USER[j].user_name);
				strcat(name_recv,":");
				strcat(name_recv,MSG);
				
				

			
			for(i=0;USER[i].flag != 0;i++)
			{
				if(USER[i].conn_fp == conn_fd)                  //将本人除外
				{
					continue;
				}
				conn = USER[i].conn_fp;
				send(conn,name_recv,sizeof(name_recv),0);
				
			}
			memset(MSG,0,sizeof(MSG));
			memset(name_recv,0,sizeof(name_recv));

		}
	}
}

void *control(void)                     //输出系统日志或退出服务端
{
	char  str[20];
	int fd;
	int ret;
	while(1)
	{
		scanf("%s",str);
		printf("\t\t====================================\n");
		printf("\t\t\t系统日志\n");
		printf("\t\t====================================\n");
		if(strcmp(str,"exit") == 0)
		{
			FLAG = 1;
			break;
		}
		
	
		else if(strcmp(str,"system_record") == 0)
		{
			fd = open("/home/shreck/c/system_record.txt",O_RDONLY);
			while(1)
			{
				memset(&msg1,0,sizeof(msg1));
				ret = read(fd,&msg1,sizeof(msg1));
				
				if(ret == 0)
				{
					break;
				}
	
				if(msg1.happen[0] == '^')
				{
					printf("%s申请了新帐号\n",msg1.happen);
					printf("%s\n",msg1.time);
				}
				else if(msg1.happen[0] == '&')
				{
					printf("%s连接到了服务器\n",msg1.happen);
					printf("%s\n",msg1.time);
				}
				else
				{
					printf("%s与服务器断开链接\n",msg1.happen);
					printf("%s\n",msg1.time);
				}
			}
		}
	}
	
		exit(0);
	
}




int main(int argc, char *argv[])
{
	int           sock_fd,conn_fd;
	int           optval;
	int           ret;
	char          recv_buf[128];
	int 	      i=0,j;
	pthread_t     thid,thid1;
	socklen_t     cli_len;
	int           fd;
	struct        sockaddr_in cli_addr,serv_addr;

//	creat("/home/shreck/c/system_record.txt",S_IRWXU);
//	creat("/home/shreck/c/system_user.txt",S_IRWXU);
//	creat("/home/shreck/c/system_error.txt",S_IRWXU);
	/*fd = open("/home/shreck/c/system_user.txt",O_RDONLY);
	for(j=0;;j++)
	{
		ret = read(fd,&USER[j],sizeof(USER[j]));
		printf("%d\n",ret);
		if(ret < 0)
		{
			break;
		}
	}
	close(fd);*/
	for(j=0;j<20;j++)         //将结构体所有成员的套接字初始化成-1
	{
		USER[i].flag = 0;
		USER[i].conn_fp = -1;
	}
	
	
	//创建一个套接字
	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	if(sock_fd < 0)
	{
		my_err("socket",__LINE__);
	}

	//设置该套接字使之可以重新绑定端口
	optval = 1;
	if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,(void *)&optval,sizeof(int)) < 0)
	{
		my_err("setsockopt",__LINE__);
	}

	//初始化服务器端地址结构
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//将套接字绑定到本地端口
	if(bind(sock_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in)) < 0)
	{
		my_err("bind",__LINE__);
	}

	//将套接字转换为监听套接字
	if(listen(sock_fd,LISTEN_LEN) < 0)
	{
		my_err("listen",__LINE__);
	}

	cli_len = sizeof(struct sockaddr_in);
	pthread_create(&thid1,NULL,(void *)control,NULL);
	while(1)
	{
		//通过accept接收客户端连接请求
		printf("444\n");
		conn_fd = accept(sock_fd,(struct sockaddr *)&cli_addr,&cli_len);
		if(conn_fd < 0)
		{
			my_err("accept",__LINE__);
		}
		printf("accept a new client ,ip:%s\n",inet_ntoa(cli_addr.sin_addr));
		
		
		do
		{
			memset(recv_buf,0,sizeof(recv_buf));
			ret = recv(conn_fd,recv_buf,sizeof(recv_buf),0);     
			if(ret < 0)
			{
				my_err("recv",__LINE__);
			}
			recv_buf[ret] = '\0';
			

			if(strcmp(recv_buf,"longin") ==0 )         //判断用户是否为登陆
			{
				 check_userinfo(conn_fd);
			}
			else if(strcmp(recv_buf,"reg") == 0)       //判断用户是否为注册
			{
				apply_account(conn_fd);

			}
			else
			{
				close(conn_fd);
			}
			
			
		}while(strcmp(recv_buf,"reg") == 0);

		printf("3333\n");
		pthread_create(&thid,NULL,(void *)send_massage,(void *)conn_fd);
		if(FLAG == 1)                           //判断是否退出服务器的标志变量
		{
			break;
		}
		printf("222\n");
		
	}
	printf("111\n");
	fd = open("/home/shreck/c/system_user.txt",O_WRONLY | O_APPEND);        //将用户结构体写入文件
	for(j=0;USER[j].flag != 0;j++)
	{
		write(fd,&USER[j],sizeof(USER[j]));
	}
	close(fd);
	

	return EXIT_SUCCESS;
}

