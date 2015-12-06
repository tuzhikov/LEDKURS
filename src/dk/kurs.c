/// KURS low level

#include "kurs.h"
#include "dk.h"
#include "structures.h"
#include "takts.h"
#include "string.h"
#include "../utime.h"
#include "../memory/ds1390.h"
#include "../multicast/cmd_fn.h"
#include "../memory/memory.h"
#include "../pins.h"


    TPROGRAM     PROG_PLAN;
    //
    typedef enum
    {
      TRAM_LEFT,
      TRAM_CENTER,
      TRAM_RIGHT,
      TRAM_STOP
    }trams_leds;

    typedef struct _tram_struct_type
    {
     BYTE  dir_left;   //направления образующие трамвайник
     BYTE  dir_right;  // 0- нет направления
     BYTE  dir_center;
     ///
     STATES_LIGHT left_sta;
     STATES_LIGHT right_sta;
     STATES_LIGHT center_sta;
    
    }tram_struct_type;
     ///
     tram_struct_type  TRAMS[DK_N][MaxTramvays]; //тра 
     int               AmountTramvays[DK_N]; //кол-во доступных
     BYTE              napr_to_trams[DK_N][MaxDirects]; 
     
     //контролировать, но не устанавливать
     BYTE     GREEN_PORT_TRAM;   
     BYTE     YEL_PORT_TRAM;   
     /// состояния защелок портов
     BYTE     GREEN_PORT;
     BYTE     RED_PORT;
     BYTE     YEL_PORT;   
     // флаги Включенных конфликтов выходов
     BYTE     GREEN_PORT_CONF;
     BYTE     RED_PORT_CONF;
     BYTE     YEL_PORT_CONF;
     /////////////////
     // словленные конфликты
     BYTE     GREEN_PORT_ERR;
     BYTE     RED_PORT_ERR;
     ///
     // соответветствие бита в GREEN_PORT_TRAM линии, которую она отслеживает
     BYTE     tram_green_drive_line[8];
     BYTE     tram_green_drive_line_num[8];
     
     
     // соответствие направлений и трамвайников
