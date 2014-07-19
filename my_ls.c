/*
 * =====================================================================================
 *
 *       Filename:  my_ls.c
 *
 *   	 Description: ls的简单命令实现 
 *
 *        Version:  1.0
 *        Created:  07/16/2014 05:06:19 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hurley (miaoshuai)
 *        
 *
 * =====================================================================================
 */

#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<linux/limits.h>
#include<dirent.h>
#include<grp.h>
#include<pwd.h>
#include<errno.h>

#define PARAM_NONE 0
#define PARAM_L 1
#define PARAM_A 2
#define PARAM_R 3
#define MAXROWLEN 80

int g_leave_len = MAXROWLEN;    //初始化的剩余长度
int g_maxlen;                    //默认为0
int N;
void my_err(const char *err_string,int line)
{
fprintf(stderr,"line:%d",line);
perror(err_string);
exit(1);
}

void display_attribute(char *name)     
{
char buf_time[40];
struct stat buf;
struct passwd *usr;    //从该结构体中获取文件所有这的用户名
struct group *grp;     //从该结构体中获取文件所有者所属组的组名
char name_wj[30];
int i,j;
//获取文件类型
if(stat(name,&buf) == -1)
{
my_err("stat",__LINE__);
}
if(S_ISLNK(buf.st_mode))
      printf("l");
else if(S_ISREG(buf.st_mode))
      printf("-");
else if(S_ISDIR(buf.st_mode))
      printf("d");
else if(S_ISCHR(buf.st_mode))
      printf("c");
else if(S_ISBLK(buf.st_mode))
      printf("b");
else if(S_ISFIFO(buf.st_mode))
      printf("f");
else if(S_ISSOCK(buf.st_mode))
      printf("s");

//获取文件所有者的权限
	if(buf.st_mode & S_IRUSR)
	      printf("r");
	else
	      printf("-");
	if(buf.st_mode & S_IWUSR)
	      printf("w");
	else
	      printf("-");
	if(buf.st_mode & S_IXUSR)
	      printf("x");
	else
	      printf("-");
	if(buf.st_mode & S_IRGRP)
	      printf("r");
	else
	      printf("-");
	if(buf.st_mode & S_IWGRP)
	      printf("w");
	else
	      printf("-");
	if(buf.st_mode & S_IXGRP)
	      printf("x");
	else
	      printf("-");
	if(buf.st_mode & S_IROTH)
	      printf("r");
	else
	      printf("-");
	if(buf.st_mode & S_IWOTH)
	      printf("w");
	else
	      printf("-");
	if(buf.st_mode & S_IXOTH)
	      printf("x");
	else
	      printf("-");
	printf(" ");
	printf("%4lu ",buf.st_nlink);  		//打印文件的链接数

	//通过uid与gid获取文件所有者的用户名和组名
	usr=getpwuid(buf.st_uid);
	grp=getgrgid(buf.st_gid);
	printf("%-8s",usr->pw_name);           //打印文件的用户名
	printf("%-8s",grp->gr_name);           //打印用户名所属的组名
	printf("%6lu ",buf.st_size);             //打印文件的大小
	strcpy(buf_time,ctime(&buf.st_atime));
	buf_time[strlen(buf_time)-1] = '\0';     //换掉字符串自带的\n
	printf("%s",buf_time);

		for(i=0,j=0;i<strlen(name);i++)
		{
			if(name[i] == '/')
			      
			{
			j=0;
			continue;
			}
			name_wj[j++] = name[i];
				 
		}
	name_wj[j]='\0';
	printf("  %s\n",name_wj);
}


//在无参下打印文件名，要求上下行之间要对齐
void display_single(char *name)
{
int i,len;
	if((g_maxlen) > g_leave_len)
	{
	printf("\n%s",name);
	len=g_maxlen - strlen(name);
	for(i=0;i<len;i++)
	{
	printf(" ");
	}
	g_leave_len = MAXROWLEN - strlen(name) - 2-len;
	}
	else if(N == 1)
	{
	printf("%s",name);
	len=g_maxlen - strlen(name);
		for(i=0;i<len;i++)
		      printf(" ");
		g_leave_len =g_leave_len - strlen(name) - 2-len;

	}
	else
	{
	printf("  %s",name);
	len=g_maxlen - strlen(name);
		for(i=0;i<len;i++)
		      printf(" ");
		g_leave_len =g_leave_len - strlen(name) - 2-len;
	}

	
}

