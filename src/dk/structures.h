//
#ifndef structuresH
#define structuresH
//------------
#define KURS_DK
#include "../types_define.h"

#pragma pack (1)

 //
 #define DK_N              4    // кол-во ДК
 #define MaxWeeksPlans     16  //Число недельных планов
 #define MaxDaysPlans      16  //Число суточных планов
 #define MaxCalendar       32  //Число разбиений для суток

 #define MaxProgFase       32   //Максимальное число фаз в программа
 //#define MaxProgFase     8   //Максимальное число фаз в программа

 #define MaxTVP            2   //Максимальное число ТВП
 #define MaxDirects        32  //Максимальное число направлений
 //#define MaxDirects      8   //Максимальное число направлений
 #define MaxProgram        32  //Максимальное число программ
 //#define MaxProgram      8   //Максимальное число программ
 #define MaxFase           32  //Максимальное число фаз
  //
 #define MaxMULBlocks      8  //Максимальное число блоков
 #define MaxChannels       3  //Число каналов в СУСО

 #define WeeksInYear       54
 #define MaxLightSlots     12  //Число слотов яркости в дневном плане
 #define MaxLightPlans     12  //Число планов яркости

 #define MaxTramvays       4    //число трамвайных светофоров
 ////  {Было 512, 12 ушло на аддоны}
 #define MaxTextLen        512    //длительность текстового комментария
 #define MaxTextLen_Ex     128
 //#define TextAdd         12     // добавочная секция

 #define TaktsN            4        // число тактов в пром. фазе
 #define TaktsALL          5        // число тактов в фазе

 //////
 // спец-программы
 #define NO_PROG           -1
 #define SPEC_PROG_NO      0   //не спец-программа
 #define SPEC_PROG_YF      1   //ЖМ
 #define SPEC_PROG_OC      2   //OC
 #define SPEC_PROG_KK      3   //KK
 //////////////
 // св-ва фаз
 #define FAZA_PROP_FIKS    0   //фиксированная фаза
 #define FAZA_PROP_COMB    1   //комбинированная
 #define FAZA_PROP_TVP     2   //вызывная ТВП


/////////

 ////////////////
 // ПромТакты
 //typedef unsigned char BYTE;
 //typedef unsigned short WORD;
 //typedef unsigned long DWORD;

 // наименование
#define P_ZD1     0
#define P_ZD2     1
#define P_KD      2

// состояния направлений
typedef enum __STATES_LIGHT
{
	ALL_OFF       = 0x00,   // Все выключены
	GREEN         = 0x01,	// Зелёный
	GREEN_FLASH   = 0x02,	// Зелёный мигающий
	RED           = 0x03,   // Красный
	RED_YELLOW    = 0x04,   // Красный-жёлтый
	YELLOW        = 0x05,   // Жёлтый
	YELLOW_FLASH  = 0x06,	// Жёлтый мигающий
	RESERVED
} STATES_LIGHT;
 ////////////////

 typedef struct  _TPROMTAKT
 {
       BYTE Zd1;     // Время выполнения такта
       BYTE Zd2;
       BYTE KD;
 }TPROMTAKT;
 ////  шаблон фазы
 #define SHABL_FAZA_DIR_RED     0
 #define SHABL_FAZA_DIR_GREEN   1
 ////

 //
 typedef struct _TSHABLONFASA
 {
    BYTE  Direct[MaxDirects];  //Флаг активного направления в фазе
                               //0x01-активное  0x0 -пассивное  (красный цвет)


 }TSHABLONFASA;
 // источники ТВП
 typedef struct _TVPSOURCE
 {
   //BYTE      addr; //адрес светофорного объекта 0..11
   BYTE        pres; //0 - нет ТВП, 1,2 - первая\вторая кнопка
 }TVPSOURCE;
 // Структура фазы в программе
 typedef struct _TFASA
 {
   BYTE      FasaProperty;        //  свойства фазы
   WORD      Tosn;                //  основное время
   WORD      Tmin;                //  минимальное время
   BYTE      NumFasa;             //  номер шаблонной фазы
   TPROMTAKT prom[MaxDirects];    // пром такты для каждого напрвления
   //BYTE      TVP1;               // ТВП1 - адрес светофорного объекта 0..11
  // BYTE      TVP1_num;           // номер ТВП
                                // 0 - нет ТВП, 1,2 - первая\вторая кнопка
   //BYTE      TVP2;               // ТВП2
   //BYTE      TVP2_num;           //
   TVPSOURCE tvps[MaxTVP];

 }TFASA;
 // FasaProperty
 //Фиксированная -  0;
 //Комбинированная, 0x01
 //ТВП - 0x02


// Структура раскладки программ по времени суток
typedef struct _TCALENDARTIME
{
	BYTE   NumProgramWorks;  //  № программы   1-32
        BYTE   SpecProg;         // флаг и номер спец-программ
	DWORD  BeginTimeWorks;   //  Время начала работы в секундах от 00:00:00
} TCALENDARTIME;

typedef struct
 {
   BYTE            AmountCalendTime;          //кол-во отрезков
   TCALENDARTIME   CalendTime[ MaxCalendar ]; // Временная диапазон выполнения программы
} TCALENDAR;

typedef struct
 {
   BYTE     Calendar[7 ]; // номера(индексы) суточных планов из списка DaysPlans
                          // 0 - Вс, 6 - Сб
}TWEEKCALENDAR;
///
typedef struct
 {
   BYTE     YearCalendar[WeeksInYear]; // номера(индексы) недельных планов из списка WeeksCalendars
}TYEARCALENDAR;
///////////////////////////////////////////////
typedef struct
{
   BYTE   AmountFasa; 		   //Количество фаз в программе
   TFASA  fazas[MaxProgFase];
}TPROGRAM;

