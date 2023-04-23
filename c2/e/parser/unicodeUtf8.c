#include "unicodeUtf8.h"
#include "common.h"
#include <stdint.h>

// return 编码后的字节数
uint32_t getByteNumOfEncodeUtf8(int value){
    ARRSET(value > 0,"Can't encode negative value!");
    if(value <= 0x7f){
        return 1;
    }
    if(value <= 0x7ff){
        return 2;
    }

    if (value <= 0xffff){
        return 3;
    }

    if(value<= 0x10ffff){
        return 4;
    }
    return 0;
    
}

// 把value 编码为UTF8后写入缓冲区 buf返回 写入的字节数量
uint8_t encodeUtf8(uint8_t* buf,int value){
        ARRSET(value > 0,"Can't encode negative value!");
        if(value <= 0x7f){
            *buf = value & 0x7f;
            return 1;
        }else if(value <= 0x7ff){
            // 11
            //1100 = 0xc0 
            // 1100 1010 1001 1011
            *buf++ = 0xc0 | ((value & 0x7c0) >> 6);
            //写入低字节
            // 1000 0000 0x80
            // 0011 1111

            *buf = 0x80 | (value & 0x3f);
            return 2;
        }else if(value <= 0xffff){
            *buf++ = 0xe0 | ((value & 0xf000) >> 12); 
            //写入低字节s
            // 1000 0000 0x80
            // 0011 1111
            *buf++ = 0x80 | ((value & 0xfc0) >> 6);
            *buf = 0x80 | (value & 0x3f);
            return 3;
        
        }else if(value <=0x10ffff){
            // 1110 101010 FFFFFF 24 -2
            //16
            *buf++ = 0xf0 | ((value & 0x1c000) >> 18);
            *buf++ = 0x80 | ((value & 0x3f000) >> 12);
            *buf++ = 0x80 | ((value & 0xfc0)>>6);
            *buf = 0x80 | (value & 0x3f);
            return 4;
        }

        NOT_REACHED();
        return 0;
}

// 求出有多少个字节码

uint32_t getByteNumOfDecodeUtf8(uint8_t byte){
    if(byte & 0xc0 == 0x80) {
        return 0;
    }
    
    if((byte & 0xe0) == 0xc0){
        return 2;
    }

    if((byte & 0xf0) == 0xe0){
        return 3;
    }
    if((byte & 0xf8) == 0xf0){
       return 4;
    }

    return 1;
}


// 读取编码 解码

int decodeUtf8(const uint8_t* bytePtr, uint32_t length){
    if (*bytePtr <= 0x7f) 
    {
       return *bytePtr;
    }

    int value;
    uint32_t remainingBytes;

    // 读取高字节解析是否是UTF-8编码
    if((*bytePtr & 0xe0) == 0xc0){
        value =  *bytePtr & 0x1f;
        remainingBytes  = 1; 
        // 二字节的uft
    }else if((*bytePtr & 0xf0) == 0xe0){
        value = *bytePtr & 0x0f;
        remainingBytes = 2;
        //三字节
    }else if((*bytePtr & 0xf8) == 0xf0){
        value = *bytePtr & 0x07;
        remainingBytes = 3;

    }else{
        //非法行为
        return  -1;
    }
    
    // 比长度大读不下去了
    if(remainingBytes > length -1){
        return -1;
    }

    // 读取低字节中的数据

    while (remainingBytes > 0){
        bytePtr++;
        remainingBytes --;
        if((*bytePtr & 0xc0) != 0x80){
            //验证是否每个字节开头是否有错误
                return -1;
        }
        value = value << 6 | (*bytePtr & 0x3f);
    }
    return value;

    
}



