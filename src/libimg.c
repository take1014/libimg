#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "libimg.h"
#include "libimgconf.h"

/* ********************************************
 * 関数名　：InitializeImage
 * 引数　　：イメージ構造体
 * 戻り値　：なし
 * 処理概要：Image構造体の初期化を行う
 * ******************************************** */
void
InitializeImage(Image *img){
	img->Width = 0;
	img->Height = 0;
	img->BitDepth = 0;
	img->XDPI = 0;
	img->YDPI = 0;
	img->Pallet = NULL;
	img->r = NULL;
	img->g = NULL;
	img->b = NULL;
}

/* ********************************************
 * 関数名　：MakeImageStruct
 * 引数1 　：イメージ高さ
 * 引数2 　：イメージ横幅
 * 戻り値　：イメージ画素格納用の構造体
 * 処理概要：イメージの画素を格納する構造体の作成
 * 　　　　　を行う。
 * ******************************************** */
unsigned char**
MakeImageStruct(int Height, int Width){
	unsigned char **Brightness = (unsigned char**)malloc(sizeof(unsigned char*)*Height);
	if (Brightness == NULL){
		return NULL;
	}
	for (int y = 0; y < Height; y++){
		Brightness[y] = (unsigned char*)malloc(sizeof(unsigned char)* Width);
		if (Brightness[y] == NULL){
			return NULL;
		}
	}
   
   for(int y = 0; y < Height; y++){
      for(int x = 0; x < Width; x++){
         /* 確保したメモリの初期化 */
         Brightness[y][x] = 0x00;
      }
   }
	return Brightness;
}

/* ********************************************
 * 関数名　：DelateImageStruct
 * 引数　　：イメージ構造体
 * 戻り値　：なし
 * 処理概要：イメージ構造体の解放を行う
 * ******************************************** */
void
DelateImageStruct(Image *img){
	free(img->Pallet->b);
	free(img->Pallet->g);
	free(img->Pallet->r);
	free(img->Pallet->Reserved);
	free(img->Pallet);
	free(img->b);
	free(img->g);
	free(img->r);
}

/* ********************************************
 * 関数名　：ReadImage
 * 引数1　 ：イメージファイルパス
 * 引数2　 ：イメージ構造体
 * 戻り値　：エラーコード
 * 処理概要：イメージファイルの読込みを行う
 * ******************************************** */
