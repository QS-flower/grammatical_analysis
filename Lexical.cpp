#include <iostream>
#include <cstring>
#include <unistd.h>
#define N 512
using namespace std;

char *pwd;          // 当前文件的绝对路径
FILE *pas;          // 源文件
FILE *err;          // 错误信息文件
FILE *dyd;          // 生成的文件
char token[N] = ""; // 当前字符串
char cha;           // 当前字符
int line = 1;       // 行数

const int type_begin = 1;
const int type_end = 2;
const int type_integer = 3;
const int type_if = 4;
const int type_then = 5;
const int type_else = 6;
const int type_function = 7;
const int type_read = 8;
const int type_write = 9;
const int type_identifier=10;
const int type_num=11;
const int type_eq=12;
const int type_ne=13;
const int type_lq=14;
const int type_l=15;
const int type_rq=16;
const int type_r=17;
const int type_minus=18;
const int type_star=19;
const int type_assign=20;
const int type_lb=21;
const int type_rb=22;
const int type_s=23;
const int type_EOLN=24;
const int type_EOF=25;

void input()
{
    cha=fgetc(pas);
}

void output(char *token, int type)
{
    fprintf(dyd, "%16s %2d\n", token, type);
}

void error(char *msg)
{
    fprintf(err, "***LINE:%d  %s\n", line, msg);
}

void concat()
{
    strcat(token, &cha);
    return;
}

void retract()
{
    ungetc(cha, pas);
}
int reserve()
{
    if (strcmp(token, "begin") == 0)
    {
        return type_begin;
    }
    else if (strcmp(token, "end")== 0)
    {
        return type_end;
    }
    else if (strcmp(token, "integer")== 0)
    {
        return type_integer;
    }
    else if (strcmp(token, "if")== 0)
    {
        return type_if;
    }
    else if (strcmp(token, "then")== 0)
    {
        return type_then;
    }
    else if (strcmp(token, "else")== 0)
    {
        return type_else;
    }
    else if (strcmp(token, "function")== 0)
    {
        return type_function;
    }
    else if (strcmp(token, "read")== 0)
    {
        return type_read;
    }
    else if (strcmp(token, "write")== 0)
    {
        return type_write;
    }
    else
    {
        return 0;
    }
    return 0;
}
bool letter()
{
    return cha >= 'a' && cha <= 'z' || cha >= 'A' && cha <= 'Z';
}
bool digit()
{
    return cha >= '0' && cha <= '9';
}
void getnbc()
{
    char temp = fgetc(pas);
    while (temp == ' ' || temp == '\t' || temp == '\r' || temp == '\v' || temp == '\f')
        temp = fgetc(pas);
    cha = temp;
}
void fileOpen(char *filename)
{
    // 获取当前文件绝对路径
    pwd = getcwd(NULL, 0);

    // 获取源文件路径，并进行操作
    char pasFile[N] = "";
    strcat(pasFile, pwd);
    strcat(pasFile, "\\");
    strcat(pasFile, filename);
    strcat(pasFile, ".pas");

    // 设置dyd文件路径
    char dydFile[N] = "";
    strcat(dydFile, pwd);
    strcat(dydFile, "\\");
    strcat(dydFile, filename);
    strcat(dydFile, ".dyd");

    // 设置err文件路径
    char errFile[N] = "";
    strcat(errFile, pwd);
    strcat(errFile, "\\");
    strcat(errFile, filename);
    strcat(errFile, ".err");

    pas = fopen(pasFile, "r");
    dyd = fopen(dydFile, "w");
    err = fopen(errFile, "w");
}

void fileClose()
{
    fclose(pas);
    fclose(err);
    fclose(dyd);
}
//符号表
int buildlist(){
    return type_identifier;
}
int dtb(){
    return type_num;
}
bool LexAnalyze()
{
    memset(token, '\0', N);
    getnbc();
    switch (cha)
    {
    case 'a':
    case 'A':
    case 'b':
    case 'B':
    case 'c':
    case 'C':
    case 'd':
    case 'D':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G':
    case 'h':
    case 'H':
    case 'i':
    case 'I':
    case 'j':
    case 'J':
    case 'k':
    case 'K':
    case 'l':
    case 'L':
    case 'm':
    case 'M':
    case 'n':
    case 'N':
    case 'o':
    case 'O':
    case 'p':
    case 'P':
    case 'q':
    case 'Q':
    case 'r':
    case 'R':
    case 's':
    case 'S':
    case 't':
    case 'T':
    case 'u':
    case 'U':
    case 'v':
    case 'V':
    case 'w':
    case 'W':
    case 'x':
    case 'X':
    case 'y':
    case 'Y':
    case 'z':
    case 'Z':
        {
            while (digit()||letter()){
                concat();
                input();
            }
            retract();
            int type=reserve();
            if(type){
                output(token,type);
            }else{
                if(strlen(token)>16) error((char *)"标识符长度溢出");
                else{
                    int num=buildlist();
                    output(token, num);
                }
            }
            break;
        }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        {
            while(digit()){
                concat();
                input();
            }
            retract();
            int type=dtb();
            output(token,type);
            break;
        }
    case '=':
        {
            output((char *)"=",type_eq);
        }
    case '<':
        {
            input();
            if(cha=='>'){
                output((char *)"<>",type_ne);
            }else if(cha=='='){
                output((char *)"<=",type_lq);
            }else{
                retract();
                output((char *)"<",type_l);
            }
            break;
        }
    case '>':
        {
            input();
            if(cha=='=') output((char *)">=",type_rq);
            else{
                retract();
                output((char *)">",type_r);
            }
            break;
        }
    case '-':
        {
            output((char *)"-",type_minus);
            break;
        }
    case '*':
        {
            output((char *)"*",type_star);
            break;
        }
    case ':':
        {
            input();
            if(cha=='=') output((char *)":=",type_assign);
            else{
                retract();
                error((char *)"冒号不匹配");
            }
            break;
        }
    case '(':
        {
            output((char *)"(",type_lb);
            break;
        }
    case ')':
        {
            output((char *)")",type_rb);
            break;
        }
    case ';':
        {
            output((char *)";",type_s);
            break;
        }
    case EOF:
        {
            output((char *)"EOF",type_EOF);
            return false;
            break;
        }
    case '\n':
        {
            output((char *)"EOLN",type_EOLN);
            line++;
            break;
        }
    default:
        {
            error((char *)"非法字符");
            break;
        }
    }
    return true;
}
int main(int argc, char *argv[])
{
    if (argc != 2)
        exit(-1);
    fileOpen(argv[1]);
    while(LexAnalyze());
    fileClose();
}