#include "headers.h"
/*
int setMyPrintLevel()
{  int i;  char* pt;
   pt = (char*) *(int*)0xD0A8;
   if (pt == 0) { return 0;}
   for(i=0; i<=31; i) 	{*(pt + 0x1C + (i<<1)) = 7; *(pt + 0x1D + (i<<1)) = 7; }
}
*/
void MyGlobalStdSet ()
{ int f1 = -1;
  while (f1==-1)  { f1=FIO_CreateFile("A:/STDOUT.TXT");  if (f1==-1) SleepTask(100); }
  ioGlobalStdSet(1,f1);    //ioGlobalStdSet(2,f1);
}

int* hMyTaskMessQue, *hMyFsTask;//, *OrgFsMesQueHnd, *hMyFaceSensorMessQue;
#define DP_PRESSED 				0x01
#define MY_MESS2 				0x02
#define MY_MESS3 				0x03
#define MODE_DIAL 				0x04
#define REQUEST_BUZZER 			0x05
#define FACE_SENSOR_ISO 		0x06
#define INFO_SCREEN 			0x07
#define SAVE_SETTINGS			0x08
#define AF_PATTERN				0x09
#define safety_shift			(*((int*)0x16C30))
#define AEB						(*((int*)0x16B90))
#define av_half_stop			(*((int*)(0x16B60+0xA8)))
#define FaceSensor 				(*(int*)(0xCD38))
#define menu_dialog				(*(int*)(0x4A2C)) //Main menu Dialog opened
#define MeteringMode			(*(int*)(0x16B60+0x4))
#define CurIsoValue				(*(int*)(0x16B60+0x28))
#define CurFlashComp			(*(int*)(0x16B60+0x08))
#define WhiteBalance			(*(int*)(0x16B60+0x10))
#define CurAvComp				(*(int*)(0x16B60+0x24))
#define GUIMode					(*(int*)(0x00001ECC))
#define hInfoCreative			(*(int*)(0x0000213C))
//#define ShootWithoutCard		(*(int*)(0x16B60+0x90))
#define	BUTTON_SET				0xA6
#define	BUTTON_UP				0xB1
#define	BUTTON_DOWN				0xB2
#define	BUTTON_RIGHT			0xB3
#define	BUTTON_LEFT				0xB4
#define	BUTTON_DP				0xB8
#define	BUTTON_AV				0xBB
#define AE_Mode					(*(int*)(0x16B60))
extern void SpotImage(); extern void AfPointExtend(int); extern void MainGUISt();
extern void SetDispIso(); char* my_GUIString();

char i100[5]="100 ", i125[5]="125 ", i160[5]="160 ";
char i200[5]="200", i250[5]="250 ", i320[5]="320 ", i400[5]="400 ", i500[5]="500 " , i640[5]="640 ", i800[5]="800 ";
char i1000[5]="1000", i1250[5]="1250", i1600[5]="1600", i2000[5]="2000", i2500[5]="2500",i3200[5]="3200";
char* iso;
int AFP_Sel;  
int AFP[42]={391, 7, 49, 385, 73, 120, 121, 126, 127, 505, //Center
			 24, 25, 26, 27, 386, 387, 409, 410, 411,  // Top
			 96, 97, 100, 101, 388, 389, 481, 484, 485,  //Bottom
 			 40, 41, 47, 168, 169, 174, 175,   //Left
			 80, 81, 87, 336, 337, 342, 343} ;  //Right
int test, modedial;  int spotmode=3, evalue=0; 
int flag, flag1, test_iso;    int ia=0;
int i=0, option_number = 1;
int  double_key=0, last_option = 10, update=1;
int flash_exp_val, av_comp_val, aeb_val, color_temp;

int iso_in_viewfinder, dp_to_change_iso=1;
int settingsbuff[2];
void ReadSettings()
{	int file = FIO_OpenFile("A:/settings", O_RDONLY, 644);
	if(file!=-1)
	{	FIO_ReadFile(file, (int *)settingsbuff, sizeof(settingsbuff));
		iso_in_viewfinder=settingsbuff[0];
		dp_to_change_iso=settingsbuff[1];
		FIO_CloseFile(file);
	}
}