int
ReadImage(char *InputFileName, Image *img){
	FILE *fp;                                           /* ファイルオープン用のポインタ     */
   int x = 0;                                          /* x方向の画像走査用                */
   int y = 0;                                          /* y方向の画像走査用                */
	int RealWidth = 0;                                  /* 実際の画像幅                     */
	unsigned char HeaderBuffer[HEADERSIZE];             /* ヘッダ格納用のバッファ           */
	unsigned char *Data;                                /* イメージデータ１行分格納用の配列 */
	unsigned char *Pallet;                              /* カラーパレット格納用             */
	int ColorpalletIndex = 0;                           /* カラーパレットインデックス       */
   int DataCount             = 0;                      /* 書き込みデータ配列のカウント用        */
   unsigned char Byte        = 0x00;                   /* ２値画像処理時の1バイト分読込み用     */
   unsigned char FirstNibble = 0x00;                   /* ２値画像処理時の前半ニブル処理用      */
   unsigned char LastNibble  = 0x00;                   /* ２値画像処理時の後半ニブル処理用      */
   
	/* イメージ構造体の初期化 */
	InitializeImage(img);
   
	/* 入力イメージのファイルオープン */
	fp = fopen(InputFileName, "rb");
	/* ファイルオープンに失敗した場合 */
	if (fp == NULL){
		return Err_ReadImage_FileOpen;
	}
   
	/* BMPヘッダの読込み */
	fread(HeaderBuffer, sizeof(unsigned char), HEADERSIZE, fp);
	/* ファイルタイプのチェック */
	if (!(HeaderBuffer[0] == 'B' && HeaderBuffer[1] == 'M')){
		return Err_ReadImage_FileType;
	}
   
	/* ========== 情報ヘッダの読取り ==========*/
	/* 画像幅 */
	memcpy(&img->Width, HeaderBuffer + 18, sizeof(int));
	/* 画像高さ */
	memcpy(&img->Height, HeaderBuffer + 22, sizeof(int));
	/* ビット深度 */
	memcpy(&img->BitDepth, HeaderBuffer + 28, sizeof(short));
	/* 水平方向解像度 */
	memcpy(&img->XDPI, HeaderBuffer + 38, sizeof(int));
	/* 垂直方向解像度 */
	memcpy(&img->YDPI, HeaderBuffer + 42, sizeof(int));
	/* ========================================*/
   
	/* ビット深度のチェック */
	if (!(img->BitDepth == 24 || img->BitDepth == 8 || img->BitDepth == 1)){
		return Err_ReadImage_BitDepth;
	}
	/* 実際の画像幅の算出 */
	if (img->BitDepth == 24){
		RealWidth = img->Width * 3 + img->Width % 4;
	}
	else if (img->BitDepth == 8){
		RealWidth = img->Width + img->Width % 4;
	}
   else{
      RealWidth = img->Width / 8 + (img->Width / 8) % 4;
   }
   
   /* データ部読込み用のメモリ確保 */
	Data = (unsigned char*)malloc(sizeof(unsigned char)* RealWidth);
   if(Data == NULL){
      return Err_ReadImage_Memory;
   }
   
   for(int i = 0; i < RealWidth; i++){
      Data[i] = 0x00;
   }
   
   img->r = MakeImageStruct(img->Height, img->Width);
   img->g = MakeImageStruct(img->Height, img->Width);
   img->b = MakeImageStruct(img->Height, img->Width);
   if (img->r == NULL || img->g == NULL || img->b == NULL){
      return Err_ReadImage_Memory;
   }
   
	if (img->BitDepth == 24){
		/* ========== 24ビットイメージの場合 ========== */
		/* 輝度の取得 */
		for (y = 0; y < img->Height; y++){
			/* イメージデータ一行分の取得 */
			fread(Data, sizeof(unsigned char), RealWidth, fp);
			for (x = 0; x < img->Width; x++){
				img->b[img->Height - y - 1][x] = Data[x * 3];
				img->g[img->Height - y - 1][x] = Data[x * 3 + 1];
				img->r[img->Height - y - 1][x] = Data[x * 3 + 2];
			}
		}
		/* =============================================*/
	}
	else if (img->BitDepth == 8){
		/* =============  8ビットイメージの場合 ============= */
      Pallet = (unsigned char*)malloc(sizeof(unsigned char) * COLORPALLETSIZE48BIT);
		/* カラーパレット取得用のメモリ確保 */
		img->Pallet = (ColorPallet*)malloc(sizeof(ColorPallet));
		img->Pallet->r = (unsigned char*)malloc(sizeof(unsigned char)* 256);
		img->Pallet->g = (unsigned char*)malloc(sizeof(unsigned char)* 256);
		img->Pallet->b = (unsigned char*)malloc(sizeof(unsigned char)* 256);
		img->Pallet->Reserved = (unsigned char*)malloc(sizeof(unsigned char)* 256);
		/* メモリ確保に失敗した場合 */
		if (img->Pallet->b == NULL || img->Pallet->g == NULL || img->Pallet->r == NULL || img->Pallet->Reserved == NULL){
			return Err_ReadImage_Memory;
		}
		/* カラーパレット読込み */
		fread(Pallet, sizeof(unsigned char), COLORPALLETSIZE48BIT, fp);
		for (int i = 0; i < 256; i++){
			img->Pallet->b[i] = Pallet[i * 4];
			img->Pallet->g[i] = Pallet[i * 4 + 1];
			img->Pallet->r[i] = Pallet[i * 4 + 2];
			img->Pallet->Reserved[i] = Pallet[i * 4 + 3];
		}
      
		/* 輝度の取得 */
		for (y = 0; y < img->Height; y++){
			/* イメージデータ部一行分の取得 */
			fread(Data, sizeof(unsigned char), RealWidth, fp);
			for (x = 0; x < img->Width; x++){
				/* カラーパレットのインデックスを取得 */
				ColorpalletIndex = (int)Data[x];
            /* インデックスに紐付いた輝度を取得 */
				img->b[img->Height - y - 1][x] = img->Pallet->b[ColorpalletIndex];
				img->g[img->Height - y - 1][x] = img->Pallet->g[ColorpalletIndex];
				img->r[img->Height - y - 1][x] = img->Pallet->r[ColorpalletIndex];
			}
		}
		/* ================================================== */
	}
   else if (img->BitDepth == 1){
		/* =============  1ビットイメージの場合 ============= */
      Pallet = (unsigned char*)malloc(sizeof(unsigned char) * COLORPALLETSIZE41BIT);
		/* カラーパレット取得用のメモリ確保 */
		img->Pallet = (ColorPallet*)malloc(sizeof(ColorPallet));
		img->Pallet->r = (unsigned char*)malloc(sizeof(unsigned char)* 2);
		img->Pallet->g = (unsigned char*)malloc(sizeof(unsigned char)* 2);
		img->Pallet->b = (unsigned char*)malloc(sizeof(unsigned char)* 2);
		img->Pallet->Reserved = (unsigned char*)malloc(sizeof(unsigned char)* 2);
		/* メモリ確保に失敗した場合 */
		if (img->Pallet->b == NULL || img->Pallet->g == NULL || img->Pallet->r == NULL || img->Pallet->Reserved == NULL){
			return Err_ReadImage_Memory;
		}
		/* カラーパレット読込み */
		fread(Pallet, sizeof(unsigned char), COLORPALLETSIZE41BIT, fp);
		for (int i = 0; i < 2; i++){
			img->Pallet->b[i] = Pallet[i * 4];
			img->Pallet->g[i] = Pallet[i * 4 + 1];
			img->Pallet->r[i] = Pallet[i * 4 + 2];
			img->Pallet->Reserved[i] = Pallet[i * 4 + 3];
		}
      
		/* 輝度の取得 */
		for (y = 0; y < img->Height; y++){
			/* イメージデータ部一行分の取得 */
			fread(Data, sizeof(unsigned char), sizeof(unsigned char)*RealWidth, fp);
         DataCount = 0;
         x = 0;
         do{
            /* ２進数計算用 */
            Byte        = 0x00;                   /* ２値画像処理時の1バイト分読込み用     */
            FirstNibble = 0x00;                   /* ２値画像処理時の前半ニブル処理用      */
            LastNibble  = 0x00;                   /* ２値画像処理時の後半ニブル処理用      */
            
            // １バイト分取得
            Byte = Data[DataCount];
            
            // 最初の４ビット取得
            FirstNibble = Byte >> 4;
            // 後半４ビット取得
            LastNibble = Byte << 4;
            LastNibble = LastNibble >> 4;
            
            for(int k = 0; k < 8; k++){
               /* 論理積を計算して、どのビットが立っているかを判別する */
               /* 前半４ビット */
               if(k < 4)
               {
                  if( 0 == (FirstNibble & BitMask[k]))
                  {
                     img->b[img->Height - y - 1][x] = 0x00;
                     img->g[img->Height - y - 1][x] = 0x00;
                     img->r[img->Height - y - 1][x] = 0x00;
                  }
                  else
                  {
                     img->b[img->Height - y - 1][x] = 0xFF;
                     img->g[img->Height - y - 1][x] = 0xFF;
                     img->r[img->Height - y - 1][x] = 0xFF;
                  }
                  
               }
               /* 後半４ビット */
               else if(k > 3)
               {
                  if( 0 == (LastNibble & BitMask[k]))
                  {
                     img->b[img->Height - y - 1][x] = 0x00;
                     img->g[img->Height - y - 1][x] = 0x00;
                     img->r[img->Height - y - 1][x] = 0x00;
                  }
                  else
                  {
                     img->b[img->Height - y - 1][x] = 0xFF;
                     img->g[img->Height - y - 1][x] = 0xFF;
                     img->r[img->Height - y - 1][x] = 0xFF;
                  }
               }
               
               x++;
               
               /* xが画像幅と同じ値の場合、ループを抜ける */
               if(x == img->Width){
                  break;
               }
            }
            
            DataCount++;
            
         }while(x < img->Width);
         
      }
   }
   /* ================================================== */
	
   
	/* メモリの解放 */
	free(Data);
	fclose(fp);
	return OK;
}