void display_dir(int param_flag, char *path)
{
DIR *dir;
struct dirent *ptr;
struct stat buf;
int count=0;
char filename[256][256],temp[256];
char filename1[256][256];    //用来记录文件的路径
int i=0,j;


	//计算该目录下总的文件数，和找出字符最多的文件名
	if((dir=opendir(path)) == NULL)
	{
	my_err("opendir",__LINE__);
	exit;
	}
	
	while((ptr=readdir(dir)) != NULL)
	{
	count++;
		if(g_maxlen < strlen(ptr->d_name))
		      g_maxlen=strlen(ptr->d_name);
		strcpy(filename[i],ptr->d_name);
		filename[i][strlen(ptr->d_name)]='\0';
		i++;
		//printf("\nptr->d_name:%s\n",ptr->d_name);
		//printf("\nfilename:%s\n",filename[i]);
	}

	//排序
	for(i=0;i<count - 1;i++)
	{
		for(j=0;j<count-i-1;j++)
		{
			if(('A' < filename[j][0] <'Z') && ('A' < filename[j][0] < 'Z'))
			{
				if((filename[j][0] > filename[j+1][0]))
				{
				strcpy(temp,filename[j]);
				temp[strlen(filename[j])]='\0';
				strcpy(filename[j],filename[j+1]);
				filename[j][strlen(filename[j+1])]='\0';
				strcpy(filename[j+1],temp);
				filename[j+1][strlen(temp)]='\0';
				}
			}
			else if((filename[j][0] < 'a') && filename[j+1][0] > 'Z')
			{
				filename[j+1][0]=filename[j+1][0]-32;
				if((filename[j][0] > filename[j+1][0]))
				{
				filename[j+1][0]=filename[j+1][0]+32;
				strcpy(temp,filename[j]);
				temp[strlen(filename[j])]='\0';
				strcpy(filename[j],filename[j+1]);
				filename[j][strlen(filename[j+1])]='\0';
				strcpy(filename[j+1],temp);
				filename[j+1][strlen(temp)]='\0';
	
				}
			}

			else if((filename[j][0] > 'Z') && (filename[j+1][0] <'a'))
			{
				filename[j][0]=filename[j][0]-32;
				if((filename[j][0] > filename[j+1][0]))
				{
				filename[j][0]=filename[j][0]+32;
				strcpy(temp,filename[j]);
				temp[strlen(filename[j])]='\0';
				strcpy(filename[j],filename[j+1]);
				filename[j][strlen(filename[j+1])]='\0';
				strcpy(filename[j+1],temp);
				filename[j+1][strlen(temp)]='\0';
				}
			}

			else
			{
				if((filename[j][0] > filename[j+1][0]))
				{
				strcpy(temp,filename[j]);
				temp[strlen(filename[j])]='\0';
				strcpy(filename[j],filename[j+1]);
				filename[j][strlen(filename[j+1])]='\0';
				strcpy(filename[j+1],temp);
				filename[j+1][strlen(temp)]='\0';
				}

			}

		}
	}
	
	if(param_flag == 0)         //ls无参
	{
	N++;
	for(i=0;i<count;i++)
	{
	if(filename[i][0] != '.' )
	
	display_single(filename[i]);
	
	
	}

	printf("\n");
	}


	else if(param_flag == 2)      //-a
	{
		for(i=0;i<count;i++)
		{
			//printf("%s\n",filename[i]);
		display_single(filename[i]);
		}
		printf("\n");
		
	}

	
	else if(param_flag == 1)     //-l
	{
		
		for(i=0;i<count;i++)
		{
	
		if(filename[i][0] != '.')
		{
		strcpy(filename[0],path);
		
		strcat(path,filename[i]);
		
		display_attribute(path);
		strcpy(path,filename[0]);
		}
		}
		printf("\n");
	}

	else                                //-R
	{
		display_dir(0,path);
		printf("\n\n");
		for(i=0;i<count;i++)
		{
		if(filename[i][0] != '.')                     //获得每个文件名的完整路径
		{
		     // printf("%s:\n",filename[i]);
		strcpy(temp,path);
		strcat(path,"/");
		strcat(path,filename[i]);
		
		if(stat(path,&buf) == 0)
		{
					
			if(S_ISDIR(buf.st_mode))
			{
			printf("./%s:\n",path);
			
			display_dir(3,path);
			
			
			}

		
			   
		}
		
		strcpy(path,temp);
		}
		}
	}
	



}





int main(int argc, char *argv[])
{

int flag_param;
	if(argc < 3)
	{
	display_dir(PARAM_NONE,argv[1]);
	}
	else if(argc == 3)
	{
		switch(argv[2][1])
		{
			case 'l':display_dir(PARAM_L,argv[1]);break;
			case 'a':display_dir(PARAM_A,argv[1]);break;
			case 'R':display_dir(PARAM_R,argv[1]);break;
			default:printf("your input is error!");
		}
	}

	
	
	return EXIT_SUCCESS;
}

