#include <windows.h>

struct DIBDATA {
	int width;
	int height;
	int color;//色数。0はフルカラー、256は256色
	unsigned char *bits;//ビットマップへのポインタ
};


typedef void (*RenderSpriteFuncPtr) (int sp,int cell,int x,int y,int a);
//exec_dllからはRenderSpriteは使えないけど、とりあえず取得の仕方だけここで解説します。

typedef void (*GetSpriteDataFuncPtr) (int sp,int *width,int *height,int *cellmax);
typedef void (*NScrGetWindowSizeFuncPtr) (int *width,int *height);
typedef void (*AllocDIBFuncPtr) (DIBDATA *dib,char *filename);
typedef void (*FreeDIBFuncPtr) (DIBDATA *dib);
typedef BOOL (*NScrDoEventsFuncPtr) (void);

typedef HWND (*NScrGetWindowFuncPtr) (void);
typedef void (*DrawClearFuncPtr) (void);
typedef void (*DrawFillFuncPtr) (int r,int g,int b);
typedef void (*DrawBGFuncPtr)  (void);
typedef void (*DrawBG2FuncPtr) (int x,int y,int xscale,int yscale,int rot);
typedef void (*DrawSPFuncPtr)  (int spnum,int cellnum,int alpha,int x,int y);
typedef void (*DrawSP2FuncPtr) (int spnum,int cellnum,int alpha,int x,int y,int xscale,int yscale,int rot);
typedef BOOL (*NSAGetFileFuncPtr) (char *filename,char *nsaname,unsigned int *ptr,unsigned int *size);




RenderSpriteFuncPtr p_RenderSprite;
GetSpriteDataFuncPtr p_GetSpriteData;
NScrGetWindowSizeFuncPtr p_NScrGetWindowSize;
AllocDIBFuncPtr p_AllocDIB;
FreeDIBFuncPtr p_FreeDIB;
NScrDoEventsFuncPtr p_NScrDoEvents;

NScrGetWindowFuncPtr p_NScrGetWindow;
DrawClearFuncPtr p_DrawClear;
DrawFillFuncPtr p_DrawFill;
DrawBGFuncPtr p_DrawBG;
DrawBG2FuncPtr p_DrawBG2;
DrawSPFuncPtr p_DrawSP;
DrawSP2FuncPtr p_DrawSP2;
NSAGetFileFuncPtr p_NSAGetFile;

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



inline HWND NScrGetWindow (void) {
	return (*p_NScrGetWindow)();
}

inline void DrawClear (void) {
	(*p_DrawClear)();
}

inline void DrawFill (int r,int g,int b) {
	(*p_DrawFill)(r,g,b);
}

inline void DrawBG  (void) {
	(*p_DrawBG)();
}

inline void DrawBG2 (int x,int y,int xscale,int yscale,int rot) {
	(*p_DrawBG2)(x,y,xscale,yscale,rot);
}

inline void DrawSP  (int spnum,int cellnum,int alpha,int x,int y) {
	(*p_DrawSP)(spnum,cellnum,alpha,x,y);
}

inline void DrawSP2 (int spnum,int cellnum,int alpha,int x,int y,int xscale,int yscale,int rot) {
	(*p_DrawSP2)(spnum,cellnum,alpha,x,y,xscale,yscale,rot);
}

inline BOOL NSAGetFile (char *filename,char *nsaname,unsigned int *ptr,unsigned int *size) {
	return (*p_NSAGetFile)(filename,nsaname,ptr,size);
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	return TRUE;
}



extern "C" {
__declspec(dllexport) BOOL NScrPlugInMain (HINSTANCE hinstance,HWND hwnd,char *param,int *ret_int,char *ret_str);
//プラグイン本体。hinstanceはNScripterのインスタンス。hwndはNScripterのウィンドウハンドル。
//paramはexec_dll命令に渡したパラメータ文字列。
//ゲームその他の処理を作るときは、このウィンドウの子ウィンドウとして
//ぴったりクライアント領域の大きさの独自のウィンドウを上から貼り付け、処理してください。
//時々NScrDoEventsを実行してNScripterのメッセージ処理をしてください。
};


