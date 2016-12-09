#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <string.h> 
#include <errno.h> 
#include <sys/socket.h> 
#include <resolv.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <sys/time.h> 
#include <sys/types.h> 
#include <netdb.h> 
#include "cJSON.h" 
#define MAXBUF 1024
#define CONLINELENGTH 100 //配置文件一行的长度最大值
#define FILENAME "config.ini" //配置文件的名称
static int print_jsom(char *json_string)  
{  
      char *out;  
      cJSON *jsonroot = cJSON_Parse(json_string);  
      out=cJSON_Print(jsonroot);  
      printf("%s\n",out);  
               
      cJSON_Delete(jsonroot);  
      free(out);  
      return 1;  
}  


//GetProfileString用来读取配置文件config.ini
int GetProfileString(char * profile, char * AppName, char * KeyName, char * KeyVal )
{
  char appname[20],keyname[20];
  char buf[CONLINELENGTH],*c;
  FILE *fp;
  int found=0;

  if( (fp=fopen( profile,"r" ))==NULL )
  {
    printf( "openfile [%s] error [%s]\n",
    profile,strerror(errno));
    return(-1);
  }
  fseek( fp, 0, SEEK_SET );

  sprintf( appname,"%s", AppName );

  memset( keyname, 0, sizeof(keyname) );

  while( !feof(fp) && fgets( buf,CONLINELENGTH,fp )!=NULL )
  {

    if( found==0 )
    {
      if( buf[0]!='[' )
      {
        continue;
      }
        else if ( strncmp(buf,appname,strlen(appname))==0 )
      {
      found=1;
      continue;
      }

    }
    else if( found==1 )
    {
      if( buf[0]=='#' )
      {
        continue;
      }
      else if ( buf[0]=='[' )
      {
        break;
      }
      else
      {
        if( (c=(char*)strchr(buf,'='))==NULL )
          continue;
          memset( keyname, 0, sizeof(keyname) );
          sscanf( buf, "%[^=]", keyname );
        if( strcmp(keyname, KeyName)==0 )
        {
          sscanf( ++c, "%[^\n\r]", KeyVal);
          found=2;
          break;
        }
        else
        {
          continue;
        }
      }
    }
  }

  fclose( fp );

  if( found==2 )
  return(0);
  else
  return(-1);
}



