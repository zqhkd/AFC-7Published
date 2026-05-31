/******************** (C) COPYRIGHT 2019 ACG Tech Co.*************************
 * 作    者： 曾庆华
 * 文 件 名： ACGRingBuffAPI.C
 *   
 * 描    述： ACG环形缓冲区接口处理函数
 *  环形缓冲区通常有一个读指针和一个写指针（一个入指针和一个出指针）。读指针指向环形缓冲区中可读的数据，写指针指向环形缓冲区中可写的缓冲区。
 *  通过移动读指针和写指针就可以实现缓冲区的数据读取和写入。在通常情况下，环形缓冲区的读用户仅仅会影响读指针，而写用户仅仅会影响写指针。
 *     如果仅仅有一个读用户和一个写用户，那么不需要添加互斥保护机制就可以保证数据的正确性。
 *     如果有多个读写用户访问环形缓冲区，那么必须添加互斥保护机制来确保多个用户互斥访问环形缓冲区
 *  原文链接：https://www.freesion.com/article/510211071/
 *             . 
 * 版    本：
 * 官    网：www.acecreator.com
 * 淘    宝：acecreator.taobao.com
 * 公 众 号：无人飞行控制
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "ACGRingBuffAPI.h"

// 初始化缓冲区	
TRingBuffer *InitRingBuffer(TRingBuffer *pRingBuffer,int buf_size)
{
//	TRingBuffer *pRingBuffer;   // 指针型函数返回的地址值，需要为全局函数或static函数，如果是函数内的局部变量的话，返回主函数时变量地址空间被释放，返回值可能无效或被其它值覆盖后出现问题。
	                            
    pRingBuffer = (TRingBuffer *)malloc(4*sizeof(uint32_t));    // 注意malloc是在堆上分配空间的，需要关注其大小
	  pRingBuffer->buf = (uint8_t *)malloc(buf_size);
    if (pRingBuffer->buf) memset(pRingBuffer->buf, 0, buf_size);
	
    pRingBuffer->size = buf_size;
    pRingBuffer->in = 0;
    pRingBuffer->out = 0;
    return pRingBuffer;
}

// 判断Ring Buffer是否为空的接口
bool RingBufferEmpty(TRingBuffer *pRingBuffer)
{
    return (pRingBuffer->in == pRingBuffer->out);
}

// 判断Ring Buffer是否为满的接口：
bool RingBufferFull(TRingBuffer *pRingBuffer)
{
    return (((pRingBuffer->in) % pRingBuffer->size) == pRingBuffer->out);
}

// 读取Ring Buffer数据长度的接口
int GetRingBufferLen(TRingBuffer *pRingBuffer)
{
	int len = (pRingBuffer->in - pRingBuffer->out + pRingBuffer->size) % pRingBuffer->size;
    return len;
}

// 清空Ring Buffer的接口
void RingBufferClear(TRingBuffer *pRingBuffer)
{
    pRingBuffer->in = 0;
    pRingBuffer->out = 0;
}

// 释放Ring Buffer的接口
void FreeBufferClear(TRingBuffer *pRingBuffer)
{
    pRingBuffer->in = 0;
    pRingBuffer->out = 0;
    pRingBuffer->size = 0;

    if(pRingBuffer->buf) {
        free(pRingBuffer->buf);
        pRingBuffer->buf = NULL;
    }
}

// 向Ring Buffer的写数据接口
int RingBufferPut(TRingBuffer *pRingBuffer, uint8_t *buf, int len)
{
    int real_int_len = 0, i = 0,  surplus_buf_len = 0;

    if( len >= pRingBuffer->size){
        return -1;
    }

    surplus_buf_len = pRingBuffer->size - GetRingBufferLen(pRingBuffer);
    if(len > surplus_buf_len)  real_int_len = surplus_buf_len;
	else{
			real_int_len = len;
	}
    for( i = 0; i < real_int_len; i++){             // 当in指针追赶到out指针时，不再往前继续写入数据，需等待数据移除才继续写入
        pRingBuffer->buf[ pRingBuffer->in % pRingBuffer->size] = buf[i];
        pRingBuffer->in = (pRingBuffer->in + 1) % pRingBuffer->size;
    }
    return i;
}

/**
 * @brief 从环形缓冲区读取数据（移动读指针）
 * @param pRingBuffer 环形缓冲区句柄
 * @param buf 输出缓冲区（若为NULL则仅丢弃数据）
 * @param buf_len 请求读取的长度
 * @return 实际读取的字节数（可能小于请求长度）
 */
// 读取Ring Buffer数据的接口。从pRingBuffer缓冲区中读取buf_len个字节个数据至buf中
int RingBufferGet(TRingBuffer *pRingBuffer, uint8_t *buf, int buf_len)
{
    if (pRingBuffer == NULL || buf_len < 0) {
        return 0; // 非法参数快速返回
    }

    int data_len = GetRingBufferLen(pRingBuffer);
    int real_out_len = (buf_len > data_len) ? data_len : buf_len;

    for (int i = 0; i < real_out_len; i++) {
        uint8_t *src = &pRingBuffer->buf[pRingBuffer->out % pRingBuffer->size];
        if (buf != NULL) {
            buf[i] = *src; // 仅当输出缓冲区有效时复制
        }
        pRingBuffer->out = (pRingBuffer->out + 1) % pRingBuffer->size; // 始终移动指针
    }

    return real_out_len;
}

/**
 * @brief 查看环形缓冲区数据（不移动读指针）
 * @param pRingBuffer 环形缓冲区句柄
 * @param buf 输出缓冲区（若为NULL则仅计算可读长度）
 * @param buf_len 请求查看的长度
 * @return 实际可读的字节数
 */
// 查看环形缓冲区，不移动读取指针
int RingBufferPeek(TRingBuffer *pRingBuffer, uint8_t *buf, int buf_len)
{
    if (pRingBuffer == NULL || buf_len < 0) {
        return 0; // 非法参数快速返回
    }

    int data_len = GetRingBufferLen(pRingBuffer);
    int real_out_len = (buf_len > data_len) ? data_len : buf_len;

    if (buf != NULL) {
        for (int i = 0; i < real_out_len; i++) {
            buf[i] = pRingBuffer->buf[(pRingBuffer->out + i) % pRingBuffer->size];
        }
    }

    return real_out_len;
}
/************************ (C) COPYRIGHT ACG Co. about ACGRingBuffAPI *****END OF FILE****/
