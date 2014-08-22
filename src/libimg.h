#ifndef LIBIMG_H
#define LIBIMG_H

/* Version and copywrite */
#define LIBIMG_VER_STRING "0.0.1"
#define LIBIMG_HEADER_VER_STRING \
   " libimg version 0.0.1 - June 16, 2014\n"
#define LIBIMG_COPYWRITE_STRING "Yuki-Takehara"

/* Version information*/
#define LIBIMG_VER_MAJOR   0
#define LIBIMG_VER_MINOR   0
#define LIBIMG_VER_RELEASE 1

/* Include Header */
#ifndef STDIO_H
#define STDIO_H
/* stdio.hがインクルードされていない場合インクルードを行う */
#   include <stdio.h>
#endif

#ifndef STDLIB_H
#define STDLIB_H
/* stdlib.hがインクルードされていない場合インクルードを行う */
#   include <stdlib.h>
#endif

#ifndef STRING_H
#define STRING_H
/* string.hがインクルードされていない場合インクルードを行う */
#   include <string.h>
#endif

#ifndef LIMITS_H
#define LIMITS_H
/* limits.hがインクルードされていない場合インクルードを行う */
#   include <limits.h>
#endif

#ifndef MATH_H
#define MATH_H
/* math.hがインクルードされていない場合インクルードを行う */
#   include <math.h>
#endif


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

/* versionとcopywrite格納用の構造体 */
typedef struct img_libinfo_struct{
   unsigned char *libimgversion;
   unsigned char *libimgcopywrite;
}img_libinfo;


/* カラーパレット格納用の構造体 */
typedef struct img_color_struct{
   unsigned char          *r;                   /* R輝度格納用       */
   unsigned char          *g;                   /* G輝度格納用       */
   unsigned char          *b;                   /* B輝度格納用       */
   unsigned char   *Reserved;                   /* 予約領域格納用    */
}img_color;

/* イメージ格納用構造体 */
typedef struct img_struct_info{
   int                 Width;                   /* 画像幅            */
   int                Height;                   /* 画像高さ          */
   short            BitDepth;                   /* ビット深度        */
   int                  XDPI;                   /* 水平方向解像度    */
   int                  YDPI;                   /* 垂直方向解像度    */
   img_color         *Pallet;                   /* カラーパレット    */
   unsigned char         **r;                   /* R輝度格納用       */
   unsigned char         **g;                   /* G輝度格納用       */
   unsigned char         **b;                   /* B輝度格納用       */
   unsigned char   Reserved1;                   /* 予約領域１        */
   unsigned char   Reserved2;                   /* 予約領域２        */
   unsigned char   Reserved3;                   /* 予約領域３        */
}img_struct;

/* 関数のプロトタイプ宣言 */
int img_read(char *inputfileName, img_struct *img);       /* イメージ入力(1,8,24ビットイメージの場合のみ) */
int img_write(char *outputfileName, img_struct *img);     /* イメージ出力 */
int img_color2gray(img_struct *img);                      /* 24bit→8bitグレーにイメージ変換 */
int img_inverse(img_struct *img);                         /* イメージ反転 */
int img_linertransform(img_struct *img);                  /* イメージの線形変換 */
int img_dither(img_struct *img,int ditherType);           /* 組織的ディザ法 */
int img_binarize(img_struct *img ,int threshold);         /* 手動で２値化 */
int img_difuse(img_struct *img);                          /* 誤差拡散処理 */
int img_sobelfilter(img_struct *img,int filterType);      /* Sobelフィルタ処理 */
int img_smooth(img_struct *img,int filterType);           /* 線形平滑化 */
int img_fft(img_struct *img);                             /* フーリエ変換 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBIMG_H */