/* ********************************************
 * 関数名　：WriteImage
 * 引数1　 ：イメージファイルパス
 * 引数2　 ：イメージ構造体
 * 戻り値　：エラーコード
 * 処理概要：イメージのファイル出力を行う
 * ******************************************** */
int
WriteImage(char *OutputFileName, Image *img){
	FILE *fp;                                       /* ファイルオープン用のポインタ */
   int x = 0;                                      /* x方向の画像走査用                */
   int y = 0;                                      /* y方向の画像走査用                */
	int RealWidth = 0;                              /* 実際の画像幅 */
	int FileSize = 0;                               /* ファイルサイズ */
	int ImageSize = 0;                              /* イメージサイズ */
	int Offset2ImageData = 0;                       /* イメージデータまでのオフセット */
	int InfoHeader = INFOHEADERSIZE;                /* 情報ヘッダのサイズ */
	short Plane = 1;                                /* プレーン数 */
	int Compression = 0;                            /* 圧縮形式 */
	int PalletUsed = 0;                             /* 8ビットイメージのパレット数 */
	unsigned char HeaderBuffer[HEADERSIZE];         /* ヘッダ処理用のバッファ */
	unsigned char *Pallet;                          /* パレット格納用の配列 */
	unsigned char *Data;                            /* イメージデータ１行分格納用の配列 */
   unsigned char BinCount = 0x00;                  /* ２値イメージの場合の演算用    */
   int BitShiftCount = 0;                          /* ビットシフトカウント用（左に何ビットシフトしたか） */
   int DataCount     = 0;                          /* 書き込みデータ配列のカウント用 */
   
	/* 出力ファイルのオープン */
	fp = fopen(OutputFileName, "wb");
	if (fp == NULL){
		return Err_OutputImage_FileOpen;
	}
   
	/* 画像の実際の幅を算出 */
	if (img->BitDepth == 24){
		RealWidth = img->Width * 3 + img->Width % 4;
	}
	else if (img->BitDepth == 8){
		RealWidth = img->Width + img->Width % 4;
	}
   else{
      RealWidth = img->Width / 8 + (img->Width / 8) % 4;
   }
   
	/* ========== ファイルヘッダの作成 ========== */
	/* ファイルタイプ */
	HeaderBuffer[0] = 'B';
	HeaderBuffer[1] = 'M';
	/* ファイルサイズ */
	FileSize = RealWidth * img->Height + HEADERSIZE;
	memcpy(HeaderBuffer + 2, &FileSize, sizeof(int));
	/* 予約領域１ */
	HeaderBuffer[6] = HeaderBuffer[7] = 0x00;
	/* 予約領域２ */
	HeaderBuffer[8] = HeaderBuffer[9] = 0x00;
	/* 画像データまでのオフセット */
	if (img->BitDepth == 24){
		Offset2ImageData = HEADERSIZE;
	}
	else if (img->BitDepth == 8){
		Offset2ImageData = HEADERSIZE + COLORPALLETSIZE48BIT;
	}
   else if(img->BitDepth == 1){
      Offset2ImageData = HEADERSIZE + COLORPALLETSIZE41BIT;
   }
	memcpy(HeaderBuffer + 10, &Offset2ImageData, sizeof(int));
	/* ========================================== */
   
	/* ============ 情報ヘッダの作成 ============ */
	/* 情報ヘッダのサイズ */
	memcpy(HeaderBuffer + 14, &InfoHeader, sizeof(int));
	/* 画像幅 */
	memcpy(HeaderBuffer + 18, &img->Width, sizeof(int));
	/* 画像高さ */
	memcpy(HeaderBuffer + 22, &img->Height, sizeof(int));
	/* プレーン数 */
	memcpy(HeaderBuffer + 26, &Plane, sizeof(short));
	/* ビット深度 */
	memcpy(HeaderBuffer + 28, &img->BitDepth, sizeof(short));
	/* 圧縮形式 */
	memcpy(HeaderBuffer + 30, &Compression, sizeof(int));
	/* 画像データのサイズ */
	ImageSize = RealWidth * img->Height;
	memcpy(HeaderBuffer + 34, &ImageSize, sizeof(int));
	/* 水平方向の解像度 */
	memcpy(HeaderBuffer + 38, &img->XDPI, sizeof(int));
	/* 垂直方向の解像度 */
	memcpy(HeaderBuffer + 42, &img->YDPI, sizeof(int));
	/* パレットの色数 */
	if (img->BitDepth == 24){
		HeaderBuffer[46] = HeaderBuffer[47] = HeaderBuffer[48] = HeaderBuffer[49] = 0x00;
	}
	else if (img->BitDepth == 8){
      PalletUsed = PALLETUSED48BIT;
		memcpy(HeaderBuffer + 46, &PalletUsed, sizeof(int));
	}
   else{
      PalletUsed = PALLETUSED41BIT;
		memcpy(HeaderBuffer + 46, &PalletUsed, sizeof(int));
   }
	/* 重要なパレットのインデックス (よくわからないのでとりあえず０)*/
	HeaderBuffer[50] = HeaderBuffer[51] = HeaderBuffer[52] = HeaderBuffer[53] = 0x00;
	/* ========================================== */
   
	/* ヘッダの書き込み */
	fwrite(HeaderBuffer, sizeof(unsigned char), HEADERSIZE, fp);
   
	/* ========== カラーパレットの作成 ========== */
	if (img->BitDepth != 24){
      if(img->BitDepth == 1){
         /* ========== 1ビット画像の場合 ========== */
         Pallet = (unsigned char*)malloc(sizeof(unsigned char)*COLORPALLETSIZE41BIT);
         /* メモリの確保に失敗した場合 */
         if (Pallet == NULL){
            return Err_OutputImage_ColorPalletMemory;
         }
         /* カラーパレットがない場合（入力が24ビットイメージだった場合） */
         if (img->Pallet == NULL){
            /* カラーパレット格納に必要なメモリの確保 */
            img->Pallet = (ColorPallet*)malloc(sizeof(ColorPallet));
            img->Pallet->r = (unsigned char*)malloc(sizeof(unsigned char)* 2);
            img->Pallet->g = (unsigned char*)malloc(sizeof(unsigned char)* 2);
            img->Pallet->b = (unsigned char*)malloc(sizeof(unsigned char)* 2);
            img->Pallet->Reserved = (unsigned char*)malloc(sizeof(unsigned char)* 2);
            /* メモリの確保に失敗した場合 */
            if (img->Pallet->b == NULL || img->Pallet->g == NULL || img->Pallet->r == NULL || img->Pallet->Reserved == NULL){
               return Err_OutputImage_ColorPalletMemory;
            }
            
            // カラーパレットの作成（2値）
            img->Pallet->r[0] = img->Pallet->g[0] = img->Pallet->b[0] = img->Pallet->Reserved[0] = 0x00;
            img->Pallet->r[1] = img->Pallet->g[1] = img->Pallet->b[1] = (unsigned char)MAXBRIGHTNESS;
            img->Pallet->Reserved[1] = 0x00;
            
         }
         else{
            /* カラーパレットの０、１番目の輝度を書き換える */
            img->Pallet->r[0] = img->Pallet->g[0] = img->Pallet->b[0] = img->Pallet->Reserved[0] = 0x00;
            img->Pallet->r[1] = img->Pallet->g[1] = img->Pallet->b[1] = (unsigned char)MAXBRIGHTNESS;
            img->Pallet->Reserved[1] = 0x00;
         }
         
         // カラーパレット書き込み用のバッファに設定
         for (int i = 0; i < 2; i++){
            Pallet[i * 4]     = img->Pallet->b[i];
            Pallet[i * 4 + 1] = img->Pallet->g[i];
            Pallet[i * 4 + 2] = img->Pallet->r[i];
            Pallet[i * 4 + 3] = img->Pallet->Reserved[i];
         }
         /* カラーパレットの書き込み */
         fwrite(Pallet, sizeof(unsigned char), COLORPALLETSIZE41BIT, fp);
         // 確保したメモリの解放
         free(Pallet);
         /* ======================================= */
         
         
      }
      else if(img->BitDepth == 8){
         /* ========== 8ビット画像の場合 ========== */
         /* カラーパレット書き込み用のメモリ確保 */
         Pallet = (unsigned char*)malloc(sizeof(unsigned char)*COLORPALLETSIZE48BIT);
         /* メモリの確保に失敗した場合 */
         if (Pallet == NULL){
            return Err_OutputImage_ColorPalletMemory;
         }
         /* カラーパレットがない場合（入力が24ビットイメージだった場合） */
         if (img->Pallet == NULL){
            /* カラーパレット格納に必要なメモリの確保 */
            img->Pallet = (ColorPallet*)malloc(sizeof(ColorPallet));
            img->Pallet->r = (unsigned char*)malloc(sizeof(unsigned char)* 256);
            img->Pallet->g = (unsigned char*)malloc(sizeof(unsigned char)* 256);
            img->Pallet->b = (unsigned char*)malloc(sizeof(unsigned char)* 256);
            img->Pallet->Reserved = (unsigned char*)malloc(sizeof(unsigned char)* 256);
            /* メモリの確保に失敗した場合 */
            if (img->Pallet->b == NULL || img->Pallet->g == NULL || img->Pallet->r == NULL || img->Pallet->Reserved == NULL){
               return Err_OutputImage_ColorPalletMemory;
            }
            
            // カラーパレットの作成（単調増加）
            for (int i = 0; i < 256; i++){
               img->Pallet->r[i] = (unsigned char)i;
               img->Pallet->g[i] = (unsigned char)i;
               img->Pallet->b[i] = (unsigned char)i;
               img->Pallet->Reserved[i] = 0;
            }
         }
         // カラーパレット書き込み用のバッファに設定
         for (int i = 0; i < 256; i++){
            Pallet[i * 4] = img->Pallet->b[i];
            Pallet[i * 4 + 1] = img->Pallet->g[i];
            Pallet[i * 4 + 2] = img->Pallet->r[i];
            Pallet[i * 4 + 3] = img->Pallet->Reserved[i];
         }
         /* カラーパレットの書き込み */
         fwrite(Pallet, sizeof(unsigned char), COLORPALLETSIZE48BIT, fp);
         // 確保したメモリの解放
         free(Pallet);
         /* ========================================= */
      }
	}
	/* ========================================== */
   
	/* =========== データ部の作成 =========== */
	/* データ部一行を読込み用のメモリを確保 */
	Data = (unsigned char*)malloc(sizeof(unsigned char)*RealWidth);
   
	if (Data == NULL){
		return Err_OutputImage_ImageDataMemory;
	}
   for(int i = 0; i < RealWidth; i++){
      Data[i] = 0x00;
   }
   
	if (img->BitDepth == 24){
		/* ========== 24ビットカラーの場合 ========== */
		for (y = 0; y < img->Height; y++){
			for (x = 0; x < img->Width; x++){
				Data[x * 3] = img->b[img->Height - y - 1][x];
				Data[x * 3 + 1] = img->g[img->Height - y - 1][x];
				Data[x * 3 + 2] = img->r[img->Height - y - 1][x];
			}
			for (int x = img->Width * 3; x < RealWidth; x++){
				Data[x] = 0;
			}
			fwrite(Data, sizeof(unsigned char), RealWidth, fp);
		}
		/* ============================================*/
	}
	else if (img->BitDepth == 8){
		/* =========== ８ビットグレーの場合 =========== */
		for (y = 0; y < img->Height; y++){
			for (x = 0; x < img->Width; x++){
				for (int i = 0; i < 256; i++){
					if (img->r[img->Height - y - 1][x] == img->Pallet->r[i]){
						Data[x] = (unsigned char)i;
					}
				}
			}
			for (x = img->Width; x < RealWidth; x++){
				Data[x] = 0;
			}
			fwrite(Data, sizeof(unsigned char), RealWidth, fp);
		}
		/* ============================================= */
	}
   else if (img->BitDepth == 1){
		/* =========== 1ビット2値の場合 =========== */
		for (y = 0; y < img->Height; y++){
         x = 0;
         DataCount = 0;
         do{
            if(img->r[img->Height - y - 1][x] == 0xFF){
               BinCount |= 0x01;
            }
            else{
               BinCount |= 0x00;
            }
            
            BitShiftCount++;
            
            if(BitShiftCount < 8){
               BinCount  = BinCount << 1;
            }
            else if(BitShiftCount == 8){
               
               Data[DataCount] = BinCount;
               BinCount = 0x00;
               BitShiftCount = 0;
               DataCount++;
            }
            
            x++;
            
            if(x == img->Width){
               /* 8ビット分の処理が完了していない場合 */
               if(BitShiftCount != 0){
                  /* +1はk<8の場合にすでに１ビット左にシフトしているため */
                  for(int n = BitShiftCount + 1; n < 8 ; n++){
                     BinCount = BinCount << 1;
                  }
                  Data[DataCount] = BinCount;
                  BinCount = 0x00;
                  BitShiftCount = 0;
                  DataCount++;
               }
               
            }
            
         }while(x < img->Width);
         
         /* パディング */
			for (x = DataCount; x < RealWidth; x++){
				Data[x] = 0x00;
			}
         /* データ一行分の書き込み */
			fwrite(Data, sizeof(unsigned char), RealWidth, fp);
		}
		/* ============================================= */
	}
	/* ========================================== */
   
	/* 確保したメモリの解放 */
	free(Data);
	DelateImageStruct(img);
	/* ファイルポインタの解放 */
	fclose(fp);
	/* リターンコードの返却 */
	return OK;
}

