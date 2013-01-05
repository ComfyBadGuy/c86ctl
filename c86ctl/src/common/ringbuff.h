/***
	RingBuffer module
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com

	note:
	Lock-Free �^�C�v�̃����O�o�b�t�@���[�`���B
	read�P�X���b�h�Awrite�P�X���b�h�܂ł�Lock�����Ɏg���܂��B
	�������Aread, write���Q�X���b�h�ȏ㓯���ɃA�N�Z�X�����ꍇ��
	�Ԃ�����̂ŊO���ی삵�Ă��������B
 */

#ifndef RINGBUFF_H_
#define RINGBUFF_H_


template <class T>
class CRingBuff {
protected:
	T *p;
	UINT sz;
	UINT mask;
	volatile UINT widx;
	volatile UINT ridx;
	
public:
	CRingBuff(){
		p = NULL;
		sz = 0;
		mask = 0;
		widx = 0;
		ridx = 0;
	};

	~CRingBuff(){
		freeres();
	};
	
	BOOL alloc( UINT asize ){
		freeres();
		p = new T[asize];
		if( p ){
			sz = asize;
			mask = sz - 1;
			widx = 0;
			ridx = 0;
		}
		return p ? TRUE : FALSE;
	};

	VOID freeres(VOID){
		if( p )
			delete [] p;

		p = NULL;
		sz = 0;
		mask = 0;
		widx = 0;
		ridx = 0;
	};

	// �c��o�b�t�@�ʂ�Ԃ�
	UINT remain(VOID){
		if( !p ) return 0;
		UINT cridx = ridx & mask;
		UINT cwidx = widx & mask;
		if( cridx <= cwidx ){
			return cridx + (sz - cwidx) - 1;
		}else{
			return cridx - cwidx - 1;
		}
	};

	// �i�[�ς݃f�[�^�ʂ�Ԃ�
	UINT length(VOID){
		if( !p ) return 0;
		UINT cridx = ridx & mask;
		UINT cwidx = widx & mask;
		if( cridx <= cwidx ){
			return cwidx - cridx;
		}else{
			return cwidx + (sz - cridx);
		}
	};

	// �i�[����Ă���f�[�^���������ǂ���
	BOOL isempty(void){
		if( !p ) return 0;
		return ( length() == 0 );
	};

	void flush(void){
		while( 0<length() )
			::InterlockedIncrement(&ridx);
	};

	T* front(void){
		if( !p )
			return 0;
		
		return p + (ridx&mask);
	};
	
	BOOL pop( T *data ){
		if( !p )
			return FALSE;

		*data = *(p + (ridx&mask));
		::InterlockedIncrement(&ridx);
		return TRUE;
	};

	BOOL pop( T *data, UINT sz ){
		if( !p ) return FALSE;
		if( length() < sz ) return FALSE;

		for( UINT i=0; i<sz; i++ ){
			T *pd = p + (ridx&mask);
			data[i] = *pd;
			::InterlockedIncrement(&ridx);
		}
		return TRUE;
	};

	VOID push( T data ){
		if( !p ) return;
		while( remain() < 1 ){
			Sleep(1);
		}

		T *pd = p + (widx&mask);
		*pd = data;
		::InterlockedIncrement(&widx);
	};

	BOOL push( const T *data, UINT sz ){

		if( !p ) return FALSE;
		while( remain() < sz ){
			Sleep(1);
		}

		for( UINT i=0; i<sz; i++ ){
			T *pd = p + (widx&mask);
			*pd = *data++;
			::InterlockedIncrement(&widx);
		}
		return TRUE;
	};
	

};

#endif
