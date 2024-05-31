#include <iostream>
#include <unistd.h>
#include <vector>
#define N 512
#define M 17
#define nowToken dydlist[dydnum].token
#define nowType dydlist[dydnum].type

using namespace std;

const int BEGIN_TYPE = 1;
const int END_TYPE = 2;
const int INTEGER_TYPE = 3;
const int IF_TYPE = 4;
const int THEN_TYPE = 5;
const int ELSE_TYPE = 6;
const int FUNCTION_TYPE = 7;
const int READ_TYPE = 8;
const int WRITE_TYPE = 9;
const int SYMBOL_TYPE = 10;
const int CONSTANT_TYPE = 11;
const int EQ_TYPE = 12;
const int NE_TYPE = 13;
const int LE_TYPE = 14;
const int LT_TYPE = 15;
const int GE_TYPE = 16;
const int GT_TYPE = 17;
const int MINUS_TYPE = 18;
const int MUL_TYPE = 19;
const int ASSIGN_TYPE = 20;
const int LEFT_PARE_TYPE = 21;
const int RIGHT_PARE_TYPE = 22;
const int SEM_TYPE = 23;
const int EOLN_TYPE = 24;
const int EOF_TYPE = 25;

FILE *dydfile;
FILE *errfile;
FILE *profile;
FILE *varfile;
FILE *dysfile;

char *pwd;
int nowLine = 1;
int level = 1;
char proc[M] = "main";

typedef struct dydItem
{
    char token[M];
    int type;
} dydItem;

typedef struct varItem
{
    char vname[M];
    char vproc[M];
    int vkind;
    int vtype;
    int vlev;
    int vadr;
    bool isDeclared;
} varItem;

typedef struct proItem
{
    char pname[M];
    int ptype;
    int plev;
    int fadr;
    int ladr;
} proItem;

typedef struct errItem
{
    char info[N];
    int line;
    bool isSerious;
} errItem;

vector<dydItem> dydlist;
int dydnum = 0;
vector<varItem> varlist;
vector<proItem> prolist;
vector<errItem> errlist;

void advance()
{
    while (dydlist[++dydnum].type == EOLN_TYPE)
        nowLine++;
}
void retract()
{
    while (dydlist[--dydnum].type == EOLN_TYPE)
        nowLine--;
}
// 文件操作
void openfile(char *filename)
{

    pwd = getcwd(NULL, 0);

    char dyd[N] = "";
    strcat(dyd, pwd);
    strcat(dyd, "\\");
    strcat(dyd, filename);
    strcat(dyd, ".dyd");

    char err[N] = "";
    strcat(err, pwd);
    strcat(err, "\\");
    strcat(err, filename);
    strcat(err, ".err");

    char var[N] = "";
    strcat(var, pwd);
    strcat(var, "\\");
    strcat(var, filename);
    strcat(var, ".var");

    char pro[N] = "";
    strcat(pro, pwd);
    strcat(pro, "\\");
    strcat(pro, filename);
    strcat(pro, ".pro");

    char dys[N] = "";
    strcat(dys, pwd);
    strcat(dys, "\\");
    strcat(dys, filename);
    strcat(dys, ".dys");

    dydfile = fopen(dyd, "r");
    errfile = fopen(err, "w");
    varfile = fopen(var, "w");
    profile = fopen(pro, "w");
    dysfile = fopen(dys, "w");
}
void closefile()
{
    fclose(dydfile);
    fclose(errfile);
    fclose(varfile);
    fclose(profile);
}

