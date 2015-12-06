//
#ifndef structuresH
#define structuresH
//------------
#define KURS_DK
#include "../types_define.h"

#pragma pack (1)

 //
 #define DK_N              4    // ���-�� ��
 #define MaxWeeksPlans     16  //����� ��������� ������
 #define MaxDaysPlans      16  //����� �������� ������
 #define MaxCalendar       32  //����� ��������� ��� �����

 #define MaxProgFase       32   //������������ ����� ��� � ���������
 //#define MaxProgFase     8   //������������ ����� ��� � ���������

 #define MaxTVP            2   //������������ ����� ���
 #define MaxDirects        32  //������������ ����� �����������
 //#define MaxDirects      8   //������������ ����� �����������
 #define MaxProgram        32  //������������ ����� ��������
 //#define MaxProgram      8   //������������ ����� ��������
 #define MaxFase           32  //������������ ����� ���
  //
 #define MaxMULBlocks      8  //������������ ����� ������
 #define MaxChannels       3  //����� ������� � ����

 #define WeeksInYear       54
 #define MaxLightSlots     12  //����� ������ ������� � ������� �����
 #define MaxLightPlans     12  //����� ������ �������

 #define MaxTramvays       4    //����� ���������� ����������
 ////  {���� 512, 12 ���� �� ������}
 #define MaxTextLen        512    //������������ ���������� �����������
 #define MaxTextLen_Ex     128
 //#define TextAdd         12     // ���������� ������

 #define TaktsN            4        // ����� ������ � ����. ����
 #define TaktsALL          5        // ����� ������ � ����

 //////
 // ����-���������
 #define NO_PROG           -1
 #define SPEC_PROG_NO      0   //�� ����-���������
 #define SPEC_PROG_YF      1   //��
 #define SPEC_PROG_OC      2   //OC
 #define SPEC_PROG_KK      3   //KK
 //////////////
 // ��-�� ���
 #define FAZA_PROP_FIKS    0   //������������� ����
 #define FAZA_PROP_COMB    1   //���������������
 #define FAZA_PROP_TVP     2   //�������� ���


/////////

 ////////////////
 // ���������
 //typedef unsigned char BYTE;
 //typedef unsigned short WORD;
 //typedef unsigned long DWORD;

 // ������������
#define P_ZD1     0
#define P_ZD2     1
#define P_KD      2

// ��������� �����������
typedef enum __STATES_LIGHT
{
	ALL_OFF       = 0x00,   // ��� ���������
	GREEN         = 0x01,	// ������
	GREEN_FLASH   = 0x02,	// ������ ��������
	RED           = 0x03,   // �������
	RED_YELLOW    = 0x04,   // �������-�����
	YELLOW        = 0x05,   // Ƹ����
	YELLOW_FLASH  = 0x06,	// Ƹ���� ��������
	RESERVED
} STATES_LIGHT;
 ////////////////

 typedef struct  _TPROMTAKT
 {
       BYTE Zd1;     // ����� ���������� �����
       BYTE Zd2;
       BYTE KD;
 }TPROMTAKT;
 ////  ������ ����
 #define SHABL_FAZA_DIR_RED     0
 #define SHABL_FAZA_DIR_GREEN   1
 ////

 //
 typedef struct _TSHABLONFASA
 {
    BYTE  Direct[MaxDirects];  //���� ��������� ����������� � ����
                               //0x01-��������  0x0 -���������  (������� ����)


 }TSHABLONFASA;
 // ��������� ���
 typedef struct _TVPSOURCE
 {
   //BYTE      addr; //����� ������������ ������� 0..11
   BYTE        pres; //0 - ��� ���, 1,2 - ������\������ ������
 }TVPSOURCE;
 // ��������� ���� � ���������
 typedef struct _TFASA
 {
   BYTE      FasaProperty;        //  �������� ����
   WORD      Tosn;                //  �������� �����
   WORD      Tmin;                //  ����������� �����
   BYTE      NumFasa;             //  ����� ��������� ����
   TPROMTAKT prom[MaxDirects];    // ���� ����� ��� ������� ����������
   //BYTE      TVP1;               // ���1 - ����� ������������ ������� 0..11
  // BYTE      TVP1_num;           // ����� ���
                                // 0 - ��� ���, 1,2 - ������\������ ������
   //BYTE      TVP2;               // ���2
   //BYTE      TVP2_num;           //
   TVPSOURCE tvps[MaxTVP];

 }TFASA;
 // FasaProperty
 //������������� -  0;
 //���������������, 0x01
 //��� - 0x02


