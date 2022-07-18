#include <windows.h>
#pragma comment(lib, "winmm.lib")

struct DIBDATA {
	int width;
	int height;
	int color;//色数。0はフルカラー、256は256色
	unsigned char *bits;//ビットマップへのポインタ
};
extern "C" {
__declspec(dllexport) void Effect (HDC screendc,HDC destdc,HDC srcdc,unsigned char *dest,unsigned char *src,RECT *r,char *param,int miliwait);
//エフェクトをこの関数で実装する。miliwaitが全体に掛ける時間（ミリ秒単位）
//paramがパラメータ( print 99,ウェイト(ミリ秒),"??.dll/パラメータ文字列"という形式です。
//screendcは画面のDC,destdcは現在の画面を保持するオフスクリーンバッファのDC,
//srcdcはこれから描く新しい画面を保持するメモリDCです。

//destは現在の画面を保持するオフスクリーンバッファのDIBセクションのBMPです。
//srcはこれから描くべき新しい画面のDIBセクションのBMPです。
//この二つは、バイトごとに処理をしたいときに使います。
//srcとdestの情報を元にdestに変化を加えた後、BitBltでdestdcをscrdcに転送してください。

//RECTは更新領域です。=NULLで全画面です。左上と右下の座標です。
//無視してもかまいませんが、最適化のために使ってください。
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
//プラグイン個別の関数
//---------------------------------------------------


BOOL first=FALSE;

void Effect (HDC screendc,HDC destdc,HDC srcdc,unsigned char *dest,unsigned char *src,RECT *r,char *param,int miliwait) {
	//サンプル：
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

