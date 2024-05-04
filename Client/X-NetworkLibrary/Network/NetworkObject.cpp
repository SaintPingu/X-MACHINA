#include "pch.h"
#include "NetworkObject.h"

NetworkObject::NetworkObject()
{

}

NetworkObject::~NetworkObject()
{
	mSocketData.Close();

}

