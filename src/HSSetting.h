#pragma once
#define READING			0x0001
#define	FIND_MEM_NAME	0X0002
#define FIND_MEM_VAL	0X0004
#define NORMAL			0X0008
#define SYNTAX_ERROR	0X0010
#define READ_ERROR		0X0020
#define WRITTING		0X0040
#define READ_SUCCESS	0X0080

#define R_OBJECT		0X0001
#define R_PARAM		0X1000
typedef struct Line_type{
	int status;
	int i;
	int heap_size;
	char *m_name;
	char *name;
	char *value;
}Line_type;
void setting_init(Web_info *web_info);
short simple_JSON_parser(char *buff,Web_info *web_info);
int request_setting(char *buf,request *req);
int read_file_setting(request *req);
short info_setting(httpURL *info);
