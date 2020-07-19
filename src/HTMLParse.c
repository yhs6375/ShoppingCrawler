#include "HSFirst.h"
#include "HSRegex.h"
#include "HSString.h"
#include "HTMLParse.h"
#include "HSUtil.h"
#include "HSSocket.h"
int make_http_header(httpURL *info){
	printf("%s %s %p\n",info->type,info->web_info->hostname,&info->web_info->setting);
	info->rq_header=makeStr(info->type==NULL?"GET":info->type," ",\
	info->param,\
	" ",info->web_info->setting.header.http_v==NULL?"HTTP/1.1":info->web_info->setting.header.http_v,\
	"\r\nHost: ",info->web_info->setting.header.host?info->web_info->setting.header.host:info->web_info->hostname,\
	"\r\nConnection: ",info->web_info->setting.header.connection==NULL?"close":info->web_info->setting.header.connection,\
	"\r\nUser-Agent: ",info->web_info->setting.header.user_agent==NULL?"TestTEst":info->web_info->setting.header.user_agent,\
	"\r\nAccept: ",info->web_info->setting.header.accept==NULL?"*/*":info->web_info->setting.header.accept,\
	"\r\nAccept-Language: ",info->web_info->setting.header.accept_language==NULL?"ko-KR,ko;q=0.8,en-US;q=0.6,en;q=0.4":info->web_info->setting.header.accept_language,\
	"\r\nAccept-Encoding: ",info->web_info->setting.header.accept_encoding==NULL?"gzip":info->web_info->setting.header.accept_encoding,\
	"\r\n\r\n",info->post_contents==NULL?0:info->post_contents,(char *)NULL);
}
int is_recv_end(char *buff,int len){
	if(buff[len-7]=='\r'&&buff[len-6]=='\n'&&buff[len-5]=='0'&&buff[len-4]=='\r'&&buff[len-3]=='\n'&&buff[len-2]=='\r'&&buff[len-1]=='\n'){
		return 1;
	}
	return 0;
}
int get_HTML_at_buff(httpURL *info){
	if(info->content_len==0){
		char *check;
		int check_len;
		info->heap_size=50000;
		info->html=(char *)malloc(info->heap_size);
		info->html[0]=0;
		char **output=&(info->html);
		check=info->buff+info->header_len;
		info->buff_len=0;
		//http패킷 body분석 후 info->html에 저장
		do{
			if((check_len=get_partial_length(&check,'\r'))==0){
				HS_free((void **)&info->buff);
				return 1;
			}
			check+=2;
			info->buff_len+=check_len;
			while(info->buff_len>info->heap_size){
				info->heap_size+=50000;
				*output=(char *)realloc(*output,info->heap_size);
			}
			memcpy(*output+info->buff_len-check_len,check,check_len);
			check+=check_len+2;
		}while(1);
	}else{
		info->html=malloc(info->buff_len-info->header_len+1);
		memcpy(info->html,info->buff+info->header_len,info->buff_len-info->header_len);
		HS_free((void **)&info->buff);
		info->buff_len-=info->header_len;
		return 1;
	}
}
char *find_node_starting(char *html,char *search_var){
	char *point=NULL;
	char *point2=NULL;
	char *search_type=NULL;
	char *attribute=NULL;
	size_t i=0;
	size_t len=0;
	if((point=strchr(search_var,'~'))==NULL){
		return NULL;
	}
	len=point-search_var;
	search_type=malloc(len+1);
	memcpy(search_type,search_var,len);
	search_type[len]=0;
	point++;
	if(strcmp(search_type,"tag")==0){
		attribute=makeStr("<",point,(char *)NULL);
		if((point2=strstr(html,attribute))==NULL){
			HS_free((void **)&attribute);
			HS_free((void **)&search_type);
			return NULL;
		}
	}else{
		attribute=makeStr(search_type,"=\"",point,"\"",(char *)NULL);
		if((point2=strstr(html,attribute))==NULL){
			printf("asdf\n");
			HS_free((void **)&attribute);
			HS_free((void **)&search_type);
			attribute=makeStr(search_type,"='",point,"'",(char *)NULL);
			if((point2=strstr(html,attribute))==NULL){
				HS_free((void **)&attribute);
				HS_free((void **)&search_type);
				return NULL;
			}
		}
	}
	HS_free((void **)&attribute);
	HS_free((void **)&search_type);
	while(point2[0]!='<'){
		point2--;
		if(html==point2){
			return NULL;
		}
	}
	return point2;
}
char *get_node_string(char *html,char *search_var,short type){
	char start_tag[15]="\0";
	char end_tag[15]="\0";
	char *result;
	short end_loop=1;
	size_t start_tag_size=0;
	size_t end_tag_size=0;
	size_t len=0;
	size_t i=0;
	char *start,*point;
	if((start=find_node_starting(html,search_var))==NULL){
		return NULL;
	}
	while(is_line_feed(start[i])==0){
		start_tag[i]=start[i];
		i++;
	}
	start_tag[i]=0;
	i=1;
	end_tag[0]='<';
	end_tag[1]='/';
	while(start_tag[i]!=0){
		end_tag[i+1]=start_tag[i++];
	}
	end_tag[i+1]='>';
	end_tag[i+2]=0;
	
	start_tag_size=strlen(start_tag);
	end_tag_size=strlen(end_tag);
	point=start+start_tag_size;
	printf("%c\n",point[0]);
	while(end_loop!=0){
		while(point[0]!='<'){
			point++;
			if(point[0]==0){
				return NULL;
			}
		}
		if(strncmp(point,start_tag,start_tag_size)==0){
			if(is_line_feed(point[start_tag_size])==1){
				printf("@%d\n",end_loop);
				end_loop++;
			}
		}else if(strncmp(point,end_tag,end_tag_size)==0){
			end_loop--;
			printf("%d\n",end_loop);
		}else{
			point++;
			if(point[0]==0){
				return NULL;
			}
		}
	}
	if(type==0){
		if((point=strchr(point,'>'))==NULL){
			return NULL;
		}
		point++;
	}else if(type==1){
		if((start=strchr(start,'>'))==NULL){
			return NULL;
		}
		start++;
	}
	len=point-start;
	result=malloc(len+1);
	memcpy(result,start,len);
	result[len]=0;
	return result;
}
short remain_page_check(httpURL *info){
	Web_info *web_info=info->web_info;
	char *page_list;
	if(web_info->setting.search.page_search_type==CHECKING_PAGE_BOX){
		if(web_info->setting.search.page_check_container==NULL||web_info->setting.search.page_check_exp==NULL){
			web_info->remain_page_count=0;
			return 0;
		}
		page_list=get_node_string(info->html,web_info->setting.search.page_check_container,1);
		while((page_list=find_node_starting(page_list,web_info->setting.search.page_check_exp))!=NULL){
			web_info->remain_page_count++;
			page_list++;
		}
	}
	if(web_info->setting.search.this_page_count==1){
		web_info->remain_page_count--;
	}
	HS_free((void **)&page_list);
	return 1;
}
short open_market_summarize(httpURL *info){
	Web_info *web_info=info->web_info;
	char *point=NULL;
	
}
short summarize_info(httpURL *info){
	Web_info *web_info=info->web_info;
	char *point=NULL;
	if(web_info->setting.search.site_type==OPEN_MARKET){
		if(open_market_summarize(info)==0){
			return 0;
		}
	}
}
short header_and_end_check(httpURL *info){
	if(info->header_check==0&&info->buff_len>=17){ //when to read starting,checking correct HTML STATUS and data
		if(strncmp(info->buff,"HTTP/1.1 200 OK\r\n",17)==0){
			info->header_check=1;
		}else if(strncmp(info->buff,"HTTP/1.1 ",9)==0){
			char status_str[4]="\0";
			memcpy(status_str,info->buff+9,3);
			if((info->status_code=hsstrtol(status_str,NULL,10))==-1){
				hserror(info,PARSER_ERROR);
			}else{
				hserror(info,N_STATUS_NORMAL);
			}
			return 0;
		}else{
			hserror(info,PARSER_ERROR);
			return 0;
		}
	}
	if(info->header_check==1){ //checking encoding type from HTTP HEADER
		char *point;
		if((point=strstr(info->buff,"\r\n\r\n"))!=NULL){
			char *save;
			unsigned int len;
			info->header_len=point-info->buff+4;
			char *header_temp=malloc(info->header_len+1);
			memcpy(header_temp,info->buff,info->header_len);
			header_temp[info->header_len]=0;
			if((point=strstr(header_temp,"Content-Encoding"))!=NULL){ //if header encoded by server,then write encoding type and check to length
				point+=17;
				if((len=ltrim_check_len(&point,"\r\n"))==-1){
					hserror(info,PARSER_ERROR);
					HS_free((void **)&header_temp);
					return 0;
				}
				memcpy(info->compress,point,len);
				info->compress[len]=0;
				if((point=strstr(header_temp,"Content-Length"))!=NULL){
						point+=15;
					if((len=ltrim_check_len(&point,"\r\n"))==-1){
						hserror(info,PARSER_ERROR);
						HS_free((void **)&header_temp);
						return 0;
					}
					if(len>6){
						hserror(info,PARSER_ERROR);
						HS_free((void **)&header_temp);
						return 0;
					}
					char content_length[6];
					memcpy(content_length,point,len);
					content_length[len]=0;
					if((info->content_len=hsstrtol(content_length,NULL,10))==-1){
						hserror(info,PARSER_ERROR);
						HS_free((void **)&header_temp);
					}
				}
			}
			//analyze Content-Type
			if((point=strstr(header_temp,"Content-Type"))==NULL){
				HS_free((void **)&header_temp);
				hserror(info,PARSER_ERROR);
				return 0;
			}
			point+=13;
			if((len=ltrim_check_len(&point,"\r\n"))==-1){
				HS_free((void **)&header_temp);
				hserror(info,PARSER_ERROR);
				return 0;
			}
			char *save_content_type=hsgstrstr(point,"\r\n");
			if((point=strstr(save_content_type,"charset="))==NULL){
				HS_free((void **)&header_temp);
				hserror(info,PARSER_ERROR);
				return 0;
			}
			point+=8;
			len=strlen(point);
			if(len>15){
				HS_free((void **)&header_temp);
				HS_free((void **)&save_content_type);
				hserror(info,PARSER_ERROR);
				return 0;
			}
			memcpy(info->enc_type,point,len);
			info->enc_type[len]=0;
			HS_free((void **)&header_temp);
			HS_free((void **)&save_content_type);
			info->header_check=2;
		}else{
			return 1;
		}
	}
	if(info->header_check==2){ //checking end
		if(strcmp(info->compress,"gzip")==0 && info->content_len!=0){
			if(info->buff_len-info->header_len==info->content_len){
				return 2;
			}
		}else{
			if(is_recv_end(info->buff,info->buff_len)==1){
				return 2;
			}
		}
	}
	return 1;
}
short check_html(httpURL *info,request *req){
	if(get_HTML_at_buff(info)!=1){
		printf("it is not html..\n");
		hserror(info,N_MATCH_HTML);
		return 0;
	}
	if(strcmp(info->compress,"gzip")==0){
		int fd;
		fd=open("./data/bb",O_RDWR);
		write(fd,info->html,info->buff_len);
		close(fd);
		if(gzip_inflate(info)==0){
			hserror(info,PARSER_ERROR);
			return 0;
		}
	}
	
	if(strcmp(info->enc_type,"UTF-8")!=0){
		target_to_utf8(&info->html,info->enc_type);
	}
	//printf("%s\n",info->html);
	printf("%s success\n",info->web_info->goods);
	if(info->isparent==1 && info->web_info->setting.parent.search_page_count!=0){
		remain_page_check(info);
		child_page_connect(info,req);
	}
	summarize_info(info);
	//struct s_reg list=full_regexec("<ul class=\"tt_list_box\">",url);
	info->connected=PARSER_SUCCESS;
	info->web_info->finish_count++;
	return 1;
}
