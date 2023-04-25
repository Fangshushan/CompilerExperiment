#include <iostream>
#include <queue>
#include <ctype.h>
#include <string>
using namespace std;
// 词法分析和语法分析
#define STOP() while(1);
typedef enum{
 statu_if=1,
 statu_else=2,
 statu_number,
 statu_id=10,
 statu_Brackets_left,
 statu_Brackets_right,
 statu_Parenthesis_left,
 statu_Parenthesis_right

}TokenType;

typedef struct{
  TokenType tokentype;
  char *src;
  char *overchar;
  char *curchar;
  char *nextchar;
  bool isaccept = false;
}Token;


typedef enum{
    letter,
    number,
    leftbrket,
    rightbrket,
    leftparen,
    rightpren
}curType;
// System函数
   void ERROR(char *msg){
    cout << msg << endl;
    while (1);
   }

// 文本对比plus
bool strcmps(char *a,char*b,int len){
   for(int i =0;i < len;i++) if(*(a+i) != *(b+i))return false;
   return true;
}

// 是否是系统保留关键字
void TokenType_isKeyword(Token *token){
   int cur= 0;
   switch (token->nextchar - token->curchar){
     case 2:
      if(strcmps(token->curchar,"if",2)) token->tokentype= statu_if;
      break;
     case 4:
      if(strcmps(token->curchar,"else",4)) token->tokentype =  statu_else;
      break;
   }
}

//========================

char inputs[101];
bool Parsering_if(queue<Token> &tokenqueue);
bool Parsering_else(queue<Token> &tokenqueue);
bool Parsering_id(queue<Token> &tokenqueue);
bool Parsering_Brackets(queue<Token> &tokenqueue);

///////////////////////////////////语法分析🍅////////////////////////////////////////////////////////////////////


Token getNextToken(queue<Token> &tokenqueue){
   return tokenqueue.front();
}

int EraseQueue(queue<Token> &tokenqueue){
  if(tokenqueue.empty()) return 1;
  tokenqueue.pop();
  return 0;
}


bool Parsering_S(queue<Token> &tokenqueue){
 Token link = tokenqueue.front();
 cout <<"S Toekn_link: " << link.tokentype << endl;
//  cout <<"start:" << (link.tokentype == statu_id) << endl;
 if(!EraseQueue(tokenqueue)) { // 当前为空我就不进入了
    if(link.tokentype == statu_if){
      // cout <<"coming if" << endl;
      return Parsering_if(tokenqueue);
    }else if(link.tokentype == statu_id){
      // cout << "coming id" << endl;
      return Parsering_id(tokenqueue);
    }
 }
 cout <<"coming other" << endl;
 return false;
}

bool Parsering_if(queue<Token> &tokenqueue){
   Token link = tokenqueue.front();
   if(!EraseQueue(tokenqueue)) {
       if(link.tokentype == statu_Parenthesis_left){
          //  cout << "coming parenthe left\n" << endl;
          bool sResult =  Parsering_S(tokenqueue);
          if(sResult){
            // cout << "coming paren left s\n";
             Token link = tokenqueue.front();
               if(!EraseQueue(tokenqueue)) {
                cout << "linktype: " << link.tokentype << endl;
                  if(link.tokentype == statu_Parenthesis_right){
                  // cout << "coming paren left right\n";
                      
                      return Parsering_Brackets(tokenqueue);
                  }else{
                    // cout << "coming paren left fail s\n";

                    return false;
                  }
               }
          }
       }else{
        return false;
       }
    }
  return false;
}

bool Parsering_Brackets(queue<Token> &tokenqueue){
 Token link = tokenqueue.front();
 if(!EraseQueue(tokenqueue)) { // 当前为空我就不进入了
    if(link.tokentype == statu_Brackets_left){
        bool sResult = Parsering_S(tokenqueue);
        if(sResult){
            link = tokenqueue.front();
            if(!EraseQueue(tokenqueue)) {
                if(link.tokentype == statu_Brackets_right){
                  link = tokenqueue.front();
                  if(!EraseQueue(tokenqueue)){
                    if(link.tokentype == statu_else){
                      return Parsering_else(tokenqueue);
                    }else{
                      return false;
                    }
                  }else{
                    return true;
                  }
                }else{
                    return false;
                }
            }
        }
    }else{
      return false;
    }
 }
 return false;
}

bool Parsering_else(queue<Token> &tokenqueue){
   Token link = tokenqueue.front();
   if(!EraseQueue(tokenqueue)) {
      if(link.tokentype==statu_Brackets_left){
         bool sResult = Parsering_S(tokenqueue);
         if(sResult){
            Token link = tokenqueue.front();
            if(!EraseQueue(tokenqueue)) {
              if(link.tokentype == statu_Brackets_right){
                return true;
              }else{
                return false;
              }
            }
         }else{
          return false;
         }
      }
   }
}