void WriteSettings()
{	int file = FIO_OpenFile("A:/settings", O_CREAT|O_WRONLY , 644);
	if(file!=-1) {
		settingsbuff[0]= iso_in_viewfinder;
		settingsbuff[1]= dp_to_change_iso;
		FIO_WriteFile(file, settingsbuff, sizeof(settingsbuff));
		FIO_CloseFile(file);
	}
}

void MyTask ()
{	//MyGlobalStdSet(); //Thai Remarked
	int* pMessage ;   int dem; int t;
	ia=*(int*)0xC300;
	int flash_exp_val_temp, av_comp_val_temp;
	SleepTask(1000);
	ReadSettings();
	while (1)
	{	//ChangeDprData(41,1); //this proc enable iso 16-80, 2000-3200
		ReceiveMessageQueue(hMyTaskMessQue,&pMessage,0);
		t=0;
		switch (pMessage[0])
		{
		case MODE_DIAL:   //Test Mode Dial 
			if (AE_Mode<6) //in creative zone
			{	MainGUISt();
			} 	 
			break;
		case REQUEST_BUZZER:
			eventproc_RiseEvent("RequestBuzzer");			
			break;
		case MY_MESS3: //Notify Custom Focus Point Setting enable 	
			if (*(int*)(0x16B60+0x38)==1) eventproc_RiseEvent("RequestBuzzer"); // if set Beep on
			eventproc_EdLedOn(); *(int*)0xC02200A0=0x46;SleepTask(50); eventproc_EdLedOff(); *(int*)0xC02200A0=0x44; 
			//Blueled on,Redled on, sleep, Blueled off,Redled off  
			break;
		case MY_MESS2: //Check double press AutoFocusPoint button   
			AFP_Sel=1;
			SleepTask(700);
			AFP_Sel=0;
			//eventproc_RiseEvent("RequestBuzzer");
			break;
		case DP_PRESSED:
			if(!*(int*)(0x16B60+0xB0))
			{	SendToIntercom(0x31, 1, 1);		// enable CFn.8 for ISO H
				SendToIntercom(0xF0,0,0); SendToIntercom(0xF1,0,0);	//Enable realtime ISO change
			}

			//Spot metering mode
			test=*(int*)(0x47EC) ; //OlMeterMode Dialog opened
			if (test!=0)
			{ 	
				pressButton_(166);   //"Set" button
				eventproc_SetMesMode(&spotmode);  //Spot metering mode
				if (*(int*)(0x16B60+0x38)==1) eventproc_RiseEvent("RequestBuzzer");  // if set Beep On
				eventproc_PrintICUInfo();
				SleepTask(30);
				break;
			} 			
			
			//Debugmode enable--------- Need placed before FactoryMenu mode check 
			test=*(int*)(0x49F4) ; //Factory main Dialog opened	
			if (test!=0)
			{ 	MyGlobalStdSet();   eventproc_RiseEvent("RequestBuzzer"); 
				break; 
			}
	
			//Factory menu enable
			if (menu_dialog!=0)
			{	EnterFactoryMode();  SleepTask(20);  ExitFactoryMode();
				break;
			}

			//extend_iso_hack
			//sub_FF82B518(9); //ISO mode
			if (AE_Mode>=6) break;
			if (dp_to_change_iso)SetDispIso();
			break;
		case FACE_SENSOR_ISO:
			if(double_key^=1){
				test_iso=CurIsoValue;			
				if(pMessage[1])
				{	if (test_iso<0x48) flag1 = 0x48;
					else if (test_iso<0x50) flag1 = 0x50;
					else if (test_iso<0x58) flag1 = 0x58;
					else if (test_iso<0x60) flag1 = 0x60;
					else if (test_iso<0x68) flag1 = 0x68;
					else{ flag1 = 0x6F;}
				}
				else
				{	if (test_iso>0x68) flag1 = 0x68;
					else if (test_iso>0x60) flag1 = 0x60;
					else if (test_iso>0x58) flag1 = 0x58;
					else if (test_iso>0x50) flag1 = 0x50;
					else flag1 = 0x48;
				}
				if(iso_in_viewfinder)
				if (AE_Mode==1 || AE_Mode==3)
				{	test=*(char*)(0x27E48);
					SendToIntercom(0x8,2,flag1+0x25);
				}	
				for (dem=1; dem<11; dem++)
				{	if (*(int*)(0x1C88)!=1) //MAIN Gui idle command
					{	eventproc_SetIsoValue(&flag1);dem=11;
						SleepTask(20);  
					} else; {SleepTask(100);}
				}
			}else if (AE_Mode==1 || AE_Mode==3) { if(iso_in_viewfinder)SendToIntercom(0x8,2,test);}
			break;
		repeat:
		case INFO_SCREEN:
			switch (pMessage[1])
			{
				case BUTTON_AV:
					switch(option_number)
					{	case 1:
						case 2:
							i^=1;update=0;break;
						case 7:
							if(color_temp<2200)color_temp=2200;
							else if(color_temp<3200)color_temp=3200;
							else if(color_temp<4000)color_temp=4000;
							else if(color_temp<5200)color_temp=5200;
							else if(color_temp<6000)color_temp=6000;
							else if(color_temp<7000)color_temp=7000;
							else if(color_temp>=7000)color_temp=2200;
							update=0;
							break;
					}
					break;
				case BUTTON_UP:
					if(option_number==1){option_number=last_option;}
					else{option_number-=1;}
					update=1;
					break;
				case BUTTON_DOWN:
					if(option_number==last_option){option_number=1;}
					else{option_number+=1;}
					update=1;
					break;
				case BUTTON_RIGHT:
					switch(option_number)
					{	case 1:av_comp_val=GetValue(av_comp_val,1);break;
						case 2:flash_exp_val=GetValue(flash_exp_val,1);break;
						case 3:aeb_val=GetValue(aeb_val,1); break;
						case 4:if (safety_shift==0) SendToIntercom(0x39,1,1);break;
						case 6:iso_in_viewfinder=1;WriteSettings();break;
						case 7:color_temp+=100;if (color_temp>11000)color_temp=1800;break;
						case 8:SendToIntercom(0x30,1,0);break;
						case 9:SendToIntercom(0x2E,1,0);break;
						case 10:dp_to_change_iso=1;WriteSettings();break;
					}
					update=0;
					break;
				case BUTTON_LEFT:			
					switch(option_number)
					{	case 1:av_comp_val=GetValue(av_comp_val,0);break;
						case 2:flash_exp_val=GetValue(flash_exp_val,0);break;
						case 3:aeb_val=GetValue(aeb_val,0); break;
						case 4:if (safety_shift==1) SendToIntercom(0x39,1,0);break;
						case 6:iso_in_viewfinder=0;WriteSettings();break;
						case 7:color_temp-=100;if (color_temp<1800)color_temp=11000;break;
						case 8:SendToIntercom(0x30,1,1);break;
						case 9:SendToIntercom(0x2E,1,1);break;
						case 10:dp_to_change_iso=0;WriteSettings();break;
					}
					update=0;
					break;
			}
			sub_FF837FA8(hInfoCreative,0x11,my_GUIString());
			do_some_with_dialog(hInfoCreative);
			switch (pMessage[1])
			{	case BUTTON_UP:
				case BUTTON_DOWN:
					for (t=0;t<5; t++){ SleepTask(100); if (!double_key) break;}
					if (double_key) goto repeat;break;
				case BUTTON_RIGHT:
				case BUTTON_LEFT:
					switch(option_number)
					{	case 1:
						case 2:
						case 3:
						case 7:
							for (;t<2; t++){ SleepTask(150); if (!double_key) break;}
							if (double_key)SleepTask(150);
							if (double_key) goto repeat;
					}
			}
			break;
		case SAVE_SETTINGS:
			switch(option_number)
			{	case 1:
					if(i)av_comp_val_temp=0-av_comp_val;else av_comp_val_temp=av_comp_val;
					SendToIntercom(0xA,1,av_comp_val_temp);
					break;
				case 2:
					if(i)flash_exp_val_temp=0-flash_exp_val;else flash_exp_val_temp=flash_exp_val;
					SendToIntercom(0x03,1,flash_exp_val_temp);
					break;
				case 3:SendToIntercom(0xd,1,aeb_val); break;
				case 7:
					SendToIntercom(0x10,2,color_temp);
					if (WhiteBalance!=0x08){SendToIntercom(0x5,1,0x08);}
					break;
			}
			break;
		case AF_PATTERN:
			AfPointExtend(pMessage[1]);
			break;
		}
	} 
}

