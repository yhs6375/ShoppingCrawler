#pragma once
typedef struct s_reg{
	int length;
	char **result;
}HS_Regex;

struct s_reg full_regexec(char *exp,char *str);
int free_regex(struct s_reg reg);
