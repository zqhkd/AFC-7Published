/******************** (C) COPYRIGHT 2025 ACE Tech Co.*************************
 * 作    者 ： 曾庆华
 * 文 件 名 ： RingBuf.c
 * 描    述 ： Ubuntu下循环缓冲区读写函数库
 * 版    本 ： ASE2.01.260417
 * 官    网 ： www.acecreator.com
 * 淘    宝 ： acecreator.taobao.com
 * 公 众 号 ： 飞行控制与仿真
*****************************************************************************/
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>
#include "RingBuff.h"

// 初始化环形缓冲区
void RingBuffer_Init(RingBuffer *rb) {
    atomic_init(&rb->head, 0);
    atomic_init(&rb->tail, 0);
    memset(rb->data, 0, RING_BUFFER_SIZE);
}

// 获取可写空间大小（线程安全）
static size_t RingBuffer_GetFreeSpace(RingBuffer *rb) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    
    if (head >= tail) {
        return RING_BUFFER_SIZE - (head - tail) - 1;
    }
    return tail - head - 1;
}

// 写入数据（线程安全）
size_t RingBuffer_Write(RingBuffer *rb, const char *data, size_t len) {
    if (len == 0 || data == NULL) {
        return 0;
    }

    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    size_t free_space = RingBuffer_GetFreeSpace(rb);

    // 空间不足时丢弃整个写入请求
    if (free_space < len) {
        return 0;
    }

    // 分两段写入：从head到缓冲区末尾，以及从缓冲区开始到剩余部分
    size_t first_part = RING_BUFFER_SIZE - head;
    if (first_part > len) {
        first_part = len;
    }
    
    memcpy(&rb->data[head], data, first_part);
    if (len > first_part) {
        memcpy(rb->data, data + first_part, len - first_part);
    }

    // 更新head指针，确保数据完全写入后再更新
    atomic_store_explicit(&rb->head, (head + len) % RING_BUFFER_SIZE, memory_order_release);
    
    return len;
}

// 获取可读数据大小（线程安全）
static size_t RingBuffer_GetAvailable(RingBuffer *rb) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    
    if (head >= tail) {
        return head - tail;
    }
    return RING_BUFFER_SIZE - (tail - head);
}

// 读取数据（线程安全）
size_t RingBuffer_Read(RingBuffer *rb, char *buffer, size_t len) {
    if (len == 0 || buffer == NULL) {
        return 0;
    }

    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    size_t available = RingBuffer_GetAvailable(rb);

    // 可读数据不足时返回0
    if (available < len) {
        return 0;
    }

    // 分两段读取：从tail到缓冲区末尾，以及从缓冲区开始到剩余部分
    size_t first_part = RING_BUFFER_SIZE - tail;
    if (first_part > len) {
        first_part = len;
    }
    
    memcpy(buffer, &rb->data[tail], first_part);
    if (len > first_part) {
        memcpy(buffer + first_part, rb->data, len - first_part);
    }

    // 更新tail指针，确保数据完全读取后再更新
    atomic_store_explicit(&rb->tail, (tail + len) % RING_BUFFER_SIZE, memory_order_release);
    
    return len;
}

// 检查缓冲区是否为空（线程安全）
bool RingBuffer_IsEmpty(RingBuffer *rb) {
    return atomic_load_explicit(&rb->head, memory_order_acquire) == 
           atomic_load_explicit(&rb->tail, memory_order_acquire);
}

// 检查是否有足够数据可读（线程安全）
bool RingBuffer_IsFrameAvailable(RingBuffer *rb, size_t len) {
    return RingBuffer_GetAvailable(rb) >= len;
}

// 查看数据但不移动tail指针（线程安全）
size_t RingBuffer_Peek(RingBuffer *rb, char *buffer, size_t len) {
    if (len == 0 || buffer == NULL) {
        return 0;
    }

    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    size_t available = RingBuffer_GetAvailable(rb);

    if (available < len) {
        len = available;
    }

    size_t first_part = RING_BUFFER_SIZE - tail;
    if (first_part > len) {
        first_part = len;
    }
    
    memcpy(buffer, &rb->data[tail], first_part);
    if (len > first_part) {
        memcpy(buffer + first_part, rb->data, len - first_part);
    }
    
    return len;
}
