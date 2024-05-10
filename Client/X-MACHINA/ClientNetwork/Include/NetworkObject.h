#pragma once

/// +-----------------------------------------------
///					 NetworkObject 
/// ________________________________________________
///	±¸Á¶	
///					 NetworkObject
///						  ¡é
///					   session
///					   Listener
/// -----------------------------------------------+

#include "SocketData.h"

class NetworkObject : public std::enable_shared_from_this<NetworkObject>
{
private:
	std::string		 mName        = {};
	class SocketData mSocketData  = {};

protected:
	UINT32		     ID			  = {};

public:
	NetworkObject();
	virtual ~NetworkObject();

public:
	virtual void Dispatch(class OverlappedObject* overlapped, UINT32 bytes = 0) abstract;

public:
	HANDLE		 GetSocketHandle()		{ return reinterpret_cast<HANDLE>(mSocketData.GetSocket()); }
	SocketData&  GetSocketData()		{ return mSocketData; }

	void		SetSocketData(SocketData& sockData) { mSocketData = sockData; }
	void		SetName(std::string name) { mName = name; }

	UINT32 GetID() { return ID; }


};

