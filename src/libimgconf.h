#ifndef LIBIMGCONF_H
#define LIBIMGCONF_H

#define OK                                    0      /* 正常値                                                */
#define ERR_IMG_READ_FILEOPEN             -1000      /* ファイルオープンエラー                                */
#define ERR_IMG_READ_FILETYPE             -1001      /* ファイルタイプエラー                                  */
#define ERR_IMG_READ_BITDEPTH             -1002      /* ビット深度エラー（８ビット、２４ビットのみ）          */
#define ERR_IMG_READ_MEMORY               -1003      /* メモリ確保エラー                                      */
#define ERR_IMG_WRITE_FILEOPEN            -2000     /* ファイルオープンエラー                                */
#define ERR_IMG_WRITE_COLORMEMORY         -2001     /* カラーパレットのメモリ確保エラー                      */
#define ERR_IMG_WRITE_IMAGEDATAMEMORY     -2002     /* イメージデータ用メモリ確保エラー                      */
#define ERR_IMG_INVERSE_BITDEPTH          -3000      /* ビット深度エラー（８ビットのみ）                      */
#define ERR_IMG_LINERTRANSFORM_BITDEPTH   -4000     /* ビット深度エラー（８ビットのみ）                      */
//#define Err_Smoothimg_struct_BitDepth               -5000     /* ビット深度エラー（８ビットのみ）                      */
#define ERR_IMG_DITHER_BITDEPTH           -6000     /* ビット深度エラー（８ビットのみ）                      */
#define ERR_IMG_DITHER_DITHERTYPE         -6001     /* ディザ行列型エラー                                    */
#define ERR_IMG_DITHER_MEMORY             -6002     /* ディザ行列メモリ確保エラー                            */
#define ERR_IMG_DITHER_IMAGESIZE          -6003     /* イメージサイズがブロックサイズで割り切れないとエラー  */
#define ERR_IMG_BINARIZE_BITDEPTH         -7000     /* ビット深度エラー（８ビットのみ）                      */
#define ERR_IMG_BINARIZE_THRESHOLD        -7001     /* 2値化閾値の範囲エラー                                 */
#define ERR_IMG_DIFUSE_BITDEPTH           -8000     /* ビット深度エラー（８ビットのみ）                      */
#define ERR_IMG_DIFUSE_MEMORY             -8001     /* メモリ確保エラー                                      */
#define ERR_IMG_SOBELFILTER_BITDEPTH      -9000     /* ビット深度エラー（８ビットのみ）                      */
#define ERR_IMG_SOBELFILTER_PIXELVALUE    -9001     /* ピクセル変換エラー                                    */

#define BMPHEADERSIZE          14                       /* ファイルヘッダのサイズ                 */
#define INFOHEADERSIZE         40                       /* 情報ヘッダのサイズ                     */
#define HEADERSIZE             54                       /* ヘッダ全体のサイズ                     */
#define PALLETUSED48BIT       256                       /* カラーパレットの使用色数(8bit)         */
#define PALLETUSED41BIT         2                       /* カラーパレットの使用色数(1bit)         */
#define COLORPALLETSIZE41BIT    8                       /* 1ビットの場合のカラーパレットサイズ    */
#define COLORPALLETSIZE48BIT 1024                       /* 8ビットの場合のカラーパレットのサイズ  */
#define MAXBRIGHTNESS         255                       /* 使用するグレー階調数（０?２５５）     */
#define BLOCKSIZE               4                       /* ディザ処理実施字のブロックの横画素数   */

unsigned char BitMask[8] = { 0x08,0x04,0x02,0x01,       /* ２値画像処理のビット演算用       */
                             0x08,0x04,0x02,0x01 };

int BayerMatrix[4][4] =    {            /* Bayer型のマトリックス */
   { 0 ,  8 ,  2 , 10},
   {12 ,  4 , 14 ,  6},
   { 3 , 11 ,  1 ,  9},
   {15 ,  7 , 13 ,  5}
};

int HalftoneMatrix[4][4] = {            /* 網点型のマトリックス */
   {11 ,  4 ,  6 ,  9},
   {12 ,  0 ,  2 , 14},
   { 7 ,  8 , 10 ,  5},
   { 3 , 15 , 13 ,  1}
};

int ScrollMatrix[4][4] =   {            /* 渦巻型のマトリックス */
   { 6 ,  7 ,  8 ,  9},
   { 5 ,  0 ,  1 , 10},
   { 4 ,  3 ,  2 , 11},
   {15 , 14 , 13 , 12}
};

int HorizontalSobel[3][3] = {                   /* Sobel水平方向微分フィルタ */
   {-1, 0, 1},
   {-2, 0, 2},
   {-1, 0, 1}
};

int VerticalSobel[3][3] = {                     /* Sobel垂直方向微分フィルタ */
   {-1,-2,-1},
   { 0, 0, 0},
   { 1, 2, 1}
};

#endif /* LIBIMGCONF_H */