/* ********************************************
 * 関数名　：ColorImage2GrayScale
 * 引数　　：イメージ構造体
 * 戻り値　：エラーコード
 * 処理概要：24ビットイメージを8ビットグレーに
 * 　　　　　変換する
 * ******************************************** */
int
ColorImage2GrayScale(Image *img){
	for (int y = 0; y < img->Height; y++){
		for (int x = 0; x < img->Width; x++){
			// 輝度の平均値を算出してグレーの輝度とする
			unsigned char GrayPos = (unsigned char)((img->b[y][x] + img->g[y][x] + img->r[y][x]) / 3);
			img->b[y][x] = img->g[y][x] = img->r[y][x] = GrayPos;
		}
	}
   
	/* イメージのビット深度を８ビットに変更 */
	img->BitDepth = 8;
   
	/* リターンコードの返却 */
	return OK;
}

/* ********************************************
 * 関数名　：InverseImage
 * 引数　　：イメージ構造体
 * 戻り値　：エラーコード
 * 処理概要：イメージの反転処理を行う
 * ******************************************** */
int
InverseImage(Image *img){
	/* ８ビットグレースケール以外の場合 */
	if (img->BitDepth != 8){
		return Err_InverseImage_BitDepth;
	}
	for (int y = 0; y < img->Height; y++){
		for (int x = 0; x < img->Width; x++){
			/* グレースケールの場合、輝度はすべて同値であるため、R輝度を使用 */
			unsigned char InversePos = (unsigned char)(MAXBRIGHTNESS - img->r[y][x]);
			img->r[y][x] = img->g[y][x] = img->b[y][x] = InversePos;
		}
	}
   
	return OK;
}

