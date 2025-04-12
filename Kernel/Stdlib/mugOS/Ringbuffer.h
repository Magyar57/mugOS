#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <stddef.h>

struct s_Ringbuffer {
	size_t size;
	size_t readBeginIndex;
	size_t inReadBuffer;
	int* buffer;
	bool isAllocated; // whether buffer has been kmalloc-ed
};

typedef struct s_Ringbuffer Ringbuffer;

/// @brief Initialize the Ringbuffer by allocating an internal buffer of n ints
void Ringbuffer_initialize(Ringbuffer* this, size_t n);

/// @brief Initialize the Ringbuffer to use the provided buffer
/// @note The buffer HAS to be of size n, otherwise it is undefinied behaviour
void Ringbuffer_initializeWithBuffer(Ringbuffer* this, size_t n, int* buffer);

/// @brief Free the Ringbuffer
/// @note Using the Ringbuffer afterwards is undefinied behaviour
void Ringbuffer_free(Ringbuffer* this);

/// @brief Returns the amount of data in the buffer (how much can still be popped)
size_t Ringbuffer_getDataSize(Ringbuffer* this);

bool Ringbuffer_isBufferFull(Ringbuffer* this);

/// @brief Pops a value out of the ringbuffer
/// @return Whether it could pop a value (ring buffer wasn't empty)
bool Ringbuffer_pop(Ringbuffer* this, int* valueOut);

/// @brief Pushes the provided value to the Ringbuffer, discard if buffer is full
/// @return true if could push, false if not (discarded)
bool Ringbuffer_pushBack(Ringbuffer* this, int value);

/// @brief Pushes the provided value to the Ringbuffer, overwrittes first value if buffer is full
/// @return true if could push, false if not (overwrote the buffer)
bool Ringbuffer_pushBackOverwrite(Ringbuffer* this, int value);

#endif
