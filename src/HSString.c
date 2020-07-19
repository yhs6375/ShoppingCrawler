#include "HSFirst.h"
#include "HSString.h"
#include "HSUtil.h"
short is_line_feed(char check){
	if(check==0x09||check==0x20||check==0x0A||check==0x0D){
		return 1;
	}
	return 0;
}
int ltrim_check_len(char **point,char *end){
	char *point2;
	while(is_line_feed((*point)[0])){
		(*point)++;
	}
	point2=*point;
	if((point2=strstr(*point,"\r\n"))==NULL){
		return -1;
	}
	return point2-*point;
}
char *hsgstrstr(char *point,char *end){
	char *point2;
	unsigned int len;
	point2=point;
	if((point2=strstr(point,"\r\n"))==NULL){
		return NULL;
	}
	len=point2-point;
	point2=malloc(len+1);
	memcpy(point2,point,len);
	point2[len]=0;
	return point2;
}
int hsstrtol(char *n_str,char **endptr,int base){
	int len=strtol(n_str,endptr,base);
	if(errno==EINVAL||errno==ERANGE){
		return -1;
	}
	return len;
}
char* makeStr(const char *first, ...){
	if(first==NULL){
		return NULL;
	}
	va_list arguments;
	int j;
	int len=0;
	int len2;
	char *temp_str;
	va_start(arguments,first);
	len+=strlen(first);
	while((temp_str=va_arg(arguments,char *))!=NULL){
		len+=strlen(temp_str);
	}
	va_end(arguments);
	
	char *result=(char *)malloc(len+1);
	len=0;
	va_start(arguments,first);
	len2=strlen(first);
	for(j=0;j<len2;j++){
		result[j]=first[j];
	}
	len+=len2;
	while((temp_str=va_arg(arguments,char *))!=NULL){
		len2=strlen(temp_str);
		for(j=0;j<len2;j++){
			result[len+j]=temp_str[j];
		}
		len+=len2;
	}
	va_end(arguments);
	result[len]=0;
	return result;
}
short convert_variable(char *var,httpURL *info,char **converted_var){
	if(strcmp(var,"goods")==0){
		*converted_var=info->web_info->goods;
	}else if(strcmp(var,"hostname")==0){
		*converted_var=info->web_info->hostname;
	}else if(strcmp(var,"page_no")==0){
		*converted_var=malloc(4);
		sprintf(*converted_var,"%d",info->web_info->total_count);
		return 2;
	}else{
		return 0;
	}
	return 1;
}
short get_static_string(char *str,char **target,httpURL *info){
	char *point;
	char *var=NULL;
	char *converted_var=NULL;
	char *temp=NULL;
	short check;
	var=(char *)malloc(20);
	if(var==NULL){
		return 0;
	}
	size_t len;
	*target=malloc(1);
	*target[0]=0;
	do{
		if(point=strstr(str,"|@")){
			len=point-str;
			temp=realloc(*target,strlen(*target)+len+1);
			if(temp==NULL){
				HS_free((void **)&var);
				return 0;
			}
			*target=temp;
			strncat(*target,str,len);
			point+=2;
			str=point;
			if(point=strstr(str,"@|")){
				len=point-str;
				if(len>19){
					HS_free((void **)&var);
					return 0;
				}
				memcpy(var,str,len);
				var[len]=0;
				if((check=convert_variable(var,info,&converted_var))==0){
					HS_free((void **)&var);
					return 0;
				}
				temp=realloc(*target,strlen(*target)+strlen(converted_var)+1);
				if(temp==NULL){
					if(check==2){
						HS_free((void **)&converted_var);
					}
					HS_free((void **)&var);
					return 0;
				}
				*target=temp;
				strcat(*target,converted_var);
				point+=2;
				str=point;
				if(check==2){
					HS_free((void **)&converted_var);
				}
			}else{
				HS_free((void **)&var);
				return 0;
			}
		}else{
			*target=realloc(*target,strlen(*target)+strlen(str)+1);
			strcat(*target,str);
			break;
		}
	}while(1);
	HS_free((void **)&var);
	return 1;
}
short convert_param(httpURL *info){
	short error;
	if(info->web_info->setting.common.param==NULL){
		if(info->isparent==1){
			error=get_static_string(info->web_info->setting.parent.param,&info->param,info);
		}else{
			error=get_static_string(info->web_info->setting.child.param,&info->param,info);
		}
	}else{
		error=get_static_string(info->web_info->setting.common.param,&info->param,info);
	}
	return error;
}
char *enc_type_convert(char *enc_type){
	char *new_enc=malloc(strlen(enc_type)+1);
	if(strcmp(enc_type,"ks_c_5601-1987")==0){
		strcpy(new_enc,"cp949");
	}else{
		strcpy(new_enc,enc_type);
	}
	return new_enc;
}
int target_to_utf8(char **input_str,char *enc_type){
	size_t input_len=strlen(*input_str);
	size_t output_len=input_len*1.5;
	char *output_str=calloc(output_len+1,1);
	char *inbuf=*input_str;
	char *outbuf=output_str;
	char *new_enc_type;
	new_enc_type=enc_type_convert(enc_type);
	iconv_t it=iconv_open("UTF-8",new_enc_type);
	while(1){
		int res=iconv(it,&inbuf,&input_len,&outbuf,&output_len);
		if(res==(size_t)-1){
			if(errno==EILSEQ){
				inbuf++;
				input_len--;
			}else if(errno==E2BIG){
				HS_free((void **)new_enc_type);
				return 0;
			}else if(errno==EINVAL){
				HS_free((void **)new_enc_type);
				return 0;
			}
			*outbuf='?';
			outbuf++;
			output_len--;
			inbuf++;
			input_len--;
		}else break;
	}
	iconv_close(it);
	HS_free((void **)input_str);
	HS_free((void **)&new_enc_type);
	*input_str=output_str;
	return 1;
}
int utf8_to_target(char **input_str,char *enc_type){
	size_t input_len=strlen(*input_str);
	size_t output_len=input_len*1.5;
	char *output_str=calloc(output_len+1,1);
	char *inbuf=*input_str;
	char *outbuf=output_str;
	iconv_t it=iconv_open(enc_type,"UTF-8");
	while(1){
		int res=iconv(it,&inbuf,&input_len,&outbuf,&output_len);
		if(res==(size_t)-1){
			if(errno==EILSEQ){
				inbuf++;
				input_len--;
			}else if(errno==E2BIG){
				return 0;
			}
			*outbuf='?';
			outbuf++;
			output_len--;
			inbuf++;
			input_len--;
		}else break;
	}
	iconv_close(it);
	HS_free((void **)input_str);
	*input_str=output_str;
	return 1;
}
int urlencode(char **source){
	unsigned char *save=*source;
	unsigned char *dest=calloc(strlen(*source)*3+1,1);
	dest[0]=0;
	unsigned char hex[4]="\0";
	int size=0;
	while(*save){
		if((*save>47&&*save<57) || (*save>64&&*save<92) || (*save>96&&*save<123) ||*save=='-'||*save=='.'||*save=='_'||*save=='+'){
			dest[strlen(dest)]=*save;
		}else{
			sprintf(hex,"%%%02X",*save);
			strncat(dest,hex,3);
		}
		save++;
	}
	HS_free((void **)source);
	*source=dest;
	*source[strlen(*source)]==0;
}
int unicode_urlencode(char **source){
	unsigned char *save=*source;
	unsigned char *dest=calloc(strlen(*source)*3+1,1);
	short check=0;
	dest[0]=0;
	unsigned char hex[4]="\0";
	int size=0;
	while(*save){
		if(*save=='-'||*save=='.'||*save=='_'||*save=='+'){
			if(check==1){
				HS_free((void **)&dest);
				HS_free((void **)source);
				return 0;
			}
			dest[strlen(dest)]=*save;
		}else if(check==0){
			sprintf(hex,"%%u%02X",*save);
			strncat(dest,hex,4);
			check++;
		}else{
			sprintf(hex,"%02X",*save);
			strncat(dest,hex,4);
			check=0;
		}
		save++;
	}
	if(check==1){
		HS_free((void **)&dest);
		HS_free((void **)source);
		return 0;
	}
	HS_free((void **)source);
	*source=dest;
	*source[strlen(*source)]==0;
	return 1;
}
int get_partial_length(char **str,char target){
	int i=0;
	char hex_str[6];
	do{
		hex_str[i]=(*str)[i];
		i++;
	}while((*str)[i]!='\r');
	hex_str[i]=0;
	*str+=i;
	return strtol(hex_str,NULL,16);
}