void DispIso( )
{//	if (CurIsoValue!=0x48 && CurIsoValue!=0x50 && CurIsoValue!=0x58 && CurIsoValue!=0x60 && CurIsoValue!=0x68){
	switch(CurIsoValue) // Set ISO String
	{
		case 0x6F: iso=i3200;break; //3200	 
		case 0x6D: iso=i2500;break; //2500	 
		case 0x6C: iso=i2000;break; //2000	 
		case 0x68: iso=i1600;break; //1600	 
		case 0x66: iso=i1250;break; //1250
		case 0x64: iso=i1000;break; //1000
		case 0x60: iso=i800;break;  //800
		case 0x5D: iso=i640;break;  //640 
		case 0x5C: iso=i500;break;  //500 
		case 0x58: iso=i400;break;  //400 
		case 0x56: iso=i320;break;  //320 
		case 0x53: iso=i250;break;  //250
		case 0x50: iso=i200;break;  //200
		case 0x4E: iso=i160;break;  //160
		case 0x4C: iso=i125;break;  //125
		case 0x48: iso=i100;break;  //100
	}
	sub_FF837FA8(*(int*)(0x47F0),0x04,iso );
	do_some_with_dialog(*(int*)(0x47F0));
} 

void SpotImage( )
{	sub_FF8382DC(*(int*)(0x47F0),0xD,246);
}

