/**
* Linux C 支持正则表达式的字符串替换函数
*
* Author: cnscn@163.com
* Homepage: www.cnscn.org  欢迎您到cns家园来，有好吃的招待哟
* Date:   2007-03-08 17:41
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

//regex
#include <regex.h>


//cns_reg函数的返回类型
typedef struct _reg_rtn_struct
{
   int rtn;       //成功与否标志0 成功， 1失败
   int pstart;    //匹配到的子串开始位移
   int pend;      //匹配到的子串尾部位移
} reg_rtn_struct;


/**
*
* 正则表达式查找函数
*/
reg_rtn_struct cns_reg(const char *str,  const char *pattern)
{
    reg_rtn_struct reg_rtn_struct_var;

    int          z;            //status
    int          pos;          //配置处的位置
    int          cflags = REG_EXTENDED;   //compile flags
    regex_t      reg;          //compiled regular expression
    char         ebuf[128];    //error buffer
    bzero(ebuf, sizeof(ebuf));
    regmatch_t   pm[10];       //pattern matches 0-9
    bzero(pm, sizeof(pm));
    const size_t nmatch = 10;  //The size of array pm[]

    //编译正则表达式
    /**
     *
     * @param const char*  pattern         将要被编译的正则表达式
     * @param regex_t*     reg             用来保存编译结果
     * @param int          cflags          决定正则表达式将如何被处理的细节
     *
     * @return  success    int        0    并把编译结果填充到reg结构中
     *          fail       int        非0
     *
     */


    z = regcomp(&reg, (const char*)pattern, cflags);

    if(z)   //此处为 if(z != 0), 因为C语言里0永远为非(False), 任何非0值都为真(True)
    {
       regerror(z, &reg, ebuf, sizeof(ebuf));
       perror("reg1");
       fprintf(stderr, "%s: pattern '%s'\n", ebuf, pattern);
       reg_rtn_struct_var.rtn    = 1;
       reg_rtn_struct_var.pstart = -1;
       reg_rtn_struct_var.pend   = -1;

       regfree(&reg);
       return reg_rtn_struct_var;
    }

    /**
     *
     * reg     指向编译后的正则表达式
     * str     指向将要进行匹配的字符串
     * pm      str字符串中可能有多处和正则表达式相匹配， pm数组用来保存这些位置
     * nmacth  指定pm数组最多可以存放的匹配位置数
     *
     * @return 函数匹配成功后，str+pm[0].rm_so到str+pm[0].rm_eo是第一个匹配的子串
     *                           str+pm[1].rm_so到str+pm[1].rm_eo是第二个匹配的子串
     *                           ....
     */
    z = regexec(&reg, str, nmatch, pm, REG_EXTENDED);

    //没有找到匹配数据
    if(z == REG_NOMATCH)
    {
       reg_rtn_struct_var.rtn    = 1;
       reg_rtn_struct_var.pstart = -1;
       reg_rtn_struct_var.pend   = -1;

       regfree(&reg);
       return reg_rtn_struct_var;
    }
    else if(z)  //if(z !=0)
    {
       perror("reg3");
       regerror(z, &reg, ebuf, sizeof(ebuf));
       fprintf(stderr, "%s: regcomp('%s')\n", ebuf, str);

       reg_rtn_struct_var.rtn    = 1;
       reg_rtn_struct_var.pstart = -1;
       reg_rtn_struct_var.pend   = -1;
       regfree(&reg);

       return reg_rtn_struct_var;
    }

    /*列出匹配的位置*/
    if(pm[0].rm_so != -1)
    {
       reg_rtn_struct_var.rtn    = 0;
       reg_rtn_struct_var.pstart = pm[0].rm_so;
       reg_rtn_struct_var.pend   = pm[0].rm_eo;
    }

    regfree(&reg);
    return reg_rtn_struct_var;
}


/*
* 正则表达式替换函数
*/
char *cns_str_ereplace(char *src, const char *pattern, const char *newsubstr)
{
    //如果pattern和newsubstr串相等，则直接返回
    if(!strcmp(pattern, newsubstr))   //if(strcmp(pattern, newsubstr)==0)
       return src;

    //定义cns_reg的返回类型结构变量
    reg_rtn_struct  reg_rtn_struct_var;
    int rtn    = 0;   //reg_rtn_struct_var.rtn
    int pstart = 0;   //reg_rtn_struct_var.pstart
    int pend   = 0;   //reg_rtn_struct_var.pend


    //把源串预dest
    char *dest=src;  //替换后生成的串指针
    char *pstr=src;  //当找到串时，pstr就指向子串后面的地址从而标识下一个要查找的源串

    //用于malloc的临时内存区
    char *tmp;
    char *new_tmp_str=dest;

    int new_tmp_str_len=0;  //new_tmp_str相对于dest地址开始处的长度


    //开始循环替换src串中符合pattern的子串为newstr
    while(!rtn)
    {
        reg_rtn_struct_var=cns_reg(new_tmp_str, pattern);

        rtn    = reg_rtn_struct_var.rtn;
        pstart = reg_rtn_struct_var.pstart;
        pend   = reg_rtn_struct_var.pend;

        if(!rtn)
        {
            //分配新的空间: strlen(newstr):新串长  pend-pstart:旧串长
            tmp=(char*)calloc(sizeof(char), strlen(dest)+strlen(newsubstr)-(pend-pstart)+1 );

            //把src内的前new_tmp_str_len+pstart个内存空间的数据，拷贝到arr
            strncpy(tmp, dest, new_tmp_str_len+pstart);

            //标识串结束
            tmp[new_tmp_str_len+pstart]='\0';

            //连接arr和newstr, 即把newstr附在arr尾部, 从而组成新串(或说字符数组)arr
            strcat(tmp, newsubstr);

            //把src中 从oldstr子串位置后的部分和arr连接在一起，组成新串arr
            strcat(tmp, new_tmp_str+pend);

            //把用malloc分配的内存，复制给指针dest
            dest = strdup(tmp);

            //释放malloc分配的内存空间
            free(tmp);

            new_tmp_str_len = new_tmp_str_len + pstart + strlen(newsubstr);
            new_tmp_str=dest+new_tmp_str_len;
        }
    }

    return dest;
}


int main()
{
  //测试正则表达式
  char str[]="唱的曲子是北宋大$natureOrder(0123)“蝶恋花”词，写的正\n怪客手掌一碰，只把她牙齿撞得隐隐生痛。$natureOrderDESC(8)开";

  reg_rtn_struct reg_rtn_struct_var;

  char *newstr=cns_str_ereplace(str,"\\$.{4,6}Order(DESC)?\\([0-9]{1,4}\\)","hello");
  puts(newstr);

  return 0;
}
