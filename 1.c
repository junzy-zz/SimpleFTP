#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <regex.h>
pid_t pid;
struct files
{
	char name[100];
	char size[100];
	char date[100];
	char type[10];
};
void becomes_client();
void becomes_server();

void FileDownload(char buf[],int *socketfd)
{
	char *pch,b[100];
	char tempo[1024];
	strcpy(tempo,buf);
	pch = strtok (tempo," ");
		//printf("%s\n",pch );
	pch = strtok (NULL," ");
	//strcpy(b,pch);
	char temp[100],error[100];
	strcpy(error,"Requested file doesnt exist");
	strcpy(temp,"Shared/");
	strcat(temp,pch);
	FILE *fd;
	if((fd=fopen(temp,"rb"))==NULL)
	{
		printf(" Requested File doesnt exist\n");
		write(*socketfd,error,29);
	}
	else
	{

		char msg[1025];
		bzero(msg,1025);
		int i;
		while((i=fread(msg,sizeof(char),1025,fd))>0)
		{
			if(write(*socketfd,msg,i)<0)
			{
				printf("ERROR: Writing to socket\n");
				break;
			}
			bzero(msg,1025);
		}
		bzero(msg,1025);
		strcpy(msg,"FILE LIST");
		sleep(1);
		if(write(*socketfd,msg,9)<0)
		{
			printf("\nERROR: Writing to socket\n");
			exit(1);
		}
		printf("SENT\n");
	}
//	fclose(fd);
//	return;
}
void IndexGet(char buf[],int *socketfd)
{
	int flag=0;
	char t1[100],t2[100];
	if(strncmp(buf,"IndexGet LongList",17)==0)
		flag=0;
	else if(strncmp(buf,"IndexGet RegEx",14)==0)
		flag=2;
	char priBuff[1025];
	struct tm time1,time2,test;
	FILE *fd,*fx;
	bzero(priBuff,1025);
	struct files arr[100];
	struct stat st;
	int size;
	int i,it=0,f=0;
	char n[100];
	char *pch;
	bzero(n,100);
	DIR *dp;
	struct dirent *ep;     
	dp=opendir ("./Shared");
	if (dp != NULL)
	{
		while(ep = readdir (dp))
		{
			strcpy(n,ep->d_name);
			if(n[0]!='.')
			{
				strcpy(arr[it].name,n);
				it++;
		//		 puts(arr[it-1].name);
			}
		}
		(void)closedir(dp);
	}
	else
		perror ("Couldn't open the directory");
	for(i=0;i<it;i++)
	{
		char tok[100];
		stat(arr[i].name, &st);
		size = st.st_size;
		strcpy(arr[i].date,ctime(&st.st_mtime));
		sprintf(arr[i].size,"%d",size);
		strcpy(tok,arr[i].name);
		pch=strtok(tok,".");
		if(pch!=NULL)
			pch=strtok(NULL,".");
		if(pch!=NULL)
		{
			strcpy(arr[i].type,pch);
		}
	}
	system("touch out");
	fx=fopen("out","w");
	if(fx<0)
		perror("[ERROR opening file descriptor]");
	if(flag==0)
	{
		for(i=0;i<it;i++)
		{
			fprintf(fx,"%s        %s        %s        %s",arr[i].name,arr[i].type,arr[i].size,arr[i].date);
			//printf("%s    %s    %s    %s\n",arr[i].name,arr[i].type,arr[i].size,arr[i].date );
		}
	}
	else if(flag==2)
	{
	//	printf("I M HERE\n");
		regex_t regex;int reti;char msgbuf[100],reg[100];
		pch = strtok (buf," ");
		pch = strtok (NULL," ");
		pch = strtok (NULL," ");
		strcpy(reg,pch);
	//	printf("%s\n",reg );
		reti=regcomp(&regex,reg,0);
		if( reti ){ fprintf(stderr, "Could not compile regex\n"); exit(1); }
		for(i=0;i<it;i++)
		{
		//	printf("!!%s!!\n",arr[i].name);
			reti=regexec(&regex,arr[i].name,0,NULL,0);
			if(!reti)
			{
		//		puts("Match");
				fprintf(fx,"%s        %s        %s        %s",arr[i].name,arr[i].type,arr[i].size,arr[i].date);
			}
		//	else if(reti==REG_NOMATCH)
		//		continue;
		}
		regfree(&regex);
	}
	fclose(fx);
	fd=fopen("out","r");
	//	printf("OutPUT READY\n");
	if(fd<0)
		perror("[ERROR opening file descriptor]");
	while((f=fread(priBuff,sizeof(char),1025,fd))>0)
	{
		//printf("111111\n");
		if(write(*socketfd,priBuff,1025)<0)
		{
			printf("\nERROR: Writing to socket\n");
			break;
		}
		bzero(priBuff,1025);
	}
	bzero(priBuff,1025);
	strcpy(priBuff,"******FILE LIST*******");
	if(write(*socketfd,priBuff,1025)<0)
	{
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
	//system("rm -rf out");
}
void FileDownload2(char buf[],int *socketfd)
{
	int n;
	char tempo[1024];
	strcpy(tempo,buf);
	int flag=0;
	n=write(*socketfd,buf,1025);
	char *pch,name[100];
	pch=strtok(tempo," ");
	pch=strtok(NULL," ");
	strcpy(name,pch);
	//strcat(name,"(Downloaded)");
	if(n<0)
	{
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
	char readBuff[1025];
	bzero(readBuff,1025);
	FILE *fd;
	fd=fopen(name,"wb");
	int f=0,f1=0;unsigned int size=0;
	while((f=read(*socketfd,readBuff,1025))>0)
	{
		if(strcmp(readBuff,"FILE LIST")==0)
			break;
		else if(strcmp(readBuff,"Requested file doesnt exist")==0)
		{
			printf("Requested file doesnt exist\n");
			flag=1;
			break;
		}
		else
			fwrite(readBuff,sizeof(char),f,fd);
	//	printf("%s",readBuff);
		bzero(readBuff,1025);
	}
	if(flag==0)
		printf("Recieved\n");
	else if(flag==1)
	{
		printf("Not Recieved\n");
		char cmd[100];
		strcpy(cmd,"rm -rf ");
		strcat(cmd,name);
		system(cmd);
	}
	fclose(fd);
	return;

}
void FileUpload(char buf[],int *socketfd)
{
	int n;
	char tempo[1024];
	strcpy(tempo,buf);
	char *pch,name[100];
	char dir[100];
	strcpy(dir,"Shared/");
	pch=strtok(tempo," ");
	pch=strtok(NULL," ");
	strcpy(name,pch);

	char readme[1024];
	read(*socketfd,readme,1024);
	puts(readme);
	bzero(readme,1024);
	read(*socketfd,readme,1024);
	puts(readme);
//strcat(name,"(Uploaded)");
	strcat(dir,name);
	char readBuff[1025];
	bzero(readBuff,1025);
	FILE *fd;
	fd=fopen(dir,"wb");
	int f=0,f1=0;unsigned int size=0;
	while((f=read(*socketfd,readBuff,1025))>0)
	{

		if(strcmp(readBuff,"FILE LIST")==0)
			break;
		else
			fwrite(readBuff,sizeof(char),f,fd);
		bzero(readBuff,1025);
	}
	printf("Recieved\n");
	fclose(fd);
	fflush(stdout);
	return;
}
void FileUpload2(char buf[],int *socketfd)
{
	int n;
	n=write(*socketfd,buf,1025);
	char *pch,b[100];
	pch = strtok (buf," ");
		//printf("%s\n",pch );
	pch = strtok (NULL," ");

	strcpy(b,pch);
	FILE *fd;
	fd=fopen(b,"rb");
	if(fd==NULL)
	{
		perror("Error");
		return;
	}
	char *times,*size,*md;
	char temp[1000];
	temp[0]='\0';
	strcpy(temp,"md5sum Shared/");
	strcat(temp,b);
	strcat(temp,"> result");
	system(temp);
	FILE *jun=fopen("result","r");
	char bufff[1024];
	fgets(bufff,1024,jun);
	write(*socketfd,bufff ,1024);
	fclose(jun);
	temp[0]='\0';
	strcpy(temp,"stat Shared/");
	strcat(temp,b);
	strcat(temp," | grep Size | cut -d ' ' -f4 > result1");
	system(temp);
	jun=fopen("result1","r");
	bzero(bufff,1024);
	fgets(bufff,1024,jun);
	write(*socketfd,bufff ,1024);
	char msg[1025];
	bzero(msg,1025);
	int i;
	while((i=fread(msg,sizeof(char),1025,fd))>0)
	{
	//	printf("%d!!!!\n",i );

		if(write(*socketfd,msg,i)<0)
		{
			printf("ERROR: Writing to socket\n");
			break;
		}
		bzero(msg,1025);
	}
	bzero(msg,1025);
	strcpy(msg,"FILE LIST");
	sleep(1);
	if(write(*socketfd,msg,9)<0)
	{
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
	printf("SENT\n");
	return;
}	
void hash(char filename[],int *socketfd)
{
	//struct stat vstat;
	//puts(filename);
//	printf("printed from hash\n");
	char fin[1024];
	char temp1[1024],temp2[1024],temp3[1024];
//	printf("shut\n" );
	system("touch tempfile1 tempfile2");
	strcpy(temp3,"stat Shared/");
	strcpy(temp1,"md5sum Shared/");
	strcpy(temp2,"");
//	puts(filename);
	strcat(temp2,filename);
	strcat(temp2," > tempfile1");
	strcat(temp1,temp2);
	strcat(temp3," ");
	strcat(temp3,filename);
	strcat(temp3," | grep Modify | awk '{print $3}' > tempfile2");
	//execution of system calls
//		printf("shut2\n" );
//	printf("temp1 is %s\n temp3 is %s\n",temp1,temp3);

	system(temp1);
	system(temp3);
	FILE *fd1=fopen("tempfile1","r");//contains hash and file name
	FILE *fd2=fopen("tempfile2","r");//contains time of modification
	bzero(temp1,1024);
	bzero(temp2,1024);
	bzero(temp3,1024);
	fgets(temp1,sizeof(temp1),fd1);
	fgets(temp2,sizeof(temp2),fd2);
//	printf("shut3\n" );

	//getline(&temp1,1024,fd1);
	//getline(&temp2,1024,fd2);
	temp1[(strlen(temp1))-1]='\0';
	temp2[(strlen(temp2))-1]='\0';
	char * pch;
	  //	printf ("Splitting string \"%s\" into tokens:\n",buffer);
	pch = strtok (temp1," ");
	//strcat(filename,temp2);
	strcat(filename,"\t");
	strcat(filename,pch);
	char write_buffer[1024];
	strcpy(write_buffer,filename);
	//printf("filename is %s\n",write_buffer );
	if(write(*socketfd,write_buffer,1024)<0)
	{
		perror("ERROR: Cannot open socket");
		exit(0);
	}
//	printf("IAJHSKSH\n");
	system("rm -rf tempfile*");



}
int filehash_filter(char buffer[])
{	
	char * pch,b[100];
	char tempo[1024];
	strcpy(tempo,buffer);
  //	printf ("Splitting string \"%s\" into tokens:\n",buffer);
	pch = strtok (tempo," ");
	pch = strtok (NULL, " ");
	strcpy(b,pch);
//	printf("%s\n",b );

	if(strcmp(pch,"Verify")==0)
	{
	//	printf("Verify found\n");
		return 1;//1 for verify

	}

	else if(strcmp(pch,"CheckAll")==0)
	{

		//printf("CheckAll found\n");
		return 0;//0for checkall

	}
	else 
		return -1;

}
void FileHash2(char buf[],int *socketfd)
{
	int n=write(*socketfd,buf,1025);
	puts(buf);
	//printf("Reched client\n");
	char readBuff[1025];
	if(n<0)
	{
		perror("ERROR: Writing to socket\n");
	}
	if((filehash_filter(buf))==0)//checkall
	{
		int flag=1;
		while((n=read(*socketfd,readBuff,1024))>0)
		{
			if(strcmp(readBuff,"END")==0)
			{
				flag=0;
				break;
			}
			else
				printf("%s\n",readBuff);
		}
		if(flag==1)
			exit(0);
	}
	else if((filehash_filter(buf))==1)
	{
		int flag=1;
	//	printf("really found\n");
		while((n=read(*socketfd,readBuff,1024))>0)
		{
			if(strcmp(readBuff,"END")==0)
			{
				flag=0;
				break;
			}
			else
				printf("%s\n",readBuff);
		}

		//printf("read\n");
		if(flag==1)
			exit(0);
	}
	else if((filehash_filter(buf))==-1)
	{
		printf("Invalid arguments\n");
	}
}
void FileHash(char buf[],int *socketfd)
{
	char fin[1024];
	//printf("Server file hash reached\n");
	if((filehash_filter(buf))==1)
	{
		puts(buf);
		char temp[1024];
		strcpy(temp,buf);
		char * pch;
	  //	printf ("Splitting string \"%s\" into tokens:\n",buffer);
		pch = strtok (temp," ");
		pch = strtok (NULL, " ");
		pch = strtok (NULL, " ");
		//pch has the filename
	//	printf("this is filehash server%s\n",pch );
		hash(pch,socketfd);
		//bzero(fin,1024);
	//	printf("Hello\n");
		strcpy(fin,"END");
		//		printf("Hello2\n");

		int n=write(*socketfd,fin,1024);
		if(n<0)
		{
			perror("ERROR: Writing to socket\n");
			exit(0);
		}
	}
	else if((filehash_filter(buf))==0)
	{
		char line[1025];
		system("touch file_list");
		system("ls -l Shared/ | grep -v ^d | awk '{print $9}' | tail -n +2 > file_list");
		FILE *fd= fopen("file_list","r");
		if (fd)
		{
			while(fgets(line,sizeof(line),fd))
			{
				hash(line,socketfd);
			}
		}
		else
			perror("ERROR: Error in opening file");

		bzero(fin,100);
		strcpy(fin,"END");
		if(write(*socketfd,fin,100)<0)
		{
			perror("ERROR: ERROR writing to socket");
			exit(0);
		}
		system("rm -rf file_list");

	}

}


void IndexGet2(char buf[],int *socketfd)
{
	int n,f,flag=0;

	n=write(*socketfd,buf,1025);
	if(n<0)
	{
		printf("\nERROR: Writing to socket\n");
		exit(1);
	}
	char readBuff[1025];
	bzero(readBuff,1025);
	//printf("\nRecieved List: %s\n",readBuff);
	sleep(1);
	if(flag==0||flag==1)
	{
		printf("File        Type        Size        TimeStamp\n");
		while((f=read(*socketfd,readBuff,1025))>0)
		{
			//	printf("LISTing\n");
			if(strcmp(readBuff,"******FILE LIST*******")==0)
				break;
			else
				printf("%s",readBuff );
		}
	}
	else if(flag==2)
	{
		printf("File        Type        Size\n");
		while((f=read(*socketfd,readBuff,1025))>0)
		{
			//	printf("LISTing\n");
			if(strcmp(readBuff,"******FILE LIST*******")==0)
				break;
			else
				printf("%s",readBuff );
		}
	}
	//n=write(*socketfd,buf,1025)
}
int main()
{
	system("bash intro.sh");
	int pp,hp;
	printf("Port No: ");
	scanf("%d %d",&pp,&hp);

	pid=fork();
	if(pid==0)//client code!!!!!
	{
		int sockfd,n;
		struct sockaddr_in serv_addr;
		char readBuff[1025];
		sockfd=socket(AF_INET,SOCK_STREAM,0);
		if(sockfd<0)
			printf("ERROR in sockfd\n");
		else
			printf("{Client} created\n");
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(pp);
		serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		while(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0);
		while(1)
		{
			bzero(readBuff,1025);
			printf("Enter the message : ");
			gets(readBuff);
			printf("\n");
			if(strcmp(readBuff,"quit")==0)
			{
				n=write(sockfd,readBuff,1025);
				if(n<0)
					printf("ERROR writing to SOCKET\n");
				printf("{Client} Ended the Connection\n" );
				kill(pid,SIGTERM);
				break;
			}
			else if(strncmp(readBuff,"IndexGet",8)==0)
				IndexGet2(readBuff,&sockfd);
			else if(strncmp(readBuff,"FileDownload",12)==0)
				FileDownload2(readBuff,&sockfd);
			else if(strncmp(readBuff,"FileUpload",10)==0)
				FileUpload2(readBuff,&sockfd);
			else if(strncmp(readBuff,"FileHash",8)==0)
				FileHash2(readBuff,&sockfd);
			else
			{
				n = write(sockfd,readBuff, 1025);
				if (n < 0)
					printf("ERROR writing to socket\n");
			}
		}
		close(sockfd);
		exit(0);
		return 0;	
	}
	else if(pid)//server code!!!
	{
		int listenfd=0,connfd=0,n;
		struct sockaddr_in serv_addr;
		char sendBuff[1025],priBuff[1025];
		listenfd=socket(AF_INET,SOCK_STREAM,0);
	//printf("I am the Initiator\n");
		if(listenfd<0)
			printf("ERROR MAKING THE SOCKET\n");
		else
			printf("[SERVER] SOCKET INITIALISED\n");
		memset(&serv_addr, '0', sizeof(serv_addr));
		memset(sendBuff, '0', sizeof(sendBuff));
		serv_addr.sin_family = AF_INET;    
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
		serv_addr.sin_port = htons(hp);
		if(bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
			perror("ERROR binding the socket");
		else
			printf("[Server] *******************Socket Binded Successfully*************************\n");
		if(listen(listenfd, 10) == -1)
		{
			printf("Failed to listen\n");
		}   
		printf("[Server]****************** Connection Pending****************\n" );
		while((connfd=accept(listenfd, (struct sockaddr*)NULL ,NULL))<0);
		printf("[CONNECTED]																					[OK]\n");
		while(1)
		{
			bzero(priBuff,1025);
			bzero(sendBuff,1025);
			n=read(connfd,sendBuff,1025);
			if(n<0)
				printf("ERROR  writing\n");
			sendBuff[n]='\0';
			strcpy(priBuff,sendBuff);
			if(strcmp(sendBuff,"quit")==0)
			{
				printf("\nFinal message: %s\n",sendBuff);
				kill(pid,SIGTERM);
				break;
			}
			else if(strncmp(priBuff,"IndexGet",8)==0)
			{	
			//	printf("Command IG\n");
				IndexGet(priBuff,&connfd);
			}
			else if(strncmp(priBuff,"FileDownload",12)==0)
				FileDownload(priBuff,&connfd);
			else if(strncmp(priBuff,"FileHash",8)==0)
				FileHash(priBuff,&connfd);
			else if(strncmp(priBuff,"FileUpload",10)==0)
			{
				FileUpload(priBuff,&connfd);
				printf("BACLK\n");
			}
			else
			{
				printf("\nMessage from peer: %s\n",priBuff);
				sleep(0.5);
			}

			while(waitpid(-1, NULL, WNOHANG) > 0);
		}
		close(connfd);
		printf("\n Connection closed by peer\n");
		close(listenfd);
		exit(0);
		return 0;
	}
	return 0;
}
//void becomes_client()
//{
	//	printf("IM A PEER\n");

//}
