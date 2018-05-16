#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>

#define Isize  512	//��舵���摜�̃T�C�YX
#define Jsize  Isize	//��舵���摜�̃T�C�YY
#define Bnum   3	//�{�^���̐�
#define Xsize  Jsize+Right+5	//�\���E�B���h�E�̃T�C�YX
#define Ysize  Isize+5	//�\���E�C���h�E�̃T�C�YY
#define Right  100	//�\���E�B���h�E���̉E���X�y�[�X�T�C�Y
#define BS     100
#define Fcol   200|200<<8|200<<16
#define Bcol   1	//�E�B���h�E�̔w�i�F

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

//��舵���摜�f�[�^�i�[�p�z��RGB
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

//�摜�t�@�C���ǂݍ���
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

//�E�B���h�E�ɉ摜��\��
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

//window�̏����ݒ�
void init_window()
{
	int i;

	//window���J���O�̏����ݒ�
	//X�T�[�o�Ƃ̐ڑ�
	if((d = XOpenDisplay( NULL )) == NULL ){
		fprintf(stderr,"�w�T�[�o�ɐڑ��ł��܂���\n");
		exit(1);
	}
	
	// �f�B�X�v���C�ϐ��̎擾
	Rtw=RootWindow(d,0);	//���[�g�E�B���h�E���w��
	Vis=XDefaultVisual(d,0);
	Dep=XDefaultDepth(d,0);

	//�E�C���h�E���쐬
	W=XCreateSimpleWindow(d,Rtw,0,0,Xsize,Ysize,2,Fcol,Bcol);	//�w�i�E�B���h�E
	W1=XCreateSimpleWindow(d,W,0,0,Jsize,Isize,2,Fcol,Bcol);	//�摜�\���p�E�B���h�E
	Side=XCreateSimpleWindow(d,W,Jsize,0,Right,Isize,2,Fcol,Bcol);	//�T�C�h�E�B���h�E
	for(i=0;i<Bnum;i++){
		Bt[i]=XCreateSimpleWindow(d,Side,0,30*i,BS,30,2,Fcol,Bcol);	//�{�^���쐬
		XSelectInput(d,Bt[i],ExposureMask | ButtonPressMask);	//�E�B���h�E���\�����ꂽ��or�{�^���������ꂽ����X�T�[�o����ʒm
	}
	XSelectInput(d,W1,ButtonPressMask);

	//�E�B���h�E����ʂɕ\��
	XMapWindow(d,W);
	XMapSubwindows(d,W1);
	XMapSubwindows(d,Side);
	XMapSubwindows(d,W);
}

//�\���摜�̏����ݒ�
void init_image()
{
	//�f�t�H���g�̃O���t�B�b�N�X�R���e�L�X�g�𐶐�
	Gc  = XCreateGC(d,W,0,0);
	GcW1= XCreateGC(d,W1,0,0);

	//�\���摜�̐ݒ�
	ImageW1=XCreateImage(d,Vis,Dep,ZPixmap,0,NULL,Jsize,Isize,BitmapPad(d),0);
	ImageW1->data = (char *)buff;
}

//�C�x���g�����p�֐�
void event_select()
{
	int x,y;
	while(1){
		//�C�x���g�ǂݍ���
		XNextEvent(d,&Ev);
		switch(Ev.type){
			//�E�B���h�E���\�����ꂽ�ꍇ
			case Expose :
				XSetForeground(d,Gc,Fcol);	//�O�i�F�̐ݒ�
				XSetBackground(d,Gc,10);	//�w�i�F�̐ݒ�
				//�{�^���֕������`��
				XDrawImageString(d,Bt[0],Gc,28,21,"Load",4);	
				XDrawImageString(d,Bt[1],Gc,28,21,"View",4);	
				XDrawImageString(d,Bt[2],Gc,28,21,"Quit",4);	
			break;
			//�{�^���������ꂽ�ꍇ
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
				//�����ꂽ�s�N�Z���̍��W�ƋP�x��\��
				if(Ev.xany.window == W1){
					x=Ev.xbutton.x; y=Ev.xbutton.y;
					printf("(%d %d) %d\n",y,x,datR[y][x]);
				}
			break;
		}
	}
}
