#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "utils.h"
#include "unicodeUtf8.h"
#include <string.h>
#include <ctype.h>

struct keywordToken{
  char* keyword;    
  uint8_t length;
  TokenType token;
};

//关键字查找表
struct keywordToken keywordsToken[] = {
 {"var",3,TOKEN_VAR},
 {"fun",3,TOKEN_FUN},
 {"if",2,TOKEN_IF},
 {"else",4,TOKEN_ELSE},
 {"true",4,TOKEN_TRUE},
 {"false",5,TOKEN_FALSE},
 {"while",5,TOKEN_WHILE},
 {"for",3,TOKEN_FOR},
 {"break",5,TOKEN_BREAK},
 {"continue",8,TOKEN_CONTINEUE},
 {"return",6,TOKEN_RETURN},
 {"null",4,TOKEN_NULL},
 {"class",5,TOKEN_CLASS},
 {"is",2,TOKEN_IS},
 {"static",6,TOKEN_STATIC},
 {"this",4,TOKEN_THIS},
 {"super",5,TOKEN_SUPER},
 {"import",6,TOKEN_IMPORT},
 {NULL,0,TOKEN_NULL}
};

//判断start是否为关键字返回对应的token
/* 判断关键字 */
static TokenType idOrkeyword(const char* start,uint32_t length){
    uint32_t idx = 0;
    while (keywordsToken[idx].keyword != NULL){
         if(keywordsToken[idx].length == length && memcmp(keywordsToken[idx].keyword,start,length)==0){
             return keywordsToken[idx].token;
         }
         idx++;
    }
    return TOKEN_ID; // 返回 认为是普通变量类型
}




// struct parser{
//     const char* file;
//     const char* sourceCode;
//     const char* nextCharPtr;
//     char curChar;
//     Token curToken;
//     Token preToken;
//     // 处于 内嵌表达式之中,期望的右括号数量,跟踪小括号嵌套
//     int interpolationExpectRightParenNum;
//     VM* vm;
// };

//////////////////////////////////////////////////////////////////////
// 向前看一个字符
char lookAheadChar(Parser* parser){
    return *parser->nextCharPtr;
}

// 获取下一个字符
static void getNextChar(Parser* parser){
    parser->curChar = *parser->nextCharPtr++;
}

/////////////////////////////////////////////////////////////////////

//下一个字符是否期望，是就返回true 否认 返回false

static bool matchNextChar(Parser* parser,char expectedChar){
    if(lookAheadChar(parser) == expectedChar){
        getNextChar(parser);
        return true;
    }
    return false;
}

// 跳过连续的空白字符，吃掉字符
static void skipBlanks(Parser* parser){
    while (isspace(parser->curChar)){ //跳过 空格 \t
        if(parser->curChar == '\n'){
            parser->curToken.lineNo++;
        }
        getNextChar(parser);
    }
}

// 2/6 解析标识符

/////////////////////////////////////////////////////////////////////////////
// 85页
static void parseId(Parser* parser,TokenType type){
  while (isalnum(parser->curChar) || parser->curChar =='_'){
    getNextChar(parser);
  }
// 会指向第一个非法字符从上个循环出来

 uint32_t length = (uint32_t) (parser->nextCharPtr - parser->curToken.start - 1);
  if(type != TOKEN_UNKNOWN){
    //已有就不会继续识别了哈哈哈直接给了
    parser->curToken.type = type;
  }else{
    parser->curToken.type = idOrkeyword(parser->curToken.start,length);
  }
  parser->curToken.length = length;

}

// 解析unicode编码

static void parseUnicodeCodePoint(Parser *parser,ByteBuffer* buf){
    uint32_t idx = 0;
    int value = 0 ;
    uint8_t digit = 0;
    // 获取数值,u后面跟着4位十六进制数字 ...
    while (idx++ < 4){
        getNextChar(parser);
        if(parser->curChar == '\0'){
            LEX_ERROR(parser,"unterminated unicode!");
        }
        // 十六进制解析
        if(parser->curChar >='0' && parser->curChar <='9'){
            digit = parser->curChar - '0';
        }else if(parser->curChar >='a' && parser->curChar<='f'){
            digit = parser->curChar - 'a' + 10;
        }else if(parser->curChar >= 'A' && parser->curChar <='F'){
            digit = parser->curChar - 'A' +10;
        }else{
            LEX_ERROR(parser,"invalid unicode!");
        }
        value = value*16 | digit; // 十六进制转换
    }
    uint32_t byteNum = getByteNumOfEncodeUtf8(value);
    ASSERT(byteNum != 0,"utf8 encode bytes should be between 1 and 4!");
    //utf8编码字节应该再 1到四字节
    
    ByteBufferFillWrite(parser->vm,buf,0,byteNum); // 🍅 重点关注

    encodeUtf8(buf->datas+buf->count - byteNum,value); // 🍅重点关注
}
/////////////////////////////////////////////////////////////////////////////

///===前方高能===///

//---3/6解析字符串--//

//== 获取token 5/6中调用 ==//

/////////////////////////////////////////////////////////////////////////////