void KImage( )
{	sub_FF8382DC(*(int*)(0x47F0),0xC,207);
}

FlashCompIm()
{	int flash_exp_val=CurFlashComp;
	int i=0, s;
	if (flash_exp_val>0x30)
	{	flash_exp_val=0x100-flash_exp_val;
		i=1;
	}
	switch (flash_exp_val)
	{	case 0x13:s=1;break;
		case 0x14:s=0;break;
		case 0x15:s=2;break;
		case 0x18:s=3;break;
		case 0x1B:s=5;break;
		case 0x1C:s=4;break;
		case 0x1D:s=6;break;
		case 0x20:s=7;break;
		case 0x23:s=9;break;
		case 0x24:s=8;break;
		case 0x25:s=10;break;
		case 0x28:s=11;break;
		case 0x2B:s=13;break;
		case 0x2C:s=12;break;
		case 0x2D:s=14;break;
		case 0x30:s=15;break;
	}
	if (i)s+=130;
	else s+=154;
	sub_FF8382DC(*(int*)(0x47F0),0xB,s);
}

void AfPointExtend(button)
{ 	int dem, currAFset;
	currAFset=*(int*)(0x16B60+0x18);
	for(dem=0;dem<41;dem++) { if(currAFset==AFP[dem])  break; }
	switch (button)
	{	case BUTTON_SET:
			if (dem>=9)dem=0;
			else dem++;
			break;
		case BUTTON_UP:
			if (dem<10 || dem>=18)dem=10;
			else dem++;
			break;
		case BUTTON_DOWN:
			if (dem<19 || dem>=27)dem=19;
			else dem++;
			break;
		case BUTTON_LEFT:
			if (dem<28 || dem>=34)dem=28;
			else dem++;
			break;
		case BUTTON_RIGHT:
			if (dem<35 || dem>=41)dem=35;
			else dem++;
			break;
	}
		SendToIntercom(7,2,AFP[dem]);
	//SendToIntercom(7,2,ia); ia++; //For test 512 pattern (2 bytes variable ia)
}
 
void SetDispIso( )
{	switch(CurIsoValue)
	{	case 0x6F: flag1=0x68; break;// 3200-> 1600	 
		case 0x6D: flag1=0x6F; break;// 2500-> 3200	 
		case 0x6C: flag1=0x6D; break;// 2000-> 2500	 
		case 0x68: flag1=0x6C; break;// 1600-> 2000
		case 0x66: flag1=0x60; break;// 1250-> 800
		case 0x64: flag1=0x66; break;// 1000-> 1250
		case 0x60: flag1=0x64; break;// 800 -> 1000
		case 0x5D: flag1=0x58; break;// 640 -> 400 
		case 0x5C: flag1=0x5D; break;// 500 -> 640
		case 0x58: flag1=0x5C; break;// 400 -> 500
		case 0x56: flag1=0x50; break;// 320 -> 200
		case 0x53: flag1=0x56; break;// 250 -> 320
		case 0x50: flag1=0x53; break;// 200 -> 250
		case 0x4E: flag1=0x48; break;// 160 -> 125 
		case 0x4C: flag1=0x4E; break;// 125 -> 160
		case 0x48: flag1=0x4C; break;// 100 -> 125
	}
	eventproc_SetIsoValue(&flag1);
	SleepTask(10);
	DispIso();
} 

