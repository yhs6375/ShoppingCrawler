#include "HSFirst.h"
#include "HSSocket.h"
#include "HSEvent.h"
#include "HTMLParse.h"
#include "HSUtil.h"
#include "HSRegex.h"
#include "HSSetting.h"
#include "HSString.h"
short epoll_setting(request *req){
	req->ep_st.epoll_fd=epoll_create1(0);
	if(req->ep_st.epoll_fd==-1){
		printf("fd create error\n");
		usleep(500);
		req->ep_st.epoll_fd=epoll_create1(0);
		if(req->ep_st.epoll_fd==-1){
			printf("can't create epoll object");
			return 0;
		}
	}
	req->ep_st.newPeerConnectionEvent.events=EPOLLOUT | EPOLLIN | EPOLLERR;
	req->ep_st.processableEvents=(struct epoll_event*)malloc(req->url_count*sizeof(struct epoll_event));
	return 1;
}
void insert_waiting_queue(int cur,request *req){
	pthread_mutex_lock(&mutex[cur]);
	HS[cur].count++;
	struct waiting_req *waiting=(struct waiting_req *)malloc(sizeof(struct waiting_req));
	waiting->cur=req;
	waiting->next=NULL;
	if(HS[cur].waiting==NULL){
		HS[cur].waiting=waiting;
	}else{
		struct waiting_req *waiting_check=HS[cur].waiting;
		while(waiting_check->next!=NULL){
			waiting_check=waiting_check->next;
		}
		waiting_check->next=waiting;
	}
	pthread_mutex_unlock(&mutex[cur]);
}
int local_request_event(int server_sockfd){
	int epoll_fd,n,s,i,ups_count;
	struct epoll_event event;
	struct epoll_event *events;
	epoll_fd = epoll_create1(0);
	if(epoll_fd==-1){
		exit(2);
	}
	memset(&event,0,sizeof(struct epoll_event));
	event.data.fd=server_sockfd;
	event.events=EPOLLIN|EPOLLET;
	s=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_sockfd,&event);
	if (s==-1){
		exit(2);
	}
	events=calloc(THREAD_LIMIT,sizeof event);
	while(1){
		n=epoll_wait(epoll_fd,events,THREAD_LIMIT,-1);
		for(i=0;i<n;i++){
			if((events[i].events & EPOLLERR)||(events[i].events & EPOLLHUP)||(!(events[i].events & EPOLLIN))){
				close(events[i].data.fd);
				continue;
			}else if(events[i].data.fd==server_sockfd){
				while(1){
					struct sockaddr_un clientaddr;
					struct ups_socket_st *ups_st=(struct ups_socket_st *)malloc(sizeof(struct ups_socket_st));
					ups_st->heap_size=5000;
					ups_st->buf=malloc(5000);
					ups_st->buf[0]=0;
					socklen_t client_len=sizeof clientaddr;
					ups_st->ups_fd=accept(server_sockfd,(struct sockaddr *)&clientaddr,&client_len);
					if(ups_st->ups_fd==-1){
						break;
					}
					if(fcntl(ups_st->ups_fd,F_SETFL,O_NONBLOCK)==-1){
						close(ups_st->ups_fd);
						HS_free((void **)&ups_st->buf);
						HS_free((void **)&ups_st);
						break;
					}
					event.data.ptr=ups_st;
					event.events=EPOLLIN | EPOLLET;
					s=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,ups_st->ups_fd,&event);
					if(s==-1){
						close(ups_st->ups_fd);
						HS_free((void **)&ups_st->buf);
						HS_free((void **)&ups_st);
						break;
					}
					printf("ups socket accept\n");
				}
				continue;
			}else{
				ssize_t count;
				struct ups_socket_st *ups_st=(struct ups_socket_st *)events[i].data.ptr;
				char buf[2048];
				if((count=recv(ups_st->ups_fd,buf,2048,0))==-1){
					printf("ups recv error\n");
					close(ups_st->ups_fd);
					HS_free((void **)&ups_st->buf);
					HS_free((void **)&ups_st);
				}else if(count==0){
					continue;
				}else{
					buf[count]=0;
					if(strlen(ups_st->buf)+count>ups_st->heap_size){
						ups_st->heap_size+=5000;
						ups_st->buf=realloc(ups_st->buf,ups_st->heap_size);
					}
					strncat(ups_st->buf,buf,count);
					size_t len;
					char *point;
					while((point=strstr(ups_st->buf,"\r\n\r\n"))!=NULL){
						len=point-ups_st->buf;
						char request_data[2048];
						strncpy(request_data,ups_st->buf,len);
						request_data[len]=0;
						strcat(request_data,"\r\r\n\0");
						ups_st->buf+=len+4;
						
						int check=0,save=HS[0].count,cur=0;
						request *req=request_init();
						for(i=0;i<9;i++){
							if(HS[i].count!=0){
								check=HS[i].count;
								if(save>check){
									save=check;
									cur=i;
								}
							}else{
								cur=i;
								break;
							}
						}
						printf("cur=%d\n",cur);
						request_setting(request_data,req);
						insert_waiting_queue(cur,req);
						//write(events[i].data.fd, request_data, strlen(buf));
					}
					memset(buf,0,2048);
				}
			}
		}
	}
}
void *t_crawler(void *data){
	char buff[200000];
	int i,numEvents;
	httpURL *cur_fd_info;
	request *save_req;
	request *cur_req;
	short delete_check;
	short waiting_check_start=0;
	pthread_detach(pthread_self());
	int id = (intptr_t) data;
	int read_len;
	while(1){
		struct waiting_req *waiting_check=HS[id].waiting;
		if(waiting_check!=NULL){
			for(struct waiting_req *cur_waiting=waiting_check;cur_waiting!=NULL;){
				if(waiting_check_start==0){
					waiting_check_start=1;
				}else{
					pthread_mutex_unlock(&mutex[id]);
				}
				read_file_setting(cur_waiting->cur);
				if(epoll_setting(cur_waiting->cur)!=1){
					cur_waiting->cur->error=EPOLL_ERR;
					cur_waiting->cur->error|=REQUEST_ERR;
				}
				if(!(cur_waiting->cur->error&REQUEST_ERR)){
					for(i=0;i<cur_waiting->cur->url_count;i++){
						httpURL *cur_info=cur_waiting->cur->info[i];
						if(hostname_to_ip(cur_info->web_info->hostname,cur_info->web_info->ip)==0){
							cur_info->connected=UNKNOWN_HOST;
							cur_info->connected|=M_ERROR;
							break;
						}
						cur_info->web_info->goods=strdup(cur_waiting->cur->goods);
						if(cur_info->web_info->setting.common.enc_type!=NULL && strcmp(cur_info->web_info->setting.common.enc_type,"UTF-8")!=0){
							utf8_to_target(&cur_info->web_info->goods,cur_info->web_info->setting.common.enc_type);
						}
						if(cur_info->web_info->setting.common.url_encoding==1){
							urlencode(&cur_info->web_info->goods);
						}else if(cur_info->web_info->setting.common.url_encoding==2){
							if(unicode_urlencode(&cur_info->web_info->goods)==0){
								cur_info->connected=UNKNOWN_HOST;
								cur_info->connected|=M_ERROR;
								break;
							}
						}
						if(info_setting(cur_info)==0){
							break;
						}
						if(connect_start(cur_info,cur_waiting->cur)){
							cur_waiting->cur->info_count++;
						}
					}
					if(HS[id].start==NULL&&HS[id].end==NULL){
						HS[id].start=HS[id].end=cur_waiting->cur;
					}else{
						cur_waiting->cur->prev=HS[id].end;
						HS[id].end->next=cur_waiting->cur;
						HS[id].end=cur_waiting->cur;
					}
				}
				pthread_mutex_lock(&mutex[id]);
				struct waiting_req *next_waiting=cur_waiting->next;
				HS_free((void **)&cur_waiting);
				cur_waiting=next_waiting;
			}
			HS[id].waiting=NULL;
			HS[id].count=0;
			waiting_check_start=0;
			pthread_mutex_unlock(&mutex[id]);
		}
		for(cur_req=HS[id].start;cur_req!=NULL;){
			delete_check=0;
			numEvents=epoll_wait(cur_req->ep_st.epoll_fd,cur_req->ep_st.processableEvents,cur_req->info_count,0);
			if(numEvents==0 && cur_req->connecting_count!=0){
				connecting_check(cur_req);
			}else if(numEvents==0&&cur_req->connecting_count==0){
				//req end(status) send to response data code
				if(is_request_end(cur_req)==1){
					save_req=cur_req;
					cur_req=cur_req->next;
					printf("@@@@@@@@@@@@@@@@@@\n");
					delete_req_in_queue(&HS[id],save_req);
					delete_check=1;
				}
			}else if(numEvents==-1){
				//req error(status) send to response data code
				if(errno==EBADF){
					printf("aaaa\n");
				}else if(errno==EFAULT){
					printf("bbbb\n");
				}else if(errno==EINTR){
					printf("ccccc\n");
				}else if(errno==EINVAL){
					printf("ddddd\n");
				}
				printf("id = %d\n",id);
				save_req=cur_req;
				cur_req=cur_req->next;
				delete_req_in_queue(&HS[id],save_req);
				delete_check=1;
			}else{
				if(cur_req->connecting_count!=0){
					connecting_check(cur_req);
				}
				for(int i=0;i<numEvents;i++){ //loop to check for waiting socket
					cur_fd_info=(httpURL *)cur_req->ep_st.processableEvents[i].data.ptr;
					if(cur_fd_info->connected&SSL_CONNECT_PROGRESS){
						ssl_connect(cur_fd_info);
					}else if(cur_fd_info->connected&CONNECT_SUCCESS){
						if(cur_req->ep_st.processableEvents[i].events & EPOLLIN){
							if(cur_fd_info->port==443){ //https(443)port recv..
								read_len=SSL_read(cur_fd_info->ssl_con,buff,16385);//ssl can read up to 16384 bytes
								if(read_len>0){//input data that are received by ssl socket
									cur_fd_info->buff_len+=read_len;
									if(cur_fd_info->buff_len>cur_fd_info->heap_size){
										cur_fd_info->heap_size+=50000;
										cur_fd_info->buff=realloc(cur_fd_info->buff,cur_fd_info->heap_size);
									}
									buff[read_len]=0;
									memcpy(cur_fd_info->buff+cur_fd_info->buff_len-read_len,buff,read_len);
								}else{
									int err=SSL_get_error(cur_fd_info->ssl_con,read_len);
									if(err==SSL_ERROR_WANT_WRITE){
										printf("dlqslek!!!!!\n");
									}
									if(err==SSL_ERROR_SYSCALL){ //ssl recv success
										epoll_ctl(cur_req->ep_st.epoll_fd,EPOLL_CTL_DEL,cur_fd_info->socket,&cur_req->ep_st.newPeerConnectionEvent);
										if(header_and_end_check(cur_fd_info)==2){
											check_html(cur_fd_info,cur_req);
										}
									}
								}
							}else{
								read_len=recv(cur_fd_info->socket,buff,BUFF_SIZE,0);
								if(read_len<=0){
									if(read_len==0){
										printf("read failed\n");
										close(cur_fd_info->socket);
										cur_fd_info->connected=PARSER_ERROR;
										cur_fd_info->connected|=M_ERROR;
									}else{
										printf("fsadf");
										close(cur_fd_info->socket);
										cur_fd_info->connected=PARSER_ERROR;
										cur_fd_info->connected|=M_ERROR;
									}
								}else if(read_len>0){
									cur_fd_info->buff_len+=read_len;
									if(cur_fd_info->buff_len>cur_fd_info->heap_size){
										cur_fd_info->heap_size+=50000;
										cur_fd_info->buff=realloc(cur_fd_info->buff,cur_fd_info->heap_size);
									}
									buff[read_len]=0;
									memcpy(cur_fd_info->buff+cur_fd_info->buff_len-read_len,buff,read_len);
									short check;
									if((check=header_and_end_check(cur_fd_info))==2){//http recv success..
										epoll_ctl(cur_req->ep_st.epoll_fd,EPOLL_CTL_DEL,cur_fd_info->socket,&cur_req->ep_st.newPeerConnectionEvent);
										check_html(cur_fd_info,cur_req);
									}else if(check==0){
										epoll_ctl(cur_req->ep_st.epoll_fd,EPOLL_CTL_DEL,cur_fd_info->socket,&cur_req->ep_st.newPeerConnectionEvent);
									}
								}
							}
						}
					}
				}
			}
			if(delete_check==0){
				cur_req=cur_req->next;
			}
		}
	}
}
