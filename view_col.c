#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>

#define Isize  512	//取り扱う画像のサイズX
#define Jsize  Isize	//取り扱う画像のサイズY
#define Bnum   3	//ボタンの数
#define Xsize  Jsize+Right+5	//表示ウィンドウのサイズX
#define Ysize  Isize+5	//表示ウインドウのサイズY
#define Right  100	//表示ウィンドウ内の右側スペースサイズ
#define BS     100
#define Fcol   200|200<<8|200<<16
#define Bcol   1	//ウィンドウの背景色

Display    *d;
Window     Rtw,W,W1,Side,Bt[Bnum];
GC         Gc,GcW1;
Colormap   Cmap;
Visual     *Vis;
XEvent     Ev;
XImage     *ImageW1;
Font       Ft;
unsigned long Dep;

void init_window(),init_color(),init_image(),event_select();

//取り扱う画像データ格納用配列RGB
unsigned char   datR[Isize][Jsize];
unsigned char   datG[Isize][Jsize];
unsigned char   datB[Isize][Jsize];
int   buff[Isize*Jsize];

main()
{
	init_window();
	init_image();
	event_select();
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
	fread(datR,1,Isize*Jsize,fi);
	fread(datG,1,Isize*Jsize,fi);
	fread(datB,1,Isize*Jsize,fi);
}

//ウィンドウに画像を表示
void view_img()
{
	int i,j,k;

	k=0;
	for(i=0;i<Isize;i++){
		for(j=0;j<Jsize;j++){
			buff[k]=datB[i][j]|datG[i][j]<<8|datR[i][j]<<16;
				k++;
		}
	}
	
	XPutImage(d,W1,GcW1,ImageW1,0,0,0,0,Jsize,Isize);
}

//windowの初期設定
void init_window()
{
	int i;

	//windowを開く前の初期設定
	//Xサーバとの接続
	if((d = XOpenDisplay( NULL )) == NULL ){
		fprintf(stderr,"Ｘサーバに接続できません\n");
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
				XDrawImageString(d,Bt[2],Gc,28,21,"Quit",4);	
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
				exit(1);
				}
				//押されたピクセルの座標と輝度を表示
				if(Ev.xany.window == W1){
					x=Ev.xbutton.x; y=Ev.xbutton.y;
					printf("(%d %d) %d\n",y,x,datR[y][x]);
				}
			break;
		}
	}
}