bool Parsering_id(queue<Token> &tokenqueue){
   return true;
}

// 语法解析主程序
void TokentoAcc(queue<Token> tokenqueue){

  if( Parsering_S(tokenqueue)){
    cout << "ACCEPT语法正确!" << endl;
  }else {
    cout << "错误:语法错误不接受!!" << endl;
  }
}

///////////////////////////////////词法分析🍅////////////////////////////////////////////////////////////////////
// Lex分类
curType curAnalyze(char s){
  if(s >='A' && s<='Z' ||s>='a' && s<='z' || s=='_' )  return letter;
  if(s>='0' && s<='9') return number;
  if(s == '<') return leftbrket;
  if(s == '>') return rightbrket;
  if(s == '(') return leftparen;
  if( s == ')') return rightpren;
}

//解析字母
int Lex_Letter(Token *token,char *src,int cur,int len){
  token->overchar = ((cur-1)>=0) ?(src+cur-1):src;
  token->curchar = src+cur;
  int now_cur = cur;
  while((curAnalyze(src[cur])== letter ||curAnalyze(src[cur])== number ) && cur < len){
    cur++;
  }
  token->nextchar = src+cur;
  token->isaccept = true;
  token->tokentype = statu_id;
  return cur - now_cur;

}


// 解析数字
int Lex_number(Token *token,char *src,int cur,int len){
  token->overchar = ((cur-1)>=0) ?(src+cur-1):src;
  token->curchar = src+cur;
  int now_cur = cur;
  while(curAnalyze(src[cur])== number && cur < len){
    cur++;
  }
  token->nextchar = src+cur;
  token->isaccept = true;
  token->tokentype = statu_number;
  return cur - now_cur;
}

//解析括号

int Lex_leftbrket(queue<Token> &Token_Queue,Token* token,int cur,int len){
    int add_num = 0;
    switch (curAnalyze(token->src[cur])){
      case letter:
        add_num = Lex_Letter(token,token->src,cur,len);
        // cout <<"add" <<add_num << "  " << *(token->src+cur+add_num) << endl;
        if(token->isaccept){
            TokenType_isKeyword(token);
            Token_Queue.push(*token);
            cout <<"start:" << *(token->curchar) << " next:" << *(token->nextchar)  <<" len:"<< token->nextchar - token->curchar << " token_type: " << token->tokentype << endl;
        }
        if(*(token->nextchar) != '>'){
          ERROR("错误：缺少 >");
        }else{
          Token* token_temp = new Token;
          token_temp->curchar = token->nextchar;
          token_temp-> isaccept = true;
          token_temp->overchar = token->curchar;
          token_temp->tokentype = statu_Brackets_right;
          token_temp->nextchar = token->nextchar+1;
          token_temp->src = token->src;
          *token = *token_temp;
        }
        return add_num+1;
        break;
      case number:
        add_num+=Lex_number(token,token->src,cur,len);
        if(token->isaccept){
            TokenType_isKeyword(token);
            Token_Queue.push(*token);
            cout <<"start:" << *(token->curchar) << " next:" << *(token->nextchar)  <<" len:"<< token->nextchar - token->curchar << " token_type: " << token->tokentype << endl;
        }

        if(*(token->nextchar) != '>'){
          ERROR("错误：缺少 >");
        }else{
          Token* token_temp = new Token;
          token_temp->curchar = token->nextchar;
          token_temp-> isaccept = true;
          token_temp->overchar = token->curchar;
          token_temp->nextchar = token->nextchar+1;
          token_temp->tokentype = statu_Brackets_right;
          token_temp->src = token->src;
          *token = *token_temp;
        }
        return add_num+1;

        break;
      case rightpren:
        ERROR("错误:不存在空表达式!");
      default:
          ERROR("错误:括号内表达式错误!");
        break;
    }

}


