#pragma once

#define OPEN_MARKET				0
#define CATEGORY_SHOPPING_MALL	1

#define CHECKING_PAGE_BOX		0
int make_http_header(httpURL *info);
int is_recv_end(char *buff,int len);
int get_HTML_at_buff(httpURL *info);
short header_and_end_check(httpURL *info);
short check_html(httpURL *info,request *req);
