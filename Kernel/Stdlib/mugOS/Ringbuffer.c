#include <stddef.h>
#include "assert.h"
#include "Logging.h"
#include "Panic.h"
#include "IRQ.h"

#include "Ringbuffer.h"
#define MODULE "Ringbuffer"

// For now, we don't have concurrency, only interrupts. So this is all we need
#define lock() \
	unsigned long flags; \
	IRQ_disableSave(flags);
#define unlock() \
	IRQ_restore(flags);

#define incrementRingBeginIndex() \
	this->readBeginIndex = (this->readBeginIndex + 1) % this->size

void Ringbuffer_initialize(Ringbuffer* this, size_t){
	this->isAllocated = true;
	// this->buffer = (int*) kmalloc(n*sizeof(int));
	// this->size = n;

	this->readBeginIndex = 0;
	this->inReadBuffer = 0;

	log(PANIC, MODULE, "Ringbuffer_initialize with kmalloc unimplemented");
	panic();
}

void Ringbuffer_initializeWithBuffer(Ringbuffer* this, size_t n, int* buffer){
	assert(buffer);
	assert(n > 0);

	// We assume that buffer is of size n, since we don't have any way to verify it
	this->size = n;
	this->buffer = buffer;

	this->readBeginIndex = 0;
	this->inReadBuffer = 0;
}

void Ringbuffer_free(Ringbuffer* this){
	if (this->isAllocated){
		// free(this->buffer);
		this->buffer = NULL;
	}
}

size_t Ringbuffer_getDataSize(Ringbuffer* this){
	return this->inReadBuffer;
}

bool Ringbuffer_isBufferFull(Ringbuffer* this){
	// We could accomodate with == but this is "safer"
	return (this->inReadBuffer >= this->size);
}

bool Ringbuffer_pop(Ringbuffer* this, int* valueOut){
	assert(this);
	if (valueOut == NULL) return false;

	if (this->inReadBuffer == 0)
		return false;

	lock();

	*valueOut = this->buffer[this->readBeginIndex];
	this->inReadBuffer--;
	incrementRingBeginIndex();

	unlock();
	return true;
}

bool Ringbuffer_pushBack(Ringbuffer* this, int value){
	assert(this);

	if (Ringbuffer_isBufferFull(this))
		return false;

	lock();

	size_t write_index = (this->readBeginIndex + this->inReadBuffer) % this->size;

	this->buffer[write_index] = value;
	this->inReadBuffer++;

	unlock();
	return true;
}

bool Ringbuffer_pushBackOverwrite(Ringbuffer* this, int value){
	assert(this);

	lock();

	if (Ringbuffer_isBufferFull(this)){
		this->buffer[this->readBeginIndex] = value;
		incrementRingBeginIndex();
		unlock();
		return false;
	}

	size_t write_index = (this->readBeginIndex + this->inReadBuffer) % this->size;

	this->buffer[write_index] = value;
	this->inReadBuffer++;

	unlock();
	return true;
}