void SetDispIso2 ( )
{		if (test_iso>0x68) {flag1=0x68;}
		else if (test_iso>0x60) {flag1=0x60;}
		else if (test_iso>0x58) {flag1=0x58;}
		else if (test_iso>0x50) {flag1=0x50;}
		else {flag1=0x48;}
		eventproc_SetIsoValue(&flag1);
}

extern void MainGUISt()
{	if (AE_Mode<6)if (hMyFsTask!=0) UnSuspendTask(hMyFsTask);
}
	
void MyFSTask()
{	while (1)
	{	if (MeteringMode==3)SpotImage();
		if (WhiteBalance==8)KImage();
		if (CurFlashComp>0x10 && CurFlashComp<0xF0)FlashCompIm();
		DispIso();
		do_some_with_dialog(*(int*)(0x47F0));
		update=1;
		SuspendTask(hMyFsTask);
	}
}

void SetEvaluativeDefault()
{ 	if ( MeteringMode==3 )  // Spot is actived 	
	{ 	eventproc_SetMesMode(&evalue); }
} 

void CreateMyTask()
{
	hMyTaskMessQue=(int*)CreateMessageQueue("MyTaskMessQue",0x40);
	CreateTask("MyTask", 0x19, 0x2000, MyTask,0);
	hMyFsTask=(int*)CreateTask("MyFSTask", 0x1A, 0x2000, MyFSTask,0);
}

void SendMyMessage(int param0, int param1)
{	int* pMessage=(int*)MainHeapAlloc(8);
	pMessage[0]=param0;  pMessage[1]=param1;
	TryPostMessageQueue(hMyTaskMessQue,pMessage,0);
}

int GetValue(int temp, int button)
{	if (temp==0 && button==0 && option_number!=3)i=1;
	if(i)button^=1;
	switch (button)
	{	case 0:
			if(av_half_stop==1)temp -= 4;
			else{ if((temp&5)==5)temp -= 2; else temp -= 3;}
			if(temp<0) temp=0;
			break;
		case 1:
			if(av_half_stop==1)temp += 4;
			else{ if((temp&3)==3)temp += 2; else temp += 3;}
			if(temp>0x30) temp=0x30;
			break;
	}
	if(temp==0 && i==1)i=0;
	return temp;
}

int one = 0, two = 0;
void HexToStr(int hex)
{			one = 0; two = 0;
			switch(hex&0xf0){
				case 0x10:
					one=2;
					break;
				case 0x20:
					one=4;
					break;
				case 0x30:
					one=6;
			}
			if((hex&0x08)==8)
				one++;
			switch(hex&0x07){
				case 3:
					two=3;
					break;
				case 4:
					two=5;
					break;
				case 5:
					two=7;
			}
}

char buff[17];
char* my_GUIString(){
	SleepTask(40);
	char sign[2] = {'+', '-'};
	switch(option_number){
		case 1:
			if (update)
			{	av_comp_val=CurAvComp;
				if (av_comp_val>0x30)
				{	av_comp_val=0x100-av_comp_val;
					i=1;
				}else i=0;
			}
			HexToStr(av_comp_val);
			sprintf(buff,"Av comp:         %c %u.%u",sign[i],one,two);
			return buff;
		case 2:
			if (update)
			{	flash_exp_val=CurFlashComp;
				if (flash_exp_val>0x30)
				{	flash_exp_val=0x100-flash_exp_val;
					i=1;
				}else i=0;
			}
			HexToStr(flash_exp_val);
			sprintf(buff,"Flash exp comp:  %c %u.%u",sign[i],one,two);
			return buff;
		case 3:
			if (update)aeb_val=AEB;i=0;
			HexToStr(aeb_val);
			sprintf(buff,"AEB:        +-%u.%u",one,two); return buff; break;
		case 4:
			if (safety_shift==0) return "Safety Shift:   Off";
			else return "Safety Shift:   On"; break;
		case 5:
			sprintf(buff,"Release Count: %u",*(int*)(0xEBFC));
			return buff;
		case 6:
			if (iso_in_viewfinder)return "Show ISO in Viewfinder: On";
			return "Show ISO in Viewfinder: Off";
		case 7:
			if (update)color_temp=*(int*)(0x16B9C);
			sprintf(buff,"Color Temperature: %uK",color_temp);
			return buff;
		case 8:
			if (*(int*)(0x16C0C))return "Flash:            Off";
			else return "Flash:            On";
		case 9:
			if (*(int*)(0x16C04))return "AF Assist Beam:       Off";
			else return "AF Assist Beam:       On";
		case 10:
			if (dp_to_change_iso)return "DP for changing ISO: On";
			return "DP for changing ISO: Off";
	}
}