//------------------------------------------------------------------------------
// проверка, является ли зеленый только контролирующим каналом
// для трамвайного направления
void Check_GREEN_TRAM(TNAPROUT *napr)
{
    int group = (napr->group);//-1;
    int group2 = (napr->group2);//-1;
    ///////////
    if ((group==0) && (group2==0))
      return;
    //RED_PORT_CONF = RED_PORT_CONF | (1<<group);
    if (group)
    if (group2)
    {
       GREEN_PORT_TRAM = GREEN_PORT_TRAM | (1<<(group2-1)); 
       tram_green_drive_line[group2-1] = napr->color;
       tram_green_drive_line_num[group2-1] = group-1;
       
    }
}
//------------------------------------------------------------------------------
// собираем описание трамвайников из направлений
void Collect_Tramvays()
{
    // индексы светофоров 
    int left_indx, right_indx, center_indx; 
    TONEDIRECT  *diR;
    left_indx=0, right_indx=0, center_indx=0;
    AmountTramvays[CUR_DK]=0;
    //
    GREEN_PORT_TRAM=0;
    ///////
    for (int i=0; i< MaxDirects; i++)
    {
        napr_to_trams[CUR_DK][i]=0;
    }
    ////
    for (int i_napr=0; i_napr< DK[CUR_DK].PROJ->AmountDirects; i_napr++)
    {
        diR = &DK[CUR_DK].PROJ->Directs.OneDirect[i_napr];
        //
        switch (diR->Type)
        {
          case DIR_TRAM_L:
          {
            Check_GREEN_TRAM(&(diR->out.red));
            TRAMS[CUR_DK][left_indx].dir_left = i_napr;
            left_indx++;
            napr_to_trams[CUR_DK][i_napr] = left_indx;
            break;  
          }///////////////
          case DIR_TRAM_R:
          {
            Check_GREEN_TRAM(&(diR->out.green));
            TRAMS[CUR_DK][right_indx].dir_right = i_napr;
            right_indx++;
            napr_to_trams[CUR_DK][i_napr] = right_indx;
            break;  
          }///////////////
          case DIR_TRAM_F:
          {
            Check_GREEN_TRAM(&(diR->out.yel));
            TRAMS[CUR_DK][center_indx].dir_center = i_napr;
            center_indx++;
            napr_to_trams[CUR_DK][i_napr] = center_indx;
            break;  
          }///////////////
          
        }//switch
    }
    ///////////////
    TNAPROUT *n_out;
    for (int i=0; i< MaxTramvays; i++)
    {
       n_out = &DK[CUR_DK].PROJ->Directs.TramStop[i].red;
       Check_GREEN_TRAM(n_out);
    }
    ////////////
    AmountTramvays[CUR_DK]=left_indx;
    if (right_indx>AmountTramvays[CUR_DK])
      AmountTramvays[CUR_DK]=right_indx;
    ///
    if (center_indx>AmountTramvays[CUR_DK])
      AmountTramvays[CUR_DK]=center_indx;
    ////
    
  
}
//---------------------------------------------------------------------------
void New_Project_KURS()
{
        ///
        memset(&PROJ,0,sizeof(PROJ));
        ///
        PROJ[CUR_DK].AmountDirects=2;
        PROJ[CUR_DK].AmountMuls=2;
        //PAP.Directs.OneDirect[1].Type=1;
        PROJ[CUR_DK].Fasa[1].Direct[0]=1;
        PROJ[CUR_DK].Fasa[2].Direct[1]=1;
        ///
        //
            PROJ[CUR_DK].Program.AmountFasa=4;
            PROJ[CUR_DK].Program.fazas[0].Tosn=10;
            PROJ[CUR_DK].Program.fazas[1].Tosn=12;
            PROJ[CUR_DK].Program.fazas[2].Tosn=10;
            PROJ[CUR_DK].Program.fazas[3].Tosn=12;

            //
            PROJ[CUR_DK].Program.fazas[0].NumFasa=0;
            PROJ[CUR_DK].Program.fazas[1].NumFasa=1;
            PROJ[CUR_DK].Program.fazas[2].NumFasa=0;
            PROJ[CUR_DK].Program.fazas[3].NumFasa=2;
            //
            memcpy(&PROG_PLAN, &PROJ[CUR_DK].Program, sizeof(PROG_PLAN));
            memcpy(&PROG_NEXT, &PROJ[CUR_DK].Program, sizeof(PROG_PLAN));
            

        //
        PROJ[CUR_DK].Directs.OneDirect[0].out.red.group=1;
        PROJ[CUR_DK].Directs.OneDirect[0].out.red.color=MUL_RED;
        //
        PROJ[CUR_DK].Directs.OneDirect[0].out.yel.group=1;
        PROJ[CUR_DK].Directs.OneDirect[0].out.yel.color=MUL_YELLOW;
        //
        PROJ[CUR_DK].Directs.OneDirect[0].out.green.group=1;
        PROJ[CUR_DK].Directs.OneDirect[0].out.green.color=MUL_GREEN;
        ///////////
        ///////////
        PROJ[CUR_DK].Directs.OneDirect[1].out.red.group=2;
        PROJ[CUR_DK].Directs.OneDirect[1].out.red.color=MUL_RED;
        //
        PROJ[CUR_DK].Directs.OneDirect[1].out.yel.group=2;
        PROJ[CUR_DK].Directs.OneDirect[1].out.yel.color=MUL_YELLOW;
        //
        PROJ[CUR_DK].Directs.OneDirect[1].out.green.group=2;
        PROJ[CUR_DK].Directs.OneDirect[1].out.green.color=MUL_GREEN;
        //
       
        //
        PROJ[CUR_DK].guard.red_min=2;
        PROJ[CUR_DK].guard.yellow=3;
        PROJ[CUR_DK].guard.redyellow=2;
        PROJ[CUR_DK].guard.greenflash=3;
        PROJ[CUR_DK].guard.green_min=1;
        PROJ[CUR_DK].guard.faza_max=3600;
        PROJ[CUR_DK].guard.kk_len = 0;
        //
        


}
//------------------------------------------------------------------------------
void Set_Conflict(unsigned char color, unsigned char group)
{
    if (group) group--;
    else return;
    /////////////
    switch (color)
    {
       case 0: //RED 
       {
            RED_PORT_CONF = RED_PORT_CONF | (1<<group);
            break;
       }/////////////
       case 1: //YEL 
       {
            YEL_PORT_CONF = YEL_PORT_CONF | (1<<group);
            break;
       }/////////////
       case 2: //GREEN 
       {
            GREEN_PORT_CONF = GREEN_PORT_CONF | (1<<group);
            break;
       }/////////////
      
    }
}
//------------------------------------------------------------------------------
void Collect_Conflicts()
{
    unsigned char   col, gr, con;
    ///
    /*
    GREEN_PORT_CONF = 0;
    RED_PORT_CONF = 0;
    YEL_PORT_CONF = 0;
    */
    ///  
    for (int in=0; in< PROJ[CUR_DK].AmountDirects; in++)
    {
        col = PROJ[CUR_DK].Directs.OneDirect[in].out.green.color;
        gr = PROJ[CUR_DK].Directs.OneDirect[in].out.green.group;
        con = PROJ[CUR_DK].Directs.OneDirect[in].out.green.control;
        if (!con)
           Set_Conflict(col,gr);
        ////
        col = PROJ[CUR_DK].Directs.OneDirect[in].out.green.color2;
        con = PROJ[CUR_DK].Directs.OneDirect[in].out.green.control2;
        gr = PROJ[CUR_DK].Directs.OneDirect[in].out.green.group2;
        if (!con)
           Set_Conflict(col,gr);
        ////////////////
        ////////////////
        col = PROJ[CUR_DK].Directs.OneDirect[in].out.yel.color;
        gr = PROJ[CUR_DK].Directs.OneDirect[in].out.yel.group;
        con = PROJ[CUR_DK].Directs.OneDirect[in].out.yel.control;
        if (!con)
           Set_Conflict(col,gr);
        ////
        col = PROJ[CUR_DK].Directs.OneDirect[in].out.yel.color2;
        gr = PROJ[CUR_DK].Directs.OneDirect[in].out.yel.group2;
        con = PROJ[CUR_DK].Directs.OneDirect[in].out.yel.control2;
        if (!con)
           Set_Conflict(col,gr);
        ////////////////
        ////////////////
        col = PROJ[CUR_DK].Directs.OneDirect[in].out.red.color;
        gr = PROJ[CUR_DK].Directs.OneDirect[in].out.red.group;
        con = PROJ[CUR_DK].Directs.OneDirect[in].out.red.control;
        if (!con)
           Set_Conflict(col,gr);
        ////
        col = PROJ[CUR_DK].Directs.OneDirect[in].out.red.color2;
        gr = PROJ[CUR_DK].Directs.OneDirect[in].out.red.group2;
        con = PROJ[CUR_DK].Directs.OneDirect[in].out.red.control2;
        if (!con)
           Set_Conflict(col,gr);
       
    }
    ///////////////
    for (int i=0; i< AmountTramvays[CUR_DK]; i++)
    {
       col = DK[CUR_DK].PROJ->Directs.TramStop[i].red.color;
       gr = DK[CUR_DK].PROJ->Directs.TramStop[i].red.group;
       con = DK[CUR_DK].PROJ->Directs.TramStop[i].red.control;
       if (!con)
           Set_Conflict(col,gr);
       ///
       col = DK[CUR_DK].PROJ->Directs.TramStop[i].red.color2;
       gr = DK[CUR_DK].PROJ->Directs.TramStop[i].red.group2;
       con = DK[CUR_DK].PROJ->Directs.TramStop[i].red.control2;
       if (!con)
           Set_Conflict(col,gr);
       
       
    }
    ////////////
  
}
//------------------------------------------------------------------------------
void Prepare_KURS_Structures()
{
    Collect_Tramvays();
    ///
    Collect_Conflicts();
}
//------------------------------------------------------------------------------
void Clear_LED()
{
    GREEN_PORT=0;
    RED_PORT = 0;
    YEL_PORT = 0;  
}
//------------------------------------------------------------------------------
// устанавливает вывод группы
// group = 0..<=MaxMULBlocks. 0 - не включать.
// gr_col = 0..<MaxChannels
// stat - состояние вкл\выкл
void Set_LED(int group, int gr_col, bool stat)
{
    if (group==0)
      return;
    //////////////
       // сохраняем для защелки 
      group--;
       switch (gr_col)
       {
          //RED
          case 0:
            {
               if (stat)
                 RED_PORT |= (1<<group);
               else
                 RED_PORT &= ~(1<<group);
               //
               break;
            }
            ///////////////////////
          //YEL
          case 1:
            {
               if (stat)
                 YEL_PORT |= (1<<group);
               else
                 YEL_PORT &= ~(1<<group);
               //
               break;
            }
          ///////////////////////
          //GREEN
          case 2:
            {
               if (stat)
                 GREEN_PORT |= (1<<group);
               else
                 GREEN_PORT &= ~(1<<group);
               //
               break;
            }
            ///////////////////////
          
            
    
       }
   /////////////////////
    
   
  /*
    switch (group)
    {
        case 0:
        {
          if (gr_col==0) 
            if (stat) pin_on(OPIN_OUT_R1); else pin_off(OPIN_OUT_R1);
          ////
          if (gr_col==1) 
            if (stat) pin_on(OPIN_OUT_A1); else pin_off(OPIN_OUT_A1);
          ////
          if (gr_col==2) 
            if (stat) pin_on(OPIN_OUT_G1); else pin_off(OPIN_OUT_G1);
          ////
          break;
        }
        ////////////////////////
        case 1:
        {
          if (gr_col==0) 
            if (stat) pin_on(OPIN_OUT_R2); else pin_off(OPIN_OUT_R2);
          ////
          if (gr_col==1) 
            if (stat) pin_on(OPIN_OUT_A2); else pin_off(OPIN_OUT_A2);
          ////
          if (gr_col==2) 
            if (stat) pin_on(OPIN_OUT_G2); else pin_off(OPIN_OUT_G2);
          ////
          break;
        }
        ////////////////////////
        // walker1
        case 2:
        {
          if (gr_col==0) 
            if (stat) pin_on(OPIN_OUT_R3); else pin_off(OPIN_OUT_R3);
          ////
          //if (gr_col==1) 
          //  if (stat) pin_on(OPIN_OUT_A1); else pin_off(OPIN_OUT_A1);
          ////
          if (gr_col==2) 
            if (stat) pin_on(OPIN_OUT_G3); else pin_off(OPIN_OUT_G3);
          ////
          break;
        }
        ////////////////////////
        case 3:
        {
          if (gr_col==0) 
            if (stat) pin_on(OPIN_OUT_R4); else pin_off(OPIN_OUT_R4);
          ////
          //if (gr_col==1) 
          //  if (stat) pin_on(OPIN_OUT_A1); else pin_off(OPIN_OUT_A1);
          ////
          if (gr_col==2) 
            if (stat) pin_on(OPIN_OUT_G4); else pin_off(OPIN_OUT_G4);
          ////
          break;
        }
        ////////////////////////
        //arrow1
        case 4:
        {
          //if (gr_col==0) 
          //  if (stat) pin_on(OPIN_OUT_R4); else pin_off(OPIN_OUT_R4);
          ////
          //if (gr_col==1) 
          //  if (stat) pin_on(OPIN_OUT_A1); else pin_off(OPIN_OUT_A1);
          ////
          if (gr_col==2) 
            if (stat) pin_on(OPIN_OUT_G5); else pin_off(OPIN_OUT_G5);
          ////
          break;
        }
        ////////////////////////
        //arrow1
        case 5:
        {
          //if (gr_col==0) 
          //  if (stat) pin_on(OPIN_OUT_R4); else pin_off(OPIN_OUT_R4);
          ////
          //if (gr_col==1) 
          //  if (stat) pin_on(OPIN_OUT_A1); else pin_off(OPIN_OUT_A1);
          ////
          if (gr_col==2) 
            if (stat) pin_on(OPIN_OUT_G6); else pin_off(OPIN_OUT_G6);
          ////
          break;
        }
        ////////////////////////
             
    }
    */
}