/* ********************************************
 * 関数名　：LinerTransformation
 * 引数　　：イメージ構造体
 * 戻り値　：エラーコード
 * 処理概要：イメージの線形変換処理を行う
 * ******************************************** */
int
LinerTransformation(Image *img){
   int min;    /* 階調の最小値 */
   int max;    /* 階調の最大値 */
   /* ８ビットグレースケール以外の場合 */
	if (img->BitDepth != 8){
		return Err_LinerTransformation_BitDepth;
	}
   
   /* 階調値の最小値、最大値を求める */
   min = INT_MAX;
   max = INT_MIN;
   for(int y = 0; y < img->Height; y++){
      for(int x = 0; x < img->Width; x++){
         if(img->r[y][x] < min){
            min = img->r[y][x];
         }
         if(img->r[y][x] > max){
            max = img->r[y][x];
         }
      }
   }
   /* 階調の線形変換 */
   for(int y = 0; y < img->Height; y++){
      for(int x = 0;  x < img->Width; x++){
         unsigned char LinerPos = (unsigned char)((img->r[y][x] - min) * MAXBRIGHTNESS / (double)(max-min));
         img->r[y][x] = img->g[y][x] = img->b[y][x] = LinerPos;
      }
   }
   return OK;
}


/* ********************************************
 * 関数名　：DitherImage
 * 引数　　：イメージ構造体
 * 戻り値　：エラーコード
 * 処理概要：組織的ディザ処理
 * ********************************************* */
