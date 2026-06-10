/******************** (C) COPYRIGHT 2025 ACE Tech Co.*************************
 * 作    者 ： 曾庆华
 * 文 件 名 ： RingBuf.h
 * 版    本 ： ASE2.01.260417
 * 描    述 ： Ubuntu下循环缓冲区读写函数库
 * 官    网 ： www.acecreator.com
 * 淘    宝 ： acecreator.taobao.com
 * 公 众 号 ： 飞行控制与仿真
*****************************************************************************/
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

// #define _GLIBCXX_USE_C99_STDINT_TR1
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#define RING_BUFFER_SIZE 4096  // 缓冲区大小（可根据需求调整）
#define RING_BUFFER_MASK  (RING_BUFFER_SIZE - 1)

typedef struct {
    char data[RING_BUFFER_SIZE];
    _Atomic size_t head;  // 写指针
    _Atomic size_t tail;  // 读指针
} RingBuffer;

// 初始化环形缓冲区
void RingBuffer_Init(RingBuffer *rb);

// 写入数据（返回实际写入字节数）
size_t RingBuffer_Write(RingBuffer *rb, const char *data, size_t len);

// 读取数据（返回实际读取字节数）
size_t RingBuffer_Read(RingBuffer *rb, char *buffer, size_t len);

// 检查缓冲区是否为空
bool RingBuffer_IsEmpty(RingBuffer *rb);

// 检查缓冲区长度是否为指定帧长度
bool RingBuffer_IsFrame(RingBuffer *rb, size_t len);

#endif