#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <commctrl.h>
#include <shlobj.h>
#include <direct.h>

#include "png.h"
#pragma comment(lib, "libpng.lib")

class DIB
{
public:
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	LPBITMAPINFO lpbmi;
	LPRGBQUAD lpColorTbl;
	unsigned char *lpbits;
	HBITMAP hDIB;
	int width,height;
	int size;
	int colorn;
	DIB() {lpbmi=NULL;}
	~DIB() {
		Delete();
	}
	void Delete (void) {
		//いらなくなったら、消すこと
		if (lpbmi!=NULL) {
			::DeleteObject (hDIB);
			free(lpbmi);
			lpbmi=NULL;
		}
	}
	BOOL Load (char *filename);
	void Save (char *filename);
	BOOL Create (int x,int y,int colorbit);
	int GetLineIndex (int y) {
		return ((height-1-y)*((((width*bi.biBitCount)+7)/8+3)/4)*4);
	}
};


BOOL DIB::Load(char *filename) {
	HANDLE hf;
	DWORD ptr=0;

	Delete();

	if (access(filename,0)!=0) return (FALSE);
	hf = CreateFile(filename,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    ReadFile(hf, &bf, sizeof(BITMAPFILEHEADER), &ptr, NULL);
    ReadFile(hf, &bi, sizeof(BITMAPINFOHEADER), &ptr, NULL);
	width=bi.biWidth;
	height=bi.biHeight;
	switch (bi.biBitCount)
	{
		case 1:colorn=2;break;
		case 4:colorn=16;break;
		case 8:colorn=256;break;
		case 16:
		case 24:
		case 32:colorn=0;break;
	}
	int numc;
	if (bi.biClrUsed!=0) numc=bi.biClrUsed; else numc=colorn;
	size=((width * bi.biBitCount+31)/32*4)*height;
	if (colorn!=0)
	{
	    lpbmi =(LPBITMAPINFO)malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * colorn);
		lpbmi->bmiHeader = bi;
	    ReadFile(hf, &((lpbmi)->bmiColors), sizeof(RGBQUAD) * numc, &ptr, NULL);
	}
	else
	{
		lpbmi=(LPBITMAPINFO)malloc(sizeof(BITMAPINFOHEADER));
	    lpbmi->bmiHeader = bi;
	}
	lpColorTbl =(LPRGBQUAD) ((LPSTR)(lpbmi) + bi.biSize);
	hDIB = CreateDIBSection(NULL,lpbmi, DIB_RGB_COLORS, (void**)&lpbits, NULL, 0);
	if (bi.biCompression==BI_RGB)ReadFile(hf, lpbits,size , &ptr, NULL);
	CloseHandle (hf);
	return (TRUE);
}

BOOL DIB::Create (int x,int y,int colorbit) {
	Delete();
	bi.biSize=sizeof(BITMAPINFOHEADER);
	bi.biPlanes=1u;bi.biBitCount=(unsigned short)colorbit;
	bi.biCompression=BI_RGB;
	switch (colorbit)
	{
		default:colorn=0;
		case 1:colorn=2;break;
		case 4:colorn=16;break;
		case 8:colorn=256;break;
		case 16:
		case 24:
		case 32:colorn=0;break;
	}
	size=((x*colorbit+31)/32*4)*y;
	width=bi.biWidth=x;
	height=bi.biHeight=y;
	bi.biClrUsed=0;bi.biClrImportant=0;
	bi.biSizeImage=size;
	if (colorn!=0)
	{
		lpbmi=(LPBITMAPINFO)malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * colorn);
		lpbmi->bmiHeader = bi;
	}
	else
	{
		lpbmi=(LPBITMAPINFO)malloc(sizeof(BITMAPINFOHEADER));
        lpbmi->bmiHeader = bi;
	}
	lpColorTbl =(LPRGBQUAD) ((LPSTR)(lpbmi) + bi.biSize);
	hDIB = CreateDIBSection(NULL,lpbmi, DIB_RGB_COLORS, (void**)&lpbits, NULL, 0);
	return (TRUE);
}

void DIB::Save(char *filename) {
//24bitと256に対応
	HANDLE hf;
	DWORD ptr=0;

	unsigned int size=((bi.biWidth * bi.biBitCount+31)/32*4)*bi.biHeight;
	bf.bfType='M'*256+'B';
	bf.bfSize=size+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	bf.bfReserved1=bf.bfReserved2=0;
	bf.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	if (bi.biBitCount==8) {
		bf.bfOffBits+=sizeof(RGBQUAD)*bi.biClrUsed;
		bf.bfSize+=sizeof(RGBQUAD)*bi.biClrUsed;
	}
	bi.biPlanes=1;
	bi.biCompression=BI_RGB;
	if (bi.biBitCount==24) {bi.biClrUsed=0;}
	bi.biClrImportant=0;
	bi.biSizeImage=0;

	hf = CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    WriteFile(hf, &bf, sizeof(BITMAPFILEHEADER), &ptr, NULL);
    WriteFile(hf, &bi, sizeof(BITMAPINFOHEADER), &ptr, NULL);
	if (bi.biBitCount==8) {
		int numc=(bi.biClrUsed)?bi.biClrUsed:256;
		WriteFile(hf,&((lpbmi)->bmiColors), sizeof(RGBQUAD)*numc, &ptr, NULL);
	}
	WriteFile(hf, lpbits,size , &ptr, NULL);
	CloseHandle (hf);
	return;
}