void dydInput()
{
    dydItem part;
    while (fscanf(dydfile, "%s %d", part.token, &part.type) != EOF)
        dydlist.push_back(part);
}
void dysOutput()
{
    rewind(dydfile);
    char buf[M];
    int res = 0;
    while ((res = fread(buf, 1, M, dydfile)) != 0)
        fwrite(buf, 1, res, dysfile);
}
void varOutput()
{
    for (auto it : varlist)
    {
        if (it.isDeclared == 0)
            continue; // 形参不输出
        fprintf(varfile, "%16s %16s %d integer %d %d\n", it.vname, it.vproc, it.vkind, it.vlev, it.vadr);
    }
}
void procOutput()
{
    // 更新ladr,fadr
    for (proItem &it : prolist)
    {
        for (auto var : varlist)
        {
            if (strcmp(it.pname, var.vproc) == 0)
            {
                if (it.fadr == -1)
                    it.fadr = var.vadr;
                if (var.vkind)
                    it.ladr = var.vadr;
            }
        }
    }
    // 输出
    for (auto it : prolist)
    {
        fprintf(profile, "%16s integer %d %d %d\n", it.pname, it.plev, it.fadr, it.ladr);
    }
}
void errOutput()
{
    int errline = 0;
    for (auto it : errlist)
    {
        if (errline == it.line && !it.isSerious)
            continue;
        if (it.isSerious)
            fprintf(errfile, "***LINE:%d 重大语法错误：%s\n", it.line, it.info);
        else
        {
            fprintf(errfile, "***LINE:%d %s\n", it.line, it.info);
        }
        errline = it.line;
    }
}
void Output()
{
    if (errlist.empty())
        dysOutput();
    varOutput();
    procOutput();
    errOutput();
}
void error(const char *info, bool serious)
{
    errItem temp;
    temp.line = nowLine;
    strcpy(temp.info, info);
    temp.isSerious = serious;
    errlist.push_back(temp);
}
void seriousError(const char *info)
{
    error(info, true);
    varOutput();
    procOutput();
    errOutput();
    closefile();
    exit(1);
}
bool varIsDeclared()
{
    for (varItem &it : varlist)
    {
        if (strcmp(nowToken, it.vname) == 0 && level >= it.vlev)
            if (it.isDeclared == 1)
                return true;
    }
    return false;
}
void addVar()
{
    varItem temp;
    strcpy(temp.vname, nowToken);
    strcpy(temp.vproc, proc);
    temp.vkind = 0;
    temp.vtype = INTEGER_TYPE;
    temp.vlev = level;
    temp.vadr = varlist.size();
    temp.isDeclared = 1;
    varlist.push_back(temp);
}
bool symIsDeclared()
{
    for (auto it : varlist)
    {
        if (strcmp(nowToken, it.vname) == 0 && level >= it.vlev && it.isDeclared == 1)
        {
            return true;
        }
    }
    if (strcmp(nowToken, proc) == 0)
        return true;
    return false;
}
bool procIsDeclared()
{
    for (auto it : prolist)
    {
        if (strcmp(nowToken, it.pname) == 0)
            return true;
    }
    return false;
}

// 文法
void Proc();
void SubProc();
void DeclareStateTable();
void DeclareStateTablePrime();
void DeclareState();
void varDeclare();
void var();
void funcDeclare();
void parameter();
void funcBody();
void execStateTable();
void execStateTablePrime();
void execState();
void read();
void write();
void assign();
void calExpress();
void calExpressPrime();
void item();
void itemPrime();
void factor();
void funcCall();
void condition();
void conExpress();

