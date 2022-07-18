#include <windows.h>
#pragma comment(lib, "winmm.lib")

struct DIBDATA {
	int width;
	int height;
	int color;//�F���B0�̓t���J���[�A256��256�F
	unsigned char *bits;//�r�b�g�}�b�v�ւ̃|�C���^
};
extern "C" {
__declspec(dllexport) void Effect (HDC screendc,HDC destdc,HDC srcdc,unsigned char *dest,unsigned char *src,RECT *r,char *param,int miliwait);
//�G�t�F�N�g�����̊֐��Ŏ�������Bmiliwait���S�̂Ɋ|���鎞�ԁi�~���b�P�ʁj
//param���p�����[�^( print 99,�E�F�C�g(�~���b),"??.dll/�p�����[�^������"�Ƃ����`���ł��B
//screendc�͉�ʂ�DC,destdc�͌��݂̉�ʂ�ێ�����I�t�X�N���[���o�b�t�@��DC,
//srcdc�͂��ꂩ��`���V������ʂ�ێ����郁����DC�ł��B

//dest�͌��݂̉�ʂ�ێ�����I�t�X�N���[���o�b�t�@��DIB�Z�N�V������BMP�ł��B
//src�͂��ꂩ��`���ׂ��V������ʂ�DIB�Z�N�V������BMP�ł��B
//���̓�́A�o�C�g���Ƃɏ������������Ƃ��Ɏg���܂��B
//src��dest�̏�������dest�ɕω�����������ABitBlt��destdc��scrdc�ɓ]�����Ă��������B

//RECT�͍X�V�̈�ł��B=NULL�őS��ʂł��B����ƉE���̍��W�ł��B
//�������Ă����܂��܂��񂪁A�œK���̂��߂Ɏg���Ă��������B
}

typedef void (*RenderSpriteFuncPtr) (int sp,int cell,int x,int y,int a);
typedef void (*GetSpriteDataFuncPtr) (int sp,int *width,int *height,int *cellmax);
typedef void (*NScrGetWindowSizeFuncPtr) (int *width,int *height);
typedef void (*AllocDIBFuncPtr) (DIBDATA *dib,char *filename);
typedef void (*FreeDIBFuncPtr) (DIBDATA *dib);
typedef BOOL (*NScrDoEventsFuncPtr) (void);

RenderSpriteFuncPtr p_RenderSprite;
GetSpriteDataFuncPtr p_GetSpriteData;
NScrGetWindowSizeFuncPtr p_NScrGetWindowSize;
AllocDIBFuncPtr p_AllocDIB;
FreeDIBFuncPtr p_FreeDIB;
NScrDoEventsFuncPtr p_NScrDoEvents;

inline void RenderSprite (int sp,int cell,int x,int y,int a) {
	(*p_RenderSprite)(sp,cell,x,y,a);
}

inline void GetSpriteData (int sp,int *width,int *height,int *cellmax) {
	(*p_GetSpriteData)(sp,width,height,cellmax);
}

inline void NScrGetWindowSize (int *width,int *height) {
	(*p_NScrGetWindowSize)(width,height);
}

inline void AllocDIB (DIBDATA *dib,char *filename) {
	(*p_AllocDIB)(dib,filename);
}

inline void FreeDIB (DIBDATA *dib) {
	(*p_FreeDIB)(dib);
}

inline BOOL NScrDoEvents(void) {
	return (*p_NScrDoEvents)();
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}



//---------------------------------------------------
//�v���O�C���ʂ̊֐�
//---------------------------------------------------


BOOL first=FALSE;

void Effect (HDC screendc,HDC destdc,HDC srcdc,unsigned char *dest,unsigned char *src,RECT *r,char *param,int miliwait) {
	//�T���v���F
	if (!first) {
		char fn[256];
		GetModuleFileName(NULL,fn,sizeof(fn));
		HMODULE hm=LoadLibrary(fn);
		p_RenderSprite=(RenderSpriteFuncPtr)GetProcAddress(hm,"RenderSprite");
		p_GetSpriteData=(GetSpriteDataFuncPtr)GetProcAddress(hm,"GetSpriteData");
		p_NScrGetWindowSize=(NScrGetWindowSizeFuncPtr)GetProcAddress(hm,"NScrGetWindowSize");
		p_AllocDIB=(AllocDIBFuncPtr)GetProcAddress(hm,"AllocDIB");
		p_FreeDIB=(FreeDIBFuncPtr)GetProcAddress(hm,"FreeDIB");
		p_NScrDoEvents=(NScrDoEventsFuncPtr)GetProcAddress(hm,"NScrDoEvents");
		first=TRUE;
		FreeLibrary(hm);
	}
	
	int width,height;
	NScrGetWindowSize(&width,&height);
	unsigned int w=miliwait/16;
	int lx,rx,ly,ry;
	if (r==NULL) {
		lx=0;ly=0;rx=width;ry=height;
	} else {
		lx=r->left;rx=r->right;ly=r->top;ry=r->bottom;
	}
	
	unsigned t,wt;
	for (int i=0;i<16;i++) {
		t=timeGetTime();
		for (int j=lx+i;j<=rx/2;j+=16) {
			::BitBlt (screendc,j,ly,1,ry-ly+1,srcdc,j,ly,SRCCOPY);
			::BitBlt (screendc,rx-j,ly,1,ry-ly+1,srcdc,rx-j,ly,SRCCOPY);
		}
		if (NScrDoEvents()) return;
		wt=timeGetTime()-t;
		if (wt<w) {
			Sleep (w-wt);
		}
	}
}