int main (void);
int filejob (char *filename);
void PngReadFunc(png_struct *pPng, png_bytep buf, png_size_t size);
void PngReadFunc(png_struct *pPng, png_bytep buf, png_size_t size){
	unsigned char** p = (unsigned char**)png_get_io_ptr(pPng);
	memcpy(buf, *p, size);
	*p += (int)size;
}	
	
 
int main (void) {
	int i;
	BROWSEINFO bInfo;
	LPITEMIDLIST pIDList;
	TCHAR szDisplayName[MAX_PATH];
	bInfo.hwndOwner = NULL;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDisplayName;
	bInfo.lpszTitle = "変換するフォルダの選択";
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	bInfo.lpfn = NULL;
	bInfo.lParam = (LPARAM)0;
	char tempbuf[256];
	pIDList = ::SHBrowseForFolder(&bInfo);
	if(pIDList == NULL){ 
		return 0;
	}
	if(!::SHGetPathFromIDList(pIDList, szDisplayName)){ 
		printf ("エラーです！終了します。\n");
		return 0;
	} else {
		printf ("フォルダ%s直下の*.pngをNScripterでのαチャンネル透過用BMPに変換します。\n",szDisplayName);
	}

	_chdir (szDisplayName);
	_finddata_t ft;
	long pt=_findfirst("*.png",&ft);
	if (pt!=-1) {
		while (1) {
			if ((ft.attrib & _A_SUBDIR)!=0) {
				goto _next;
			}
			strcpy (tempbuf,ft.name);
			i=strlen(tempbuf);
			while (1) {
				if (tempbuf[i]=='.') break;
				i--;
			}
			i++;
//			if (tempbuf[i]=='p' && tempbuf[i+1]=='n' && tempbuf[i+2]=='g') {
				filejob(tempbuf);
//			}
		
		_next:
			int ret;
			ret=_findnext(pt,&ft);
			if (ret==-1) break;
		}
	}
	_findclose(pt);
	printf ("終了しました。\n");
	system ("pause");
	return 0;
}

int filejob (char *filename) {
	FILE *fp;
	char fnm[256];
	strcpy (fnm,filename);
	fp = fopen(fnm, "rb");
	unsigned char *buf;
	fseek(fp,0,SEEK_END);
	int fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf=(unsigned char *)malloc(fsize);
	fread (buf,fsize,1,fp);
	if (!png_check_sig(buf,fsize)) {
		printf ("正しいpngファイルではありません。\n");
		return 0;
	}
	png_struct *pPng;
	png_info *pInfo;
    if(!(pPng=png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) return 0;
    if(!(pInfo = png_create_info_struct(pPng))) return 0;
	unsigned char* filepos = buf;
    png_set_read_fn(pPng,(png_voidp)&filepos,(png_rw_ptr)PngReadFunc);
    png_read_info(pPng, pInfo);
    png_uint_32 PngWidth;
    png_uint_32 PngHeight;
    int bpp;
    int ColorType;
    png_get_IHDR(pPng, pInfo, &PngWidth, &PngHeight, &bpp, &ColorType, NULL, NULL, NULL);
	//形式指定
	if(png_get_valid(pPng, pInfo, PNG_INFO_tRNS)) png_set_expand(pPng);
    if(ColorType == PNG_COLOR_TYPE_PALETTE) png_set_expand(pPng);
    if(ColorType == PNG_COLOR_TYPE_GRAY && bpp < 8) png_set_expand(pPng);
    if(bpp > 8) png_set_strip_16(pPng);
    if(ColorType == PNG_COLOR_TYPE_GRAY) png_set_gray_to_rgb(pPng);
	//ここまで

	//読み込み
	unsigned char *BmpBuffer;
    unsigned char **Lines;
    BmpBuffer = (unsigned char *)malloc(PngWidth * PngHeight * 4);
    Lines = (unsigned char **)malloc(sizeof(unsigned char *) * PngHeight);
    for(int i = 0; i < PngHeight; i++)Lines[i] = BmpBuffer + PngWidth * 4 * i;
    if(!(ColorType & PNG_COLOR_MASK_ALPHA))png_set_filler(pPng, 0, 1);
    png_set_bgr(pPng);
    png_read_image(pPng, Lines);
    free(Lines);
 
	//ＢＭＰに反映	
	DIB dest;
	int ofs=PngWidth*3;
	unsigned char *db,*sb=BmpBuffer;
	dest.Create(PngWidth*2,PngHeight,24);
	for (int j=0;j<PngHeight;j++) {
		db=dest.lpbits+dest.GetLineIndex(j);
		for (int i=0;i<PngWidth;i++) {
			int alpha=255-*(sb+3);
			*(db+ofs)=alpha;
			*(db+ofs+1)=alpha;
			*(db+ofs+2)=alpha;
			if (alpha!=255) {
				*db=*sb;db++;sb++;
				*db=*sb;db++;sb++;
				*db=*sb;db++;sb++;
			} else {
				*db=0;db++;sb++;
				*db=255;db++;sb++;
				*db=0;db++;sb++;
			}
			sb++;
		}
	}
	int l=strlen(fnm);
	fnm[l-3]='b';
	fnm[l-2]='m';
	fnm[l-1]='p';
	mkdir ("out");
	chdir("out");
	dest.Save(fnm);
	chdir("..");
 	png_destroy_read_struct(&pPng,&pInfo,NULL);
	free(BmpBuffer);
	free(buf);
	return 0;
}
