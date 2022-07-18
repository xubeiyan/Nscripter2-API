#include <windows.h>

struct DIBDATA {
	int width;
	int height;
	int color;//�F���B0�̓t���J���[�A256��256�F
	unsigned char *bits;//�r�b�g�}�b�v�ւ̃|�C���^
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
//�ȉ��A���C�����Ǘ����邽�߂̊֐��B��������̂c�k�k�Ŏ�������B
//--------------------------------------------------------------
__declspec(dllexport) void Initialize (char *str);
//���������ɌĂ΂��֐��Bstr�͏�����������B�ȗ������ꍇ�������Ȃ��B
//*define�߂�game����ǂݍ��񂾂Ƃ��ɌĂ΂��B
//�X�v���C�g���Q�[���X�^�[�g��̏����g���ď���������Ƃ��́A
//Initialize�ł͂Ȃ�Message�Ŏ�������B

__declspec(dllexport) void Finalize (void);
//�v���O�����I�����ɌĂ΂��֐��B�ȗ������ꍇ�������Ȃ��B

__declspec(dllexport) void Draw (unsigned char *ptr,RECT *r);
//��ʂ�`�悷��Ƃ��ɌĂ΂��֐��B�ȗ������ꍇ���������Ȃ��B
//ptr�́A24bit�t���J���[��DIB�`���̃r�b�g�}�b�v�ւ̃|�C���^�ł��B(������lpbits�j
//�傫���͉�ʂƓ����ŁA���̃��C����艺���S��������Ă����Ԃ̃f�[�^�������Ă��܂��B
//���ڃr�b�g�}�b�v�𑀍삷��ꍇ�͂��̃|�C���^�ōs���Ă��������B
//rect�ɂ͗v�����ꂽ�̈悪����ƉE���̍��W�œ����Ă��܂��B
//�̈�O�ɏ����Ă���肠��܂��񂪁A�œK������ꍇ�͂��̗̈�̒��֏����Ă��������B
//���̊֐��͂����������炢�낢��ȃ^�C�~���O��͈͎w��ŌĂ΂�܂��B
//���̊֐��œ�����Ԃ�؂�ւ���悤�Ȃ��Ƃ����Ȃ��ł��������B
//������Ԃ̐؂�ւ��ɂ́ATimer��Message���g���Ă��������B

__declspec(dllexport) BOOL Timer (void);
//�^�C�}���荞�݂ł��̊֐����Ă΂�܂��B���܂��܂ȓ����������L�q���Ă��������B
//�߂�l�͉�ʂ��X�V����K�v�����邩��Ԃ��܂��B�K�v�ȏꍇ��TRUE��Ԃ��܂��B
//����ƁANScr������Draw���Ă΂�ĕ`�悳��邱�ƂɂȂ�܂��B

__declspec(dllexport) BOOL Message (char *message,int *returnint,char *returnstr);
//���C���Ƀ��b�Z�[�W�𑗂�܂��B�i�ȗ������ꍇ���b�Z�[�W�����͏o���Ȃ��j
//NScr����́Alayermessage ���C���ԍ�,���M������ŌĂяo���܂��B
//�����getret $�����ŕ�����̖߂�l�Agetret %�����Ő��l�̖߂�l���擾�ł��܂��B
//�߂�l�͉�ʂ��X�V����K�v�����邩��Ԃ��܂��B�K�v�ȏꍇ��TRUE��Ԃ��܂��B
//���̖��߂̏ꍇ�͂����ɂ�Draw�͌Ă΂�܂���Blsp�Ȃǂ̃X�v���C�g���߂Ɠ��l�A
//print���̃G�t�F�N�g�ɂ���ĉ�ʂ֔��f����܂��B

};


//---------------------------------------------------
//�v���O�C���ʂ̊֐�
//---------------------------------------------------


#define YUKIMAX 32	//��ő吔
int x[YUKIMAX];//��w���W
int y[YUKIMAX];//��x���W
int z[YUKIMAX];//�ቜ�s��(�X�v���C�g�̊e�Z���ɑΉ��A�E�֍s���قǑ�j
int spnum;	//��̃X�v���C�g�ԍ��AInitialize�Ŏw��
int kaze=0;	//�w�����ɐ����Ă��镗�̕���
int w,h,c;	//�X�v���C�g�̕��A�����A�Z����

BOOL init=FALSE;//�܂����������Ă��Ȃ�

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
	//����~�点��i�e�X�g�j
	int alpha;
	for (int i=0;i<YUKIMAX;i++) {
		//�X�V�̈�O�̐��`�悷��K�v�͂Ȃ�
		if (x[i]<r->left-w || y[i]<r->top-h || x[i]>r->right+w || y[i]>r->bottom+h) continue;

		//y�̒l�œ��ߓx���v�Z
		if (y[i]<0) alpha=0;
		else if (y[i]<64) alpha=y[i]*4;
		else if (y[i]>height-64) alpha=(height-y[i])*4;
		else alpha=255;

		//�X�v���C�g��]��
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
	//�����������܂�
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