//------------------------------------------------------------------------------

// фильтрует сигналы с зависимости от типа
void Filter_Signal(TONEDIRECT  *diR, bool red_s, bool yel_s, bool green_s) 
{
    DIR_TYPE   d_type = diR->Type;
    int red_g, red_gc;
    int yel_g, yel_gc;
    int green_g,  green_gc;
   //////////////
    red_g = diR->out.red.group; red_gc = diR->out.red.color;
    yel_g = diR->out.yel.group; yel_gc = diR->out.yel.color;
    green_g = diR->out.green.group; green_gc = diR->out.green.color;
    ///  
    switch (d_type)
    {
      case DIR_TRANSP:case DIR_REV:
      {
         Set_LED(red_g, red_gc, red_s);
         Set_LED(yel_g, yel_gc, yel_s);
         Set_LED(green_g, green_gc, green_s);
         break; 
      }
      ///
      case DIR_PESH:
      {
         Set_LED(red_g, red_gc, red_s);
         Set_LED(green_g, green_gc, green_s);
         break;
      }
      ///
      case DIR_ARROW:
      {
         Set_LED(green_g, green_gc, green_s);
         break;
      }
      ///
      
    }
  
}
//------------------------------------------------------------------------------
void Set_TRANSP_NAPR(STATES_LIGHT  sta_light,TONEDIRECT  *diR, TNAPROUTALL *diRO,
                     bool flash)
{
              
               ////  
              Set_LED(diRO->red.group2, diRO->red.color2, false);
              Set_LED(diRO->yel.group2, diRO->yel.color2, false);
              Set_LED(diRO->green.group2, diRO->green.color2, false);
              ////       
              switch (sta_light)
              {
                case ALL_OFF: 
                {
                  Filter_Signal(diR, false, false,false);
                  break;
                }
                /////
                case GREEN: 
                {
                  Filter_Signal(diR, false, false, true);
                  break;
                }
                /////
                case GREEN_FLASH: 
                {
                  Filter_Signal(diR, false, false, flash);
                  break;
                }
                /////
                case RED: 
                {
                  Filter_Signal(diR, true, false, false);
                  break;
                }
                /////
                case RED_YELLOW: 
                {
                  Filter_Signal(diR, true, true, false);
                  break;
                }
                /////
                case YELLOW: 
                {
                  Filter_Signal(diR, false, true, false);
                  break;
                }
                /////
                case YELLOW_FLASH: 
                {
                  Filter_Signal(diR, false, flash, false);
                  break;
                }
                /////
              }
              //////  
  
}