int test3, test4;
void my_IntercomHandler (int r0, char* ptr)
{   
/*
	char s[255]; int i;
	// Write Log
	s[0]=0;
    for (i=0;  i<ptr[0];  i++)   {sprintf(s+i*2,"%02X",ptr[i]);}
    printf_log(8,8,"[!] sz: %02X, code: %2X, hex: %s",ptr[0],ptr[1],s);
	if(ptr[1]>0x50 && ptr[1]<0x95) {SendMyMessage(REQUEST_BUZZER,0);}  // Test some key
//*/
	
	switch (ptr[1])
	{   case BUTTON_DP:
			if(AE_Mode>5){SendToIntercom(0x22,1,*(int*)(0x16B60+0x74)^3);break;} //Switch to RAW or JPG in auto mode
			if(GUIMode==4){SendMyMessage(SAVE_SETTINGS,0);return;}
			if(GUIMode!=6)SendMyMessage(DP_PRESSED,0);break;  // Press Dp to set Iso
			
	//	case 0x90:
	//	case 0x91:
	//	case 0x92:
		case 0x93:
			SendMyMessage(MODE_DIAL,0);//Iso at switch on & roll dial
			break;
		case 0x50: 
			if(*(int*)(0x4804)!=0) {ptr[1]=0x51; IntercomHandler(r0, ptr); ptr[1]=0x50;}break;// AFP pattern
		case 0xB9: SendMyMessage(MY_MESS2,0);break; //Auto focus point selection dialog on
		case 0xA7:  //Auto focus point selection dialog off and custom on 
			if(AFP_Sel==1) {/*IntercomHandler(r0, ptr);*/ ptr[1]=0xB9; SendMyMessage(MY_MESS3,0);}break;
		case BUTTON_AV:if(GUIMode==4)if(double_key^=1){SendMyMessage(INFO_SCREEN,ptr[1]);return;}break;
		case BUTTON_SET:
			if(GUIMode==0x10){SendMyMessage(AF_PATTERN,ptr[1]);return;}
			if(GUIMode==4){SendMyMessage(SAVE_SETTINGS,0);return;}break;
		case BUTTON_UP:
			if(GUIMode==4)if(double_key^=1){SendMyMessage(INFO_SCREEN,ptr[1]);return;}
			if(GUIMode==0x11 || GUIMode==0)
			{	test_iso=CurIsoValue;
				if (test_iso!=0x48 && test_iso!=0x50 && test_iso!=0x58 && test_iso!=0x60 && test_iso!=0x68)
				{	SetDispIso2();break;}  //Change ISO value when use default camera feature.
			}			
			if(GUIMode==0x10)if(double_key^=1)SendMyMessage(AF_PATTERN,ptr[1]);
			break;
		case BUTTON_DOWN: 
			if(GUIMode==4)if(double_key^=1){SendMyMessage(INFO_SCREEN,ptr[1]);return;}
			if(GUIMode==0x11 || GUIMode==0)if (WhiteBalance==0x08){SendToIntercom(0x5,1,0x00);break;}
			if(GUIMode==0x10)if(double_key^=1)SendMyMessage(AF_PATTERN,ptr[1]);
			break;
		case BUTTON_RIGHT:
			if (FaceSensor){SendMyMessage(FACE_SENSOR_ISO,1);return;}
			if(GUIMode==4)if(double_key^=1){SendMyMessage(INFO_SCREEN,ptr[1]);return;}
			if(GUIMode==0x10)if(double_key^=1)SendMyMessage(AF_PATTERN,ptr[1]);
			break;
		case BUTTON_LEFT:
			if (FaceSensor){SendMyMessage(FACE_SENSOR_ISO,0);return;}
			else if(GUIMode==0x11 || GUIMode==0){SetEvaluativeDefault();break;}//Set Evaluative when "Active Meter Mode is Spot"
			if(GUIMode==4)if(double_key^=1){SendMyMessage(INFO_SCREEN,ptr[1]);return;}
			if(GUIMode==0x10)if(double_key^=1)SendMyMessage(AF_PATTERN,ptr[1]);
			break;
	}
	IntercomHandler(r0, ptr);
}



