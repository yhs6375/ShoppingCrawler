#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <regex.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <wchar.h>
#include <time.h>
#include <locale.h>
#include <openssl/ssl.h>
#include <iconv.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <zlib.h>
#include <sys/un.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <math.h>

#define BUFF_SIZE				400000
#define MAX_EVENT_NUM			1
#define THREAD_LIMIT			9
#define URL_FAILED				0x00001
#define URL_SUCCESS				0x00002
#define GET_HTML_SUCCESS		0x00004
#define CONNECT_ERROR			0x00008
#define CONNECT_SUCCESS			0x00010
#define PARSER_SUCCESS			0x00020
#define PARSER_PROGRESS			0x00040
#define PARSER_ERROR			0x00080
#define N_MATCH_HTML			0x00100
#define CONNECT_PROGRESS		0x00200
#define CONNECT_TIMEOUT			0x00400
#define CONNECT_REFUSE			0x00800
#define CONNECT_UNKNOWN			0x01000
#define SSL_CONNECT_PROGRESS	0X02000
#define SSL_CONNECT_SUCCESS		0X04000
#define DATA_READ_FAILED		0X08000
#define UNKNOWN_HOST			0X10000
#define UNKNOWN_PORT			0X20000
#define M_ERROR					0X40000
#define N_STATUS_NORMAL			0X80000

#define EPOLL_ERR				0X00001
#define REQUEST_ERR				0X00002


struct ups_socket_st{
	char *buf;
	int ups_fd;
	size_t heap_size;
};
typedef struct ep_struct{
	struct epoll_event newPeerConnectionEvent;
	struct epoll_event *processableEvents;
	int epoll_fd;
}ep_struct;
typedef struct connecting_info{
	int *list;
	int connecting;
	int heap_size;
	int info_count;
}connecting_info;
typedef struct http_header{
	char *http_v;
	char *host;
	char *connection;
	char *user_agent;
	char *accept;
	char *accept_language;
	char *accept_encoding;
}http_header;
typedef struct search_expression{
	short site_type;
	short page_search_type;
	char *page_check_container;
	char *page_check_exp;
	short this_page_count;
	char *goods_count;
	char *goods_list1_container1;
	char *goods_list1_container2;
}search_expression;
typedef struct common_search{
	char *enc_type;
	char *type;
	int port;
	char *post_contents;
	char *param;
	short url_encoding;
}common_search;
typedef struct parent_search{
	char *type;
	int port;
	char *post_contents;
	char *param;
	short search_page_count;
}parent_search;
typedef struct child_search{
	char *type;
	int port;
	char *post_contents;
	char *param;
}child_search;
typedef struct Web_data{
	http_header header;
	search_expression search;
	common_search common;
	parent_search parent;
	child_search child;
}Web_data;
typedef struct Web_info{
	Web_data setting;
	char *hostname;
	char ip[15];
	int total_count;
	int finish_count;
	int error_count;
	int connect_count;
	char *goods;
	int remain_page_count;
	short isconnecting;
}Web_info;
typedef struct httpURL{
	Web_info *web_info;
	int socket;
	int port;
	char *param;
	int connected;
	char *buff;
	char *html;
	int buff_len;
	int heap_size;
	char *rq_header;
	SSL *ssl_con;
	char enc_type[15];
	char compress[11];
	char *type;
	int count;
	short isparent;
	char *post_contents;
	time_t timeout_s;
	long timeout_ms;
	short header_check;
	int status_code;
	unsigned int header_len;
	unsigned int content_len;
	struct httpURL *next;
}httpURL;
typedef struct request{
	int id;
	httpURL **info;
	Web_info **web_info;
	char *goods;
	int connecting_count;
	int url_count;
	int info_count;
	struct request *next;
	struct request *prev;
	ep_struct ep_st;
	int error;
}request;
struct waiting_req{
	request *cur;
	struct waiting_req *next;
};
typedef struct hosung{
	request *start;
	request *end;
	struct waiting_req *waiting;
	size_t count;
}hosung;
hosung *HS;
SSL_CTX *ssl_ctx;
pthread_mutex_t mutex[8];
int hostname_to_ip(char * hostname , char* ip);
void free_req(request *req);
void info_init(httpURL *info);
short insert_information(request *req);
request *request_init();
