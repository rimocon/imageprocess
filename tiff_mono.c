#include<stdio.h>
#include<math.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<stdlib.h>
#include<tiff-4.0.3/libtiff/tiffio.h>

#define Isize  512	//取り扱う画像のサイズX
#define Jsize  Isize	//取り扱う画像のサイズY
#define Bnum   4 	//ボタンの数
#define Xsize  Jsize+Right+5	//表示ウィンドウのサイズX
#define Ysize  Isize+5	//表示ウインドウのサイズY
#define Right  100	//表示ウィンドウ内の右側スペースサイズ
#define BS     100  	
#define Fcol   255|255<<8|255<<16	
#define Bcol   1	//ウィンドウの背景色

Display    *d;
Window     Rtw,W,W1,Side,Bt[Bnum];
GC         Gc,GcW1;
Visual     *Vis;
XEvent     Ev;
XImage     *ImageW1;
Font       Ft;
unsigned long Dep;

unsigned char dat[Isize][Jsize];	//取り扱う画像データ格納用
int   buff[Isize*Jsize];
unsigned char   buffer[Isize*Jsize];

//表示画像をTIFF形式で保存する関数
void tiff_save(){
	TIFF *image;
	
	int i,j,k;
	char save_fname[256];

	printf("File name (save) : ");
	scanf("%s",save_fname);
	
	k=0;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			buffer[k]=dat[i][j];
				k++;
		}
	}
	// Open the TIFF file
	if((image = TIFFOpen(save_fname, "w")) == NULL){
		printf("Could not open output file for writing\n");
		exit(42);
	}

	// We need to set some values for basic tags before we can add any data
	TIFFSetField(image, TIFFTAG_IMAGEWIDTH, Isize);	//画像の幅
	TIFFSetField(image, TIFFTAG_IMAGELENGTH, Jsize);	//画像の高さ
	TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 8);	//ピクセルの色深度（ビット数）
	TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);	//ピクセルあたりの色数
	//TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, Jsize);	//数行をひとまとめにして1ストリップと定義している場合

	TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);	//画像の種別(0:mono,1:gray,2:RGB,3:index color,6:YCrCb)
	TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);	//このタグの数値が1（CONTIG）ならばBIP配列、2（SEPARATE）ならBIL配列

	TIFFSetField(image, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);	//原点開始位置を指定
	//TIFFSetField(image, TIFFTAG_XRESOLUTION, 512.0);	//Xにおけるpixels/resolutionを意味する。実寸のサイズと画面上のサイズの比を指定。いわゆるdpi。
	//TIFFSetField(image, TIFFTAG_YRESOLUTION, 512.0);
	//TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER);
  
	// Write the information to the file
	TIFFWriteEncodedStrip(image, 0, buffer, Isize * Jsize);	//画像データをTiff形式のファイルに保存

	// Close the file
	TIFFClose(image);	
}

//画像ファイル読み込み
void read_file()
{
	FILE  *fi; 
	char name[150];
	
	printf("File Name  : ");
	scanf("%s",name);
	if((fi=fopen(name,"r"))==NULL){
		printf("Read Error\n");
		exit(1);
	}
	fread(dat,1,Isize*Jsize,fi);
}

//ウィンドウに画像を表示
void view_img()
{
	int i,j,k;

	k=0;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			buff[k]=dat[i][j]|dat[i][j]<<8|dat[i][j]<<16;
			//buff[k]=dat[i][j];
				k++;
		}
	}
	//画像を表示
	XPutImage(d,W1,GcW1,ImageW1,0,0,0,0,Jsize,Isize);
}

//windowの初期設定
void init_window()
{
	int i;
	
	//windowを開く前の初期設定
	//d=XOpenDisplay(NULL);	
  	// Xサーバとの接続
	if( (d = XOpenDisplay( NULL )) == NULL ) {
		fprintf(stderr, "Cannot Open Display\n");
		exit(1);
	}
	
	// ディスプレイ変数の取得
	Rtw=RootWindow(d,0);	//ルートウィンドウを指定
    Vis=XDefaultVisual(d,0);
    Dep=XDefaultDepth(d,0);
    //ウインドウを作成
    W=XCreateSimpleWindow(d,Rtw,0,0,Xsize,Ysize,2,Fcol,Bcol);	//背景ウィンドウ
    W1=XCreateSimpleWindow(d,W,0,0,Jsize,Isize,2,Fcol,Bcol);	//画像表示用ウィンドウ
    Side=XCreateSimpleWindow(d,W,Jsize,0,Right,Isize,2,Fcol,Bcol);	//サイドウィンドウ
    for(i=0;i<Bnum;i++){
        Bt[i]=XCreateSimpleWindow(d,Side,0,30*i,BS,30,2,Fcol,Bcol);	//ボタン作成
        XSelectInput(d,Bt[i],ExposureMask | ButtonPressMask);	//ウィンドウが表示された時orボタンが押された時にXサーバから通知
    }

    XSelectInput(d,W1,ButtonPressMask);

	//ウィンドウを画面に表示
    XMapWindow(d,W);
    XMapSubwindows(d,W1);
    XMapSubwindows(d,Side);
    XMapSubwindows(d,W);
}

//表示画像の初期設定
void init_image()
{
	//デフォルトのグラフィックスコンテキストを生成
    Gc  = XCreateGC(d,W,0,0);
    GcW1= XCreateGC(d,W1,0,0);
  
	//表示画像の設定
    ImageW1=XCreateImage(d,Vis,Dep,ZPixmap,0,NULL,Jsize,Isize,BitmapPad(d),0);
    ImageW1->data = (char *)buff;
}

//イベント発生用関数
void event_select()
{
     int x,y;
     while(1){
     	//イベント読み込み
     	XNextEvent(d,&Ev);
     	switch(Ev.type){
     		//ウィンドウが表示された場合
            case Expose :
                XSetForeground(d,Gc,Fcol);	//前景色の設定
                XSetBackground(d,Gc,10);	//背景色の設定
                //ボタンへ文字列を描画
                XDrawImageString(d,Bt[0],Gc,28,21,"Load",4);
                XDrawImageString(d,Bt[1],Gc,28,21,"View",4);
                XDrawImageString(d,Bt[2],Gc,28,21,"Save",4);
                XDrawImageString(d,Bt[Bnum-1],Gc,28,21,"Quit",4);
            break;
            //ボタンが押された場合
            case ButtonPress :
                if(Ev.xany.window == Bt[0]){
                   read_file();
                }
                if(Ev.xany.window == Bt[1]){
                   view_img();          
                }
                if(Ev.xany.window == Bt[2]){
                   tiff_save();        
                }
                if(Ev.xany.window == Bt[Bnum-1]){
                   exit(1);
                }
                //押されたピクセルの座標と輝度を表示
                if(Ev.xany.window == W1){
                   x=Ev.xbutton.x; y=Ev.xbutton.y;
                   printf("(%d %d) %d\n",y,x,dat[y][x]);
                }
            break;
		}
	}
} 


main()
{
	init_window();
	init_image();
	event_select();
}



