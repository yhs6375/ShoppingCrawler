#pragma once
#define CHUNK 100000
void HS_free(void **target);
void hserror(httpURL *info,int error_code);
void delete_req_in_queue(hosung *hs,request *req);
short is_request_end(request *req);
int gzip_inflate(httpURL *info);