//---------------------------------------------------
//プラグイン個別の関数
//---------------------------------------------------

LRESULT CALLBACK WndProc (HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

char text[256];
BOOL program_end=FALSE;

BOOL NScrPlugInMain (HINSTANCE hinstance,HWND hwnd,char *param,int *ret_int,char *ret_str) {
	char fn[256];
	GetModuleFileName(NULL,fn,sizeof(fn));
	HMODULE hm=LoadLibrary(fn);
	p_RenderSprite=(RenderSpriteFuncPtr)GetProcAddress(hm,"RenderSprite");
	p_GetSpriteData=(GetSpriteDataFuncPtr)GetProcAddress(hm,"GetSpriteData");
	p_NScrGetWindowSize=(NScrGetWindowSizeFuncPtr)GetProcAddress(hm,"NScrGetWindowSize");
	p_AllocDIB=(AllocDIBFuncPtr)GetProcAddress(hm,"AllocDIB");
	p_FreeDIB=(FreeDIBFuncPtr)GetProcAddress(hm,"FreeDIB");
	p_NScrDoEvents=(NScrDoEventsFuncPtr)GetProcAddress(hm,"NScrDoEvents");

	p_NScrGetWindow=(NScrGetWindowFuncPtr)GetProcAddress(hm,"NScrGetWindow");
	p_DrawClear=(DrawClearFuncPtr)GetProcAddress(hm,"DrawClear");
	p_DrawFill=(DrawFillFuncPtr)GetProcAddress(hm,"DrawFill");
	p_DrawBG=(DrawBGFuncPtr)GetProcAddress(hm,"DrawBG");
	p_DrawBG2=(DrawBG2FuncPtr)GetProcAddress(hm,"DrawBG2");
	p_DrawSP=(DrawSPFuncPtr)GetProcAddress(hm,"DrawSP");
	p_DrawSP2=(DrawSP2FuncPtr)GetProcAddress(hm,"DrawSP2");
	p_NSAGetFile=(NSAGetFileFuncPtr)GetProcAddress(hm,"NSAGetFile");
	FreeLibrary(hm);

	int width,height;
	strcpy (text,param);
	NScrGetWindowSize(&width,&height);

	strcpy (ret_str,"あいうえお");
	*ret_int=999;

	WNDCLASS wc;
	DWORD wstyle=WS_CHILD;
	wc.lpszClassName = "NScr_Plugin_Sample";
	wc.lpfnWndProc = WndProc;
	wc.style=wc.cbClsExtra=wc.cbWndExtra=0;
	wc.lpszMenuName = NULL;
	wc.hbrBackground =(HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance=hinstance;
	wc.hIcon=NULL;
	wc.hCursor=LoadCursor (0,IDC_ARROW);
	RegisterClass (&wc);
	RECT rect={0,0,width,height};
	AdjustWindowRect(&rect,wstyle,FALSE);
	HWND childwnd = CreateWindow (
		"NScr_Plugin_Sample","",
		wstyle,
		rect.left,rect.top,rect.right-rect.left+1,rect.bottom-rect.top+1,
		hwnd,NULL,hinstance,NULL
	);
	ShowWindow(childwnd,SW_SHOW);
	UpdateWindow(childwnd);
	while (1) {
		MSG msg;
		//↓GetMessageを使ってはいけません。NScripter側のメッセージ処理が出来なくなります。
		if (PeekMessage(&msg,childwnd, 0, 0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (NScrDoEvents()) break;
		if (program_end) break;
		//NScrDoEventsは内部でSleep (1);を呼んでいます。ＣＰＵパワーを使い切る心配はありません。
	}
	return FALSE;
}


LRESULT CALLBACK WndProc (HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message) {
		case WM_PAINT:
			hdc=BeginPaint (hwnd,&ps);
			TextOut (hdc,20,20,text,strlen(text));
			EndPaint(hwnd,&ps);
			return 0;
		case WM_LBUTTONUP:
			PostMessage(hwnd,WM_CLOSE,0,0);
			return 0;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			return 1;

		case WM_DESTROY:
			PostQuitMessage (0);
			program_end=TRUE;
			return 0;

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
}

