#include "pch.h"
#include "OverlappedObject.h"

/* Overlapped Object */
OverlappedObject::OverlappedObject(OverlappedIO::Type ioType) : mIoType(static_cast<UINT8>(ioType))
{
	::ZeroMemory(this, sizeof(OVERLAPPED));
}

OverlappedObject::~OverlappedObject()
{
}

void OverlappedObject::Clear_OVERLAPPED()
{
	::ZeroMemory(this, sizeof(OVERLAPPED));
}
	