void Proc()
{
    //<程序>→<分程序>
    SubProc();
}
void SubProc()
{
    //<分程序>→begin <说明语句表>;<执行语句表> end

    // begin错写
    if (nowType != BEGIN_TYPE)
        error("分程序缺少 begin", false);
    advance();
    DeclareStateTable();
    //;漏写
    if (nowType != SEM_TYPE)
        error("分程序缺少 ;", false);
    else
        advance(); // else存疑
    execStateTable();
    if (nowType != END_TYPE)
        error("分程序缺少 end", false);
}
void DeclareStateTable()
{
    //<说明语句表>→<说明语句>│<说明语句表> ；<说明语句>
    // 消除左递归：
    // 1. <说明语句表>→<说明语句> <说明语句表'>
    // 2. <说明语句表'>→; <说明语句> <说明语句表'> | ε
    DeclareState();
    DeclareStateTablePrime();
}
void DeclareStateTablePrime()
{
    // <说明语句表'>→; <说明语句> <说明语句表'> | ε
    if (nowType == SEM_TYPE)
    {
        advance();
        if (nowType == INTEGER_TYPE)
        {
            DeclareState();
            DeclareStateTablePrime();
        }
        else
        {
            retract();
            return;
        }
    }
    if (nowType == INTEGER_TYPE)
    {
        error("说明语句缺少 ;", false);
        DeclareState();
        DeclareStateTablePrime();
    }
}
void DeclareState()
{
    //<说明语句>→<变量说明>│<函数说明>
    if (nowType != INTEGER_TYPE)
        error("说明语句缺少 integer 变量类型说明", false);
    advance();

    if (nowType == FUNCTION_TYPE)
    {
        advance();
        funcDeclare();
    }
    else if (nowType == SYMBOL_TYPE)
    {
        varDeclare();
    }
    else
    {
        seriousError("说明语句无效，既不是函数说明，也不是变量说明");
    }
}
void varDeclare()
{
    //<变量说明>→integer <变量>
    // integer已经分析过了
    if (varIsDeclared())
    {
        char errinfo[N] = "变量";
        strcat(errinfo, nowToken);
        strcat(errinfo, "重复声明");
        error(errinfo, false);
    }
    else
    {
        addVar();
    }
    var();
}
void var()
{
    //<变量>→<标识符>
    if (!varIsDeclared())
    {
        char errinfo[N] = "变量";
        strcat(errinfo, nowToken);
        strcat(errinfo, "未声明");
        error(errinfo, false);
    }
    advance();
}
void funcDeclare()
{
    //<函数说明>→integer function <标识符>（<参数>）；<函数体>
    // integer,function均已分析
    if (nowType != SYMBOL_TYPE)
        seriousError("函数标识符出错");

    proItem temp;
    strcpy(temp.pname, nowToken);
    temp.ptype = INTEGER_TYPE;
    temp.plev = level;
    temp.fadr = -1;
    temp.ladr = -1;
    prolist.push_back(temp);

    // 更改状态
    level++;
    char lastproc[M] = "";
    strcpy(lastproc, proc);
    strcpy(proc, nowToken);

    advance();
    if (nowType == LEFT_PARE_TYPE)
        advance();
    else
        error("函数声明缺少左括号", false);
    parameter();
    if (nowType == RIGHT_PARE_TYPE)
        advance();
    else
        error("函数声明缺少右括号", false);
    if (nowType == SEM_TYPE)
        advance();
    else
        error("函数声明缺少 ;", false);
    funcBody();
    level--;
    strcpy(proc, lastproc);
    advance();
}
void parameter()
{
    //<参数>→<变量>
    if (nowType != INTEGER_TYPE)
        error("说明语句缺少 integer 变量类型说明", false);
    advance();
    if (nowType != SYMBOL_TYPE)
    {
        error("函数形参标识符出错", false);
        return;
    }
    varItem temp;
    strcpy(temp.vname, nowToken);
    strcpy(temp.vproc, proc);
    temp.vkind = 1;
    temp.vtype = INTEGER_TYPE;
    temp.vlev = level;
    temp.vadr = varlist.size();
    temp.isDeclared = 1; // 形参声明非正式
    varlist.push_back(temp);
    advance();
}

void funcBody()
{
    //<函数体>→begin <说明语句表>；<执行语句表> end
    if (nowType != BEGIN_TYPE)
        error("函数体缺少begin", false);
    else
        advance();
    if (nowType != READ_TYPE && nowType != WRITE_TYPE && nowType != IF_TYPE && nowType != SYMBOL_TYPE)
    {
        DeclareStateTable();
        if (nowType != SEM_TYPE)
            error("函数体缺少 ;", false);
        else
            advance();
    }
    execStateTable();
    if (nowType != END_TYPE)
        error("函数体缺少 end", false);
}
void execStateTable()
{
    //<执行语句表>→<执行语句>│<执行语句表>；<执行语句>
    // 消除左递归：
    // 1. <执行语句表>→<执行语句> <执行语句表'>
    // 2. <执行语句表'>→; <执行语句> <执行语句表'> | ε
    execState();
    execStateTablePrime();
}