typedef struct
{
    TPROGRAM             Program[ MaxProgram ];          // набор программ
    DWORD                ProgramsSize;	        // Размер программ
    unsigned long        CRC32;
} TPROGRAMS;
//Модули - Сигнальные группы
//MaxMULBlocks= Число чигнальных групп

typedef enum {
	MUL_RED       = 0x00,   // Красный
        MUL_YELLOW    = 0x01,	// желтый
	MUL_GREEN     = 0x02,	// Зелёный
	MUL_COLOR_COUNT
} MUL_COLOR;
 ////////////////

// один выход
// цвета - 0-красный, 1 - зеленый, 2 - желтый
typedef struct
{
   BYTE  napr;  // 0 - нет привязки, 1 - первое направление, ....
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
//……………………………………………………………
//Направления
//Тип направления Трансп=0 Пешеходн=1  Стрелка=2 Реверсивное=3

#define DIRECT_TRANSP   0
#define DIRECT_PESH     1
#define DIRECT_ARROW    2
#define DIRECT_REV      3
#define DIRECT_TRAM_L   4
#define DIRECT_TRAM_R   5
#define DIRECT_TRAM_F   6

 typedef enum {
	DIR_TRANSP       = 0x00,   // Красный
	DIR_PESH     = 0x01,	// Зелёный
	DIR_ARROW    = 0x02,	// желтый
	DIR_REV   =  0x03,   // левый глаз трамвайника
	DIR_TRAM_L  = 0x04,   // правый
	DIR_TRAM_R  = 0x05,   // центральный..
	DIR_TRAM_F   = 0x06,
        DIR_COUNT
} DIR_TYPE;
 ////
 // привязка - к 2-м линиям
 typedef struct _TNAPROUT
 {
   BYTE       group; //номер силовой группы  0 - нет привязки, 1 - первая группа
   MUL_COLOR  color;// номер выхода группы 0,1,2
   BYTE       control; //флаг отключения контроля
   ///
   BYTE       group2; //номер силовой группы  0 - нет привязки, 1 - первая группа
   MUL_COLOR  color2;// номер выхода группы 0,1,2
   BYTE       control2; //флаг отключения контроля
   ///


 } TNAPROUT;
 ////
 typedef struct _TNAPROUTALL
 {
   TNAPROUT  red;   // красный сигнал направления
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
   TNAPROUTALL   TramStop[MaxTramvays]; //привязка нижнего глазика трамвайников
 } TDIRECTS;
 ///
 typedef struct _TGUARD
 {
        // >0 - время между зелеными конфликтных направлений
        BYTE  ConfMatrix[MaxDirects][MaxDirects]; //
        BYTE  ConfMatrixFlag[MaxDirects][MaxDirects]; //  флаги
        //
        BYTE  red_min;   // минимальный
        BYTE  yellow;
        BYTE  redyellow;
        BYTE  greenflash;
        BYTE  green_min;
        WORD  faza_max;
        BYTE  green_fill;  //флаг заполнения пром. тактов 0-красным, 1 - зеленым
        ///
        //BYTE  porog_green;
        //BYTE  porog_red;
        //BYTE  alarm_off;          //флаг отключенных аварий
        BYTE   restart_interval;   //  интервал перезапуска
        BYTE   restarts;            // кол-во попыток перезапуска
        BYTE   time_clear;         // время очистки аварий
        BYTE   green_flash_tramv_off;   // флаг отключения мигания трамвайника
        //
        BYTE   kk_len;   //КК длительнсоть

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
       BYTE     rings;        // кол-во колец в проекте
       BYTE     synhro_mode;  //флаг синхронного режима
       WORD     syhhro_add;   // добавочное время

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
     TCOMMENT_INNER       inner;  //вложенная структура
     unsigned char        comm_add[MaxTextLen-sizeof(TCOMMENT_INNER) - MaxTextLen_Ex];
   } TCOMMENT_EX;

 //#define TCOMMENT_EX_size       (sizeof(TCOMMENT_EX))
 //#if TCOMMENT_EX_size!=MaxTextLen
 // #error "TCOMMENT_EX size=!512 !!!!"
// #endif

// Структура проекта для ДК
typedef struct _TPROJECT
{
   WORD                 ProjectSize;	        // Размер проекта
   BYTE                 AmountDirects;	        // используемых направлений
   TDIRECTS             Directs;	        // Направления
   BYTE                 AmountMuls;             // кол-во СГ
   TSHABLONFASA         Fasa[MaxFase] ;         // Фазы - шаблоны
   TCALENDAR            DaysPlans[MaxDaysPlans];        // набор дневных планов
   TWEEKCALENDAR        WeeksPlans[MaxWeeksPlans];      // набор недельных планов
   TYEARCALENDAR        Year;                           // годовой план
   TPROGRAM             Program;                   // набор программ
   TGUARD               guard;                          // параметры безопасности
   //////////////
   //512 byte block
   //unsigned char        comment[MaxTextLen];   //комментарий
   TCOMMENT_EX            comments;
   //unsigned char        comm_add[TextAdd-1];   // добавочная секция
   //signed char          gmt;                   // добавка в часах
   //////////////
   TJORNAL              jornal;                // настройки журналирования
   unsigned long        CRC32_programs;        //от структуры программ
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