int Lex_leftParenthesis(queue<Token> &Token_Queue,Token* token,int cur,int len){
    int add_num = 0;
    switch (curAnalyze(token->src[cur])){
      case letter:
        add_num = Lex_Letter(token,token->src,cur,len);
        cout << "left_letter_returnaddum:" << add_num<<endl;
        if(token->isaccept){
            TokenType_isKeyword(token);
            Token_Queue.push(*token);
            cout <<"startl1:" << *(token->curchar) << " next:" << *(token->nextchar)  <<" len:"<< token->nextchar - token->curchar << " token_type: " << token->tokentype << endl;
        }

        if(*(token->nextchar) != ')'){
          ERROR("错误：缺少 )1");
        }else{
          Token* token_temp = new Token;
          token_temp->curchar = token->nextchar;
          token_temp-> isaccept = true;
          token_temp->tokentype = statu_Parenthesis_right;
          token_temp->overchar = token->curchar;
          token_temp->nextchar = token->nextchar+1;
          token_temp->src = token->src;
          *token = *token_temp;
        }
        return add_num+1;
        break;
      case number:
        add_num = Lex_number(token,token->src,cur,len);
        if(token->isaccept){
            TokenType_isKeyword(token);
            Token_Queue.push(*token);
            cout <<"startl2:" << *(token->curchar) << " next:" << *(token->nextchar)  <<" len:"<< token->nextchar - token->curchar << " token_type: " << token->tokentype << endl;
        }

        if(*(token->nextchar) != ')'){
          ERROR("错误：缺少 )2");
        }else{
          Token* token_temp = new Token;
          token_temp->curchar = token->nextchar;
          token_temp-> isaccept = true;
          token_temp->tokentype = statu_Parenthesis_right;
          token_temp->overchar = token->curchar;
          token_temp->nextchar = token->nextchar+1;
          token_temp->src = token->src;
          *token = *token_temp;
        }

        return add_num+1;

        break;
      case rightpren:
        ERROR("错误:不存在空表达式!");
      default:
          ERROR("错误:括号内表达式错误!");
        break;
    }

}




void LextoToken(queue<Token> &Token_Queue,char *srcString,int len){
   if(len == 0 ){
    cout <<"错误:输入串为0";
    return;
   }
   Token *token = new Token;
   token->src = srcString;
   int add_number = 0;

   for (int startcur = 0; startcur < len;){

      switch (curAnalyze(srcString[startcur])){
        case letter:
          add_number = Lex_Letter(token,srcString,startcur,len);
        break;
        case number:
         add_number = Lex_number(token,srcString,startcur,len);
        break;
        case leftbrket:
        //  先压入 <
             token->overchar = ((startcur-1)>=0)?(srcString+startcur -1 ):srcString;
             token->curchar = srcString+startcur;
             token->isaccept = true;
             token->tokentype = statu_Brackets_left;
             token->nextchar = ((startcur+1)< len)?(srcString+startcur+1 ):srcString+len;
            Token_Queue.push(*token);
            // new token
              token = new Token;
              token->src = srcString;
              startcur+=1;
            add_number = Lex_leftbrket(Token_Queue,token,startcur,len);
            break;
        case leftparen:
                //  先压入(
             token->overchar = ((startcur-1)>=0)?(srcString+startcur -1 ):srcString;
             token->curchar = srcString+startcur;
             token->isaccept = true;
             token->tokentype = statu_Parenthesis_left;
             token->nextchar = ((startcur+1)< len)?(srcString+startcur+1 ):srcString+len;
            Token_Queue.push(*token);
            // new token
              token = new Token;
              token->src = srcString;
              startcur+=1;

            add_number = Lex_leftParenthesis(Token_Queue,token,startcur,len);

            cout <<"return left:" << add_number << endl;
            break;
        default:
            cout <<"错误:非法字符"<<srcString[startcur] <<"\n";
            return;
      }
      if(token->isaccept){
         // 如果缺少> )
          TokenType_isKeyword(token);
          Token_Queue.push(*token);
          cout <<"start:" << *(token->curchar) << " next:" << *(token->nextchar)  <<" len:"<< token->nextchar - token->curchar << " token_type: " << token->tokentype << endl;
          token = new Token;
      }
      cout <<"cur_now: " << startcur << "  addnum: " <<  add_number << "len " << len << endl;
      startcur += add_number;
      add_number = 0;
   }
   
}
////////////////////////////////////////////////////////词法分析结束🍎/////////////////////////////////////////////////////////////////////////////////////////////////////
int main(){
  //🍉
    cout <<"请输入字符串(!):\n";
    int i = 0;
    char temp;
    while(1){
        cin >> temp;
        if(temp=='!') break;
        if(temp==' ' || temp == '\t' || temp == '\n') continue;
        inputs[i++] = temp;
    }
    queue<Token> token_queue;
    LextoToken(token_queue,inputs,i);  
     TokentoAcc(token_queue);
                 //====================>调试 
              cout <<"========词法队列检查=========\n"<<endl;
              Token temps;
              while(!token_queue.empty()){
                temps = token_queue.front();
                token_queue.pop(); 
                
                cout <<"<queue>  curchar:   "<< *(temps.curchar) << "  nextchar:  " << *(temps.nextchar) <<" tokentype:  " << temps.tokentype << "  len: " << temps.nextchar - temps.curchar << endl; 
              }
            //====================>调试结束🍫  

    
   return 0;

}


/*
 //if(123)<45678>else<abc>!
 if
 (
 123
 123

*/