int
DitherImage(Image *img, int DitherType){
   double dbWidth     =  0.0;          /* 16段階画像の階調値の単位幅 */
   int NewGray        =    0;          /* 新しい階調                 */
   int XBlock         =    0;          /* 横ブロック数               */
   int YBlock         =    0;          /* 縦ブロック数               */
   int x              =    0;          /* 制御用変数                 */
   int y              =    0;          /* 制御用変数                 */
   int **DitherMatrix = NULL;          /* ディザ行列格納用のバッファ */
   
   /* ８ビットグレースケール以外の場合 */
	if (img->BitDepth != 8){
		return Err_DitherImage_BitDepth;
	}
   /* ディザ行列型のチェック */
   if(!(DitherType == 1 || DitherType == 2 || DitherType == 3)){
      return Err_DitherImage_DitherType;
   }
   
   /* ディザ行列のメモリの動的確保（現状は4x4のみ） */
   DitherMatrix = (int**)malloc(sizeof(int*)*4);
   if(DitherMatrix == NULL){
      return Err_DitherImage_Memory;
   }
   for(int i = 0; i < 4; i++){
      DitherMatrix[i] = (int*)malloc(sizeof(int)*4);
      if(DitherMatrix[i] == NULL){
         return Err_DitherImage_Memory;
      }
   }
   
   if(DitherType == 1){
      /* ========== Bayer型の場合 ========== */
      for(int j = 0; j < 4; j++){
         for(int k = 0; k < 4; k++){
            DitherMatrix[j][k] = BayerMatrix[j][k];
         }
      }
      /* =================================== */
      
   }
   else if(DitherType == 2){
      /* ========== 網点型の場合 ========== */
      for(int j = 0; j < 4; j++){
         for(int k = 0; k < 4; k++){
            DitherMatrix[j][k] = HalftoneMatrix[j][k];
         }
      }
      /* ================================== */
      
   }
   else{
      /* ========== 渦巻型の場合 ========== */
      for(int j = 0; j < 4; j++){
         for(int k = 0; k < 4; k++){
            DitherMatrix[j][k] = ScrollMatrix[j][k];
         }
      }
      /* ================================== */
   }
   
   /* 縦、横幅がブロックサイズで割り切れないとエラー */
   if(img->Width % BLOCKSIZE == 0 && img->Height % BLOCKSIZE == 0){
      return Err_DitherImage_ImageSize;
   }
   
   /* 16階調の画像を作成 */
   dbWidth = MAXBRIGHTNESS / (double)(BLOCKSIZE*BLOCKSIZE);
   for(int y1 = 0; y1 < img->Height; y1 ++){
      for(int x1 = 0; x1 < img->Width; x1++){
         NewGray = (int)(img->r[y1][x1] / dbWidth);
         /* 新しく求めた輝度が15より大きい場合、15とする（０〜１５の範囲（１６個） */
         if(NewGray > BLOCKSIZE * BLOCKSIZE - 1){
            NewGray = BLOCKSIZE * BLOCKSIZE -1;
         }
         img->r[y1][x1] = img->g[y1][x1] = img->b[y1][x1] = (unsigned char)NewGray;
      }
   }
   
   /* ディザ画像を作成 */
   /* 横のブロック数 */
   XBlock = img->Width / BLOCKSIZE;
   /* 縦のブロック数 */
   YBlock = img->Height / BLOCKSIZE;
   for(int y2 = 0; y2 < YBlock; y2++){
      for(int x2 = 0; x2 < XBlock; x2++){
         x = BLOCKSIZE * x2;
         y = BLOCKSIZE * y2;
         for(int m = 0; m < BLOCKSIZE; m++){
            for(int n = 0; n < BLOCKSIZE; n++){
               if(img->r[y + m][x + n] <= DitherMatrix[m][n]){
                  img->r[y + m][x + n] = 0;
               }
               else{
                  img->r[y + m][x + n] = MAXBRIGHTNESS;
               }
            }
         }
      }
   }
   
   img->BitDepth = 1;
   
   return OK;
}


