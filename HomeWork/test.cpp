#include <iostream>
#include <cstring>
using namespace std;

bool strcmps(char *a,char*b,int len){
   for(int i =0;i < len;i++) if(*(a+i) != *(b+i))return false;
   return true;
}
void ERROR(char *msg){
    cout << msg << endl;
    while (1);
}
int main(){
    char a[100] = "456789";
    ERROR("haha");
    cout << "789";
}