// 🍅 433行调用
static void parseString(Parser* parser){
    ByteBuffer str; //要解析的结果 🍅
    ByteBufferInit(&str);
    while (true){
        getNextChar(parser);

        if(parser->curChar == '\0'){
            LEX_ERROR(parser,"unterminated string!");
        }

        if(parser->curChar == '"'){
            parser->curToken.type= TOKEN_STRING;
        }

        if(parser->curChar = '%'){
            if(matchNextChar(parser,'(')){
                LEX_ERROR(parser,"'%' should followed by '('!");
            }
            // 🍅 👇 分析等待括号逻辑
            if(parser->interpolationExpectRightParenNum > 0){
                COMPILE_ERROR(parser,"sorry,I don't support nest interpolate expression!");
            }
            parser->interpolationExpectRightParenNum = 1;
            parser->curToken.type = TOKEN_INTERPOLATION;
            break;
        }

        if(parser->curChar == '\\'){ //转义字符
            getNextChar(parser);
            switch (parser->curChar){
                case '0':
                    ByteBufferAdd(parser->vm,&str,'\0');
                    break;
                case 'a':
                    ByteBufferAdd(parser->vm,&str,'\a');
                    break;
                case 'b':
                    ByteBufferAdd(parser->vm,&str,'\b');
                    break;
                case 'f':
                    ByteBufferAdd(parser->vm,&str,'\f');
                    break;
                case 'n':
                    ByteBufferAdd(parser->vm,&str,'\n');
                    break;
                case 'r':
                    ByteBufferAdd(parser->vm,&str,'\r');
                    break;       
                case 't':
                    ByteBufferAdd(parser->vm,&str,'\t');
                    break;       
                case 'u':
                    parseUnicodeCodePoint(parser,&str);
                    break;       
                case '"':
                    ByteBufferAdd(parser->vm,&str,'\n');
                    break;      
                case '\\':
                    ByteBufferAdd(parser->vm,&str,'\\');
                    break;
                default:
                    LEX_ERROR(parser,"unsupport escape \\%c",parser->curChar);
                    break;                                                                        
            }
        }else{
            //普通字符
            ByteBufferAdd(parser->vm,&str,parser->curChar);
        }
    }

    ByteBufferClear(parser->vm,&str);
    
    
}



/////////////////////////////////////////////////////////////////////////////

///===跳过注解和空行===///

//---4/6--//

// Take Me Higher

/////////////////////////////////////////////////////////////////////////////

static void skipAline(Parser* parser){
    getNextChar(parser); // 获取下一个字符 然后continue
    while (parser->curChar != '\0'){
        if(parser->curChar == '\n'){
            parser->curToken.lineNo++;
            getNextChar(parser);
            break;
        }
        getNextChar(parser);
    }
    
}

static void skipComment(Parser* parser){
    char nextChar = lookAheadChar(parser);
    if(parser->curChar == '/'){
        skipAline(parser);
    } else { // 区块注释
        while (nextChar != '*' && nextChar !='\0'){
            getNextChar(parser);
            if(parser->curChar == '\n'){
                parser->curToken.lineNo++;
            }
            nextChar = lookAheadChar(parser);
        }
        if(matchNextChar(parser,'*')){
            if(!matchNextChar(parser,'/')){
                LEX_ERROR(parser,"expect '\' after '*'!");
            }
            getNextChar(parser);
        }else{
            LEX_ERROR(parser,"expect '*/' before file end!");
        }
        
    }
    skipBlanks(parser); //注释之后有可能会有空白字符
}

/////////////////////////////////////////////////////////////////////////////

// 获取token

///===前方高能===///


/*
    1.调用3/6
    2.被谁调用？
      - 
*/
//---5/6--//

// 重要✨

/////////////////////////////////////////////////////////////////////////////


