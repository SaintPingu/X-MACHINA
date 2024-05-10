#pragma once


/// +-----------------------------------------------
///					 PacketSendBuf 
/// ________________________________________________
/// 
/// �� ���ǻ���
///  	
/// - WSASend()�� �� �� �Ϸ� ������ �ö����� �����Ͱ� 
///   �����Ǿ���Ѵ�. ==>  std::shared_ptr �� ��������!
/// -----------------------------------------------+

/*
	������ ��Ŷ�� �������� ���̵� ���� ��Ŷ�� ũ��� �������ִ�.
	ex) Transform ��Ŷ �̶�� �� �� sizeof(float) * 3 * 3 �̰���?

	�׷��� �޸� Ǯ�� ����� ����. 
	�� ��Ŷ�� �ش��ϴ� �޸� Ǯ�� �̸� ������ ����� ���� 
	��Ŷ�� ���� �� �޸� Ǯ���� �޸𸮸� ������ 
	Make_Shared �� ���� ������ ���� ��ٰ�
	�ٽ� �޸� Ǯ�� ���ٰ� ����.

	�� ��Ŷ�� �´� �޸𸮸� �޸� Ǯ���� �ܾ� �ö��� 
	unordered_map �� �̿�����
	O(1) 

	�׷��ٸ� ���� ���� ���۴� ��� �ϳ�?
	�׷��� 32 64 128 256 512 ����Ʈ �޸�Ǯ���� �̸� ��������
	����ٰ� ���� !
*/


class PacketSendBuf
{
public:
	/* �ٽ� �ݳ��ϱ� ���ؼ� �ʿ��� ���� */
	BYTE*  mPtrFromMemPool  = nullptr;
	UINT16 mMemoryPoolSize  = 0;

private:
	BYTE*					mBuffer          = {}; // �޸� ������
	UINT32					mBufferTotalSize = 0;  // SListMemoryPool ���� ������ �޸��� ��ü ũ�� 

public:
	PacketSendBuf(BYTE* ptrMem, UINT16 MemSize, BYTE* buffer, UINT32 allocSize);
	~PacketSendBuf();

	void    SetOwnerInfo(BYTE* ownerPtr, UINT16 ownerMemSize) { mPtrFromMemPool = ownerPtr; mMemoryPoolSize = ownerMemSize; }
	void    SetPacketHederInfo(struct PacketHeader info);

	void     SetBuffer(BYTE* buf);


	BYTE*	GetBuffer()	   { return mBuffer; }
	UINT32	GetTotalSize() { return mBufferTotalSize; }
};