//--------------------------------
//Button Value
//160 (0xA0) Menu
//0xA5, A1: Trash, Jump  // not work
//164 (A4) Play
//166 (A6)Set butt
//B9: Disp. ? or Focus Frame
//BA: Drive Mode
//0xBB AV +- button
//0xB3->B4:   L, R;   //0xB1=UP  0xB2=Dn  
//0x90->0x93 Mode Dial Tv Av M aDep

//---------------0x16B60 store value same as 0x25E20---------------------
//AeValue address 0x16B60
	//0: P
	//1:Tv
	//2:Av
	//3:M
	//4:unknow1
	//5:A-DEP
	//8:Full Auto (Green rectangular) 
//MesureValue address 0x16B60+4
	//3 Spot 
	//0 Evaluative
//Drive mode address 0x16B60+0xC
	//0 Single shooting
	//1 Continuous shooting
//AvComp Value address 0x16B60+0x24
//IsoValue address 0x16B60+0x28

//-----------------------==SetPropertie
//SendToIntercom(0x1,1,1); //(0x0,1,2);  Zonedial mode P TV AV....
//SendToIntercom(0x2,1,1); //(0x2,1,0);  Meter mode Eval, Center... 
//SendToIntercom(0x3,1,1); //(0x3,1,0);  Flash ex comp 
//SendToIntercom(0x4,1,1); //(0x4,1,0);  drive mode 
//SendToIntercom(0x5,1,1); //(0x5,1,0);  WB chose 
//SendToIntercom(0x6,1,1); //(0x6,1,0);  MF manual focus
//SendToIntercom(0x7,1,1); // AF point selecttion: can extend some special: 7, 27, 41,47, 49, 73, 81,87, 97, 101, 105,113, 120,121, 116, 127, 135,
//					139,165,168,169, 175, 185,   
//SendToIntercom(0x8,2,1); //Tv value:
//SendToIntercom(0xA,1,1); //AV comp ex selecttion: can extend some special  
//SendToIntercom(0xB,1,1); // ISO set 
//SendToIntercom(0xC,1,1); // Red eye 
//SendToIntercom(0xD,1,1); //AEB BRK  
//SendToIntercom(0xE,1,1); //WB BRK 
//SendToIntercom(0xF,1,1); // Beep
//SendToIntercom(22,1,1); // LCDBrightness 
//SendToIntercom(34,1,1); // RAW only, L+RAW, L only
//SendToIntercom(35,1,1); // S,M,L. Combine no.34 to M+RAW OK, S+RAW not ok
//SendToIntercom(36,1,1); // JPG Fine or Medium quality.
//SendToIntercom(43,1,1); // wb+-
//SendToIntercom(44~~>49,1,1); // Cf1-->6
//SendToIntercom(54,1,1); // Cf7
//SendToIntercom(56,1,1); // Cf8
//SendToIntercom(0x3C,1,0); //(0x3C,1,1); Cfn10 
//SendToIntercom(0x3B,1,1); //(0x3B,1,0); Cfn11 
//SendToIntercom(0x39,1,1); //(0x39,1,0);  Cfn9 
//SendToIntercom(0x59,1,1); // Clear camera seting  
//SendToIntercom(0x61,1,1); // Start update firmware   
//SendToIntercom(0x6B,1,1); // Full CF viewfinder message
//SendToIntercom(0x6C,1,1); // Change Battery dialog
//SendToIntercom(0x6D,1,1); // Burst counter set 


//-------------------------Unknown_1CF0
//1CF0: AeMode
//1CF2: MesMode
//1CF6: DriveMode
//1CFA: AfMode
//1CFE: TvValue
//1D00: AvValue  
//1D02: AvCompValue 
//1D04: IsoValue


//--------------------------
//6D58:   =1 Display on, =0 disp off
