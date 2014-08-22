#include "libimg.h"
#include "libimgconf.h"

/* ********************************************
* 関数名　：img_init
* 引数　　：イメージ構造体
* 戻り値　：なし
* 処理概要：img_struct構造体の初期化を行う
* *********************************************/
static void
img_init(img_struct *img){
   img->Width = 0;
   img->Height = 0;
   img->BitDepth = 0;
   img->XDPI = 0;
   img->YDPI = 0;
   img->Pallet = NULL;
   img->r = NULL;/Users/takeharayuk/Desktop/libimg/src/libimg.h
   img->g = NULL;
   img->b = NULL;
}

/* ********************************************
* 関数名　：img_uchar_malloc_array
* 引数1 　：イメージ高さ
* 引数2 　：イメージ横幅
* 戻り値　：イメージ画素格納用の構造体
* 処理概要：イメージの画素を格納する構造体の作成
* 　　　　　を行う。
* *********************************************/
static unsigned char**
img_uchar_malloc_array(int Height, int Width){
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

   for (int y = 0; y < Height; y++){
      for (int x = 0; x < Width; x++){
         /* 確保したメモリの初期化 */
         Brightness[y][x] = 0x00;
      }
   }
   return Brightness;
}

/* ********************************************
 * 関数名　：img_double_malloc_array
 * 引数1 　：イメージ高さ
 * 引数2 　：イメージ横幅
 * 戻り値　：イメージ画素格納用の構造体
 * 処理概要：イメージの画素を格納する構造体の作成
 * 　　　　　を行う。
 * *********************************************/
static double**
img_double_malloc_array(int Height, int Width){
   double **Brightness = (double**)malloc(sizeof(double*)*Height);
   if (Brightness == NULL){
      return NULL;
   }
   for (int y = 0; y < Height; y++){
      Brightness[y] = (double*)malloc(sizeof(double)* Width);
      if (Brightness[y] == NULL){
         return NULL;
      }
   }
   
   for (int y = 0; y < Height; y++){
      for (int x = 0; x < Width; x++){
         /* 確保したメモリの初期化 */
         Brightness[y][x] = 0x00;
      }
   }
   return Brightness;
}

