#include "HSFirst.h"
#include "HSSetting.h"
#include "HSString.h"
#include "HSUtil.h"
#include "HSSocket.h"
#include "HTMLParse.h"
void setting_init(Web_info *web_info){
	web_info->hostname=NULL;
	web_info->goods=NULL;
	web_info->setting.header.accept=NULL;
	web_info->setting.header.accept_encoding=NULL;
	web_info->setting.header.accept_language=NULL;
	web_info->setting.header.connection=NULL;
	web_info->setting.header.host=NULL;
	web_info->setting.header.http_v=NULL;
	web_info->setting.header.user_agent=NULL;
	
	web_info->setting.search.page_search_type=0;
	web_info->setting.search.goods_count=NULL;
	web_info->setting.search.site_type=0;
	web_info->setting.search.page_check_container=NULL;
	web_info->setting.search.page_check_exp=NULL;
	web_info->setting.search.this_page_count=0;
	web_info->setting.search.goods_list1_container1=NULL;
	web_info->setting.search.goods_list1_container2=NULL;
	
	web_info->setting.common.enc_type=NULL;
	web_info->setting.common.type=NULL;
	web_info->setting.common.port=0;
	web_info->setting.common.post_contents=NULL;
	web_info->setting.common.param=NULL;
	web_info->setting.common.url_encoding=0;
	
	web_info->setting.parent.type=NULL;
	web_info->setting.parent.port=0;
	web_info->setting.parent.post_contents=NULL;
	web_info->setting.parent.param=NULL;
	web_info->setting.parent.search_page_count=0;
	
	web_info->setting.child.type=NULL;
	web_info->setting.child.port=0;
	web_info->setting.child.post_contents=NULL;
	web_info->setting.child.param=NULL;
}
short insert_set(Web_info *web_info,Line_type *line_type){
	if(strcmp(line_type->m_name,"header")==0){
		if(strcmp(line_type->name,"HTTPv")==0&&web_info->setting.header.http_v==NULL){
			web_info->setting.header.http_v=strdup(line_type->value);
		}else if(strcmp(line_type->name,"Host")==0&&web_info->setting.header.host==NULL){
			web_info->setting.header.host=strdup(line_type->value);
		}else if(strcmp(line_type->name,"Connection")==0&&web_info->setting.header.connection==NULL){
			web_info->setting.header.connection=strdup(line_type->value);
		}else if(strcmp(line_type->name,"User-Agent")==0&&web_info->setting.header.user_agent==NULL){
			web_info->setting.header.user_agent=strdup(line_type->value);
		}else if(strcmp(line_type->name,"Accept")==0&&web_info->setting.header.accept==NULL){
			web_info->setting.header.accept=strdup(line_type->value);
		}else if(strcmp(line_type->name,"Accept-Language")==0&&web_info->setting.header.accept_language==NULL){
			web_info->setting.header.accept_language=strdup(line_type->value);
		}else if(strcmp(line_type->name,"Accept-Encoding")==0&&web_info->setting.header.accept_encoding==NULL){
			web_info->setting.header.accept_encoding=strdup(line_type->value);
		}
	}else if(strcmp(line_type->m_name,"search_expression")==0){
		if(strcmp(line_type->name,"page_search_type")==0&&web_info->setting.search.page_search_type==0){
			short check_type=strtol(line_type->name,NULL,10);
			if(errno!=ERANGE&&errno!=EINVAL){
				if(check_type<=2||check_type>=0){
					web_info->setting.search.page_search_type=check_type;
				}
			}
		}else if(strcmp(line_type->name,"site_type")==0&&web_info->setting.search.site_type==0){
			short check_type=strtol(line_type->name,NULL,10);
			if(errno!=ERANGE&&errno!=EINVAL){
				if(check_type<=2||check_type>=0){
					web_info->setting.search.site_type=check_type;
				}
			}
		}else if(strcmp(line_type->name,"goods_count")==0&&web_info->setting.search.goods_count==NULL){
			web_info->setting.search.goods_count=strdup(line_type->value);
		}else if(strcmp(line_type->name,"page_check_exp")==0&&web_info->setting.search.page_check_exp==NULL){
			web_info->setting.search.page_check_exp=strdup(line_type->value);
		}else if(strcmp(line_type->name,"page_check_container")==0&&web_info->setting.search.page_check_container==NULL){
			web_info->setting.search.page_check_container=strdup(line_type->value);
		}else if(strcmp(line_type->name,"goods_list1_container1")==0&&web_info->setting.search.goods_list1_container1==NULL){
			web_info->setting.search.goods_list1_container1=strdup(line_type->value);
		}else if(strcmp(line_type->name,"goods_list1_container2")==0&&web_info->setting.search.goods_list1_container2==NULL){
			web_info->setting.search.goods_list1_container2=strdup(line_type->value);
		}
	}else if(strcmp(line_type->m_name,"common_search")==0){
		if(strcmp(line_type->name,"encoding")==0&&web_info->setting.common.enc_type==NULL){
			web_info->setting.common.enc_type=strdup(line_type->value);
		}else if(strcmp(line_type->name,"type")==0&&web_info->setting.common.type==NULL){
			web_info->setting.common.type=strdup(line_type->value);
		}else if(strcmp(line_type->name,"ssl")==0&&web_info->setting.common.port==0){
			if(strcmp(line_type->value,"0")==0){
				web_info->setting.common.port=80;
			}else{
				web_info->setting.common.port=443;
			}
		}else if(strcmp(line_type->name,"post_contents")==0&&web_info->setting.common.post_contents==NULL){
			web_info->setting.common.type=strdup(line_type->value);
		}else if(strcmp(line_type->name,"param")==0&&web_info->setting.common.param==NULL){
			web_info->setting.common.param=strdup(line_type->value);
		}else if(strcmp(line_type->name,"url_encoding")==0&&web_info->setting.common.url_encoding==0){
			if(strcmp(line_type->value,"1")==0){
				web_info->setting.common.url_encoding=1;
			}else if(strcmp(line_type->value,"2")==0){
				web_info->setting.common.url_encoding=2;
			}
		}
	}else if(strcmp(line_type->m_name,"parent_search")==0){
		if(strcmp(line_type->name,"type")==0&&web_info->setting.parent.type==NULL){
			web_info->setting.parent.type=strdup(line_type->value);
		}else if(strcmp(line_type->name,"ssl")==0&&web_info->setting.parent.port==0){
			if(strcmp(line_type->value,"0")==0){
				web_info->setting.parent.port=80;
			}else{
				web_info->setting.parent.port=443;
			}
		}else if(strcmp(line_type->name,"post_contents")==0&&web_info->setting.parent.post_contents==NULL){
			web_info->setting.parent.type=strdup(line_type->value);
		}else if(strcmp(line_type->name,"param")==0&&web_info->setting.parent.param==NULL){
			web_info->setting.parent.param=strdup(line_type->value);
		}else if(strcmp(line_type->name,"search_page_count")==0&&web_info->setting.parent.search_page_count==0){
			short check_type=strtol(line_type->name,NULL,10);
			if(errno!=ERANGE&&errno!=EINVAL){
				if(check_type<=2||check_type>=0){
					web_info->setting.parent.search_page_count=check_type;
				}
			}
		}
	}else if(strcmp(line_type->m_name,"child_search")==0){
		if(strcmp(line_type->name,"type")==0&&web_info->setting.child.type==NULL){
			web_info->setting.child.type=strdup(line_type->value);
		}else if(strcmp(line_type->name,"ssl")==0&&web_info->setting.child.port==0){
			if(strcmp(line_type->value,"0")==0){
				web_info->setting.child.port=80;
			}else{
				web_info->setting.child.port=443;
			}
		}else if(strcmp(line_type->name,"post_contents")==0&&web_info->setting.child.post_contents==NULL){
			web_info->setting.child.type=strdup(line_type->value);
		}else if(strcmp(line_type->name,"param")==0&&web_info->setting.child.param==NULL){
			web_info->setting.child.param=strdup(line_type->value);
		}
	}else{
		HS_free((void **)&(line_type->m_name));
		HS_free((void **)&(line_type->name));
		HS_free((void **)&(line_type->value));
		line_type->heap_size=0;
		return 0;
	}
	HS_free((void **)&(line_type->name));
	HS_free((void **)&(line_type->value));
	if(line_type->status&NORMAL){
		HS_free((void **)&(line_type->m_name));
	}
	line_type->heap_size=0;
	return 1;
}
void input_char(char **data,char c,int *heap_size,int *i){
	if(*heap_size==0){
		*heap_size=10;
		*data=malloc(*heap_size);
		(*data)[0]=0;
	}
	if(*i+1>=*heap_size){
		*heap_size+=10;
		*data=realloc(*data,*heap_size);
	}
	(*data)[*i]=c;
	(*data)[*i+1]=0;
	(*i)++;
}
short get_JSON_member(char **buff,Line_type *line_type){
	while(1){
		(*buff)++;
		if(is_line_feed((*buff)[0])){
		}else if((*buff)[0]==0x22){
			if(line_type->status&WRITTING){
				(*buff)++;
				line_type->heap_size=0;
				line_type->i=0;
				line_type->status&=~WRITTING;
				if(line_type->status&NORMAL || line_type->status&FIND_MEM_NAME){
					while(is_line_feed((*buff)[0])){
						(*buff)++;
					}
					if((*buff)[0]==0x3a){
						(*buff)++;
						while(is_line_feed((*buff)[0])){
							(*buff)++;
						}
					}else{
						line_type->status|=SYNTAX_ERROR;
						return 0;
					}
					
					if((*buff)[0]==0x7b && line_type->status&NORMAL){
						(*buff)++;
						while(is_line_feed((*buff)[0])){
							(*buff)++;
						}
					}else if(line_type->status&NORMAL){
						line_type->status|=SYNTAX_ERROR;
						return 0;
					}
					if((*buff)[0]==0x22){
						if(line_type->status&NORMAL){
							line_type->status&=~NORMAL;
							line_type->status|=FIND_MEM_NAME;
						}else if(line_type->status&FIND_MEM_NAME){
							line_type->status&=~FIND_MEM_NAME;
							line_type->status|=FIND_MEM_VAL;
						}else{
							line_type->status|=SYNTAX_ERROR;
							return 0;
						}
					}else{
						line_type->status|=SYNTAX_ERROR;
						return 0;
					}
					line_type->status|=WRITTING;
				}else if(line_type->status&FIND_MEM_VAL){
					line_type->status&=~FIND_MEM_VAL;
					while(is_line_feed((*buff)[0])){
						(*buff)++;
					}
					if((*buff)[0]==0x2c){
						line_type->status|=FIND_MEM_NAME;
					}else if((*buff)[0]==0x7d){
						line_type->status|=NORMAL;
						(*buff)++;
						while(is_line_feed((*buff)[0])){
							(*buff)++;
						}
						if((*buff)[0]==0x2c){
						}else if((*buff)[0]==0x7d){
							line_type->status|=READ_SUCCESS;
							return 1;
						}else{
							line_type->status|=SYNTAX_ERROR;
							return 0;
						}
					}
					return 1;
				}
			}else{
				line_type->status|=WRITTING;
			}
		}else{
			if(!(line_type->status&WRITTING)){
				line_type->status|=SYNTAX_ERROR;
				return 0;
			}
			if((*buff)[0]==0x5C){
				(*buff)++;
				if((*buff)[0]!=0x5C&&(*buff)[0]!=0x22){
					line_type->status|=SYNTAX_ERROR;
					return 0;
				}
			}
			if(line_type->status&NORMAL){
				input_char(&(line_type->m_name),(*buff)[0],&(line_type->heap_size),&(line_type->i));
			}else if(line_type->status&FIND_MEM_NAME){
				input_char(&(line_type->name),(*buff)[0],&(line_type->heap_size),&(line_type->i));
			}else{
				input_char(&(line_type->value),(*buff)[0],&(line_type->heap_size),&(line_type->i));
			}
		}
	}
}
short simple_JSON_parser(char *buff,Web_info *web_info){
	char *point;
	Line_type line_type;
	line_type.heap_size=0;
	line_type.status=NORMAL;
	line_type.i=0;
	line_type.m_name=NULL;
	line_type.name=NULL;
	line_type.value=NULL;
	int error;
	if(buff[0]!=0x7b){
		line_type.status|=SYNTAX_ERROR;
		error=0;
	}else{
		point=buff;
		while(is_line_feed(point[0])){
			point++;
		}
		if(point[1]==0x7D){
			return 1;
		}
		while((error=get_JSON_member(&buff,&line_type))==1){
			if(insert_set(web_info,&line_type)==0){
				line_type.status|=SYNTAX_ERROR;
				error=0;
				break;
			}
			if(line_type.status&READ_SUCCESS){
				error=2;
				break;
			}
		}
	}
	if(error==0){
		if(line_type.status&SYNTAX_ERROR){
			HS_free((void **)&(line_type.m_name));
			HS_free((void **)&(line_type.name));
			HS_free((void **)&(line_type.value));
			return 0;
		}
	}else if(error==2){
		HS_free((void **)&(line_type.m_name));
		HS_free((void **)&(line_type.name));
		HS_free((void **)&(line_type.value));
		return 1;
	}
}
int request_setting(char *buf,request *req){
	char decimal_str[10];
	int id,i=0;
	short end,check=0;
	size_t len;
	char *save_JSON;
	char *point;
	int url_count=0;
	while(1){
		if(buf[0]==0){
			break;
		}
		if(check==0){
			do{
				decimal_str[i]=buf[0];
				buf++;
				i++;
			}while(buf[0]!='\r'&&buf[0]!=0);
			decimal_str[i]=0;
			id=strtol(decimal_str,NULL,10);
			if(id==0){
				return 0;
			}else{
				check=1;
				if(buf[0]=='\r'&&buf[1]=='\r'&&buf[2]=='\n'){
					buf+=3;
				}else{
					return 0;
				}
			}
		}else if(check==1){
			point=strstr(buf,"\r\r\n");
			if(point==NULL){
				return 0;
			}
			len=point-buf;
			req->goods=calloc(len+1,sizeof(char));
			strncat(req->goods,buf,len);
			buf=point;
			buf+=3;
			check++;
		}else if(check==2){
			req->url_count++;
			if(insert_information(req)==0){
				return 0;
			}
			req->info[req->url_count-1]->isparent=1;
			point=strstr(buf,"\r\r\n");
			if(point==NULL){
				return 0;
			}
			len=point-buf;
			req->web_info[req->url_count-1]->hostname=calloc(len+1,sizeof(char));
			strncat(req->web_info[req->url_count-1]->hostname,buf,len);
			buf=point;
			buf+=3;
			check++;
		}else if(check==3){
			point=strstr(buf,"\r\r\n");
			if(point==NULL){
				return 0;
			}
			len=point-buf;
			save_JSON=calloc(len+1,sizeof(char));
			memcpy(save_JSON,buf,len);
			save_JSON[len]=0;
			if(simple_JSON_parser(save_JSON,req->web_info[req->url_count-1])==0){
				req->info[req->url_count-1]->connected|=DATA_READ_FAILED;
			}
			HS_free((void **)&save_JSON);
			buf=point;
			buf+=3;
			if(buf[0]==0){
				break;
			}else{
				point=buf;
				check=2;
			}
		}
	}
}
int read_file_setting(request *req){
	for(int i=0;i<req->url_count;i++){
		httpURL *info=req->info[i];
		char *file_name=makeStr("./data/",info->web_info->hostname,(char *)NULL);
		if(access(file_name,F_OK)==0){
			FILE *fp=NULL;
			char *buff=malloc(4096);
			char *temp=NULL;
			size_t len=4096;
			int read_size;
			short error;
			fp=fopen(file_name,"r");
			if(fp==NULL){
				printf("can't read setting file\n");
				error=0;
			}else{
				if((read_size=fread(buff,1,len,fp))>0){
					buff[read_size]=0;
					error=simple_JSON_parser(buff,info->web_info);
				}else{
					error=0;
				}
				fclose(fp);
			}
			HS_free((void **)&file_name);
			HS_free((void **)&buff);
			if(error==0){
				info->connected=DATA_READ_FAILED;
				info->connected|=M_ERROR;
			}
		}else{
			HS_free((void **)&file_name);
			info->connected=DATA_READ_FAILED;
			info->connected|=M_ERROR;
		}
	}
}
short info_setting(httpURL *info){
	if(get_port(info)==0){
		info->connected=UNKNOWN_PORT;
		info->connected|=M_ERROR;
		return 0;
	}
	if(info->web_info->setting.common.type!=NULL){
		info->type=malloc(strlen(info->web_info->setting.common.type)+1);
		strcpy(info->type,info->web_info->setting.common.type);
	}else if(info->isparent==1&&info->web_info->setting.parent.type!=NULL){
		info->type=malloc(strlen(info->web_info->setting.parent.type)+1);
		strcpy(info->type,info->web_info->setting.parent.type);
	}else if(info->isparent==0&&info->web_info->setting.child.type!=NULL){
		info->type=malloc(strlen(info->web_info->setting.child.type)+1);
		strcpy(info->type,info->web_info->setting.child.type);
	}else{
		info->connected=DATA_READ_FAILED;
		info->connected|=M_ERROR;
		return 0;
	}
	if(convert_param(info)==0){
		info->connected=DATA_READ_FAILED;
		info->connected|=M_ERROR;
		return 0;
	}
	make_http_header(info);
}
