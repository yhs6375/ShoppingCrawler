#include "HSFirst.h"
#include "HSRegex.h"

struct s_reg full_regexec(char *exp,char *str){
	regex_t reg_t;
	int ret,len;
	struct s_reg reg;
	int size1=0;
	char **result=(char**)malloc((size1)*sizeof(char*));
	reg.result=result;
	int i;
	regmatch_t pmatch;
	regcomp(&reg_t,exp,REG_EXTENDED);
	i=0;
	while((ret=regexec(&reg_t,str,1,&pmatch,0))==0){
		if((size1+1)<=i){
			size1++;
			result=(char**)realloc(result,(size1+1)*sizeof(char*));
			if(result==0){
				exit(1);
			}
		}
		len=pmatch.rm_eo-pmatch.rm_so;
		result[i]=(char*)calloc(sizeof(char),len);
		strncpy(result[i],str+pmatch.rm_so,len);
		i++;
		str+=pmatch.rm_eo;
	}
	
	reg.length=size1;
	regfree(&reg_t);
	return reg;
}
int free_regex(struct s_reg reg){
	int length=reg.length,i;
	for(i=0;i<length;i++){
		free(reg.result[i]);
	}
	
	free(reg.result);
	return 1;
}
