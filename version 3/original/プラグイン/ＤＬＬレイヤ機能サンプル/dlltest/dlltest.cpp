#include <windows.h>

struct DIBDATA {
	int width;
	int height;
	int color;//色数。0はフルカラー、256は256色
	unsigned char *bits;//ビットマップへのポインタ
};


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



extern "C" {
//--------------------------------------------------------------
//以下、レイヤを管理するための関数。これをこのＤＬＬで実装する。
//--------------------------------------------------------------
__declspec(dllexport) void Initialize (char *str);
//初期化時に呼ばれる関数。strは初期化文字列。省略した場合何もしない。
//*define節でgame文を読み込んだときに呼ばれる。
//スプライト等ゲームスタート後の情報を使って初期化するときは、
//InitializeではなくMessageで実装する。

__declspec(dllexport) void Finalize (void);
//プログラム終了時に呼ばれる関数。省略した場合何もしない。

__declspec(dllexport) void Draw (unsigned char *ptr,RECT *r);
//画面を描画するときに呼ばれる関数。省略した場合何も書かない。
//ptrは、24bitフルカラーのDIB形式のビットマップへのポインタです。(いわゆるlpbits）
//大きさは画面と同じで、このレイヤより下が全部書かれている状態のデータが入っています。
//直接ビットマップを操作する場合はこのポインタで行ってください。
//rectには要求された領域が左上と右下の座標で入ってきます。
//領域外に書いても問題ありませんが、最適化する場合はこの領域の中へ書いてください。
//この関数はあちこちからいろいろなタイミングや範囲指定で呼ばれます。
//この関数で内部状態を切り替えるようなことをしないでください。
//内部状態の切り替えには、TimerやMessageを使ってください。

__declspec(dllexport) BOOL Timer (void);
//タイマ割り込みでこの関数が呼ばれます。さまざまな内部処理を記述してください。
//戻り値は画面を更新する必要があるかを返します。必要な場合はTRUEを返します。
//すると、NScr側からDrawが呼ばれて描画されることになります。

__declspec(dllexport) BOOL Message (char *message,int *returnint,char *returnstr);
//レイヤにメッセージを送ります。（省略した場合メッセージ処理は出来ない）
//NScrからは、layermessage レイヤ番号,送信文字列で呼び出します。
//直後にgetret $数字で文字列の戻り値、getret %数字で数値の戻り値が取得できます。
//戻り値は画面を更新する必要があるかを返します。必要な場合はTRUEを返します。
//この命令の場合はすぐにはDrawは呼ばれません。lspなどのスプライト命令と同様、
//print等のエフェクトによって画面へ反映されます。

};


//---------------------------------------------------
//プラグイン個別の関数
//---------------------------------------------------


#define YUKIMAX 32	//雪最大数
int x[YUKIMAX];//雪Ｘ座標
int y[YUKIMAX];//雪Ｙ座標
int z[YUKIMAX];//雪奥行き(スプライトの各セルに対応、右へ行くほど大）
int spnum;	//雪のスプライト番号、Initializeで指定
int kaze=0;	//Ｘ方向に吹いている風の風向
int w,h,c;	//スプライトの幅、高さ、セル数

BOOL init=FALSE;//まだ初期化していない

int width,height;

void Initialize (char *str) {
	char fn[256];
	GetModuleFileName(NULL,fn,sizeof(fn));
	HMODULE hm=LoadLibrary(fn);
	p_RenderSprite=(RenderSpriteFuncPtr)GetProcAddress(hm,"RenderSprite");
	p_GetSpriteData=(GetSpriteDataFuncPtr)GetProcAddress(hm,"GetSpriteData");
	p_NScrGetWindowSize=(NScrGetWindowSizeFuncPtr)GetProcAddress(hm,"NScrGetWindowSize");
	p_AllocDIB=(AllocDIBFuncPtr)GetProcAddress(hm,"AllocDIB");
	p_FreeDIB=(FreeDIBFuncPtr)GetProcAddress(hm,"FreeDIB");
	p_NScrDoEvents=(NScrDoEventsFuncPtr)GetProcAddress(hm,"NScrDoEvents");
	FreeLibrary(hm);

	NScrGetWindowSize(&width,&height);
}

void Draw (unsigned char *ptr,RECT *r) {
	if (!init) return;
	//雪を降らせる（テスト）
	int alpha;
	for (int i=0;i<YUKIMAX;i++) {
		//更新領域外の雪を描画する必要はない
		if (x[i]<r->left-w || y[i]<r->top-h || x[i]>r->right+w || y[i]>r->bottom+h) continue;

		//yの値で透過度を計算
		if (y[i]<0) alpha=0;
		else if (y[i]<64) alpha=y[i]*4;
		else if (y[i]>height-64) alpha=(height-y[i])*4;
		else alpha=255;

		//スプライトを転送
		RenderSprite(spnum,z[i],x[i],y[i],alpha);
	}
}

BOOL Timer (void) {
	if (!init) return FALSE;
	for (int i=0;i<YUKIMAX;i++) {
		y[i]+=(z[i]*4+rand()%5-1);
		x[i]+=(kaze*z[i]+rand()%3-1);
		if (x[i]<-w || x[i]>width || y[i]>height) {
			x[i]=rand()%640;
			y[i]=0;
			z[i]=rand()%c;
		}
		if (rand()%4000==0) kaze=rand()%5-2;
	}
	return TRUE;
}

BOOL Message (char *message,int *returnint,char *returnstr) {
	//初期化をします
	init=TRUE;
	spnum=atoi(message);
	GetSpriteData(spnum,&w,&h,&c);
	for (int i=0;i<YUKIMAX;i++) {
		x[i]=rand()%width;
		y[i]=-rand()%height;
		z[i]=rand()%c;
	}
	return FALSE;
}