// ��������� ��������� �������� �� ������� �����
typedef struct _TCALENDARTIME
{
	BYTE   NumProgramWorks;  //  � ���������   1-32
        BYTE   SpecProg;         // ���� � ����� ����-��������
	DWORD  BeginTimeWorks;   //  ����� ������ ������ � �������� �� 00:00:00
} TCALENDARTIME;

typedef struct
 {
   BYTE            AmountCalendTime;          //���-�� ��������
   TCALENDARTIME   CalendTime[ MaxCalendar ]; // ��������� �������� ���������� ���������
} TCALENDAR;

typedef struct
 {
   BYTE     Calendar[7 ]; // ������(�������) �������� ������ �� ������ DaysPlans
                          // 0 - ��, 6 - ��
}TWEEKCALENDAR;
///
typedef struct
 {
   BYTE     YearCalendar[WeeksInYear]; // ������(�������) ��������� ������ �� ������ WeeksCalendars
}TYEARCALENDAR;
///////////////////////////////////////////////
typedef struct
{
   BYTE   AmountFasa; 		   //���������� ��� � ���������
   TFASA  fazas[MaxProgFase];
}TPROGRAM;

typedef struct
{
    TPROGRAM             Program[ MaxProgram ];          // ����� ��������
    DWORD                ProgramsSize;	        // ������ ��������
    unsigned long        CRC32;
} TPROGRAMS;
//������ - ���������� ������
//MaxMULBlocks= ����� ���������� �����

typedef enum {
	MUL_RED       = 0x00,   // �������
        MUL_YELLOW    = 0x01,	// ������
	MUL_GREEN     = 0x02,	// ������
	MUL_COLOR_COUNT
} MUL_COLOR;
 ////////////////

// ���� �����
// ����� - 0-�������, 1 - �������, 2 - ������
typedef struct
{
   BYTE  napr;  // 0 - ��� ��������, 1 - ������ �����������, ....
   MUL_COLOR  color;
}TONECHANNEL;

 typedef  struct _TONEMUL
  {
     TONECHANNEL        red;
     TONECHANNEL        yel;
     TONECHANNEL        green;
  } TONEMUL;
  //
  typedef  struct _TONETRAMVAY
  {
     TONECHANNEL        left;
     TONECHANNEL        right;
     TONECHANNEL        center;
  } TONETRAMVAY;
  //

  typedef struct _TMULS
  {
   TONEMUL        OneMUL[MaxMULBlocks];
   TONETRAMVAY    OneTRAMVAY[MaxTramvays];

  }TMULS;
//�����������������������
//�����������
//��� ����������� ������=0 ��������=1  �������=2 �����������=3