void execStateTablePrime()
{
    //<执行语句表'>→; <执行语句> <执行语句表'> | ε
    if (nowType == END_TYPE||!nowType)
    {
        return;
    }
    if (nowType == SEM_TYPE)
    {
        advance();
        execState();
        execStateTablePrime();
        return;
    }
    if (nowType != SEM_TYPE)
    {
        error("执行语句缺少 ;", false);
        execState();
        execStateTablePrime();
    }
}
void execState()
{
    //<执行语句>→<读语句>│<写语句>│<赋值语句>│<条件语句>
    if (nowType == READ_TYPE)
        read();
    else if (nowType == WRITE_TYPE)
        write();
    else if (nowType == IF_TYPE)
        condition();
    else if (nowType == SYMBOL_TYPE)
        assign();
    else if (nowType == INTEGER_TYPE)
    {
        error("说明语句在执行语句下方", false);
        DeclareState();
    }
    else
    {
        seriousError("不是读、写、赋值、条件语句的其中一种，执行语句非法");
    }
}
void read()
{
    //<读语句>→read(<变量>)
    if (nowType != READ_TYPE)
        error("读语句缺少read", false);
    advance();
    if (nowType == LEFT_PARE_TYPE)
        advance();
    else
        error("读语句缺少左括号", false);
    if (nowType != SYMBOL_TYPE)
    {
        seriousError("读语句缺少合法标识符");
    }
    var();
    if (nowType == RIGHT_PARE_TYPE)
        advance();
    else
        error("读语句缺少右括号", false);
}
void write()
{
    //<写语句>→write(<变量>)
    if (nowType != WRITE_TYPE)
        error("写语句缺少 write", false);
    advance();
    if (nowType == LEFT_PARE_TYPE)
        advance();
    else
        error("写语句缺少左括号", false);
    if (nowType != SYMBOL_TYPE)
    {
        seriousError("写语句缺少合法标识符");
    }
    var();
    if (nowType == RIGHT_PARE_TYPE)
        advance();
    else
        error("写语句缺少右括号", false);
}
void assign()
{
    //<赋值语句>→<变量>:=<算术表达式>
    if (!symIsDeclared())
    {
        char errorinfo[N] = "变量";
        strcat(errorinfo, nowToken);
        strcat(errorinfo, "未声明");
        error(errorinfo, false);
    }
    advance();
    if (nowType != ASSIGN_TYPE)
        error("赋值语句缺少 :=", false);
    advance();
    calExpress();
}
void calExpress()
{
    //<算术表达式>→<算术表达式>-<项>│<项>
    // 消除左递归
    // 1. <算术表达式>→<项> <算术表达式'>
    // 2. <算术表达式'>→- <项> <算术表达式'> | ε
    item();
    calExpressPrime();
}
void calExpressPrime()
{
    //<算术表达式'>→- <项> <算术表达式'> | ε
    if (nowType == MINUS_TYPE)
    {
        advance();
        item();
        calExpressPrime();
    }
}
void item()
{
    //<项>→<项>*<因子>│<因子>
    // 消除左递归
    // 1. <项>→<因子> <项'>
    // 2. <项'>→* <因子> <项'> | ε
    factor();
    itemPrime();
}
void itemPrime()
{
    //<项'>→* <因子> <项'> | ε
    if (nowType == MUL_TYPE)
    {
        advance();
        factor();
        itemPrime();
    }
}
void factor()
{
    //<因子>→<变量>│<常数>│<函数调用>
    if (nowType == CONSTANT_TYPE)
        advance();
    else if (nowType == SYMBOL_TYPE)
    {
        advance();
        if (nowType == LEFT_PARE_TYPE)
        {
            retract();
            funcCall();
        }
        else
        {
            retract();
            var();
        }
    }
    else
    {
        error("算术因子出错", false);
    }
}
void funcCall()
{
    //<函数调用>→<函数标识符> (<算术表达式>)
    if (!procIsDeclared())
    {
        char errorinfo[N] = "";
        strcat(errorinfo, nowToken);
        strcat(errorinfo, "函数未声明");
        error(errorinfo, false);
    }
    advance();
    if (nowType == LEFT_PARE_TYPE)
        advance();
    else
        error("函数调用缺少左括号", false);
    calExpress();
    if (nowType == RIGHT_PARE_TYPE)
        advance();
    else
        error("函数调用缺少右括号", false);
}

void condition()
{
    //<条件语句>→if<条件表达式>then<执行语句>else <执行语句>
    if (nowType != IF_TYPE)
        error("条件语句缺少 if", false);
    advance();
    conExpress();
    if (nowType != THEN_TYPE)
        error("条件语句缺少 then", false);
    advance();
    execState();
    if (nowType != ELSE_TYPE)
        error("条件语句缺少 else", false);
    advance();
    execState();
}
void conExpress()
{
    //<条件表达式>→<算术表达式><关系运算符><算术表达式>
    calExpress();
    if (nowType < EQ_TYPE || nowType > GT_TYPE)
        error("非法关系运算符", false);
    advance();
    calExpress();
}
int main(int argc, char *argv[])
{
    if (argc != 2)
        exit(-1);
    openfile(argv[1]);
    dydInput();
    Proc();
    Output();
    closefile();
}
