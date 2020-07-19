#pragma once
short is_line_feed(char check);
int ltrim_check_len(char **point,char *end);
char *hsgstrstr(char *point,char *end);
int hsstrtol(char *n_str,char **endptr,int base);
char* makeStr(const char *first, ...);
short get_static_string(char *str,char **target,httpURL *info);
short convert_param(httpURL *info);
int target_to_utf8(char **input_str,char *enc_type);
int utf8_to_target(char **input_str,char *enc_type);
int urlencode(char **source);
int unicode_urlencode(char **source);
int get_partial_length(char **str,char target);
