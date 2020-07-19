#pragma once
int init_SSL();
int connecting_check(request *req);
int ssl_connect(httpURL *info);
int get_port(httpURL *info);
int hostname_to_ip(char *hostname,char *ip);
int ups_server_create();
void status_con_set(int *status,int set);
int connect_start(httpURL *info,request *req);
short child_page_connect(httpURL *info,request *req);