//------------------------------------------------------------------------------
// проверяем ОБЩЕЕ состояние трамвайника. 
// возвращает ALL_OFF, RED, GREEN
// если GREEN - дальше надо анализировать
STATES_LIGHT  Check_Tramvay(int i_tram)
{
  if ((TRAMS[CUR_DK][i_tram].left_sta==GREEN) || 
                (TRAMS[CUR_DK][i_tram].left_sta==GREEN_FLASH)    )
    return(GREEN);
  /////
  if ((TRAMS[CUR_DK][i_tram].right_sta==GREEN) || 
                (TRAMS[CUR_DK][i_tram].right_sta==GREEN_FLASH)    )
    return(GREEN);
  /////
  if ((TRAMS[CUR_DK][i_tram].center_sta==GREEN) || 
                (TRAMS[CUR_DK][i_tram].center_sta==GREEN_FLASH)    )
    return(GREEN);
  ///////////
  /// RED
  if ((TRAMS[CUR_DK][i_tram].left_sta == RED)) 
    return (RED);
  ///////
  // ALL_OFF  
  return (ALL_OFF);
  
}
//------------------------------------------------------------------------------
// номер трамвайника, номер глаза, состояние
void Set_Tram_LED(int i_tram, trams_leds led, bool stat)
{
  int i_napr;
  ////  
  switch(led)
    {
      case TRAM_LEFT:
      {
        i_napr = TRAMS[CUR_DK][i_tram].dir_left;
        //
        Set_LED(DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.red.group,
                DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.red.color,stat);
        //
        Set_LED(DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.red.group2,
                DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.red.color2,stat);
        //
        break;
      }
      /////////////////////
      case TRAM_CENTER:
      {
        i_napr = TRAMS[CUR_DK][i_tram].dir_center;
        //
        Set_LED(DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.yel.group,
                DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.yel.color,stat);
        //
        Set_LED(DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.yel.group2,
                DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.yel.color2,stat);
        //
        break;
      }
      /////////////////////
      case TRAM_RIGHT:
      {
        i_napr = TRAMS[CUR_DK][i_tram].dir_right;
        //
        Set_LED(DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.green.group,
                DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.green.color,stat);
        //
        Set_LED(DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.green.group2,
                DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out.green.color2,stat);
        //
        break;
      }
      /////////////////////
      case TRAM_STOP:
      {
        Set_LED(DK[CUR_DK].PROJ->Directs.TramStop[i_tram].red.group,
                DK[CUR_DK].PROJ->Directs.TramStop[i_tram].red.color, stat);
        ///
        Set_LED(DK[CUR_DK].PROJ->Directs.TramStop[i_tram].red.group2,
                DK[CUR_DK].PROJ->Directs.TramStop[i_tram].red.color2, stat);
                    
        //
        break;
      }
      /////////////////////
      
    }
}
//------------------------------------------------------------------------------
void Write_DX(BYTE dat)
{
    
    if (dat & (1<<0)) pin_on(OPIN_D0); else pin_off(OPIN_D0);
    if (dat & (1<<1)) pin_on(OPIN_D1); else pin_off(OPIN_D1);
    if (dat & (1<<2)) pin_on(OPIN_D2); else pin_off(OPIN_D2);
    if (dat & (1<<3)) pin_on(OPIN_D3); else pin_off(OPIN_D3);
    if (dat & (1<<4)) pin_on(OPIN_D4); else pin_off(OPIN_D4);
    if (dat & (1<<5)) pin_on(OPIN_D5); else pin_off(OPIN_D5);
    if (dat & (1<<6)) pin_on(OPIN_D6); else pin_off(OPIN_D6);
    if (dat & (1<<7)) pin_on(OPIN_D7); else pin_off(OPIN_D7);
   
  
}
//------------------------------------------------------------------------------
// Пишем с защелки
void  SET_OUTPUTS()
{
  //OPIN_TR_EN
  //pin_off(OPIN_TR_EN );
  pin_off(OPIN_R_LOAD);
  pin_off(OPIN_G_LOAD);
  pin_off(OPIN_Y_LOAD);
  //////
  // убираем по маске каналы, которые не нужно устанавливать
  GREEN_PORT = GREEN_PORT & (~GREEN_PORT_TRAM);
  //YEL_PORT =  YEL_PORT & (~YEL_PORT_TRAM);

  //////
    //OPIN_R_LOAD 
    Write_DX(RED_PORT);
    //
    pin_on(OPIN_R_LOAD );
    //wait
    pin_off(OPIN_R_LOAD );
    ///////////////////////
    Write_DX(YEL_PORT);
    //
    pin_on(OPIN_Y_LOAD );
    //wait
    pin_off(OPIN_Y_LOAD );
    //////////////////////
    Write_DX(GREEN_PORT);
    //
    pin_on(OPIN_G_LOAD );
    //wait
    pin_off(OPIN_G_LOAD );
    ///
    
    
}
        