int main( int argc, char **argv) {
	int sockfd, len;
	struct sockaddr_in	dest;
	char buffer[MAXBUF];
	fd_set rfds;
	struct timeval tv;
	int retval, maxfd = -1; // char * sbuff;
  WriteSysLog("===============TCP Client Start!==================");
  //read config
  char *filename=FILENAME;
  char str_port[20]={0},str_host[20]={0},str_name[20]={0};
  char cfg[]="[config]",p_port[]="port",p_host[]="host",p_name[]="name";
  GetProfileString(filename,cfg,p_port,str_port);
  GetProfileString(filename,cfg,p_host,str_host);
  GetProfileString(filename,cfg,p_name,str_name);
  printf("host: %s\n" , str_host);
  printf("port: %s\n" , str_port);
  printf("name: %s\n", str_name);
  char *log=malloc(strlen("host: ")+strlen(str_host)+1);
  strcpy(log,"host: ");
  strcat(log,str_host);
  WriteSysLog(log);
  log=malloc(strlen("port: ")+strlen(str_port)+1);
  strcpy(log,"port: ");
  strcat(log,str_port);
  WriteSysLog(log);
  log=malloc(strlen("name: ")+strlen(str_name)+1);
  strcpy(log,"name: ");
  strcat(log,str_name);
  WriteSysLog(log);
  //system("ls ./");
//	if ( argc != 4 )
//	{
//		printf( "error！the right format should be：\n\t\t%s IP port name\n\t eg:\t%s 127.0.0.1 80 test\n", argv[0], argv[0] );
//		exit( 0 );
//	}
	if ( (sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		perror( "Socket" ); exit( errno );
	}
	bzero( &dest, sizeof(dest) );
	dest.sin_family = AF_INET;
	//dest.sin_port	= htons( atoi( argv[2] ) );
	dest.sin_port	= htons( atoi( str_port ) );
	//---解析域名--start--
	//const char* hostname = argv[1];
	const char* hostname = str_host;
  struct hostent* host;
	host = gethostbyname(str_host);
  if (NULL == host)
  {
      perror("can not get host by hostname");
      exit(EXIT_FAILURE);
  }
  //---解析域名--end--
  char * IP = inet_ntoa(*((struct in_addr*)host->h_addr)); //取IP
  printf("IP: %s",IP);

	if ( inet_aton( IP, (struct in_addr *) &dest.sin_addr.s_addr ) == 0 )
	{
		perror( str_host ); exit( errno );
	}
	
	
	if ( connect( sockfd, (struct sockaddr *) &dest, sizeof(dest) ) != 0 )
	{
		perror( "Connect " );
		exit( errno );
	}
	//printf( "\n Ready to start chatting. %s \n" , argv[3]);
	printf( "\n Ready to start chatting. %s \n" , str_name);
  //char *sbuff = malloc(strlen("{\"type\":\"login\",\"client_name\":\"")+strlen(argv[3])+strlen("\",\"room_id\":\"1\"}")+1);//+1 for the zero-terminator
  char *sbuff = malloc(strlen("{\"type\":\"login\",\"client_name\":\"")+strlen(str_name)+strlen("\",\"room_id\":\"1\"}")+1);//+1 for the zero-terminator
  //in real code you would check for errors in malloc here
  if (sbuff == NULL) exit (1);

  strcpy(sbuff, "{\"type\":\"login\",\"client_name\":\"");
  //strcat(sbuff, argv[3]);
  strcat(sbuff, str_name);
  strcat(sbuff, "\",\"room_id\":\"1\"}");
	send( sockfd, sbuff, strlen( sbuff ) + 1, 0 );
	while ( 1 )
	{
		FD_ZERO( &rfds );
		FD_SET( 0, &rfds );
		maxfd = 0;
		FD_SET( sockfd, &rfds );
		if ( sockfd > maxfd )
			maxfd = sockfd;
		    tv.tv_sec = 1;
		    tv.tv_usec = 0;
		    retval = select( maxfd + 1, &rfds, NULL, NULL, &tv );
	    if ( retval == -1 )
		{
			printf( "Will exit and the select is error！ %s", strerror( errno ) ); break;
		} else if ( retval == 0 )
		{ /* printf ("No massage comes, no buttons, continue to wait ...\n"); */
			continue;
		} else {
		    
		    if ( FD_ISSET( sockfd, &rfds ) )
			 {
				 bzero( buffer, MAXBUF + 1 );
				 len = recv( sockfd, buffer, MAXBUF, 0 );
				 if ( len > 0 ){
//					 printf( "Successfully received the message: '% s',% d bytes of data\n", buffer, len );
           char *out;
           cJSON *jsonroot = cJSON_Parse(buffer);
           out=cJSON_Print(jsonroot);
           printf("%s\n",out);
           char *type = NULL;
           type = cJSON_GetObjectItem(jsonroot,"type")->valuestring;
           if (type != NULL) {
              printf("type: %s\n",type);
              log=malloc(strlen("type: ")+strlen(type)+1);
              strcpy(log,"type: ");
              strcat(log,type);
              WriteSysLog(log);
              if(strcmp(type,"login")==0){
                char *client_name=NULL;
                client_name = cJSON_GetObjectItem(jsonroot,"client_name")->valuestring;
                if(client_name != NULL){
                  printf("client_name: %s\n",client_name);
                  log=malloc(strlen("client_name: ")+strlen(client_name)+1);
                  strcpy(log,"client_name: ");
                  strcat(log,client_name);
                  WriteSysLog(log);
                }
              }
              if(strcmp(type,"say")==0){
                char *content = NULL;
                content = cJSON_GetObjectItem(jsonroot,"content")->valuestring;
                if(content != NULL){
                  printf("content: %s\n", content);
                  log=malloc(strlen("content: ")+strlen(content)+1);
                  strcpy(log,"content: ");
                  strcat(log,content);
                  WriteSysLog(log);
                  if(strcmp(content,"ls")==0){
                    system("ls");
                    system("./1.sh");
                  }
                  if(strcmp(content,"df")==0){
                    char def1[500];
                    FILE * fp;
                    fp = popen("df -h|grep /","r");
                    fgets(def1,sizeof(def1),fp);
                    sscanf( def1, "%[^\n\r]", def1);
                    sbuff=malloc(strlen("{\"type\":\"say\",\"to_client_id\":\"all\",\"content\":\"") +strlen(def1)+strlen("\"}"));
                    strcpy(sbuff,"{\"type\":\"say\",\"to_client_id\":\"all\",\"content\":\"");
                    strcat(sbuff,def1);
                    strcat(sbuff,"\"}");
                    printf("%s\n",def1);
                    send( sockfd, sbuff, strlen(sbuff) + 1, 0 );
                    pclose(fp);
                  }
                }
              }
           }
           cJSON_Delete(jsonroot);
           free(out);
         }
				 else {
				    if ( len < 0 )
						printf( "Failed to recerve the message! The error code is %d, error message is'%s'\n", errno, strerror( errno ) );
				  	else printf( "Chat to terminate！\n" );
					  break;
				     
				 }
			 }
			 if ( FD_ISSET( 0, &rfds ) )
			 {
         if ( len < 0 )
          {
            printf( "Message '%s' failed to send! The error code is%d, error message '%s'\n", sbuff, errno, strerror( errno ) );
            break;
          }
       //   else
//          printf( "News:%s send, sent a total of %d bytes！\n", sbuff, len );
        }
     }
   }
   close( sockfd );
   return(0);
}
 
int WriteSysLog(char *str)
{
   char buf[512];
   long MAXLEN = 10*1024*1024;//10MB
   time_t timep; 
   FILE *fp = NULL;
   struct tm *p; 
        
   time(&timep); 
   p = localtime(&timep); 
   memset(buf,0,sizeof(buf));
   sprintf(buf,"%d-%d-%d %d:%d:%d : ",(1900+p->tm_year),(1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec); //星期p->tm_wday
   strcat(buf,str);
   strcat(buf,"\r\n");
             
   fp = fopen("./tcp.log","r");
   if(fp==NULL)
   {
     fp = fopen("./tcp.log","w+");
   }
   else
   {
     fseek(fp,0,2);
     if(ftell(fp) >= MAXLEN)
     {
       fclose(fp);
       fp = fopen("./tcp.log","w+");
       //大于10MB则清空原日志文件
     }
     else
     {
       fclose(fp);
       fp = fopen("./tcp.log","a");
     }
   }
   fwrite(buf,1,strlen(buf),fp);
   fflush(fp);
   fsync(fileno(fp));
   fclose(fp);
}
