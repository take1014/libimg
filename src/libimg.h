#ifndef LIBIMG_H
#  define LIBIMG_H

/* Version and copyright */
#define LIBIMG_VER_STRING "0.0.1"
#define LIBIMG_HEADER_VER_STRING \
   " libimg version 0.0.1 - June 16, 2014\n"
#define LIBIMG_COPYRIGHT "Yuki-Takehara"


/* These should match the first 3 components of LIBIMG_VER_STRING: */
#define LIBIMG_VER_MAJOR   0
#define LIBIMG_VER_MINOR   0
#define LIBIMG_VER_RELEASE 1


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

/* カラーパレット格納用の構造体 */
typedef struct _ColorPallet{
	unsigned char        *r;			/* R輝度格納用       */
	unsigned char        *g;			/* G輝度格納用       */
	unsigned char        *b;			/* B輝度格納用       */
	unsigned char *Reserved;	      /* 予約領域格納用    */
}ColorPallet;

/* イメージ格納用構造体 */
typedef struct _Image{
	int           Width;				   /* 画像幅            */
	int          Height;				   /* 画像高さ          */
	short      BitDepth;				   /* ビット深度        */
	int            XDPI;				   /* 水平方向解像度    */
	int            YDPI;				   /* 垂直方向解像度    */
	ColorPallet *Pallet;				   /* カラーパレット    */
	unsigned char   **r;				   /* R輝度格納用       */
	unsigned char   **g;				   /* G輝度格納用       */
	unsigned char   **b;				   /* B輝度格納用       */
   unsigned char   Reserved1;       /* 予約領域１        */
   unsigned char   Reserved2;       /* 予約領域２        */
   unsigned char   Reserved3;       /* 予約領域３        */
}Image;

/* 関数のプロトタイプ宣言 */
int ReadImage(char *InputFileName, Image *img);			/* イメージ入力(1,8,24ビットイメージの場合のみ) */
int OutputImage(char *OutputFileName, Image *img);		/* イメージ出力 */
int ColorImage2GrayScale(Image *img);						/* 24bit→8bitグレーにイメージ変換 */
int InverseImage(Image *img);									/* イメージ反転 */
int LinerTransformation(Image *img);                  /* イメージの線形変換 */
int DitherImage(Image *img,int DitherType);           /* 組織的ディザ法 */
int Binarization(Image *img ,int Threshold);          /* 手動で２値化 */
int Difusion(Image *img);                             /* 誤差拡散処理 */
int SobelFiltering(Image *img,int FilterType);        /* Sobelフィルタ処理 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

