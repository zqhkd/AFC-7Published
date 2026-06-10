/******************** (C) COPYRIGHT 2019 ACG Tech Co.*************************
 * 作    者： 曾庆华
 * 文 件 名： ACGRingBuffAPI.h
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
#ifndef __ACG_RINGBUFF_API_H__
#define __ACG_RINGBUFF_API_H__	

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{  
	uint8_t		*buf;	  // 缓冲区	
	uint32_t	size;		 // 缓冲区的大小  
	uint32_t	in;		 // 入口位置 ，写入数据位置
	uint32_t	out;       // 出口位置，读出数据位置
}TRingBuffer;	
  
// 初始化缓冲区	
TRingBuffer *InitRingBuffer(TRingBuffer *pRingBuffer,int buf_size);
// 判断Ring Buffer是否为空的接口
bool RingBufferEmpty(TRingBuffer *pRingBuffer);
// 判断Ring Buffer是否为满的接口：
bool RingBufferFull(TRingBuffer *pRingBuffer);
// 读取Ring Buffer数据长度的接口
int GetRingBufferLen(TRingBuffer *pRingBuffer);
// 清空Ring Buffer的接口
void RingBufferClear(TRingBuffer *pRingBuffer);
// 释放Ring Buffer的接口
void FreeBufferClear(TRingBuffer *pRingBuffer);
// 向Ring Buffer的写数据接口
int RingBufferPut(TRingBuffer *pRingBuffer, uint8_t *buf, int len);
// 读取Ring Buffer数据的接口
int RingBufferGet(TRingBuffer *pRingBuffer, uint8_t *buf, int buf_len);
// 查看环形缓冲区，不移动读取指针
int RingBufferPeek(TRingBuffer *pRingBuffer, uint8_t *buf, int buf_len);
#endif

/************************ (C) COPYRIGHT ACG Co. about ACGRingBuffAPI *****END OF FILE****/