void getNextToken(Parser* parser){
    parser->preToken = parser->curToken; // 把当前的token记录为上一个token
    skipBlanks(parser); //跳过 待识别单次之前的空格
    parser->curToken.length = 0;
    parser->curToken.start = parser->nextCharPtr - 1;

    while(parser->curChar != '\0'){ // 开始批量读取 🍅

        switch (parser->curChar){
            case ',':
                parser->curToken.type = TOKEN_COMMA;
                break;
            case ':':
                parser->curToken.type = TOKEN_COLON;
                break;
            case '(':
                if(parser->interpolationExpectRightParenNum > 0){
                    parser->interpolationExpectRightParenNum++;
                }
                parser->curToken.type = TOKEN_LEFT_PAREN;
                break;
            case ')':
                if(parser->interpolationExpectRightParenNum > 0 ){
                    parser->interpolationExpectRightParenNum--; // 匹配上一个
                    if(parser->interpolationExpectRightParenNum == 0){
                        parseString(parser); // 🍅扔到 解析
                        break;
                    }
                }
                parser->curToken.type = TOKEN_RIGTH_PAREN;
                break;
            case '[' :
                parser->curToken.type  = TOKEN_LEFT_BACKET;
                break;
            case ']':
                parser->curToken.type = TOKEN_RIGTHT_BACKET;
                break;
            case '{':
                parser->curToken.type = TOKEN_LEFT_BRACE;
                break;
            case '}':
                parser->curToken.type = TOKEN_RIGHT_BRACE;
                break;
            case '.':
                if(matchNextChar(parser,'.')){
                    parser->curToken.type  = TOKEN_DOT_DOT;
                }else{
                    parser->curToken.type = TOKEN_DOT; //🍅 什么是 dot dot
                }
                break;
            case '=':
                if(matchNextChar(parser,'=')){
                    parser->curToken.type = TOKEN_EQUAL;
                }else {
                    parser->curToken.type = TOKEN_ASSIGN;
                }
                break;
            case '+':
                parser->curToken.type = TOKEN_ADD;
                break;
            case '-':
                parser->curToken.type = TOKEN_SUB;
                break;
            case '*':
                parser->curToken.type = TOKEN_MUL;
                break;
            case '/':
                if(matchNextChar(parser,'/') || matchNextChar(parser,'*')){
                    skipComment(parser); // 跳过
                    // 重置下一个token起始地址 
                    parser->curToken.start = parser->nextCharPtr - 1;
                    continue;
                }else{
                    parser->curToken.type = TOKEN_DIV;
                }
                break;
            case '%':
                parser->curToken.type = TOKEN_MOD;
                break;
            case '&':
                if(matchNextChar(parser,'&')){
                    parser->curToken.type = TOKEN_LOGIC_AND;
                }else{
                    parser->curToken.type = TOKEN_BIT_AND;
                }
                break;

            case '|':
                if(matchNextChar(parser,'|')){
                    parser->curToken.type = TOKEN_LOGIC_OR;
                }else{
                    parser->curToken.type = TOKEN_BIT_OR;
                }
                break;
            case '~':
                parser->curToken.type = TOKEN_BIT_NOT;
                break;
            case '?':
                parser->curToken.type = TOKEN_QUESTION;
                break;
            case '>':
                if(matchNextChar(parser,'=')){
                    parser->curToken.type = TOKEN_GREATE_EQUAL;
                }else if (matchNextChar(parser,'>')){
                    parser->curToken.type = TOKEN_BIT_SHIFT_RIGTH;
                }else{
                    parser->curToken.type = TOKEN_GREATE;
                }
                break;
            case '<':
                if(matchNextChar(parser,'=')){
                    parser->curToken.type = TOKEN_LESS_EQUAL;
                }else if(matchNextChar(parser,'<')){
                    parser->curToken.type = TOKEN_BIT_SHIFT_RIGTH;
                }else{
                    parser->curToken.type = TOKEN_LESS;
                }
                break;
            case '!':
                if(matchNextChar(parser,'=')){
                    parser->curToken.type = TOKEN_NOT_EQUAL;
                }else {
                    parser->curToken.type = TOKEN_LOGIC_NOT;
                }
                break;
            case '"':
                parseString(parser);
                break;
            default:
            // 📔：👇
                // 处理变量名及数字
                // 进入此分支字符肯定是数字或变量名首字符
                // 后面会调用相应函数把其余字符一一解析
                // >==不过识别数字需要一些依赖，目前暂时去掉==<
                
                // 若首字母是"_"则是变量名字
            // ===========================================
             if(isalpha(parser->curChar)|| parser->curChar=='_'){
                parseId(parser,TOKEN_UNKNOWN); // 解析其他部分
             }else{

                // 🍅 师爷翻译翻译 什么是 重置下一个token地址！
                if(parser->curChar == '#' && matchNextChar(parser,'!')){
                    skipAline(parser);
                    parser->curToken.start = parser->nextCharPtr - 1;
                    //重置下一个token起始地址
                    continue;
                }
                LEX_ERROR(parser,"unsupport char:\'%c\',quit.",parser->curChar);
             }
             return;
        }
        parser->curToken.length = (uint32_t)(parser->nextCharPtr - parser->curToken.start);
        getNextChar(parser);
        return;
    } // 结束 🍅

    // p83👆
}


/////////////////////////////////////////////////////////////////////////////

// token匹配

///===前方高能===///


/*
 
*/
//---6/6--//

// 重要✨

/////////////////////////////////////////////////////////////////////////////

//若当前token 为expected 则读如下一个token 并返回 true 否则相反条件 

bool matchToken(Parser* parser,TokenType expected){
   if(parser->curToken.type ==expected){
        getNextToken(parser);
        return true;
   } 
   return false;
}

// 断言下一个token为expected 否则 报错errMsg
//🍅 const char * 区别[]

void consumNextToken(Parser* parser,TokenType expected,const char *errMsg){ 
    getNextToken(parser);
    if(parser->curToken.type != expected){
        COMPILE_ERROR(parser,errMsg);
    }
}

void initParser(VM* vm,Parser* parser,const char* file ,const char*sourceCode){
    parser->file = file;
    parser->sourceCode = sourceCode;
    parser->curChar = *parser->sourceCode;
    parser->nextCharPtr = parser->sourceCode+1;
    parser->curToken.lineNo = 1;
    parser->curToken.type = TOKEN_UNKNOWN;
    parser->curToken.start = NULL;
    parser->interpolationExpectRightParenNum = 0;
    parser->vm = vm;
}