#define DIRECT_TRANSP   0
#define DIRECT_PESH     1
#define DIRECT_ARROW    2
#define DIRECT_REV      3
#define DIRECT_TRAM_L   4
#define DIRECT_TRAM_R   5
#define DIRECT_TRAM_F   6

 typedef enum {
	DIR_TRANSP       = 0x00,   // �������
	DIR_PESH     = 0x01,	// ������
	DIR_ARROW    = 0x02,	// ������
	DIR_REV   =  0x03,   // ����� ���� �����������
	DIR_TRAM_L  = 0x04,   // ������
	DIR_TRAM_R  = 0x05,   // �����������..
	DIR_TRAM_F   = 0x06,
        DIR_COUNT
} DIR_TYPE;
 ////
 // �������� - � 2-� ������
 typedef struct _TNAPROUT
 {
   BYTE       group; //����� ������� ������  0 - ��� ��������, 1 - ������ ������
   MUL_COLOR  color;// ����� ������ ������ 0,1,2
   BYTE       control; //���� ���������� ��������
   ///
   BYTE       group2; //����� ������� ������  0 - ��� ��������, 1 - ������ ������
   MUL_COLOR  color2;// ����� ������ ������ 0,1,2
   BYTE       control2; //���� ���������� ��������
   ///


 } TNAPROUT;
 ////
 typedef struct _TNAPROUTALL
 {
   TNAPROUT  red;   // ������� ������ �����������
   TNAPROUT  yel;
   TNAPROUT  green;
 } TNAPROUTALL;
 ////



  typedef struct _TONEDIRECT
  {
    DIR_TYPE Type;
    TNAPROUTALL out;
  }TONEDIRECT;


 typedef struct _TDIRECTS
 {
   TONEDIRECT    OneDirect  [ MaxDirects ];   // MaxDirects=32
   TNAPROUTALL   TramStop[MaxTramvays]; //�������� ������� ������� ������������
 } TDIRECTS;
 ///
 typedef struct _TGUARD
 {
        // >0 - ����� ����� �������� ����������� �����������
        BYTE  ConfMatrix[MaxDirects][MaxDirects]; //
        BYTE  ConfMatrixFlag[MaxDirects][MaxDirects]; //  �����
        //
        BYTE  red_min;   // �����������
        BYTE  yellow;
        BYTE  redyellow;
        BYTE  greenflash;
        BYTE  green_min;
        WORD  faza_max;
        BYTE  green_fill;  //���� ���������� ����. ������ 0-�������, 1 - �������
        ///
        //BYTE  porog_green;
        //BYTE  porog_red;
        //BYTE  alarm_off;          //���� ����������� ������
        BYTE   restart_interval;   //  �������� �����������
        BYTE   restarts;            // ���-�� ������� �����������
        BYTE   time_clear;         // ����� ������� ������
        BYTE   green_flash_tramv_off;   // ���� ���������� ������� �����������
        //
        BYTE   kk_len;   //�� ������������

 }TGUARD;
///////////////////////////////////////////////
   typedef struct _TJORNAL
   {
     BYTE  power_on;
     BYTE  power_off;
     BYTE  inputs;
     signed char  gmt;
     BYTE  faz;
     BYTE  alarm;

   } TJORNAL;

///////////////////////////////////////////////
   typedef struct _TCOMMENT_INNER2
   {
       BYTE     rings;        // ���-�� ����� � �������
       BYTE     synhro_mode;  //���� ����������� ������
       WORD     syhhro_add;   // ���������� �����

   }TCOMMENT_INNER2;
   //////////
   typedef struct _TCOMMENT_INNER
   {
       WORD     preambule; //0xABBA
       //////////////////
       TCOMMENT_INNER2   in;

   }TCOMMENT_INNER;
////////////
   typedef struct _TCOMMENT_EX
   {
     unsigned char        comment[MaxTextLen_Ex];
     TCOMMENT_INNER       inner;  //��������� ���������
     unsigned char        comm_add[MaxTextLen-sizeof(TCOMMENT_INNER) - MaxTextLen_Ex];
   } TCOMMENT_EX;

 //#define TCOMMENT_EX_size       (sizeof(TCOMMENT_EX))
 //#if TCOMMENT_EX_size!=MaxTextLen
 // #error "TCOMMENT_EX size=!512 !!!!"
// #endif

// ��������� ������� ��� ��
typedef struct _TPROJECT
{
   WORD                 ProjectSize;	        // ������ �������
   BYTE                 AmountDirects;	        // ������������ �����������
   TDIRECTS             Directs;	        // �����������
   BYTE                 AmountMuls;             // ���-�� ��
   TSHABLONFASA         Fasa[MaxFase] ;         // ���� - �������
   TCALENDAR            DaysPlans[MaxDaysPlans];        // ����� ������� ������
   TWEEKCALENDAR        WeeksPlans[MaxWeeksPlans];      // ����� ��������� ������
   TYEARCALENDAR        Year;                           // ������� ����
   TPROGRAM             Program;                   // ����� ��������
   TGUARD               guard;                          // ��������� ������������
   //////////////
   //512 byte block
   //unsigned char        comment[MaxTextLen];   //�����������
   TCOMMENT_EX            comments;
   //unsigned char        comm_add[TextAdd-1];   // ���������� ������
   //signed char          gmt;                   // ������� � �����
   //////////////
   TJORNAL              jornal;                // ��������� ��������������
   unsigned long        CRC32_programs;        //�� ��������� ��������
   ///
   unsigned long        CRC32;


}  TPROJECT;

/////////
typedef struct _TALLPROJECT
{
        TPROJECT   PROJ;
        ///
        TPROGRAMS  PROGS;
        //



} TALLPROJECT;


/////
#endif






