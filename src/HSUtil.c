#include "HSFirst.h"
#include "HSUtil.h"
#include "HSString.h"
void HS_free(void **target){
	if(*target!=NULL){
		free(*target);
		*target=NULL;
	}
}
void hserror(httpURL *info,int error_code){
	info->connected=error_code;
	info->connected|=M_ERROR;
}
void delete_req_in_queue(hosung *hs,request *req){
	if(req->prev==NULL){
		hs->start=req->next;
		if(req->next==NULL){
			hs->end=NULL;
		}else{
			hs->start->prev=NULL;
		}
	}else{
		req->prev->next=req->next;
		if(req->next==NULL){
			hs->end=req->prev;
		}else{
			req->next->prev=req->prev;
		}
	}
	free_req(req);
}
short is_request_end(request *req){
	short check=1;
	for(int i=0;i<req->url_count;i++){
		Web_info *cur_w_info=req->web_info[i];
		if(cur_w_info->total_count!=cur_w_info->finish_count+cur_w_info->error_count){
			check=0;
		}
	}
	return check;
}
int gzip_inflate(httpURL *info){
	int ret;
	int heap_size=150000;
	size_t check_len=0;
	unsigned have;
	z_stream strm;
	unsigned char *out=malloc(heap_size+1);
	void *new_pointer;
	out[0]=0;
	out[heap_size]=0;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	strm.avail_out=0;
	ret=inflateInit2(&strm,16+MAX_WBITS);
	if (ret != Z_OK)
		return ret;
	strm.avail_in=info->buff_len;
	strm.next_in=info->html;
	strm.avail_out = 150000;
	do{
		if(heap_size<check_len+CHUNK){
			heap_size+=CHUNK;
			if(new_pointer=realloc(out,heap_size+1)){
				out=new_pointer;
				out[heap_size]=0;
			}else{
				HS_free((void **)&out);
				inflateEnd(&strm);
				return 0;
			}
		}
		strm.next_out=out+check_len;
		ret=inflate(&strm,Z_NO_FLUSH);
		if(ret<0){
			inflateEnd(&strm);
			return 0;
		}
		if(strm.avail_in==0){
			out[check_len+(CHUNK-strm.avail_out)]=0;
			free(info->html);
			info->html=out;
			inflateEnd(&strm);
			return 1;
		}
		strm.avail_out=CHUNK;
		
		check_len=strlen(out);
	}while(1);
}
