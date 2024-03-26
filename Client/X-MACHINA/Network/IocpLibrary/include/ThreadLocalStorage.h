#pragma once
#include <stack>

extern thread_local uint32				TLS_ThreadID;
extern thread_local uint64				TLS_EndTickCount;

extern thread_local std::stack<int32>	TLS_LockStack;
extern thread_local SendBufferChunkRef	TLS_SendBufferChunk;
extern thread_local class JobQueue*		TLS_CurrentJobQueue;