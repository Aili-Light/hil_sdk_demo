/*
 The MIT License (MIT)

Copyright (c) 2022 Aili-Light. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RingBuffer.h"

// constructor
RingBuffer::RingBuffer(uint32_t flags)
{
	mFlags = flags;
	mBuffers = NULL;
	mBufferSize = 0;
	mNumBuffers = 0;
	mReadOnce = false;
	mLatestRead = 0;
	mLatestWrite = 0;
}

// destructor
RingBuffer::~RingBuffer()
{
	// Free();

	if (mBuffers != NULL)
	{
		free(mBuffers);
		mBuffers = NULL;
	}
}

// Alloc
bool RingBuffer::Alloc(uint32_t numBuffers, size_t size, uint32_t flags)
{
	if (numBuffers == mNumBuffers && size == mBufferSize && (flags & ZeroCopy) == (mFlags & ZeroCopy))
		return true;

	Free();

	if (mBuffers != NULL && mNumBuffers != numBuffers)
	{
		free(mBuffers);
		mBuffers = NULL;
	}

	if (mBuffers == NULL)
	{
		const size_t bufferListSize = numBuffers * sizeof(void *);
		mBuffers = (void **)malloc(bufferListSize);
		memset(mBuffers, 0, bufferListSize);
	}

	for (uint32_t n = 0; n < numBuffers; n++)
	{
		mBuffers[n] = malloc(size);
	}
	printf("RingBuffer -- allocated %u buffers (%zu bytes each, %zu bytes total)\n", numBuffers, size, size * numBuffers);

	mNumBuffers = numBuffers;
	mBufferSize = size;
	mFlags |= flags;

	return true;
}

// Free
void RingBuffer::Free()
{
	if (!mBuffers || mNumBuffers == 0)
		return;

	for (uint32_t n = 0; n < mNumBuffers; n++)
	{
		free(mBuffers[n]);
		mBuffers[n] = NULL;
	}
}

// Peek
void *RingBuffer::Peek(uint32_t flags)
{
	flags |= mFlags;

	if (!mBuffers || mNumBuffers == 0)
	{
		printf("RingBuffer::Peek() -- error, must call RingBuffer::Alloc() first\n");
		return NULL;
	}

	if (flags & Threaded)
		mMutex.Lock();

	int bufferIndex = -1;

	if (flags & Write)
	{
		bufferIndex = (mLatestWrite + 1) % mNumBuffers;
	}
	else if (flags & ReadLatest)
	{
		bufferIndex = mLatestWrite;
	}
	else if (flags & Read)
	{
		bufferIndex = mLatestRead;
	}

	if (flags & Threaded)
		mMutex.Unlock();

	if (bufferIndex < 0)
	{
		printf("RingBuffer::Peek() -- error, invalid flags (must be Write or Read flags)\n");
		return NULL;
	}

	// printf("RingBuffer : latest read [%d], flag [%d]\n", mLatestRead, flags);
	return mBuffers[bufferIndex];
}

// Next
void *RingBuffer::Next(uint32_t flags)
{
	flags |= mFlags;

	if (!mBuffers || mNumBuffers == 0)
	{
		printf("RingBuffer::Next() -- error, must call RingBuffer::Alloc() first\n");
		return NULL;
	}

	if (flags & Threaded)
		mMutex.Lock();

	int bufferIndex = -1;

	if (flags & Write)
	{
		mLatestWrite = (mLatestWrite + 1) % mNumBuffers;
		bufferIndex = mLatestWrite;
		mReadOnce = false;
	}
	else if ((flags & ReadOnce) && mReadOnce)
	{
		if (flags & Threaded)
			mMutex.Unlock();

		return NULL;
	}
	else if (flags & Read)
	{
		mLatestRead = (mLatestRead + 1) % mNumBuffers;
		bufferIndex = mLatestRead;
		mReadOnce = true;
	}
	else if (flags & ReadLatest)
	{
		mLatestRead = mLatestWrite;
		bufferIndex = mLatestWrite;
		mReadOnce = true;
	}

	if (flags & Threaded)
		mMutex.Unlock();

	if (bufferIndex < 0)
	{
		printf("RingBuffer::Next() -- error, invalid flags (must be Write or Read flags)\n");
		return NULL;
	}

	// printf("RingBuffer : Index [%d], LastRead [%d], LastWrite [%d], flag [%d]\n", bufferIndex, mLatestRead, mLatestWrite, flags);
	return mBuffers[bufferIndex];
}

// GetFlags
uint32_t RingBuffer::GetFlags() const
{
	return mFlags;
}

// SetFlags
void RingBuffer::SetFlags(uint32_t flags)
{
	mFlags = flags;
}

// SetThreaded
void RingBuffer::SetThreaded(bool threaded)
{
	if (threaded)
		mFlags |= Threaded;
	else
		mFlags &= ~Threaded;
}

// Is buffer empty
bool RingBuffer::Empty()
{
	if (mLatestWrite == mLatestRead)
		return true;
	else
		return false;
}

// Is buffer full
bool RingBuffer::Full()
{
	if (((mLatestWrite + 1) % mNumBuffers) == mLatestRead)
		return true;
	else
		return false;
}