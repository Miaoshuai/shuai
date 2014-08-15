/*
 * =====================================================================================
 *
 *       Filename:  cc.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/09/2014 02:38:31 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         
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
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <termios.h> 
#include <sys/stat.h>
#include <fcntl.h>

#define ECHOFLAGS (ECHO | ECHOE | ECHOK | ECHONL) 
#define INVALID    'n'
#define VALID      'y'

char              NAME[20];                           //保存发送人的姓名
char              NAME_RECV[20];                      //保存接收人的姓名
char              USER_ID[20];
pthread_mutex_t   mutex;

struct record
{
	char str1[200];
	char str2[50];
};
struct record recd;
void my_err(const char *err_string,int line)
{
	fprintf(stderr,"line:%d",line);
	perror(err_string);
	exit(1);
}

void print_record(char *user_name)
{
	int fd;
	int ret;
	char name[20];
	char str3[200];
	int i,j;
	fd = open("/home/shreck/c/record.txt",O_RDONLY);
	
	while(1)
	{
		ret = read(fd,&recd,sizeof(struct record));
	
		if(ret == 0)
		{
			break;
		}
		
		for(i=0;recd.str1[i] != ' ';i++)
		{
			name[i] = recd.str1[i];
		}
		name[i] = '\0';
		for(i = i+1,j = 0;recd.str1[i] != '\0';i++,j++)
		{
			str3[j] = recd.str1[i]; 
		}
		str3[j] = '\0';
		if(strcmp(name,user_name) == 0)
		{
			printf("%s\n",str3);
			printf("%s\n",recd.str2);
			memset(&recd,0,sizeof(struct record));
		}
	}
	close(fd);
}

int set_disp_mode(int fd,int option) //关掉系统回写
{  
   	int err;  
  	 struct termios term;  
  	 if(tcgetattr(fd,&term)==-1)
 	  {  
      	 	 perror("Cannot get the attribution of the terminal");  
       		 return 1;  
  	  }  
   	 if(option)  
       		 term.c_lflag|=ECHOFLAGS;  
  	 else  
      		  term.c_lflag &=~ECHOFLAGS;  
  	 err=tcsetattr(fd,TCSAFLUSH,&term);  
   	 if(err==-1 && err==EINTR)
	 {  
         	  perror("Cannot set the attribution of the terminal");  
          	 return 1;  
     	 }  
   return 0;  
}

void online_user(int conn_fd)
{
	char name[20];
	int ret;
	int i,j;
	for(i=0;;i++)
	{
		
		ret = recv(conn_fd,name,sizeof(name),0);
		name[ret] = '\0';
		
		if(strcmp(name,"over") == 0)
		{
			if(i == 0)
			      printf("无在线用户\n");
			break;
		}

		
		else
		{
			printf("%s  ",name);
			for(j=0;j<10-strlen(name);j++)
			{
				printf(" ");
			}
		}
		if((i+1)%3 == 0)
		{
			printf("\n");
		}
		memset(name,0,sizeof(name));
	}
	printf("\nwelcome to weechat!\n");
}

void check_userinfo(int conn_fd)
{
	int ret;
	char password[20];
	char recv_buf[20];
	while(1)
	{
		
		printf("请输入您的用户名:");
		scanf("%s",NAME);
		if(send(conn_fd,NAME,strlen(NAME),0) < 0)
		{
			my_err("send",__LINE__);
		}
		set_disp_mode(STDIN_FILENO,0);  
		printf("请输入您的密码:");
		scanf("%s",password);
		set_disp_mode(STDIN_FILENO,1); 
		if(send(conn_fd,password,strlen(password),0) < 0)
		{
			my_err("send",__LINE__);
		}
	
		ret = recv(conn_fd,recv_buf,sizeof(recv_buf),0);
		if(ret < 0)
		{
			my_err("recv",__LINE__);
		}
		recv_buf[ret] = '\0';
		
	
		if(recv_buf[0] != 'y')
		{
			printf("your username or  password is not correct!\n");
			
		}
		else
		{
			strcpy(USER_ID,NAME);
			printf("\nlongin succeed!\n");
			printf("\t\t\tonline user\n\n");
			online_user(conn_fd);
			break;
		}
		
		

	}
}

void reg(int conn_fd)
{
	char name[20];
	char password[20];
	char recv_buf[128];
	
	printf("\t\t\t新用户注册\n\n");
	printf("username:");
	scanf("%s",name);
	set_disp_mode(STDIN_FILENO,0);
	send(conn_fd,name,strlen(name),0);
	printf("password:");
	scanf("%s",password);
	set_disp_mode(STDIN_FILENO,1);
	send(conn_fd,password,strlen(password),0);
	recv(conn_fd,recv_buf,sizeof(recv_buf),0);
		
	if(recv_buf[0] == 'y')
	{
		printf("\n注册成功!\n");
	}
	else
	{
		printf("注册失败!\n");
	}
}


void menu(int conn_fd)
{
	int x;
	char buf1[20]="longin";
	char buf2[20]="reg";
	do
	{
	
		printf("\t\t\t聊天室\n");
		printf("1.登陆\n");
		printf("2.注册新用户\n");
		printf("3.退出\n");
		scanf("%d",&x);
		switch(x)
		{
			case 1:
			       send(conn_fd,buf1,strlen(buf1),0);
			       check_userinfo(conn_fd);
			       break;
			case 2:
			       send(conn_fd,buf2,strlen(buf2),0);
			       reg(conn_fd);
			       break;
			case 3:exit(0);break;
			default:printf("您的操作有误请重新输入");
		}

	}while(x != 1);
}





void send_msg(int conn_fd)
{
	char   msg[200];
	time_t now;
	struct tm *timenow;
	int fd;
	char  name[20];
	char user_name[20];
	time(&now);                             	
	timenow = localtime(&now);
	strcpy(user_name,USER_ID);
	
	while(1)
	{
		
		while(1)
		{
		scanf("%s",msg);
		if(strlen(msg)>sizeof(msg))
		{
		      printf("数据过长，请重新输入\n");
		}
		else
		      break;
		}
		if(strcmp(msg,"record") == 0)
		{
			printf("\t\t======================\n");
			printf("\n\t\t\t聊天记录\n");
			printf("\t\t======================\n\n");
			print_record(user_name);
		}
		else if(strcmp(msg,"exit") == 0)
		{
			send(conn_fd,msg,strlen(msg),0);
			close(conn_fd);
			exit(0);
		}
		else
		{
		printf("%s\n",asctime(timenow));
		}
		pthread_mutex_lock(&mutex);
		
		strcpy(name,user_name);
		strcat(user_name," ");
		strcat(user_name,msg);
		strcpy(recd.str1,user_name);
		memset(user_name,0,sizeof(user_name));
		strcpy(user_name,name);
		memset(name,0,sizeof(name));
		strcpy(recd.str2,asctime(timenow));

		fd = open("/home/shreck/c/record.txt",O_APPEND | O_WRONLY);
		write(fd,&recd,sizeof(struct record));
		memset(&recd,0,sizeof(recd));
		close(fd);
		pthread_mutex_unlock(&mutex);
		if(send(conn_fd,msg,strlen(msg),0) < 0)
		{
			my_err("send",__LINE__);
		}
	}
		
	

}

void *recv_msg(int conn_fd)
{
	int      ret;
	char     recv_buf[200];    //用来接收数据
	time_t   now;
	struct   tm *timenow;
	int      fd;
	char     name[20];
	char     user_name[20];
	strcpy(user_name,USER_ID);
	time(&now);
	timenow = localtime(&now);

	
	while(1)
	{
		ret = recv(conn_fd,recv_buf,sizeof(recv_buf),0);
                recv_buf[ret] = '\0';
		pthread_mutex_lock(&mutex);
		strcpy(name,user_name);
		strcat(user_name," ");
		strcat(user_name,recv_buf);
		strcpy(recd.str1,user_name);
		memset(user_name,0,sizeof(user_name));
		strcpy(user_name,name);
		memset(name,0,sizeof(name));
		strcpy(recd.str2,asctime(timenow));


		
		
		
		fd = open("/home/shreck/c/record.txt",O_APPEND | O_WRONLY);
		write(fd,&recd,sizeof(struct record));
		memset(&recd,0,sizeof(recd));
		close(fd);

		pthread_mutex_unlock(&mutex);
		printf("%s\n",recv_buf);
		printf("%s\n",asctime(timenow));	
		memset(recv_buf,0,sizeof(recv_buf));
		
	}
}

int main(int argc, char *argv[])
{

	int                   i;
	int                   ret;
	int                   conn_fd;
	int                   serv_port;
	pthread_t             thid1,thid2;
	struct sockaddr_in    serv_addr;
	char                  recv_buf[128];
	fd_set                readfds;
	struct timeval        timeout;
	creat("/home/shreck/c/record.txt",S_IRWXU);
	pthread_mutex_init(&mutex,NULL);


	//初始化服务器端地址结构
	
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	
	//从命令行获得服务器端的端口与地址
	
	for(i = 1;i < argc;i++ )
	{
		if(strcmp(argv[1],"-p") != 0)
		{
			printf("您输入的参数有误\n");
		}

		else
		{
			serv_port = atoi(argv[2]);
		}

		if(serv_port < 0 || serv_port > 65535)
		{
			printf("您输入的端口号无效\n");
		}
		
		else
		{
			serv_addr.sin_port = htons(serv_port);
		}
		if(strcmp(argv[3],"-a") != 0)
		{
			printf("您的输入有误\n");
		}

		else
		{
			if(inet_aton(argv[4],&serv_addr.sin_addr) == 0)
			{
				printf("invalid server ip address\n");
			}
		}
	}

	//创建套接字
	
	conn_fd = socket(AF_INET,SOCK_STREAM,0);
	if(conn_fd < 0)
	{
		my_err("socket",__LINE__);
	}

	//建立连接
	if(connect(conn_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) < 0)
	{
		my_err("connect",__LINE__);
	}

	menu(conn_fd);
	pthread_create(&thid1,NULL,(void *)recv_msg,(void *)conn_fd);
	send_msg(conn_fd);	
	return 0;
}

