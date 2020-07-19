#include "HSFirst.h"
#include "HSString.h"
#include "HTMLParse.h"
#include "HSRegex.h"
#include "HSUtil.h"
#include "HSSocket.h"
#include "HSEvent.h"
#include "HSSetting.h"

void free_req(request *req){
	int i;
	short check=0;
	Web_info *web_info;
	httpURL *info;
	httpURL *next_info;
	if(req->url_count==0){
		req->url_count=1;
	}
	for(i=0;i<req->url_count;i++){
		web_info=req->web_info[i];
		HS_free((void **)&web_info->setting.header.http_v);
		HS_free((void **)&web_info->setting.header.host);
		HS_free((void **)&web_info->setting.header.connection);
		HS_free((void **)&web_info->setting.header.user_agent);
		HS_free((void **)&web_info->setting.header.accept);
		HS_free((void **)&web_info->setting.header.accept_language);
		HS_free((void **)&web_info->setting.header.accept_encoding);
		HS_free((void **)&web_info->setting.search.goods_count);
		HS_free((void **)&web_info->setting.search.page_check_container);
		HS_free((void **)&web_info->setting.search.page_check_exp);
		HS_free((void **)&web_info->setting.search.goods_list1_container1);
		HS_free((void **)&web_info->setting.search.goods_list1_container2);
		HS_free((void **)&web_info->setting.common.enc_type);
		HS_free((void **)&web_info->setting.common.type);
		HS_free((void **)&web_info->setting.common.post_contents);
		HS_free((void **)&web_info->setting.common.param);
		HS_free((void **)&web_info->setting.parent.type);
		HS_free((void **)&web_info->setting.parent.post_contents);
		HS_free((void **)&web_info->setting.parent.param);
		HS_free((void **)&web_info->setting.child.type);
		HS_free((void **)&web_info->setting.child.post_contents);
		HS_free((void **)&web_info->setting.child.param);
		HS_free((void **)&web_info->hostname);
		HS_free((void **)&web_info->goods);
		HS_free((void **)&web_info);
		for(info=req->info[i];info!=NULL;){
			HS_free((void **)&info->param);
			HS_free((void **)&info->buff);
			HS_free((void **)&info->html);
			HS_free((void **)&info->rq_header);
			HS_free((void **)&info->type);
			HS_free((void **)&info->post_contents);
			next_info=info->next;
			if(check>0){
				HS_free((void **)&info);
			}
			info=next_info;
			check++;
		}
	}
	free(req->web_info);
	req->web_info=NULL;
	free(req->info);
	req->info=NULL;
	HS_free((void **)&req->goods);
	HS_free((void **)&req);
}
void info_init(httpURL *info){
	info->web_info=NULL;
	info->param=NULL;
	info->buff=NULL;
	info->html=NULL;
	info->rq_header=NULL;
	info->ssl_con=NULL;
	info->type=NULL;
	info->post_contents=NULL;
	info->next=NULL;
	info->isparent=0;
	info->socket=0;
	info->heap_size=0;
	info->buff_len=0;
	info->connected=0;
	info->header_check=0;
	info->status_code=0;
	info->header_len=0;
}
request *request_init(){
	request *req;
	req=(request *)malloc(sizeof(request));
	req->info=malloc(sizeof(httpURL *));
	req->web_info=malloc(sizeof(Web_info *));
	req->url_count=0;
	req->next=NULL;
	req->prev=NULL;
	req->goods=NULL;
	req->url_count=0;
	req->info_count=0;
	req->connecting_count=0;
	req->error=0;
	return req;
}
short insert_information(request *req){
	unsigned short size=req->url_count;
	if(req->url_count>1){
		Web_info **temp1;
		httpURL **temp2;
		if((temp1=realloc(req->web_info,sizeof(Web_info *)*size))==NULL){
			return 0;
		}
		req->web_info=temp1;
		if((temp2=realloc(req->info,sizeof(httpURL *)*size))==NULL){
			return 0;
		}
		req->info=temp2;
	}
	
	req->web_info[size-1]=malloc(sizeof(Web_info));
	setting_init(req->web_info[size-1]);
	req->web_info[size-1]->isconnecting=0;
	req->web_info[size-1]->total_count=1;
	req->web_info[size-1]->finish_count=0;
	req->web_info[size-1]->error_count=0;
	req->web_info[size-1]->connect_count=0;
	req->info[size-1]=malloc(sizeof(httpURL));
	info_init(req->info[size-1]);
	req->info[size-1]->web_info=req->web_info[size-1];
	
	return 1;
}
void HS_init(){
	HS=(hosung *)malloc(sizeof(hosung)*THREAD_LIMIT);
	for(int i=0;i<THREAD_LIMIT;i++){
		HS[i].start=NULL;
		HS[i].end=NULL;
		HS[i].count=0;
		HS[i].waiting=NULL;
		pthread_mutex_init(&mutex[i],NULL);
	}
}

void create_thread(pthread_t *p_thread,int *save){
	int thr_id,i;
	for(i=0;i<THREAD_LIMIT;i++){
		save[i]=i;
		thr_id=pthread_create(&(p_thread[0]), NULL, t_crawler,(void *)(intptr_t)save[i]);
		if (thr_id<0){
			printf("thread create error\n");
			exit(0);
		}
	}
}
int main(int argc,char **argv){
	int server_sockfd,state,i;
	int save[THREAD_LIMIT-1];
	FILE *fp;
	pthread_t p_thread[THREAD_LIMIT];
	HS_init();
	init_SSL();
	create_thread(p_thread,save);
	server_sockfd=ups_server_create();
	local_request_event(server_sockfd);
	return 0;
}