//------------------------------------------------------------------------------
void Light_TVP_Wait(int p_faz)
{
            // вызываемая фаза
            //int p_faz = DK[CUR_DK].TVP.cur.prog_faza;
            int sh_faz; //шаблонная
            // ищем зелененькое пешеходное 
            for (int ina=0; ina< PROJ[CUR_DK].AmountDirects; ina++)
            {
                if (PROJ[CUR_DK].Directs.OneDirect[ina].Type==DIR_PESH)
                {
                   sh_faz=PROJ[CUR_DK].Program.fazas[p_faz].NumFasa;
                   if (PROJ[CUR_DK].Fasa[sh_faz].Direct[ina])
                   {
                       //о, это наше направление
                       Set_LED(PROJ[CUR_DK].Directs.OneDirect[ina].out.yel.group,
                              PROJ[CUR_DK].Directs.OneDirect[ina].out.yel.color, true);
                       
                   }
                }     
            }
}
//------------------------------------------------------------------------------
// обновить 
void Update_STATES_KURS(bool flash)
{
        int i_napr;
        int n_n = DK[CUR_DK].PROJ->AmountDirects;
        STATES_LIGHT  sta_light, tra_sta;
        TONEDIRECT  *diR;
        TNAPROUTALL *diRO;
        int i_tram;
        /////// MaxTramvays
        
        //
        for (i_napr=0; i_napr<n_n; i_napr++)
        {
          sta_light = DK[CUR_DK].control.napr[i_napr];
          diR = &DK[CUR_DK].PROJ->Directs.OneDirect[i_napr];
          diRO = &DK[CUR_DK].PROJ->Directs.OneDirect[i_napr].out;
          // не трамвайное
          if (diR->Type < DIR_TRAM_L)
          {
             Set_TRANSP_NAPR(sta_light,diR, diRO,flash); 
          }
          else
          {
             //трумвайники
            // номер
             i_tram = napr_to_trams[CUR_DK][i_napr]-1;
             //
             switch (diR->Type)
             { 
               case DIR_TRAM_L:
               {
                 TRAMS[CUR_DK][i_tram].left_sta=sta_light; 
                 break;  
               }///////////////
               case DIR_TRAM_R:
               { 
                 TRAMS[CUR_DK][i_tram].right_sta=sta_light; 
                 break;  
               }///////////////
               case DIR_TRAM_F:
               {
                 TRAMS[CUR_DK][i_tram].center_sta=sta_light; 
                 break;  
               }///////////////
          
              }//switch
            ///////////
            
          }//else
          
        }//for 
        /////////////////////////
        // Ждите
        //if ((DK[CUR_DK].TVP.cur.presence) && (DK[CUR_DK].TVP.enabled))
      //  if ((DK[CUR_DK].tvps[0].pres) || (DK[CUR_DK].tvps[1].pres))  
      //  {
        if (DK[CUR_DK].TVP.enabled)
        if ((DK[CUR_DK].CUR.source==PLAN) || (DK[CUR_DK].CUR.source==TVP))
          for (int tvp_i=0; tvp_i< MaxTVP; tvp_i++)
          {
            if (DK[CUR_DK].tvps[tvp_i].pres) 
              Light_TVP_Wait(DK[CUR_DK].tvp_faza[tvp_i]);
            ///
          }  
          //  if ((DK[CUR_DK].tvps[0].pres==2) || (DK[CUR_DK].tvps[1].pres==2))  
          //    Light_TVP_Wait(DK[CUR_DK].tvp2_faza);
            
     //   }
        /*
        else
          
          // гасим все
        for (int ina=0; ina< PROJ.AmountDirects; ina++)
        {
           if (PROJ.Directs.OneDirect[ina].Type==DIR_PESH)
           { 
                       Set_LED(PROJ.Directs.OneDirect[ina].out.yel.group,
                              PROJ.Directs.OneDirect[ina].out.yel.color, false);
                       
           }
        
        }
        */
        /////////////////////////
        //return; //!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //
        for (i_tram=0; i_tram < AmountTramvays[CUR_DK]; i_tram++)
        {
           tra_sta = Check_Tramvay(i_tram);
           //
           switch (tra_sta)
           {
                case GREEN:
                {
                     /// включим СТОП-сигнал
                     Set_Tram_LED(i_tram,TRAM_STOP,true);
                     ///
                     if (TRAMS[CUR_DK][i_tram].left_sta==GREEN)
                         Set_Tram_LED(i_tram,TRAM_LEFT,true);
                     else
                     if (TRAMS[CUR_DK][i_tram].left_sta==GREEN_FLASH)
                         Set_Tram_LED(i_tram,TRAM_LEFT,flash);
                     else
                         Set_Tram_LED(i_tram,TRAM_LEFT,false);
                     //////////////////////
                     ////////////////////
                     if (TRAMS[CUR_DK][i_tram].center_sta==GREEN)
                        Set_Tram_LED(i_tram,TRAM_CENTER,true);
                     else
                     if (TRAMS[CUR_DK][i_tram].center_sta==GREEN_FLASH)
                        Set_Tram_LED(i_tram,TRAM_CENTER,flash);
                     else
                        Set_Tram_LED(i_tram,TRAM_CENTER,false);
                     //////////////////////
                     ////////////////////////
                     if (TRAMS[CUR_DK][i_tram].right_sta==GREEN)
                         Set_Tram_LED(i_tram,TRAM_RIGHT,true);
                     else
                     if (TRAMS[CUR_DK][i_tram].right_sta==GREEN_FLASH)
                         Set_Tram_LED(i_tram,TRAM_RIGHT,flash);
                     else
                        Set_Tram_LED(i_tram,TRAM_RIGHT,false);
                     //////////////////////
                     break;  
                }
                ///////
                case RED:
                {
                   Set_Tram_LED(i_tram,TRAM_STOP,false);
                   //
                   Set_Tram_LED(i_tram,TRAM_LEFT,true);
                   Set_Tram_LED(i_tram,TRAM_CENTER,true);
                   Set_Tram_LED(i_tram,TRAM_RIGHT,true);
                   break;   
                }
                //////
                case ALL_OFF:
                {
                   Set_Tram_LED(i_tram,TRAM_STOP,false);
                   //
                   Set_Tram_LED(i_tram,TRAM_LEFT,false);
                   Set_Tram_LED(i_tram,TRAM_CENTER,false);
                   Set_Tram_LED(i_tram,TRAM_RIGHT,false);
                   break;   
                }
                //////
           }//switch
              
          
        }//for
        ///////////////////////////
        // устанавливаем выходы на защелках
        
         
        //}
        ///////
}
//-----------------------------------------------------------------------------