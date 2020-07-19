#include "HSFirst.h"
#include "HSSocket.h"
#include "HSRegex.h"
#include "HTMLParse.h"
#include "HSUtil.h"
#include "HSSetting.h"
void status_con_set(int *status,int set){
	*status &= ~CONNECT_ERROR;
	*status &= ~CONNECT_SUCCESS;
	*status &= ~CONNECT_PROGRESS;
	*status |= set;
}
int init_SSL(){
	SSL_load_error_strings();
	SSL_library_init();
	ssl_ctx=SSL_CTX_new(SSLv23_client_method());
}
int is_connected(httpURL *info){
	struct sockaddr nn;
	socklen_t nnn=sizeof(struct sockaddr);
	int opt_check;
	socklen_t err_len = sizeof(opt_check);
	if(getsockopt(info->socket,SOL_SOCKET,SO_ERROR,&opt_check,&err_len)!=-1){
		if(getpeername(info->socket,&nn,&nnn)!=-1){
			status_con_set(&(info->connected),CONNECT_SUCCESS);
			return 1;
		}
	}else{
		if(opt_check==ECONNREFUSED){
			printf("refused\n");
			info->connected&=~CONNECT_SUCCESS;
			info->connected&=~CONNECT_PROGRESS;
			info->connected|=CONNECT_REFUSE;
			return 2;
		}else if(opt_check==ETIMEDOUT){
			printf("timedout\n");
			info->connected&=~CONNECT_SUCCESS;
			info->connected&=~CONNECT_PROGRESS;
			info->connected|=CONNECT_TIMEOUT;
			return 2;
		}else{
			printf("unknown error\n");
			info->connected&=~CONNECT_SUCCESS;
			info->connected&=~CONNECT_PROGRESS;
			info->connected|=CONNECT_UNKNOWN;
		}
		close(info->socket);
		info->socket=0;
	}
	return 0;
}
int send_message(httpURL *info){
	if(info->port==443){
		SSL_write(info->ssl_con,info->rq_header,strlen(info->rq_header)+1);
		info->connected&=~SSL_CONNECT_PROGRESS;
	}else{
		printf("@@@@@@@@@@@\n");
		send(info->socket,info->rq_header,strlen(info->rq_header)+1,0);
	}
	info->connected|=PARSER_PROGRESS;
}
int connecting_check(request *req){
	httpURL *cur_info;
	int check;
	for(int i=0;i<req->url_count;i++){
		if(req->web_info[i]->isconnecting==1){
			for(cur_info=req->info[i];cur_info!=NULL;cur_info=cur_info->next){
				if(cur_info->connected&CONNECT_PROGRESS){
					if((check=is_connected(cur_info))==1){
						if(cur_info->port==80){
							send_message(cur_info);
						}else{
							cur_info->connected|=SSL_CONNECT_PROGRESS;
						}
					}
					if(check>0){
						req->connecting_count--;
						req->web_info[i]->connect_count--;
						if(req->web_info[i]->connect_count==0){
							req->web_info[i]->isconnecting=0;
						}else if(req->web_info[i]->connect_count<0){
							//error code
						}
					}
				}
			}
		}
	}
}
int ssl_connect(httpURL *info){
	int check;
	if(check=SSL_connect(info->ssl_con)==-1){
		switch(SSL_get_error(info->ssl_con,check)){
			case SSL_ERROR_WANT_READ:
				printf("aa\n");
				break;
			case SSL_ERROR_WANT_WRITE:
				printf("bb\n");
				break;
			case SSL_ERROR_SYSCALL:
				printf("cc\n");
				break;
		}
	}else{
		send_message(info);
		info->connected&=~SSL_CONNECT_PROGRESS;
		return 0;
	}
}
int get_port(httpURL *info){
	if(info->web_info->setting.common.port!=0){
		info->port=info->web_info->setting.common.port;
	}else{
		if(info->isparent==1){
			info->port=info->web_info->setting.parent.port;
		}else{
			info->port=info->web_info->setting.child.port;
		}
	}
	if(info->port!=80&&info->port!=443){
		return 0;
	}
	return 1;
}
int hostname_to_ip(char *hostname,char *ip){
	struct hostent hbuf,*result;
	int i=0,rc,err,len=1024;
	char *buf=malloc(len);
	while((rc=gethostbyname_r(hostname,&hbuf,buf,len,&result,&err))==ERANGE) {
		len*=2;
		void *tmp=realloc(buf,len);
		if (NULL==tmp) {
			free(buf);
			return 0;
		}else{
			buf=tmp;
		}
	}
	if(rc!=0||result==NULL){
		free(buf);
		return 0;
	}
	while(result->h_addr_list[i]!=NULL){
		strcpy(ip,inet_ntoa((struct in_addr) *((struct in_addr *)result->h_addr_list[i])));
		i++;
	}
	free(buf);
	return 1;
}
int ups_server_create(){
	int server_sockfd,state;
	char *socket_filename="./data/sockettest";
	struct sockaddr_un serveraddr;
	char buf[1000];
	if(access(socket_filename, F_OK)==0){
		unlink(socket_filename);
	}
	if ((server_sockfd=socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
		perror("socket error\n");
		exit(0);
	}
	memset(&serveraddr,0, sizeof(serveraddr));
	serveraddr.sun_family = AF_UNIX;
	strcpy(serveraddr.sun_path, socket_filename);
	state=bind(server_sockfd , (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (state == -1){
		perror("bind error : ");
		exit(0);
	}
	state = listen(server_sockfd, 5);
	if (state == -1){
		perror("listen error : ");
		exit(0);
	}
	if(fcntl(server_sockfd,F_SETFL,O_NONBLOCK)==-1){
		return 0;
	}
	return server_sockfd;
}
int connect_start(httpURL *info,request *req){
	int con_result;
	Web_info *web_info=info->web_info;
	struct sockaddr_in server_addr;
	info->socket=socket(PF_INET,SOCK_STREAM,0);
	if(fcntl(info->socket,F_SETFL,O_NONBLOCK)==-1){
		info->connected|=CONNECT_ERROR;
		info->connected|=M_ERROR;
		return 0;
	}
	memset(&server_addr,0,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(info->port);
	server_addr.sin_addr.s_addr=inet_addr(web_info->ip);
	printf("%s %s %s:%d connect..\n",req->goods,web_info->hostname,web_info->ip,info->port);
	con_result=connect(info->socket,(struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));
	if(con_result!=0){
		if(errno==EINPROGRESS){
			struct timespec spec;
			clock_gettime(CLOCK_REALTIME, &spec);
			info->timeout_s=spec.tv_sec;
			info->timeout_ms=round(spec.tv_nsec/1.0e6);
			web_info->isconnecting=1;
			web_info->connect_count++;
			req->connecting_count++;
			status_con_set(&(info->connected),CONNECT_PROGRESS);
		}else{
			printf("connect failed\n");
			close(info->socket);
			status_con_set(&(info->connected),CONNECT_ERROR);
			info->connected|=M_ERROR;
			return 0;
		}
	}else{
		status_con_set(&(info->connected),CONNECT_SUCCESS);
		if(info->port==80){
			send_message(info);
		}else{
			info->connected|=SSL_CONNECT_PROGRESS;
		}
	}
	if(info->port==443){
		info->ssl_con=SSL_new(ssl_ctx);
		SSL_set_fd(info->ssl_con,info->socket);
	}
	req->ep_st.newPeerConnectionEvent.data.ptr=info;
	if(epoll_ctl(req->ep_st.epoll_fd,EPOLL_CTL_ADD,info->socket,&(req->ep_st.newPeerConnectionEvent))==-1){
		printf("epoll_ctl error\n");
		web_info->isconnecting=0;
		status_con_set(&(info->connected),CONNECT_ERROR);
		info->connected|=M_ERROR;
	}
	info->buff=malloc(40000);
	info->heap_size=40000;
	info->buff[0]=0;
	return 1;
}
short child_page_connect(httpURL *info,request *req){
	Web_info *web_info=info->web_info;
	httpURL *prev_info=info;
	int i=0;
	if(web_info->setting.parent.search_page_count==0){
		for(i=0;i<web_info->setting.parent.search_page_count;i++){
			httpURL *child_info=(httpURL *)malloc(sizeof(httpURL));
			memset(child_info,0,sizeof(httpURL));
			info_init(child_info);
			prev_info->next=child_info;
			child_info->web_info=web_info;
			web_info->total_count++;
			req->info_count++;
			info->isparent=0;
			info_setting(child_info);
			connect_start(child_info,req);
			prev_info=child_info;
		}
		web_info->remain_page_count-=web_info->setting.parent.search_page_count;
	}
}