/* ********************************************
* 関数名　：img_free
* 引数　　：イメージ構造体
* 戻り値　：なし
* 処理概要：イメージ構造体の解放を行う
* *********************************************/
static void
img_free(img_struct *img){
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
 * 関数名　：img_calc_power_of_two
 * 引数 　 ：イメージ構造体
 * 戻り値　：乗数
 * 処理概要：引数が２の何乗であるかを確認して
 *           乗数を返却する。
 * *********************************************/
static int
img_calc_power_of_two(int num){
   /* 2の乗数 */
   int power = 0;
   while(num != 1){
      /* 乗数カウント */
      power++;
      /* 右方向に1ビットシフト（2で割る） */
      num = num >> 1;
   }
   /* 乗数を返却 */
   return power;
}

/* ********************************************
 * 関数名　：img_make_initial_data
 * 引数1 　：元データ（実数部または虚数部）の先頭アドレス
 * 引数3 　：データサイズ（縦幅または横幅）
 * 引数4 　：乗数（data_sizeは2のpower乗）
 * 戻り値　：なし
 * 処理概要：FFTの初期データを作成する
 * *********************************************/
static void
img_make_initial_data(double *data,int data_size,int power){
   int ptr = 0;      /* 元データの要素番号を決めるための変数 */
   int offset = 0;   /* 元データの要素番号を決めるためのオフセット */
   int new_ptr = 0;  /* 計算後のデータを代入する配列の要素番号 */
   int dft = 0;      /* DFT点数（iのループで半分ずつになる） */
   double buf[data_size];     /* 計算結果格納用配列 */
   
   /* DFT点数にデータ総数を格納 */
   dft = data_size;
   /* 乗数の数だけループ */
   for(int i = 1;i < power; i++){
      /* 要素番号の初期化 */
      new_ptr = 0;
      /* オフセットの初期化 */
      offset = 0;
      /* データが存在する限りループ */
      while(new_ptr < data_size){
         /* 要素番号の初期化 */
         ptr = 0;
         /* 要素番号がDFT点数より小さくなるまでループ */
         while(ptr < dft){
            buf[new_ptr] = *(data + offset + ptr);
            new_ptr++;
            ptr = ptr + 2;
            if(ptr == dft){
               ptr = 1;
            }
            
         }
         /* オフセットのカウント */
         offset = offset + dft;
      }
      
      /* 計算結果を元のデータ配列にコピーする */
      for(int j = 0;j < data_size; j++){
         *(data + j) = buf[j];
      }
      dft = dft / 2;
      if(dft % 2 != 0){
         break;
      }
   }
}


/* ********************************************
 * 関数名　：img_fft1
 * 引数1 　：元データ（実数部または虚数部）の先頭アドレス
 * 引数2 　：データサイズ（縦幅または横幅）
 * 引数3 　：フーリエ変換ブラグ（１：FFT、-1：逆FFT）
 * 引数4   ；データ種別（１：実数部、1以外：虚数部）
 * 戻り値　：なし
 * 処理概要：1次元フーリエ変換を行う
 * *********************************************/
static void
img_fft1(double *data,int data_size,int flag,int type){
   int power = 0;             /* べき乗 */
   double unit_angle = 0;     /* 角度用 */
   double step_angle = 0;     /* 角度用 */
   int dft = 0;               /* DFT点数 */
   int half = 0;              /* DFT点数の半数 */
   int num_of_dft = 0;        /* DFT実行回数 */
   int num_out = 0;           /* DFT出力信号線の番号 */
   int num_in1 = 0;           /* DFT入力信号線の番号 */
   int num_in2 = 0;           /* DFT入力信号線の番号 */
   double buf = 0.0;       /* 実数部 */
   double angle = 0.0;        /* 角度 */
   
   /* 計算結果を一時的に保存するための配列 */
   double data_new[data_size];
   
   /* 逆FFTのとき、各データをnum_of_dataで割り複素共役を取る */
   if(flag == -1){
      for(int i = 0; i < data_size; i++){
         *(data + i) = *(data + i) / data_size;
      }
   }
   
   /* num_of_dataが2の何乗であるかを調べる */
   power = img_calc_power_of_two(data_size);
   /* 初期データのための元データの順番の入れ替え */
   img_make_initial_data(data,data_size,power);
   /* 2点、4点、8点DFT・・・の順に実行する */
   unit_angle = 2.0 * PI / data_size;
   /* DFT点数の初期値→２倍していく */
   dft = 2;
   
   for(int i = 0; i < power; i++){
      /* DFT点のDFTを行う */
      /* i=0 -> 2点、i=1 -> 4点、i=2 -> 8点・・・ */
      num_of_dft = data_size / dft;     /* 実行回数 */
      step_angle = unit_angle * num_of_dft;
      half = dft / 2;
      for(int j = 0; j < num_of_dft; j++){
         angle = 0.0;
         for(int k = 0; k < dft; k++){
            /* num_in1,num_in2からnum_outを出力
             （係数が1の方の信号をnum_in1としている）*/
            num_out = j * dft + k;
            if(k < half){
               num_in1 = num_out;
               num_in2 = num_in1 + half;
            }else{
               num_in2 = num_out;
               num_in1 = num_out - half;
            }
            /* 実数部、虚数部に分けて計算する */
            buf = *(data + num_in2);
            if(type == 1){
               /* 実数部の場合 */
               data_new[num_out] = *(data + num_in1) + buf * cos(angle) + buf * sin(angle);
            }
            else{
               /* 虚数部の場合 */
               data_new[num_out] = *(data + num_in1) + buf * cos(angle) - buf * sin(angle);
            }

            /* 角度の更新 */
            angle = angle + step_angle;
         }
      }
      /* 計算後のデータを元の配列に戻す */
      for(int j = 0; j < data_size; j++){
         *(data + j) = data_new[j];
      }
      /* dftの数を２倍に更新 */
      dft = dft * 2;
   }
   
   /* 逆FFTの場合、各データの複素共役を取る */
   if(flag == -1){
      for(int j = 0; j < data_size; j++){
         *(data + j) = -*(data + j);
      }
   }
}

/* ********************************************
 * 関数名　：img_fft2
 * 引数1 　：元データ（実数部）の先頭アドレス
 * 引数2 　：元データ（虚数部）の先頭アドレス
 * 引数3 　：フーリエ変換ブラグ（1：FFT、-1：逆FFT）
 * 引数4 　：画像横幅
 * 引数5 　：画像縦幅
 * 戻り値　：なし
 * 処理概要：2次元フーリエ変換を行う
 * *********************************************/
static void
img_fft2(double **data,double **jdata,
                        int flag,int x_size,int y_size){
   double re[x_size];
   double im[y_size];
   
   for(int y = 0; y < y_size; y++){
      for(int x = 0; x < x_size; x++){
         /* 実数部、虚数部のデータを取得 */
         re[x] =  data[y][x];
         im[x] = jdata[y][x];
      }
      /* 一次元フーリエ変換実施 */
      img_fft1(re,x_size,flag,1);
      img_fft1(im,y_size,flag,-1);
      for(int x = 0; x < x_size; x++){
         /* フーリエ変換したデータを格納 */
         data[y][x] = re[x];
         jdata[y][x] = im[x];
      }
   }
   
   for(int x = 0; x < x_size; x++){
      for(int y = 0; y < y_size; y++){
         /* 実数部、虚数部のデータを取得 */
         re[x] =  data[y][x];
         im[x] = jdata[y][x];
      }
      /* 一次元フーリエ変換実施 */
      img_fft1(re,x_size,flag,1);
      img_fft1(im,y_size,flag,-1);
      for(int y = 0; y < y_size; y++){
         /* フーリエ変換したデータを格納 */
         data[y][x]  = re[y];
         jdata[y][x] = im[y];
      }
      
   }
   
}

/* ********************************************
 * 関数名　：img_make_fft_data
 * 引数1 　：イメージ構造体
 * 引数2 　：実数部データ
 * 引数3 　：虚数部データ
 * 戻り値　：なし
 * 処理概要：フーリエ変換用データの作成を行う
 * *********************************************/
static void
img_make_fft_data(img_struct img,double **data,double **jdata){
   for(int y = 0; y < img.Height; y++){
      for(int x= 0; x < img.Width; x++){
         data[y][x] = (double)img.r[y][x];
         jdata[y][x] = 0.0;
      }
   }
}

/* ********************************************
 * 関数名　：img_fft_filter
 * 引数1 　：実数部データ
 * 引数2 　：虚数部データ
 * 引数3 　：画像横幅
 * 引数4 　：画像縦幅
 * 戻り値　：なし
 * 処理概要：周波数領域に対するフィルタリング
 * *********************************************/
static void
img_fft_filter(double **data,double **jdata,int x_size,int y_size){
   int max = 0;
   max = 8;
   for(int y = 0; y < y_size; y++){
      for(int x = 0; x < x_size; x++){
         if((max < y) && (y < (y_size - max)) || (max < x) && (x < (x_size -max))){
            data[y][x] = 0.0;
            jdata[y][x] = 0.0;
         }
      }
   }
}

/* ********************************************
* 関数名　：img_read
* 引数1　 ：イメージファイルパス
* 引数2　 ：イメージ構造体
* 戻り値　：エラーコード
* 処理概要：イメージファイルの読込みを行う
* *********************************************/
int
img_read(char *inputfileName, img_struct *img){
   FILE *fp;                                           /* ファイルオープン用のポインタ     */
   int x = 0;                                          /* x方向の画像走査用                */
   int y = 0;                                          /* y方向の画像走査用                */
   int RealWidth = 0;                                  /* 実際の画像幅                     */
   unsigned char HeaderBuffer[HEADERSIZE];             /* ヘッダ格納用のバッファ           */
   unsigned char *Data;                                /* イメージデータ１行分格納用の配列 */
   unsigned char *Pallet;                              /* カラーパレット格納用             */
   int img_colorIndex = 0;                           /* カラーパレットインデックス       */
   int DataCount = 0;                      /* 書き込みデータ配列のカウント用        */
   unsigned char Byte = 0x00;                   /* ２値画像処理時の1バイト分読込み用     */
   unsigned char FirstNibble = 0x00;                   /* ２値画像処理時の前半ニブル処理用      */
   unsigned char LastNibble = 0x00;                   /* ２値画像処理時の後半ニブル処理用      */

   /* イメージ構造体の初期化 */
   img_init(img);

   /* 入力イメージのファイルオープン */
   fp = fopen(inputfileName, "rb");
   /* ファイルオープンに失敗した場合 */
   if (fp == NULL){
      return ERR_IMG_READ_FILEOPEN;
   }

   /* BMPヘッダの読込み */
   fread(HeaderBuffer, sizeof(unsigned char), HEADERSIZE, fp);
   /* ファイルタイプのチェック */
   if (!(HeaderBuffer[0] == 'B' && HeaderBuffer[1] == 'M')){
      return ERR_IMG_READ_FILETYPE;
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
      return ERR_IMG_READ_BITDEPTH;
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
   if (Data == NULL){
      return ERR_IMG_READ_MEMORY;
   }

   for (int i = 0; i < RealWidth; i++){
      Data[i] = 0x00;
   }

   img->r =img_uchar_malloc_array(img->Height, img->Width);
   img->g =img_uchar_malloc_array(img->Height, img->Width);
   img->b =img_uchar_malloc_array(img->Height, img->Width);
   if (img->r == NULL || img->g == NULL || img->b == NULL){
      img_free(img);
      return ERR_IMG_READ_MEMORY;
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
      Pallet = (unsigned char*)malloc(sizeof(unsigned char)* COLORPALLETSIZE48BIT);
      /* カラーパレット取得用のメモリ確保 */
      img->Pallet = (img_color*)malloc(sizeof(img_color));
      img->Pallet->r = (unsigned char*)malloc(sizeof(unsigned char)* 256);
      img->Pallet->g = (unsigned char*)malloc(sizeof(unsigned char)* 256);
      img->Pallet->b = (unsigned char*)malloc(sizeof(unsigned char)* 256);
      img->Pallet->Reserved = (unsigned char*)malloc(sizeof(unsigned char)* 256);
      /* メモリ確保に失敗した場合 */
      if (img->Pallet->b == NULL || img->Pallet->g == NULL || img->Pallet->r == NULL || img->Pallet->Reserved == NULL){
         img_free(img);
         return ERR_IMG_READ_MEMORY;
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
            img_colorIndex = (int)Data[x];
            /* インデックスに紐付いた輝度を取得 */
            img->b[img->Height - y - 1][x] = img->Pallet->b[img_colorIndex];
            img->g[img->Height - y - 1][x] = img->Pallet->g[img_colorIndex];
            img->r[img->Height - y - 1][x] = img->Pallet->r[img_colorIndex];
         }
      }
      /* ================================================== */
   }
   else if (img->BitDepth == 1){
      /* =============  1ビットイメージの場合 ============= */
      Pallet = (unsigned char*)malloc(sizeof(unsigned char)* COLORPALLETSIZE41BIT);
      /* カラーパレット取得用のメモリ確保 */
      img->Pallet = (img_color*)malloc(sizeof(img_color));
      img->Pallet->r = (unsigned char*)malloc(sizeof(unsigned char)* 2);
      img->Pallet->g = (unsigned char*)malloc(sizeof(unsigned char)* 2);
      img->Pallet->b = (unsigned char*)malloc(sizeof(unsigned char)* 2);
      img->Pallet->Reserved = (unsigned char*)malloc(sizeof(unsigned char)* 2);
      /* メモリ確保に失敗した場合 */
      if (img->Pallet->b == NULL || img->Pallet->g == NULL || img->Pallet->r == NULL || img->Pallet->Reserved == NULL){
         img_free(img);
         return ERR_IMG_READ_MEMORY;
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
            Byte = 0x00;                   /* ２値画像処理時の1バイト分読込み用     */
            FirstNibble = 0x00;                   /* ２値画像処理時の前半ニブル処理用      */
            LastNibble = 0x00;                   /* ２値画像処理時の後半ニブル処理用      */

            // １バイト分取得
            Byte = Data[DataCount];

            // 最初の４ビット取得
            FirstNibble = Byte >> 4;
            // 後半４ビット取得
            LastNibble = Byte << 4;
            LastNibble = LastNibble >> 4;

            for (int k = 0; k < 8; k++){
               /* 論理積を計算して、どのビットが立っているかを判別する */
               /* 前半４ビット */
               if (k < 4)
               {
                  if (0 == (FirstNibble & BitMask[k]))
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
               else if (k > 3)
               {
                  if (0 == (LastNibble & BitMask[k]))
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
               if (x == img->Width){
                  break;
               }
            }

            DataCount++;

         } while (x < img->Width);

      }
   }
   /* ================================================== */


   /* メモリの解放 */
   free(Data);
   fclose(fp);
   return OK;
}

/* ********************************************
* 関数名　：img_write
* 引数1　 ：イメージファイルパス
* 引数2　 ：イメージ構造体
* 戻り値　：エラーコード
* 処理概要：イメージのファイル出力を行う
* *********************************************/
int
img_write(char *outputfileName, img_struct *img){
   FILE *fp;                                       /* ファイルオープン用のポインタ */
   int x = 0;                                      /* x方向の画像走査用                */
   int y = 0;                                      /* y方向の画像走査用                */
   int RealWidth = 0;                              /* 実際の画像幅 */
   int FileSize = 0;                               /* ファイルサイズ */
   int img_structSize = 0;                              /* イメージサイズ */
   int Offset2img_structData = 0;                       /* イメージデータまでのオフセット */
   int InfoHeader = INFOHEADERSIZE;                /* 情報ヘッダのサイズ */
   short Plane = 1;                                /* プレーン数 */
   int Compression = 0;                            /* 圧縮形式 */
   int PalletUsed = 0;                             /* 8ビットイメージのパレット数 */
   unsigned char HeaderBuffer[HEADERSIZE];         /* ヘッダ処理用のバッファ */
   unsigned char *Pallet;                          /* パレット格納用の配列 */
   unsigned char *Data;                            /* イメージデータ１行分格納用の配列 */
   unsigned char BinCount = 0x00;                  /* ２値イメージの場合の演算用    */
   int BitShiftCount = 0;                          /* ビットシフトカウント用（左に何ビットシフトしたか） */
   int DataCount = 0;                          /* 書き込みデータ配列のカウント用 */

   /* 出力ファイルのオープン */
   fp = fopen(outputfileName, "wb");
   if (fp == NULL){
      return ERR_IMG_WRITE_FILEOPEN;
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
      Offset2img_structData = HEADERSIZE;
   }
   else if (img->BitDepth == 8){
      Offset2img_structData = HEADERSIZE + COLORPALLETSIZE48BIT;
   }
   else if (img->BitDepth == 1){
      Offset2img_structData = HEADERSIZE + COLORPALLETSIZE41BIT;
   }
   memcpy(HeaderBuffer + 10, &Offset2img_structData, sizeof(int));
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
   img_structSize = RealWidth * img->Height;
   memcpy(HeaderBuffer + 34, &img_structSize, sizeof(int));
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
      if (img->BitDepth == 1){
         /* ========== 1ビット画像の場合 ========== */
         Pallet = (unsigned char*)malloc(sizeof(unsigned char)*COLORPALLETSIZE41BIT);
         /* メモリの確保に失敗した場合 */
         if (Pallet == NULL){
            return ERR_IMG_WRITE_COLORMEMORY;
         }
         /* カラーパレットがない場合（入力が24ビットイメージだった場合） */
         if (img->Pallet == NULL){
            /* カラーパレット格納に必要なメモリの確保 */
            img->Pallet = (img_color*)malloc(sizeof(img_color));
            img->Pallet->r = (unsigned char*)malloc(sizeof(unsigned char)* 2);
            img->Pallet->g = (unsigned char*)malloc(sizeof(unsigned char)* 2);
            img->Pallet->b = (unsigned char*)malloc(sizeof(unsigned char)* 2);
            img->Pallet->Reserved = (unsigned char*)malloc(sizeof(unsigned char)* 2);
            /* メモリの確保に失敗した場合 */
            if (img->Pallet->b == NULL || img->Pallet->g == NULL || img->Pallet->r == NULL || img->Pallet->Reserved == NULL){
               return ERR_IMG_WRITE_COLORMEMORY;
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
            Pallet[i * 4] = img->Pallet->b[i];
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
      else if (img->BitDepth == 8){
         /* ========== 8ビット画像の場合 ========== */
         /* カラーパレット書き込み用のメモリ確保 */
         Pallet = (unsigned char*)malloc(sizeof(unsigned char)*COLORPALLETSIZE48BIT);
         /* メモリの確保に失敗した場合 */
         if (Pallet == NULL){
            return ERR_IMG_WRITE_COLORMEMORY;
         }
         /* カラーパレットがない場合（入力が24ビットイメージだった場合） */
         if (img->Pallet == NULL){
            /* カラーパレット格納に必要なメモリの確保 */
            img->Pallet = (img_color*)malloc(sizeof(img_color));
            img->Pallet->r = (unsigned char*)malloc(sizeof(unsigned char)* 256);
            img->Pallet->g = (unsigned char*)malloc(sizeof(unsigned char)* 256);
            img->Pallet->b = (unsigned char*)malloc(sizeof(unsigned char)* 256);
            img->Pallet->Reserved = (unsigned char*)malloc(sizeof(unsigned char)* 256);
            /* メモリの確保に失敗した場合 */
            if (img->Pallet->b == NULL || img->Pallet->g == NULL || img->Pallet->r == NULL || img->Pallet->Reserved == NULL){
               return ERR_IMG_WRITE_COLORMEMORY;
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
      return ERR_IMG_WRITE_IMAGEDATAMEMORY;
   }
   for (int i = 0; i < RealWidth; i++){
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
            if (img->r[img->Height - y - 1][x] == 0xFF){
               BinCount |= 0x01;
            }
            else{
               BinCount |= 0x00;
            }

            BitShiftCount++;

            if (BitShiftCount < 8){
               BinCount = BinCount << 1;
            }
            else if (BitShiftCount == 8){

               Data[DataCount] = BinCount;
               BinCount = 0x00;
               BitShiftCount = 0;
               DataCount++;
            }

            x++;

            if (x == img->Width){
               /* 8ビット分の処理が完了していない場合 */
               if (BitShiftCount != 0){
                  /* +1はk<8の場合にすでに１ビット左にシフトしているため */
                  for (int n = BitShiftCount + 1; n < 8; n++){
                     BinCount = BinCount << 1;
                  }
                  Data[DataCount] = BinCount;
                  BinCount = 0x00;
                  BitShiftCount = 0;
                  DataCount++;
               }

            }

         } while (x < img->Width);

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
   img_free(img);
   /* ファイルポインタの解放 */
   fclose(fp);
   /* リターンコードの返却 */
   return OK;
}

/* ********************************************
* 関数名　：img_color2gray
* 引数　　：イメージ構造体
* 戻り値　：エラーコード
* 処理概要：24ビットイメージを8ビットグレーに
* 　　　　　変換する
* *********************************************/
int
img_color2gray(img_struct *img){
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
* 関数名　：img_inverse
* 引数　　：イメージ構造体
* 戻り値　：エラーコード
* 処理概要：イメージの反転処理を行う
* *********************************************/
int
img_inverse(img_struct *img){
   /* ８ビットグレースケール以外の場合 */
   if (img->BitDepth != 8){
      return ERR_IMG_INVERSE_BITDEPTH;
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
* 関数名　：img_linertransform
* 引数　　：イメージ構造体
* 戻り値　：エラーコード
* 処理概要：イメージの線形変換処理を行う
* *********************************************/
int
img_linertransform(img_struct *img){
   int min;    /* 階調の最小値 */
   int max;    /* 階調の最大値 */
   /* ８ビットグレースケール以外の場合 */
   if (img->BitDepth != 8){
      return ERR_IMG_LINERTRANSFORM_BITDEPTH;
   }

   /* 階調値の最小値、最大値を求める */
   min = INT_MAX;
   max = INT_MIN;
   for (int y = 0; y < img->Height; y++){
      for (int x = 0; x < img->Width; x++){
         if (img->r[y][x] < min){
            min = img->r[y][x];
         }
         if (img->r[y][x] > max){
            max = img->r[y][x];
         }
      }
   }
   /* 階調の線形変換 */
   for (int y = 0; y < img->Height; y++){
      for (int x = 0; x < img->Width; x++){
         unsigned char LinerPos = (unsigned char)((img->r[y][x] - min) * MAXBRIGHTNESS / (double)(max - min));
         img->r[y][x] = img->g[y][x] = img->b[y][x] = LinerPos;
      }
   }
   return OK;
}


/* ********************************************
* 関数名　：img_dither
* 引数　　：イメージ構造体
* 戻り値　：エラーコード
* 処理概要：組織的ディザ処理
* **********************************************/
int
img_dither(img_struct *img, int dithertype){
   double dbWidth = 0.0;          /* 16段階画像の階調値の単位幅 */
   int NewGray = 0;          /* 新しい階調                 */
   int XBlock = 0;          /* 横ブロック数               */
   int YBlock = 0;          /* 縦ブロック数               */
   int x = 0;          /* 制御用変数                 */
   int y = 0;          /* 制御用変数                 */
   int **DitherMatrix = NULL;          /* ディザ行列格納用のバッファ */

   /* ８ビットグレースケール以外の場合 */
   if (img->BitDepth != 8){
      return ERR_IMG_DITHER_BITDEPTH;
   }
   /* ディザ行列型のチェック */
   if (!(dithertype == 1 || dithertype == 2 || dithertype == 3)){
      return ERR_IMG_DITHER_DITHERTYPE;
   }

   /* ディザ行列のメモリの動的確保（現状は4x4のみ） */
   DitherMatrix = (int**)malloc(sizeof(int*)* 4);
   if (DitherMatrix == NULL){
      return ERR_IMG_DITHER_MEMORY;
   }
   for (int i = 0; i < 4; i++){
      DitherMatrix[i] = (int*)malloc(sizeof(int)* 4);
      if (DitherMatrix[i] == NULL){
         return ERR_IMG_DITHER_MEMORY;
      }
   }

   if (dithertype == 1){
      /* ========== Bayer型の場合 ========== */
      for (int j = 0; j < 4; j++){
         for (int k = 0; k < 4; k++){
            DitherMatrix[j][k] = BayerMatrix[j][k];
         }
      }
      /* =================================== */

   }
   else if (dithertype == 2){
      /* ========== 網点型の場合 ========== */
      for (int j = 0; j < 4; j++){
         for (int k = 0; k < 4; k++){
            DitherMatrix[j][k] = HalftoneMatrix[j][k];
         }
      }
      /* ================================== */

   }
   else{
      /* ========== 渦巻型の場合 ========== */
      for (int j = 0; j < 4; j++){
         for (int k = 0; k < 4; k++){
            DitherMatrix[j][k] = ScrollMatrix[j][k];
         }
      }
      /* ================================== */
   }

   /* 縦、横幅がブロックサイズで割り切れないとエラー */
   if (img->Width % BLOCKSIZE == 0 && img->Height % BLOCKSIZE == 0){
      return ERR_IMG_DITHER_IMAGESIZE;
   }

   /* 16階調の画像を作成 */
   dbWidth = MAXBRIGHTNESS / (double)(BLOCKSIZE*BLOCKSIZE);
   for (int y1 = 0; y1 < img->Height; y1++){
      for (int x1 = 0; x1 < img->Width; x1++){
         NewGray = (int)(img->r[y1][x1] / dbWidth);
         /* 新しく求めた輝度が15より大きい場合、15とする（０?１５の範囲（１６個） */
         if (NewGray > BLOCKSIZE * BLOCKSIZE - 1){
            NewGray = BLOCKSIZE * BLOCKSIZE - 1;
         }
         img->r[y1][x1] = img->g[y1][x1] = img->b[y1][x1] = (unsigned char)NewGray;
      }
   }

   /* ディザ画像を作成 */
   /* 横のブロック数 */
   XBlock = img->Width / BLOCKSIZE;
   /* 縦のブロック数 */
   YBlock = img->Height / BLOCKSIZE;
   for (int y2 = 0; y2 < YBlock; y2++){
      for (int x2 = 0; x2 < XBlock; x2++){
         x = BLOCKSIZE * x2;
         y = BLOCKSIZE * y2;
         for (int m = 0; m < BLOCKSIZE; m++){
            for (int n = 0; n < BLOCKSIZE; n++){
               if (img->r[y + m][x + n] <= DitherMatrix[m][n]){
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
* 関数名　：img_binarize
* 引数1 　：イメージ構造体
* 引数2 　：2値化閾値（-255?255）
* 戻り値　：エラーコード
* 処理概要：手動による2値化処理
* **********************************************/
int
img_binarize(img_struct *img, int threshold){
   /* ８ビットグレースケール以外の場合 */
   if (img->BitDepth != 8){
      return ERR_IMG_BINARIZE_BITDEPTH;
   }

   /* 2値化閾値の判定 */
   if (threshold < -255 || threshold > 255){
      return ERR_IMG_BINARIZE_THRESHOLD;
   }

   for (int y = 0; y < img->Height; y++){
      for (int x = 0; x < img->Width; x++){
         if ((int)img->r[y][x] > threshold){
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
* 関数名　：img_difuse
* 引数  　：イメージ構造体
* 戻り値　：エラーコード
* 処理概要：誤差拡散処理を行う
* **********************************************/
int
img_difuse(img_struct *img){
   int x = 0;                       /* ループ用変数x */
   int y = 0;                       /* ループ用変数y */
   double error1 = 0.0;             /* 階調誤差1 */
   double error2 = 0.0;             /* 階調誤差2 */
   double error3 = 0.0;             /* 階調誤差3 */
   double **buffer = NULL;          /* 誤差保存用のバッファ */
   double gray = 0.0;

   /* ８ビットグレースケール以外の場合 */
   if (img->BitDepth != 8){
      return ERR_IMG_DIFUSE_BITDEPTH;
   }

   /* 2次元配列の確保 */
   buffer = (double**)malloc(sizeof(double*)* 2);
   if (buffer == NULL){
      return ERR_IMG_DIFUSE_MEMORY;
   }
   for (int y = 0; y < 2; y++){
      buffer[y] = (double*)malloc(sizeof(double)*img->Width);
      if (buffer[y] == NULL){
         free(buffer);
         return ERR_IMG_DIFUSE_MEMORY;
      }
   }

   /* 2次元配列の初期化 */
   for (int y = 0; y < 2; y++){
      for (int x = 0; x < img->Width; x++){
         buffer[y][x] = 0.0;
      }
   }

   for (y = 0; y < img->Height; y++){
      for (x = 0; x < img->Width; x++){
         /* 近傍が外に出るときは単純2値化 */
         if (x == img->Width - 1 || y == img->Height - 1){
            if (img->r[y][x] <= MAXBRIGHTNESS / 2){
               img->r[y][x] = img->g[y][x] = img->b[y][x] = 0;
            }
            else{
               img->r[y][x] = img->g[y][x] = img->b[y][x] = MAXBRIGHTNESS;
            }
         }
         else{
            gray = img->r[y][x] + buffer[0][x];
            error1 = gray - 0.0;
            error2 = gray - MAXBRIGHTNESS;
            if (error1*error1 < error2*error2){
               img->r[y][x] = img->g[y][x] = img->b[y][x] = 0;
               error3 = error2;
            }
            else{
               img->r[y][x] = img->g[y][x] = img->b[y][x] = MAXBRIGHTNESS;
               error3 = error2;
            }

            buffer[0][x + 1] = buffer[0][x + 1] + error3 * 3.0 / 8.0;
            buffer[1][x + 1] = buffer[1][x + 1] + error3 * 2.0 / 8.0;
            buffer[1][x] = buffer[1][x] + error3 * 3.0 / 8.0;

         }
      }
      for (x = 0; x < img->Width; x++){
         buffer[0][x] = buffer[1][x];
         buffer[1][x] = 0.0;
      }
   }

   return OK;
}


/* ********************************************
* 関数名　：img_sobelfilter
* 引数1   ：イメージ構造体
* 引数2 　：フィルタタイプ
* 戻り値　：エラーコード
* 処理概要：誤差拡散処理を行う
* **********************************************/
int
img_sobelfilter(img_struct *img, int filtertype){
   int x = 0;   /* ループ用変数x  */
   int y = 0;   /* ループ用変数y  */
   int i = 0;   /* ループ用変数i  */
   int j = 0;   /* ループ用変数j  */
   double DivConst = 1.0;   /* 最後に割る値   */
   double PixelValue = 0.0;   /* 計算用         */
   double min = 0.0;   /* 計算値の最小値 */
   double max = 0.0;   /* 計算値の最大値 */
   int weight[3][3];   /* フィルタ用配列 */

   /* ８ビットグレースケール以外の場合 */
   if (img->BitDepth != 8){
      return ERR_IMG_SOBELFILTER_BITDEPTH;
   }


   /* フィルタ用配列の初期化 */
   for (y = 0; y < 3; y++){
      for (x = 0; x < 3; x++){
         weight[y][x] = 0;
      }
   }

   /* フィルタタイプの取得 */
   for (y = 0; y < 3; y++){
      for (x = 0; x < 3; x++){
         if (filtertype == 0){
            weight[y][x] = HorizontalSobel[y][x];
         }
         else if(filtertype == 1){
            weight[y][x] = VerticalSobel[y][x];
         }
         else{
            return ERR_IMG_SOBELFILTER_FILTERTYPE;
         }
      }
   }
   /* Pixelの最大値、最小値の設定 */
   min = (double)INT_MAX;
   max = (double)INT_MIN;

   /* Pixelの最大値、最小値を求める */
   for (y = 1; y < img->Height - 1; y++){
      for (x = 1; x < img->Width - 1; x++){
         PixelValue = 0.0;
         for (i = -1; i < 2; i++){
            for (j = -1; j < 2; j++){
               PixelValue = PixelValue + (double)(weight[i + 1][j + 1] * (int)img->r[y + i][x + j]);
            }
            PixelValue = PixelValue / DivConst;
            if (PixelValue < min){
               min = PixelValue;
            }
            if (PixelValue > max){
               max = PixelValue;
            }
         }
      }
   }

   /* Pixelの最大値、最小値の差分が0の場合 */
   if ((int)(max - min) == 0){
      return ERR_IMG_SOBELFILTER_PIXELVALUE;
   }

   /* イメージの変換 */
   for (y = 1; y < img->Height - 1; y++){
      for (x = 1; x < img->Width - 1; x++){
         PixelValue = 0.0;
         for (i = -1; i < 2; i++){
            for (j = -1; j < 2; j++){
               PixelValue = PixelValue + (double)(weight[i + 1][j + 1] * (int)img->r[y + i][x + j]);
            }
            PixelValue = PixelValue / DivConst;
            PixelValue = MAXBRIGHTNESS / (max - min)*(PixelValue - min);
            img->r[y][x] = img->g[y][x] = img->b[y][x] = (unsigned char)PixelValue;
         }
      }
   }

   return OK;
}

/* ********************************************
* 関数名　：img_smooth
* 引数1   ：イメージ構造体
* 引数2 　：フィルタタイプ
* 戻り値　：エラーコード
* 処理概要：線形平滑化を行う
* **********************************************/
int img_smooth(img_struct *img,int filtertype){
   double div_const = 0.0;       /* 最後に割る値 */
   double new_value = 0.0;       /* 処理後の値   */
   int x = 0;              /* ループ用変数x  */
   int y = 0;              /* ループ用変数y  */
   int i = 0;              /* ループ用変数i  */
   int j = 0;              /* ループ用変数j  */
   int weight[3][3];       /* フィルタ用配列  */

   /* ８ビットグレースケール以外の場合 */
   if (img->BitDepth != 8){
      return ERR_IMG_SMOOTH_BITDEPTH;
   }

   if(filtertype == 0){
      div_const = 9.0;
   }
   else if(filtertype == 1){
      div_const = 10.0;
   }
   else{
      return ERR_IMG_SMOOTH_FILTERTYPE;
   }

      /* フィルタ用配列の初期化 */
   for (y = 0; y < 3; y++){
      for (x = 0; x < 3; x++){
         weight[y][x] = 0;
      }
   }

   /* フィルタタイプの取得 */
   for (y = 0; y < 3; y++){
      for (x = 0; x < 3; x++){
         if (filtertype == 0){
            weight[y][x] = SmoothFilter1[y][x];
         }
         else{
            weight[y][x] = SmoothFilter2[y][x];
         }
      }
   }

   /* フィルタリングの実施 */
   for(y = 1; y < img->Height - 1; y++){
      for(x = 1; x < img->Width - 1; x++){
         new_value = 0.0;
         for(i = -1; i < 2; i++){
            for(j = -1; j < 2; j++){
               new_value = new_value + (double)(weight[i + 1][j + 1] * (int)img->r[y + i][x + j]);
            }
            img->r[y][x] = img->g[y][x] = img->b[y][x] = (unsigned char )(new_value/div_const);
         }
      }
   }

   return OK;

}


/* ********************************************
 * 関数名　：img_fft
 * 引数1   ：イメージ構造体
 * 戻り値　：エラーコード
 * 処理概要：イメージに対してフーリエ変換を行う
 * **********************************************/
int
img_fft(img_struct *img){
   
   /* ８ビットグレースケール以外の場合 */
   if (img->BitDepth != 8){
      return ERR_IMG_FFT_BITDEPTH;
   }
   
   double  **data;         /* 実数部用 */
   double **jdata;         /* 虚数部用 */
   
   /* 配列の作成 */
   data =  img_double_malloc_array(img->Height, img->Width);
   jdata = img_double_malloc_array(img->Height, img->Width);
   
   /* メモリの確保に失敗した場合はエラーとする */
   if(data == NULL || jdata == NULL){
      return ERR_IMG_FFT_MEMORY;
   }
   
   /* フーリエ変換用データの作成 */
   img_make_fft_data(*img,data,jdata);

   /* 2次元フーリエ変換 */
   img_fft2(data,jdata,1,img->Width,img->Height);
   
   /* 周波数領域でのフィルタリング */
   img_fft_filter(data, jdata, img->Width, img->Height);
   
   /* 2次元逆フーリエ変換 */
   img_fft2(data,jdata,-1,img->Width,img->Height);
   
   /* 変換後データの保存 */
   for(int y = 0; y < img->Height; y++){
      for(int x = 0; x < img->Width; x++){
         if(data[y][x] < 0){
            data[y][x] = 0;
         }
         if(data[y][x] > MAXBRIGHTNESS){
            data[y][x] = MAXBRIGHTNESS;
         }
         img->r[y][x] = img->g[y][x] = img->b[y][x] = data[y][x];
      }
   }
   
   /* 確保したメモリの解放 */
   free(data);
   free(jdata);
   
   return OK;
   
}

