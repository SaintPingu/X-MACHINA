#pragma once

/// +-----------------------------------------------
///					   Listener 
/// ________________________________________________

/// -----------------------------------------------+

#include "NetworkObject.h"

class Listener : public NetworkObject
{
private:
	std::vector<Overlapped_Accept*> mAccepts	      = {};
	SPtr_NI							mOwnerNI          = {}; // Owner Network Interface 

private:
	void Register_OverlappedIO_Accept(Overlapped_Accept* overlapped);
	void Process_OverlappedIO_Accept(Overlapped_Accept* overlapped);


public:
	Listener();
	virtual ~Listener();

public:
	bool Start(std::wstring ip, UINT16 portNum, SPtr_NI netInterface);
	void Close();
	void RegisterAccept();

	void DecRef_Accepts();


	virtual void Dispatch(class OverlappedObject* overlapped, UINT32 bytes = 0) override;

};