/* ********************************************
 * 関数名　：Binarization
 * 引数1 　：イメージ構造体
 * 引数2 　：2値化閾値（-255〜255）
 * 戻り値　：エラーコード
 * 処理概要：手動による2値化処理
 * ********************************************* */
int
Binarization(Image *img ,int Threshold){
   /* ８ビットグレースケール以外の場合 */
	if (img->BitDepth != 8){
		return Err_Binarization_BitDepth;
	}
   
   /* 2値化閾値の判定 */
   if(Threshold < -255 || Threshold > 255){
      return Err_Binarization_Threshold;
   }
   
   for(int y = 0; y < img->Height; y++){
      for(int x = 0; x < img->Width; x++){
         if((int)img->r[y][x] > Threshold){
            img->r[y][x] = img->g[y][x] = img->b[y][x] = 0xFF;
         }
         else{
            img->r[y][x] = img->g[y][x] = img->b[y][x] = 0x00;
         }
      }
   }
   img->BitDepth = 1;
   return OK;
}

/* ********************************************
 * 関数名　：Difusion
 * 引数  　：イメージ構造体
 * 戻り値　：エラーコード
 * 処理概要：誤差拡散処理を行う
 * ********************************************* */
int
Difusion(Image *img){
   int x = 0;                       /* ループ用変数x */
   int y = 0;                       /* ループ用変数y */
   double error1 = 0.0;             /* 階調誤差1 */
   double error2 = 0.0;             /* 階調誤差2 */
   double error3 = 0.0;             /* 階調誤差3 */
   double buffer[2][img->Width];    /* 誤差保存用のバッファ */
   double gray = 0.0;

   /* ８ビットグレースケール以外の場合 */
	if (img->BitDepth != 8){
		return Err_Difusion_BitDepth;
	}
   
   /* 誤差保存用のバッファのクリア */
   for(int y = 0; y < 2; y++){
      for(int x = 0; x < img->Width; x++){
         buffer[y][x] = 0.0;
      }
   }

   for(y = 0; y < img->Height; y++){
      for(x = 0; x < img->Width; x++){
         /* 近傍が外に出るときは単純2値化 */
         if(x == img->Width -1 || y == img->Height - 1){
            if(img->r[y][x] <= MAXBRIGHTNESS / 2){
               img->r[y][x] = img->g[y][x] = img->b[y][x] = 0;
            }
            else{
               img->r[y][x] = img->g[y][x] = img->b[y][x] = MAXBRIGHTNESS;
            }
         }else{
            gray = img->r[y][x] + buffer[0][x];
            error1 = gray - 0.0;
            error2 = gray - MAXBRIGHTNESS;
            if(error1*error1 < error2*error2){
               img->r[y][x] = img->g[y][x] = img->b[y][x] = 0;
               error3 = error2;
            }else{
               img->r[y][x] = img->g[y][x] = img->b[y][x] = MAXBRIGHTNESS;
               error3 = error2;
            }
            
            buffer[0][x+1] = buffer[0][x+1] + error3 * 3.0 / 8.0;
            buffer[1][x+1] = buffer[1][x+1] + error3 * 2.0 / 8.0;
            buffer[1][x]   = buffer[1][x]   + error3 * 3.0 / 8.0;
            
         }
      }
      for(x = 0; x < img->Width; x++){
         buffer[0][x] = buffer[1][x];
         buffer[1][x] = 0.0;
      }
   }
   
   return OK;
}


/* ********************************************
 * 関数名　：SobelFiltering
 * 引数1   ：イメージ構造体
 * 引数2 　：フィルタタイプ
 * 戻り値　：エラーコード
 * 処理概要：誤差拡散処理を行う
 * ********************************************* */
int
SobelFiltering(Image *img ,int FilterType){
   int x             =   0;   /* ループ用変数x  */
   int y             =   0;   /* ループ用変数y  */
   int i             =   0;   /* ループ用変数i  */
   int j             =   0;   /* ループ用変数j  */
   double DivConst   = 1.0;   /* 最後に割る値   */
   double PixelValue = 0.0;   /* 計算用         */
   double min        = 0.0;   /* 計算値の最小値 */
   double max        = 0.0;   /* 計算値の最大値 */
   int        weight[3][3];   /* フィルタ用配列 */

   /* ８ビットグレースケール以外の場合 */
	if (img->BitDepth != 8){
		return Err_SobelFiltering_BitDepth;
	}
   
   
   /* フィルタ用配列の初期化 */
   for(y = 0; y < 3; y++){
      for(x = 0; x < 3; x++){
         weight[y][x] = 0;
      }
   }

   /* フィルタタイプの取得 */
   for(y = 0; y < 3; y++){
      for(x = 0; x < 3; x++){
         if(FilterType == 0){
            weight[y][x] = HorizontalSobel[y][x];
         }
         else{
            weight[y][x] = VerticalSobel[y][x];
         }
      }
   }
   
   min = (double)INT_MAX;
   max = (double)INT_MIN;

   for(y = 1; y < img->Height - 1; y++){
      for(x = 1; x < img->Width - 1; x++){
         PixelValue = 0.0;
         for(i = -1; i < 2; i++){
            for(j = -1; j < 2; j++){
               PixelValue = PixelValue + weight[i+1][j+1]*img->r[y+i][x+j];
            }
            PixelValue = PixelValue / DivConst;
            if(PixelValue < min){
               min = PixelValue;
            }
            if(PixelValue > max){
               max = PixelValue;
            }
         }
      }
   }

   if((int)(max - min) == 0){
      return Err_SobelFiltering_PixelValue;
   }

   for(y = 0; y < img->Height - 1; y++){
      for(x = 0; x < img->Width - 1; x++){
         PixelValue = 0.0;
         for(i = -1; i < 2; i++){
            for(j = -1; j < 2; j++){
               PixelValue = PixelValue + weight[i+1][j+1]*img->r[y+i][x+j];
            }
            PixelValue = PixelValue / DivConst;
            PixelValue = MAXBRIGHTNESS / (max - min)*(PixelValue - min);
            img->r[y][x] = img->g[y][x] = img->b[y][x] = (unsigned char)PixelValue;
         }
      }
   }

   return OK;
}
