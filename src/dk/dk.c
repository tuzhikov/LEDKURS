/*----------------------------------------------------------------------------*/
//  ������ ��
// ����������� - model-controller
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dk.h"
#include "kurs.h"
#include "takts.h"
#include "structures.h"
#include "../adcc.h"
#include "../utime.h"
#include "../memory/ds1390.h"
#include "../multicast/cmd_fn.h"
#include "../memory/memory.h"
#include "../pins.h"
#include "../light/light.h"
#include "../event/evt_fifo.h"

#ifdef KURS_DK
  #include "kurs.h"
#endif
/*----------------------------------------------------------------------------*/
__DK   DK[DK_N];
TPROJECT     PROJ[DK_N];
BYTE   CUR_DK;
BYTE   dk_num; //���-�� ������������ ��
SYSTEMTIME   CT; // control time
unsigned char dw_buff[255];

//#define D_W_ENABLE
bool Check_TVP_En();
/*-------------------------------local function-------------------------------*/
int  Get_Next_Viz_Faz(int prog, int cur_faz);
/*----------------------------------------------------------------------------*/
void D_W(const char*s)
{
   #ifdef D_W_ENABLE
     debugee(s);
   #endif
}
/*----------------------------------------------------------------------------*/
#ifndef KURS_DK
// ��������
unsigned short Update_STATES_VO()
{
        int i_napr;
        unsigned short fire=0;
        //n_n = DK[CUR_DK].PROJ->AmountDirects;
        ///
        //for (int i_dk=0; i_dk<n_n; i_dk++)
        //{
        int i_dk=0;
           // ������ ������������
           i_napr = DK[CUR_DK].PROJ->Muls.OneMUL[i_dk].direct1;
           if (i_napr)
               fire |=DK[CUR_DK].control.napr[i_napr-1];
           //    Set_LEDS(DK[CUR_DK].control.napr[i_napr-1],DIRECT1);
           ///
           // ������ ������������
           i_napr = PROJ.Muls.OneMUL[i_dk].direct2;
           if (i_napr)
              fire |= DK[CUR_DK].control.napr[i_napr-1]<<8;
           //   Set_LEDS(DK[CUR_DK].control.napr[i_napr-1], DIRECT2);
           ///
           // ������ ����������
           i_napr = PROJ.Muls.OneMUL[i_dk].walker1;
           if (i_napr && DK[CUR_DK].control.napr[i_napr-1] <= RED )
               fire |= DK[CUR_DK].control.napr[i_napr-1]<<4;
              //Set_LEDS(DK[CUR_DK].control.napr[i_napr-1], WALKER1);;
           ///
           // ������ ����������
           i_napr = PROJ.Muls.OneMUL[i_dk].walker2;
           if (i_napr && DK[CUR_DK].control.napr[i_napr-1] <= RED )
               fire |= DK[CUR_DK].control.napr[i_napr-1]<<12;
              //Set_LEDS(DK[CUR_DK].control.napr[i_napr-1], WALKER2);;
           ///
           // ������ �������
           i_napr = PROJ.Muls.OneMUL[i_dk].arrow1;
           if (i_napr && DK[CUR_DK].control.napr[i_napr-1] <= GREEN_FLASH)
              fire |= DK[CUR_DK].control.napr[i_napr-1]<<6;
              //Set_LEDS(DK[CUR_DK].control.napr[i_napr-1], ARROW1);
           ///
           // ������ �������
           i_napr = PROJ.Muls.OneMUL[i_dk].arrow2;
           if (i_napr && DK[CUR_DK].control.napr[i_napr-1] <= GREEN_FLASH)
               fire |= DK[CUR_DK].control.napr[i_napr-1]<<14;
              //Set_LEDS(DK[CUR_DK].control.napr[i_napr-1], ARROW2);
           ///
          return fire;
        //}
        ///////
}
#endif
/*----------------------------------------------------------------------------*/
unsigned short Update_STATES(bool flash)
{
unsigned short fire_light=0;
   #ifdef KURS_DK
      Update_STATES_KURS(flash);
   #else
      fire_light=Update_STATES_VO();
   #endif
return(fire_light);
}
/*----------------------------------------------------------------------------*/
int Seconds_Between(SYSTEMTIME *tt, SYSTEMTIME *tl)
{
time_t tim, lim;
tim = mktime(tt);
lim = mktime(tl);
return(lim-tim);
}
/*----------------------------------------------------------------------------*/
void New_Project()
{
#ifdef KURS_DK
   New_Project_KURS();
#endif
}
/*----------------------------------------------------------------------------*/
int Init_DK(void)
{
        //
        memset(&DK[CUR_DK],0,sizeof(DK[CUR_DK]));
        DK[CUR_DK].PROJ = &PROJ[CUR_DK];
        DK[CUR_DK].REQ.prior_req = PLAN;
        //
        DK[CUR_DK].synhro_mode = PROJ[CUR_DK].comments.inner.in.synhro_mode;
        //
        DK[CUR_DK].CUR.source=SERVICE;
        DK[CUR_DK].CUR.work = SPEC_PROG;
        DK[CUR_DK].CUR.spec_prog = SPEC_PROG_OC;
        //
        DK[CUR_DK].NEXT.work = SPEC_PROG;
        DK[CUR_DK].NEXT.spec_prog = SPEC_PROG_OC;
        //
        DK[CUR_DK].control.STA = STA_INIT;
        DK[CUR_DK].TVP.STA = STA_FIRST;
        DK[CUR_DK].VPU.STA = STA_FIRST;
        DK[CUR_DK].SERVICE.STA = STA_FIRST;
        //
        if (PROJ[CUR_DK].guard.restarts==0)
          PROJ[CUR_DK].guard.restarts=3;
        //
        if (PROJ[CUR_DK].guard.restart_interval==0)
          PROJ[CUR_DK].guard.restart_interval=3;
        //
        Init_TAKTS(&PROJ[CUR_DK]);
        //
        #ifdef KURS_DK
           Prepare_KURS_Structures();
        #endif
return (0);
}
/*----------------------------------------------------------------------------*/
// ���������� 2 ������� � ��� �����-������ - true
bool Compare_Times(SYSTEMTIME *tt, SYSTEMTIME *tl)
{
        //TDateTime tim, lim;
        ////
        time_t tim, lim;
        ////
        tim = mktime(tt);
        lim = mktime(tl);
        ///
        if (tim>=lim)
            return true;
        else
            return false;
        ////
}
/*----------------------------------------------------------------------------*/
// ��������� �������� ���������
BOOL TIME_END(void)
{
return (!DK[CUR_DK].control.len);
}
/*----------------------------------------------------------------------------*/
// ������� � ������� �������
void TIME_PLUS(SYSTEMTIME *tt, SYSTEMTIME *tplus, int sec_plus)
{
//
time_t tim;
//
tim = mktime(tt);
//
tim = tim + sec_plus;
//
gmtime(tim,tplus);
}
/*----------------------------------------------------------------------------*/
//��������� ����� ����� ������� ��������� (T�����)
int Calc_Tc(int prog)
{
       int c_f, n_f;
       int faz_n;
       int tc=0;
       //
       #ifdef   KURS_DK
            faz_n = PROG_PLAN.AmountFasa;
       #else
            faz_n = DK[CUR_DK].PROJ->Program[prog].AmountFasa;
       #endif
       ///
       for (c_f=0; c_f < faz_n; c_f++)
       {
          n_f = Get_Next_Viz_Faz(prog, c_f);
          //
          Build_Takts(prog, prog, c_f, n_f);
          //
          tc+= osn_takt_time[CUR_DK] + Tprom_len[CUR_DK];
          //
       }
DK[CUR_DK].control.Tproga = tc;
return Null;
}
/*----------------------------------------------------------------------------*/
int GO_INIT()
{
   // ���������
   typedef enum {START, EXIT} GO_STA;
   static GO_STA state; //
   //
   switch (state)
   {
       case START:
        {
                //GetLocalTime(&st);
                //WeekOfTheYear(Now());
        }
   }
return (0);
}
/*----------------------------------------------------------------------------*/
// �������� �� ������ ���� ����������� � �����
// prog - ����� ���������, prog_faza - ���������� ����� ���� � ���������
bool  Is_Faz_Vis(int prog, int   prog_faza)
{
bool  ret_b=false;

#ifdef KURS_DK
        if (PROG_PLAN.fazas[prog_faza].FasaProperty!=FAZA_PROP_TVP)
   #else
        if (DK[CUR_DK].PROJ->Program[prog].fazas[prog_faza].FasaProperty!=FAZA_PROP_TVP)
#endif
ret_b = true;
return (ret_b);
}
/*----------------------------------------------------------------------------*/
// �������� ��������� ������� ���� ��� ����������� �� ��������� ���������
int  Get_Next_Viz_Faz(int prog, int cur_faz)
{
        int     i, ret_i, n_faz;
        ///////////
        ret_i = cur_faz;
        ///

    #ifdef KURS_DK
        n_faz= PROG_PLAN.AmountFasa;
    #else
        n_faz = DK[CUR_DK].PROJ->Program[prog].AmountFasa;
    #endif
        ////
        for (i = cur_faz + 1; i < n_faz; i++)
        {
          if ( Is_Faz_Vis(prog,i) )
          {
            ret_i = i;
            break;
          }//if
        }//for
        ///////////////
        if (ret_i == cur_faz)
        {
             for (i = 0; i < cur_faz; i++)
             {
                if ( Is_Faz_Vis(prog,i) )
                {
                  ret_i = i;
                  break;
                }//if
             }//for

        }// if ret_i
        /////////////////////
        return (ret_i);
}
/*----------------------------------------------------------------------------*/
// ��������� ���������
// prog, faza-  ������� �� ��������� work
bool TEST_STA(const STATE *sta, WORK_STATE  w_sta, BYTE prog, BYTE faza)
{
        bool res=false;
        //
        if (sta->work == w_sta)
        switch (w_sta)
        {
           case SPEC_PROG:
           {
             if (sta->spec_prog == prog)
              res=true;
             break;
           }
           //
           case SINGLE_FAZA:
           {
             if (sta->faza == faza)
              res=true;
             break;
           }
           //
           case PROG_FAZA:
           {
             if (sta->prog == prog)
             if (sta->prog_faza == faza)
               res=true;
             break;
           }
        }
return (res);
}
/*----------------------------------------------------------------------------*/
// ����������� ���������
int Copy_STATES(STATE *dest_sta,const STATE *source_sta)
{
memcpy(dest_sta,source_sta,sizeof(STATE));
return (0);
}
/*----------------------------------------------------------------------------*/
void Clear_STATE(STATE *sta)
{
memset(sta,0,sizeof(STATE));
sta->spec_prog = SPEC_PROG_OC;
}
/*----------------------------------------------------------------------------*/
// �o�������� ����� ������
int  WeekOfTheYear(SYSTEMTIME *st)
{
DS1390_TIME time;
//
time.hour =  st->tm_hour ;
time.date = st->tm_mday;
time.min = st->tm_min;
//
time.month = st->tm_mon ;
time.sec = st->tm_sec;
time.year = st->tm_year;
//
const int iwn = get_week_num(&time);
return (iwn);
}
/*----------------------------------------------------------------------------*/
//{"SUN","MON","TUE","WED","THU","FRI","SAT" };
//  0      1     2     3     4     5     6
int DayOfWeek(SYSTEMTIME *st)
{
        DS1390_TIME time;
        //
        time.hour =  st->tm_hour ;
        time.date = st->tm_mday;
        time.min = st->tm_min;
        //
        time.month = st->tm_mon ;
        time.sec = st->tm_sec;
        time.year = st->tm_year;
        //
        int id = get_day(&time);
        if (id)
          id--;
return (id);
}
/*----------------------------------------------------------------------------*/
// ���������� ����� ��������� �� ������� CT
int Get_Calend_Prog(STATE *sta)
{
               int cur_week, cur_day_of_week;
               int day_plan, week_plan;
               //
               //wek=1..53
                cur_week = WeekOfTheYear(&CT);
                week_plan = DK[CUR_DK].PROJ->Year.YearCalendar[cur_week-1];

                //cur_day_of_week = now.DayOfWeek();
                cur_day_of_week = DayOfWeek(&CT);
                day_plan = DK[CUR_DK].PROJ->WeeksPlans[week_plan].Calendar[cur_day_of_week];
                //
                // seech for time slot and current program
                int cur_sec=CT.tm_hour*3600 + CT.tm_min*60 + CT.tm_sec;
                //
                int cur_slot=0;
                int i_s=0;
                int c_slots = DK[CUR_DK].PROJ->DaysPlans[day_plan].AmountCalendTime;
                //
                for (int i=1; i< c_slots; i++)
                {
                  i_s = DK[CUR_DK].PROJ->DaysPlans[day_plan].CalendTime[i].BeginTimeWorks;
                  //
                  if (cur_sec < i_s)
                        break;
                  else
                    cur_slot++;
                }
                // �������� ����� ������ �����
                if (DK[CUR_DK].synhro_mode)
                {
                   i_s = DK[CUR_DK].PROJ->DaysPlans[day_plan].CalendTime[cur_slot].BeginTimeWorks +
                       DK[CUR_DK].PROJ->comments.inner.in.syhhro_add;
                   DK[CUR_DK].cur_slot_start = i_s;
                }
                //
                sta->prog =
                    DK[CUR_DK].PROJ->DaysPlans[day_plan].CalendTime[cur_slot].NumProgramWorks;
                sta->spec_prog =
                   DK[CUR_DK].PROJ->DaysPlans[day_plan].CalendTime[cur_slot].SpecProg;
return (0);
}
/*----------------------------------------------------------------------------*/
#ifdef KURS_DK
void Load_Plan_Prog(int i_prog)
{
if (!DK[CUR_DK].no_LOAD_PROG_PLAN)
  flash_rd(FLASH_PROGS, CUR_DK*sizeof(TPROGRAMS) +  i_prog*sizeof(TPROGRAM),
                  (unsigned char*)&PROG_PLAN, sizeof(TPROGRAM));
}
#endif
/*----------------------------------------------------------------------------*/
// ��������� ���� - work
void  Fill_STATE_work(STATE *sta)
{
if (sta->spec_prog)
      sta->work = SPEC_PROG;
    else
      sta->work = PROG_FAZA;
}
/*----------------------------------------------------------------------------*/
// ������ �� ������������ ����� � ����������
int GO_PLAN()
{
BYTE  cur_prog_faza;// ����� ���� � ���������

switch (DK[CUR_DK].PLAN.STA)
   {
        // ������ ����
        case STA_INIT:
        {
                Get_Calend_Prog(&DK[CUR_DK].PLAN.cur);
                Fill_STATE_work(&DK[CUR_DK].PLAN.cur);
                DK[CUR_DK].PLAN.cur.source = PLAN;
                //
                #ifdef KURS_DK
                    Load_Plan_Prog(DK[CUR_DK].PLAN.cur.prog);
                    cur_prog_faza =  PROG_PLAN.AmountFasa-1;
                #else
                    cur_prog_faza =  DK[CUR_DK].PROJ->Program[DK[CUR_DK].PLAN.cur.prog].AmountFasa-1;
                #endif
                //
                 //Calc_Tc(cur_prog_faza);
                ///
                    // ���� ������ ������� ����
                 DK[CUR_DK].PLAN.cur.prog_faza =
                         Get_Next_Viz_Faz(DK[CUR_DK].PLAN.cur.prog, cur_prog_faza);
                 //

                 //DK[CUR_DK].PLAN.cur.work = PROG_FAZA;

                ////
                Copy_STATES(&DK[CUR_DK].NEXT, &DK[CUR_DK].PLAN.cur);
                //
                Calc_Tc(DK[CUR_DK].PLAN.cur.prog);
                //GEN_TAKTS();
                DK[CUR_DK].NEXT.presence=true;
                //DK[CUR_DK].NEXT.source = PLAN;
                //
                DK[CUR_DK].PLAN.STA = STA_GET_REQ;

                //DK[CUR_DK].PLAN.STA = STA_GET_REQ;
                //
                break;
        }
        // ���� ������������
        case STA_GET_REQ:
        {
                D_W("STA_GET_REQ");
                //
                if (DK[CUR_DK].NEXT.presence==false)
                {
                   D_W("DK[CUR_DK].NEXT.presence==false");
                   //
                   Get_Calend_Prog(&DK[CUR_DK].PLAN.next);
                   ////////////
                   // �����
                   if (DK[CUR_DK].PLAN.next.spec_prog)
                      DK[CUR_DK].PLAN.next.work = SPEC_PROG;
                   else
                      DK[CUR_DK].PLAN.next.work = PROG_FAZA;
                   ////
                   // PROG1->PROG2
                   if ((!DK[CUR_DK].PLAN.cur.spec_prog) &&
                       (!DK[CUR_DK].PLAN.next.spec_prog))
                   {
                       // �� �� ���������
                       if (DK[CUR_DK].PLAN.cur.prog == DK[CUR_DK].PLAN.next.prog)
                       {
                              // ��������� �� ���� �����������
                              if (!DK[CUR_DK].PLAN.cur.set)
                              {
                                    Copy_STATES(&DK[CUR_DK].PLAN.next, &DK[CUR_DK].PLAN.cur);
                              }
                              else
                              {
                                    D_W("��������� �� �����");
                                   // ��������� �� �����
                                   DK[CUR_DK].PLAN.next.prog_faza =
                                       //Get_Next_Viz_Faz(-1000, DK[CUR_DK].PLAN.cur.prog_faza);
                                       Get_Next_Viz_Faz(DK[CUR_DK].PLAN.cur.prog,
                                                    DK[CUR_DK].PLAN.cur.prog_faza);
                                   //
                              }
                              ///
                       }
                       else  //��������� �� ������ ���������
                       {
                           #ifdef KURS_DK
                              Load_Plan_Prog(DK[CUR_DK].PLAN.cur.prog);
                              // ���� ������ ������� ����
                              cur_prog_faza =  PROG_PLAN.AmountFasa-1;
                            #else
                              cur_prog_faza =  DK[CUR_DK].PROJ->Program[DK[CUR_DK].PLAN.cur.prog].AmountFasa-1;
                            #endif
                            ///
                            //Calc_Tc(cur_prog_faza);
                            Get_Next_Viz_Faz(DK[CUR_DK].PLAN.cur.prog, cur_prog_faza);
                            //
                       }
                   } // PROG1->PROG2
                   //
                   Copy_STATES(&DK[CUR_DK].PLAN.cur, &DK[CUR_DK].PLAN.next);
                   Copy_STATES(&DK[CUR_DK].NEXT, &DK[CUR_DK].PLAN.cur);
                   //
                   DK[CUR_DK].NEXT.presence=true;
                   DK[CUR_DK].NEXT.source = PLAN;
                }
        }
   }
return (0);
}
/*----------------------------------------------------------------------------*/
// ������������ ��������� DK
int  Switch_DK_States()
{
return (0);
}
/*----------------------------------------------------------------------------*/
void Note_tvp_res(int f_i)
{
//DK[CUR_DK].REQ.req[TVP].prog_faza = f_i;
}
/*----------------------------------------------------------------------------*/
// ���� ���� �������������� ������
bool Calculate_TVP()
{
        //int cur_prog;
        TPROGRAM *proga;
        bool found=false;
        int  pres1, pres2;
        int  fpres1, fpres2;
        ///
        //cur_prog = DK[CUR_DK].CUR.prog;

        //Copy_STATES(DK[CUR_DK].requests.TVP, DK[CUR_DK].CUR);
        DK[CUR_DK].REQ.req[TVP].work = PROG_FAZA;//, DK[CUR_DK].CUR
        DK[CUR_DK].REQ.req[TVP].prog =  DK[CUR_DK].CUR.prog;
        DK[CUR_DK].REQ.req[TVP].source = TVP;

        //
        #ifdef KURS_DK
           proga = & DK[CUR_DK].PROJ->Program;
        #else
           proga = &DK[CUR_DK].PROJ->Program[cur_prog];
        #endif
        //DK[CUR_DK].requests.TVP.prog = DK[CUR_DK].CUR.prog;
        for (int f_i=0; f_i < proga->AmountFasa; f_i++)
        {
           if (proga->fazas[f_i].FasaProperty != FAZA_PROP_FIKS)
           {
              fpres1= proga->fazas[f_i].tvps[0].pres;
              fpres2= proga->fazas[f_i].tvps[1].pres;
              //
              pres1 = DK[CUR_DK].tvps[0].pres;
              pres2 = DK[CUR_DK].tvps[1].pres;
              ///////////
              //if (proga->fazas[f_i].tvps[0].pres == DK[CUR_DK].tvps.pres )
             //if ( pres1)
             //if ((fpres1==pres1) || (fpres2==pres1))
             if (fpres1 && pres1)
             {
               DK[CUR_DK].REQ.req[TVP].prog_faza = f_i;
               found=true;
               break;
             }
             ////////////
             //if ( pres2)
             //if ((fpres1==pres2) || (fpres2==pres2))
             if (fpres2 && pres2)
             {
               DK[CUR_DK].REQ.req[TVP].prog_faza = f_i;
               found=true;
               break;
             }
           }
        }
        //
        if (found)
          Copy_STATES(&DK[CUR_DK].TVP.cur, &DK[CUR_DK].REQ.req[TVP]);
        //
        return (found);

}
/*----------------------------------------------------------------------------*/
int GO_TVP()
{
        switch (DK[CUR_DK].TVP.STA)
        {
                // ������ ���� - ��������� �������
                case STA_FIRST:
                {
                   if (  Calculate_TVP() )
                     DK[CUR_DK].TVP.STA=STA_SEE_REQUEST;
                }
                ///
                case STA_SEE_REQUEST:
                {
                    //���� ��������� ����� ����� ���� �� ���
                    if (DK[CUR_DK].CUR.source!=TVP)
                    {
                       // ���� ����������� ���������� ���������
                       if (!DK[CUR_DK].NEXT.set)
                       if (DK[CUR_DK].CUR.prog_faza!= DK[CUR_DK].TVP.cur.prog_faza)
                       {
                          if (DK[CUR_DK].NEXT.source!=TVP)
                          {
                             Copy_STATES(&DK[CUR_DK].NEXT, &DK[CUR_DK].TVP.cur);
                             DK[CUR_DK].TVP.STA=STA_EXIT;
                          }
                       }
                       else
                       {
                            DK[CUR_DK].TVP.STA=STA_EXIT;
                            DK[CUR_DK].TVP.cur.set=true;

                       }
                       ////

                    }
                    break;
                }
                //////////

                //// �����
                case STA_EXIT:
                {
                    // ���� ���������
                    //if (DK[CUR_DK].CUR.source == TVP)
                    if (DK[CUR_DK].TVP.cur.set)
                    {
                       // ����� ������ ����������
                      for (int tvp_i=0; tvp_i< MaxTVP; tvp_i++)
                      {
                        if (DK[CUR_DK].CUR.prog_faza ==
                               DK[CUR_DK].tvp_faza[tvp_i])
                          DK[CUR_DK].tvps[tvp_i].pres=0;
                      }
                       ///
                      // if (DK[CUR_DK].CUR.prog_faza == DK[CUR_DK].tvp2_faza)
                      //   DK[CUR_DK].tvps[1].pres=0;
                       ///
                       /*
                       if (DK[CUR_DK].tvps[0].pres || DK[CUR_DK].tvps[1].pres)
                       {
                          //������� ��� ���� ������
                          DK[CUR_DK].TVP.STA = STA_SEE_REQUEST;
                          break;
                       }
                       else
                       {
                       */
                          //����������, ������� ���� �������
                          DK[CUR_DK].REQ.req[TVP].presence=false;
                          DK[CUR_DK].TVP.cur.presence=false;
                          //DK[CUR_DK].TVP.enabled = false;
                          DK[CUR_DK].TVP.STA=STA_FIRST;
                       //}
                    }
                    ////
                    if (DK[CUR_DK].CUR.source!=TVP)
                    if (DK[CUR_DK].NEXT.source!=TVP)
                    {
                       // �� �� ����� ������ ������
                       DK[CUR_DK].TVP.STA=STA_FIRST;

                    }
                    ////
                    break;
                }
                ///

        }
    return (0);
}
/*----------------------------------------------------------------------------*/
void Calculate_SERVICE()
{
}
/*----------------------------------------------------------------------------*/
int GO_ALARM()
{
                  if (DK[CUR_DK].REQ.req[ALARM].presence)
                    {
                       if ((DK[CUR_DK].REQ.req[ALARM].work==SPEC_PROG))
                       {
                         Copy_STATES(&DK[CUR_DK].NEXT, &DK[CUR_DK].REQ.req[ALARM]);
                         Copy_STATES(&DK[CUR_DK].SERVICE.cur, &DK[CUR_DK].REQ.req[ALARM]);
                       }

                    }
return Null;
}
/*----------------------------------------------------------------------------*/
int GO_TUBMLER()
{
                  if (DK[CUR_DK].REQ.req[TUMBLER].presence)
                    {
                       if ((DK[CUR_DK].REQ.req[TUMBLER].work==SPEC_PROG) ||
                          (!DK[CUR_DK].NEXT.set) )
                       {
                         Copy_STATES(&DK[CUR_DK].NEXT, &DK[CUR_DK].REQ.req[TUMBLER]);
                         Copy_STATES(&DK[CUR_DK].SERVICE.cur, &DK[CUR_DK].REQ.req[TUMBLER]);
                       }

                    }
return Null;
}
/*----------------------------------------------------------------------------*/
int GO_SERVICE()
{
    switch (DK[CUR_DK].SERVICE.STA)
        {
                // ������ ���� - ��������� �������
                case STA_FIRST:
                {
                    if (DK[CUR_DK].REQ.req[SERVICE].presence)
                    {
                       if ((DK[CUR_DK].REQ.req[SERVICE].work==SPEC_PROG) ||
                          (!DK[CUR_DK].NEXT.set) )
                       {
                         Copy_STATES(&DK[CUR_DK].NEXT, &DK[CUR_DK].REQ.req[SERVICE]);
                         Copy_STATES(&DK[CUR_DK].SERVICE.cur, &DK[CUR_DK].REQ.req[SERVICE]);
                       }

                    }
                       break;
                }
                //// �����
                case STA_EXIT:
                {
                    break;
                }
        }
return Null;
}
/*----------------------------------------------------------------------------*/
int GO_VPU()
{
        switch (DK[CUR_DK].VPU.STA)
        {
                // ������ ���� - ��������� �������
                case STA_FIRST:
                {
                    if (DK[CUR_DK].REQ.req[VPU].presence)
                    {
                       // ���� ����������� ���������� ���������
                       if ((!DK[CUR_DK].NEXT.set) ||
                           (DK[CUR_DK].REQ.req[VPU].work==SPEC_PROG))
                       {
                          Copy_STATES(&DK[CUR_DK].NEXT, &DK[CUR_DK].REQ.req[VPU]);
                          Copy_STATES(&DK[CUR_DK].VPU.cur, &DK[CUR_DK].REQ.req[VPU]);
                          //
                          //DK[CUR_DK].requests.VPU.presence = false;
                          //DK[CUR_DK].VPU.STA = STA_EXIT;
                       }
                    }
                       break;
                }
                // �����
                case STA_EXIT:
                {
                    break;
                }
        }
return Null;
}
/*----------------------------------------------------------------------------*/
int   MODEL()
{
// ����� ��������� ��
switch (DK[CUR_DK].REQ.prior_req)
  {
  case ALARM:   {GO_ALARM();  break;}
  case TUMBLER: {GO_TUBMLER();break;}
  case SERVICE: {GO_SERVICE();break;}
  case VPU:     {GO_VPU();    break;}
  case TVP:     {GO_TVP();    break;}
  case PLAN:    {GO_PLAN();   break;}
  }
return Null;
}
/*----------------------------------------------------------------------------*/
//  ��������� ����������� ��������
int REQUESTS()
{
        // �����-���� ������� �������� �������
        // � ����������� � ��������� DK[CUR_DK].requests
        if (DK[CUR_DK].REQ.req[ALARM].presence)
        {
           DK[CUR_DK].REQ.prior_req=ALARM;
           return (RET_OK);
        }
        //
        if (DK[CUR_DK].REQ.req[TUMBLER].presence)
        {
           DK[CUR_DK].REQ.prior_req=TUMBLER;
           return (RET_OK);
        }
        //
        if (DK[CUR_DK].REQ.req[SERVICE].presence)
        {
           DK[CUR_DK].REQ.prior_req=SERVICE;
           return (RET_OK);
        }
        //
        if (DK[CUR_DK].REQ.req[VPU].presence)
        {
           DK[CUR_DK].REQ.prior_req=VPU;
           return (RET_OK);
        }
        //
        if (DK[CUR_DK].REQ.req[TVP].presence)
        {
          if (DK[CUR_DK].TVP.enabled)
          {
             DK[CUR_DK].REQ.prior_req=TVP;
             return (RET_OK);
          }
        }
DK[CUR_DK].REQ.prior_req=PLAN;
return (RET_OK);
}
/*----------------------------------------------------------------------------*/
//���������� ������� ������ �� ��������� CUR->NEXT
int GEN_TAKTS()
{
     #ifdef KURS_DK
       //������ ������� ���������
        if (!DK[CUR_DK].no_LOAD_PROG_PLAN)
         //flash_rd(FLASH_PROGS, (DK[CUR_DK].CUR.prog) *sizeof(TPROGRAM),
         //         (unsigned char*)&PROJ.Program, sizeof(TPROGRAM));
          Load_Plan_Prog(DK[CUR_DK].CUR.prog);

     #endif
         Check_TVP_En();
        // ����. ����-> ......
        if (DK[CUR_DK].CUR.work == SINGLE_FAZA)
        {
          //����. ����-> ����. ����
          if (DK[CUR_DK].NEXT.work == SINGLE_FAZA)
            Build_Takts(NO_PROG, NO_PROG, DK[CUR_DK].CUR.faza, DK[CUR_DK].NEXT.faza);
          //////
          //����. ����-> ���� ���������
           if (DK[CUR_DK].NEXT.work == PROG_FAZA)
            Build_Takts(NO_PROG, DK[CUR_DK].NEXT.prog, DK[CUR_DK].CUR.faza,
                        DK[CUR_DK].NEXT.prog_faza);
          //
           //����. ����-> ����. ���������
           if (DK[CUR_DK].NEXT.work == SPEC_PROG)
            Build_Takts(NO_PROG, NO_PROG, DK[CUR_DK].CUR.faza,
                        DK[CUR_DK].NEXT.faza);

        }
        // ���� ���������-> ......
        if (DK[CUR_DK].CUR.work == PROG_FAZA)
        {
          // .... -> ����. ����
          if (DK[CUR_DK].NEXT.work == SINGLE_FAZA)
            Build_Takts(DK[CUR_DK].CUR.prog,
                        NO_PROG, DK[CUR_DK].CUR.prog_faza, DK[CUR_DK].NEXT.faza);
          //////
          // ......-> ���� ���������
           if (DK[CUR_DK].NEXT.work == PROG_FAZA)
            Build_Takts(DK[CUR_DK].CUR.prog, DK[CUR_DK].NEXT.prog,
                        DK[CUR_DK].CUR.prog_faza, DK[CUR_DK].NEXT.prog_faza);
          //
          // .... -> ����. ���������
          if (DK[CUR_DK].NEXT.work == SPEC_PROG)
            Build_Takts(DK[CUR_DK].CUR.prog, NO_PROG,
                        DK[CUR_DK].CUR.prog_faza, DK[CUR_DK].NEXT.faza);

        }
        ////
        /*
        if (DK[CUR_DK].CUR.work == SPEC_PROG)
        {
           //����. ����-> ���� ���������
           if (DK[CUR_DK].NEXT.work == PROG_FAZA)
            Build_Takts(NO_PROG, DK[CUR_DK].NEXT.prog, DK[CUR_DK].CUR.faza,
                        DK[CUR_DK].NEXT.prog_faza);

        }
        */
return (0);
}
/*----------------------------------------------------------------------------*/
// ������������� ��������� ��������� �� ����
int SET_PROM_STATE_LEDS()
{
int prom_indx;

for(int i_napr=0; i_napr< DK[CUR_DK].PROJ->AmountDirects; i_napr++)
  {
  prom_indx = DK[CUR_DK].control.prom_indx[i_napr];
  DK[CUR_DK].control.napr[i_napr] = prom_takts[CUR_DK][i_napr][prom_indx].col;
  }
return (0);
}
/*----------------------------------------------------------------------------*/
// ���������� ��������� �� ��������� ������� �������
int SET_OSN_STATE_LEDS()
{
for (int i_napr=0; i_napr < DK[CUR_DK].PROJ->AmountDirects; i_napr++)
  {
  DK[CUR_DK].control.napr[i_napr] = osn_takt[CUR_DK][i_napr];
  }
return (0);
}
/*----------------------------------------------------------------------------*/
// ���������� ��������� �� ��������� ������� �������
int SET_SPEC_PROG_LEDS()
{
        STATES_LIGHT  fill_col=ALL_OFF;
        //
        switch (DK[CUR_DK].CUR.spec_prog)
        {
           case SPEC_PROG_YF:
           {
              fill_col= YELLOW_FLASH;
              break;
           }
           //
           case SPEC_PROG_OC:
           {
              fill_col= ALL_OFF;
              break;

           }
           //
           case SPEC_PROG_KK:
           {
              fill_col= RED;
              break;

           }
        }//sw
        //
        for (int i_napr=0; i_napr< DK[CUR_DK].PROJ->AmountDirects; i_napr++)
                DK[CUR_DK].control.napr[i_napr] = fill_col;
return (0);
}
/*----------------------------------------------------------------------------*/
// ���������, ���� �� � ������� ��������� ���
bool Check_TVP_En()
{
       TPROGRAM  *proga;
       int   faz_n;
       BOOL   ret_b = false;
       //int   tvp_n;
       //
       DK[CUR_DK].TVP.enabled=false;
       ///
       #ifdef   KURS_DK
            faz_n = PROG_PLAN.AmountFasa;
            proga = &PROG_PLAN;
       #else
            faz_n = DK[CUR_DK].PROJ->Program[DK[CUR_DK].CUR.prog].AmountFasa;
            proga = &DK[CUR_DK].PROJ->Program[DK[CUR_DK].CUR.prog];
       #endif
       /// ������
       for (int tvp_i=0; tvp_i< MaxTVP; tvp_i++)
       {
         DK[CUR_DK].tvp_en[tvp_i] = false;
       }
    for (int f_i=0; f_i < faz_n; f_i++)
        {

           if (proga->fazas[f_i].FasaProperty != FAZA_PROP_FIKS)
           {
             DK[CUR_DK].TVP.enabled=true;
             //
             for (int tvp_i=0; tvp_i< MaxTVP; tvp_i++)
             {
                if (proga->fazas[f_i].tvps[tvp_i].pres)
                {
                  DK[CUR_DK].tvp_faza[tvp_i]=f_i;
                  //
                  DK[CUR_DK].tvp_en[tvp_i] = true;
                  // proga->fazas[f_i].tvps[tvp_i].pres;
                }
             }
             //
             /*
             tvp_n = proga->fazas[f_i].tvps[0].pres;
             if (tvp_n==1)
               DK[CUR_DK].tvp1_faza=f_i;
             //
             if (tvp_n==2)
               DK[CUR_DK].tvp2_faza=f_i;
             ///////////////
             ////////////
             tvp_n = proga->fazas[f_i].tvps[1].pres;
             if (tvp_n==1)
               DK[CUR_DK].tvp1_faza=f_i;
             //
             if (tvp_n==2)
               DK[CUR_DK].tvp2_faza=f_i;
             ///////////////
             */
             ret_b = true;
             //return (true);
           }
        }
return (ret_b);
}
/*----------------------------------------------------------------------------*/
// ��������� �� ������� ������ ����
// ���������� ����� ��������
int  Check_Synhro(bool noKK)
{
      int sec_part=0;
      //
      if ((DK[CUR_DK].CUR.source == PLAN) &&
          (DK[CUR_DK].CUR.spec_prog == SPEC_PROG_YF))
          return 0;
      if (DK[CUR_DK].synhro_mode)
      if (DK[CUR_DK].NEXT.work==PROG_FAZA)
      //if (DK[CUR_DK].CUR.work==PROG_FAZA)
      if (DK[CUR_DK].NEXT.prog_faza==0)
      {
         //������� �����
         int cur_sec = CT.tm_hour*3600 + CT.tm_min*60 + CT.tm_sec;
         // �������� ������ �������� ���������� �����
         cur_sec = cur_sec - DK[CUR_DK].cur_slot_start;
         //������� �����
         sec_part = cur_sec % DK[CUR_DK].control.Tproga;
         // ��������� � ������ ��
         //if (sec_part)
         //   kk = sec_part;
         //if (sec_part==0)
         if (sec_part<3)
         if (noKK)
           return 0;
         // ���� ������ � ����� �������������
         int tim = DK[CUR_DK].control.Tproga - sec_part;
         //if (noKK==false)
             tim=tim- DK[CUR_DK].PROJ->guard.kk_len ;
         //
         if (noKK)
         { // true - ����� � ������� �����, ��� ��
           // ���� ������
           if (abs(tim)<3)
            return 0;
         }
         else
         { // false - ����� � KK, ��� ������
           if (tim==0)
             return 0;
         }
         //������� ������ ����� � ����� ��
         if (tim<0)
            tim = tim + DK[CUR_DK].control.Tproga; //
         DK[CUR_DK].control.len = tim;
         DK[CUR_DK].CUR.spec_prog = SPEC_PROG_YF;
         DK[CUR_DK].CUR.work = SPEC_PROG;
         DK[CUR_DK].CUR.source = PLAN;
         SET_SPEC_PROG_LEDS();
         DK[CUR_DK].control.STA=STA_SPEC_PROG;
         return tim;
      }
return (0);
}
/*----------------------------------------------------------------------------*/
int Correct_Synhro()
{
  /*
    int tim = DK[CUR_DK].control.Tproga - Check_Synhro() -
              DK[CUR_DK].PROJ->guard.kk_len ;
    ////������� ������ ����� � ����� ��
    if (tim<0)
      tim = tim + DK[CUR_DK].control.Tproga; //????????
    ///
    if (tim==0)
       return tim;
    /////
    DK[CUR_DK].control.len = tim;
    DK[CUR_DK].CUR.spec_prog = SPEC_PROG_YF;
    DK[CUR_DK].CUR.work = SPEC_PROG;
    DK[CUR_DK].CUR.source = PLAN;
    SET_SPEC_PROG_LEDS();
    DK[CUR_DK].control.STA=STA_SPEC_PROG;
    return tim;
  */
  return 0;
}
/*----------------------------------------------------------------------------*/
// ���������� ���������
BOOL EQ_States(STATE *sta1, STATE *sta2)
{
       bool res=false;
       //
       if (sta1->work == sta2->work)
       switch (sta1->work)
        {
           case SPEC_PROG:
           {
             if (sta1->spec_prog == sta2->spec_prog)
              res=true;
             break;
           }
           //
           case SINGLE_FAZA:
           {
             if (sta1->faza == sta2->faza)
              res=true;
             break;
           }
           //
           case PROG_FAZA:
           {
             if (sta1->prog == sta2->prog)
             if (sta1->prog_faza == sta2->prog_faza)
               res=true;
             break;
           }
        }
return (res);
}
/*----------------------------------------------------------------------------*/
void Event_Change_Fase()
{
     char buf[128];
     ///
     if (!PROJ[CUR_DK].jornal.faz)
       return;
     ////
        if (DK[CUR_DK].CUR.work == PROG_FAZA)
        {
          snprintf(buf, sizeof(buf), "�����=%d, ����=%d",
             (DK[CUR_DK].CUR.prog+1),(DK[CUR_DK].CUR.prog_faza+1));

        }
        //
        if (DK[CUR_DK].CUR.work == SINGLE_FAZA)
        {
           snprintf(buf, sizeof(buf), "����=%d",
             (DK[CUR_DK].CUR.faza+1));

           //S_W("Oaae. oaca=" + IntToStr(DK[CUR_DK].CUR.faza+1));
        }
        //
        if (DK[CUR_DK].CUR.work == SPEC_PROG)
        {
          //snprintf(buf, sizeof(buf), "SUCCESS:WORK=SPEC_PROG SPEC_PROG=%d",
          //   (DK[CUR_DK].CUR.spec_prog));
          strcpy(buf,"����. ��.=");
          if (DK[CUR_DK].CUR.spec_prog==1)
            strcat(buf,"��");
          ///
          if (DK[CUR_DK].CUR.spec_prog==2)
            strcat(buf,"��");
          ///
        }
        ////
        strcat(buf,". ���=");
        if (DK[CUR_DK].CUR.source==ALARM) strcat(buf,"������");
        if (DK[CUR_DK].CUR.source==TUMBLER) strcat(buf,"�������");

        if (DK[CUR_DK].CUR.source==SERVICE) strcat(buf,"������");
        if (DK[CUR_DK].CUR.source==VPU) strcat(buf,"���");
        if (DK[CUR_DK].CUR.source==TVP) strcat(buf,"���");
        if (DK[CUR_DK].CUR.source==PLAN) strcat(buf,"����");
        //
        strcat(buf,"\n");
        //
        Event_Push_Str(buf);
}
/*----------------------------------------------------------------------------*/
// ���������� ��� ��������� ���������
int CUR_NEXT()
{

                        D_W("CUR_NEXT. Change STATE\n");
                        //
                        if ((TEST_STA(&DK[CUR_DK].CUR, SPEC_PROG, SPEC_PROG_OC,0)) ||
                            (TEST_STA(&DK[CUR_DK].CUR, SPEC_PROG, SPEC_PROG_YF,0)) )
                        //if (DK[CUR_DK].CUR.work==SPEC_PROG)
                        if (DK[CUR_DK].NEXT.work!=SPEC_PROG)
                        {
                           if (DK[CUR_DK].flash)
                           {
                              if (!ligh_load_init())
                                DK_HALT();
                           }
                           //
                           POWER_SET(true);
                           //
                           //if (TEST_STA(&DK[CUR_DK].CUR, SPEC_PROG, SPEC_PROG_YF,0))
                           //{
                              DK[CUR_DK].PLAN.STA=STA_INIT;;
                              MODEL();
                           //}
                           // ������ KK
                           // �� �������� ����������������
                           if (Check_Synhro(false))
                           {
                             //if (Correct_Synhro())
                              return (0);
                           }
                           //
                           memcpy(&DK[CUR_DK].control.start, &CT, sizeof(SYSTEMTIME));
                           TIME_PLUS(&CT, &DK[CUR_DK].control.end, DK[CUR_DK].PROJ->guard.kk_len);
                           //
                           /*
                           if (TEST_STA(&DK[CUR_DK].CUR, SPEC_PROG, SPEC_PROG_YF,0))
                           {
                              DK[CUR_DK].PLAN.STA=STA_INIT;;
                              MODEL();

                           }
                           */
                           ////
                           DK[CUR_DK].control.len = DK[CUR_DK].PROJ->guard.kk_len;
                           DK[CUR_DK].CUR.spec_prog = SPEC_PROG_KK;
                           DK[CUR_DK].CUR.source = PLAN;
                           SET_SPEC_PROG_LEDS();
                           ///
                           return 0;
                        }
                        //
                        if (TEST_STA(&DK[CUR_DK].CUR, SPEC_PROG, SPEC_PROG_KK,0))
                        if (DK[CUR_DK].NEXT.work!=SPEC_PROG)
                        {
                             DK[CUR_DK].PLAN.STA=STA_INIT;;
                             MODEL();
                        }
                        //
                        if (Check_Synhro(true))
                           {
                             //if (Correct_Synhro())
                              return (0);
                           }

                        /*
                        // �������� ����������������
                        if (Check_Synhro())
                        {
                          Correct_Synhro();
                          return (0);
                        }
                        */
                        //
                        Copy_STATES(&DK[CUR_DK].OLD, &DK[CUR_DK].CUR);
                        Copy_STATES(&DK[CUR_DK].CUR, &DK[CUR_DK].NEXT);
                        // �������� ��������� ���������
                        Clear_STATE(&DK[CUR_DK].NEXT);
                        //
                        if (!EQ_States(&DK[CUR_DK].CUR, &DK[CUR_DK].OLD))
                          Event_Change_Fase();
                        //
                        // ������� �� ��?
                        if (TEST_STA(&DK[CUR_DK].CUR, SPEC_PROG, SPEC_PROG_OC,0))
                        {
                           POWER_SET(false);
                        }
                        else
                        {
                          POWER_SET(true);
                        }

                        //memset(&DK[CUR_DK].NEXT,0,sizeof(STATE));
                        //DK[CUR_DK].NEXT.spec_prog = SPEC_PROG_OC;
                        ////
                        //// �������� �����
                        //���������� ��������
                        if (DK[CUR_DK].CUR.source == PLAN)
                            DK[CUR_DK].PLAN.cur.set = true;
                        //
                        if (DK[CUR_DK].CUR.source == TVP)
                            DK[CUR_DK].TVP.cur.set = true;
                        ///
                        if (DK[CUR_DK].CUR.source == VPU)
                            DK[CUR_DK].VPU.cur.set = true;
                        //
                        if (DK[CUR_DK].CUR.source == SERVICE)
                            DK[CUR_DK].SERVICE.cur.set = true;
                        // ���������� - ��� ����� ���������
                        // ���� ����. ����� - ��� ���������
                        if (DK[CUR_DK].CUR.work== SPEC_PROG)
                        {
                          SET_SPEC_PROG_LEDS();
                          DK[CUR_DK].control.STA = STA_SPEC_PROG;
                          //break;
                        }
                        else
                        {
                           Calc_Tc(DK[CUR_DK].CUR.prog);
                           GEN_TAKTS();
                           SET_OSN_STATE_LEDS();
                           DK[CUR_DK].control.STA = STA_OSN_TAKT;
                           //
                           //DK[CUR_DK].control.Tosn =0;
                        }
                        ///////////////////////////
                        // ��������� ����������� ������ ���
                        //DK[CUR_DK].TVP.enabled=false;
                        //if (DK[CUR_DK].CUR.work== PROG_FAZA)
                        //    Check_TVP_En();
                        ////////////////////////////
                        // ��������� ����� ������ ���������
                        memcpy(&DK[CUR_DK].control.start, &CT, sizeof(SYSTEMTIME));
                        // ���������� ����� ���������
                        if ((DK[CUR_DK].CUR.source==PLAN) || (DK[CUR_DK].CUR.source==TVP))
                        {
                        //TIME_PLUS(&CT, &DK[CUR_DK].control.end,
                        //  osn_takt_time);
                        if (TEST_STA(&DK[CUR_DK].CUR, SPEC_PROG, SPEC_PROG_YF,0))
                              osn_takt_time[CUR_DK]=1;
                        //
                        DK[CUR_DK].control.len = osn_takt_time[CUR_DK];
                        }
                        else
                        {
                        // ��� ALARM... - ������������ - �� ����������
                        TIME_PLUS(&CT, &DK[CUR_DK].control.end, 10);
                        //DK[CUR_DK].PROJ->guard.faza_max????
                        DK[CUR_DK].control.len = 1;
                        //���������� �������� ����� � ������ ���
                        if((DK[CUR_DK].CUR.source==VPU)&&(DK[CUR_DK].OLD.source==VPU)){
                          if(DK[CUR_DK].NEXT.faza!=DK[CUR_DK].OLD.faza){
                            const WORD Tmin = DK[CUR_DK].PROJ->Program.fazas[DK[CUR_DK].CUR.prog_faza].Tmin;
                            DK[CUR_DK].control.len = Tmin;
                            // ����� ���������
                            TIME_PLUS(&CT, &DK[CUR_DK].control.end,DK[CUR_DK].control.len);
                            }
                          }
                        }
return (0);
}
/*----------------------------------------------------------------------------*/
// ������������� ��������� ��������� �� ��������� ����
// ������������ ����� ���������, ��������, ��� ��� ��� ������� ��������
// �������� ������������ � ������ �����. ��������� ����� ����� ������ �
// ����������� ��������� �������� ��������� ��-�� ������������ �����
int SET_NEXT_STATE()
{
        D_W("SET_NEXT_STATE()\n");
        // ���������� ��� ����� ���
        if(DK[CUR_DK].CUR.source==TVP){
          if(DK[CUR_DK].NEXT.source==PLAN){
            if(DK[CUR_DK].CUR.prog_faza==DK[CUR_DK].NEXT.prog_faza){
              DK[CUR_DK].NEXT.presence=false;
              DK[CUR_DK].PLAN.cur.set = true;
              GO_PLAN();
              }
            }
          }
        // ���� ����� ���
       if((DK[CUR_DK].CUR.source==VPU)&&(DK[CUR_DK].NEXT.source==PLAN)){
          // ���������� � ������ ����
          DK[CUR_DK].CUR.prog_faza = DK[CUR_DK].NEXT.prog_faza = DK[CUR_DK].CUR.faza;
          DK[CUR_DK].PLAN.cur.prog_faza = DK[CUR_DK].PLAN.next.prog_faza = DK[CUR_DK].CUR.faza; // �������� � ���� ����������� ���
          //������� � ����� ����
          /*DK[CUR_DK].NEXT.presence=false;
          DK[CUR_DK].PLAN.cur.set = true;
          GO_PLAN();*/
          }
        //�������� �� ����� ������ ������ �� �� PLAN
        if (DK[CUR_DK].CUR.source > DK[CUR_DK].NEXT.source)
        {
           D_W("MOre prioritet\n");
           D_W("short time\n");
           //
           if (DK[CUR_DK].CUR.work == SPEC_PROG)
           {
           memcpy(&DK[CUR_DK].control.end, &CT, sizeof(SYSTEMTIME));
           CUR_NEXT();
           }
           else
           {
           if (DK[CUR_DK].control.STA==STA_OSN_TAKT){
            //��������� ����
            const WORD Tmin = DK[CUR_DK].PROJ->Program.fazas[DK[CUR_DK].CUR.prog_faza].Tmin; //DK[CUR_DK].PROJ->Program.fazas[DK[CUR_DK].CUR.prog_faza].Tmin
            if((osn_takt_time[CUR_DK] - DK[CUR_DK].control.len) > Tmin){
              DK[CUR_DK].control.len = 0;
              }
            // ���������� ��� ����� ��� � ���
            if(DK[CUR_DK].CUR.source==PLAN){
              if(DK[CUR_DK].OLD.source==PLAN){
                if(DK[CUR_DK].CUR.prog_faza==DK[CUR_DK].OLD.prog_faza){
                  DK[CUR_DK].control.len = 0;
                  }
                }
              }
            }
           }
        }// end if()
return Null;
}
/*----------------------------------------------------------------------------*/
void Check_Low_Level_Spec_Prog()
{
    static BYTE  req_n;
    req_n = DK[CUR_DK].REQ.prior_req;
    if (req_n == PLAN)
      return;
    //
    if (DK[CUR_DK].REQ.prior_req <= DK[CUR_DK].CUR.source)
    if (DK[CUR_DK].REQ.req[req_n].work == SPEC_PROG){
      //�������� ����. �����
      memcpy(&DK[CUR_DK].control.end, &CT, sizeof(SYSTEMTIME));
      DK[CUR_DK].control.len=0;
      CUR_NEXT();
      }
}
/*----------------------------------------------------------------------------*/
// ������������� ��������� ��.
int CONTROL()
{
        int prom_indx;
        // ������
        if (DK[CUR_DK].control.len)
          DK[CUR_DK].control.len--;
        // �������� ���������� ��������
        Check_Low_Level_Spec_Prog();
        //
        switch (DK[CUR_DK].control.STA)
        {
                case STA_INIT:
                {
                    // ������������� ����� CUR=OS
                    memcpy(&DK[CUR_DK].control.start, &CT, sizeof(SYSTEMTIME));
                    TIME_PLUS(&DK[CUR_DK].control.start, &DK[CUR_DK].control.end, 2);
                    //
                    DK[CUR_DK].control.len = 2;
                    DK[CUR_DK].control.STA = STA_SPEC_PROG;
                    SET_SPEC_PROG_LEDS();
                    ///
                    break;
                }
                case STA_SPEC_PROG:
                {
                   if (TIME_END())
                   {
                      CUR_NEXT();
                      break;
                   }
                   // ������� ����� �������
                   if (DK[CUR_DK].NEXT.presence)
                   {
                        SET_NEXT_STATE();
                   }
                   break;
                }
                //
                case STA_OSN_TAKT:
                {
                   // ������� ����� �������
                   if (DK[CUR_DK].NEXT.presence)
                   {
                        SET_NEXT_STATE();
                   }
                   //
                   if (TIME_END())
                   {
                      //����������� �����
                      //��������� ��������� ���������
                      DK[CUR_DK].NEXT.set = true;
                      GEN_TAKTS();
                      //��� ����. ������ - �������
                      //if (Tprom_len==0)
                      if (Tprom_len[CUR_DK]==0)
                      {
                         CUR_NEXT();
                         break;
                      }
                      //
                      DK[CUR_DK].control.len = Tprom_len[CUR_DK];
                      //
                      //� control.end - ���� ��������� �������� ��������� - ��������� �����
                      memcpy(&DK[CUR_DK].control.start, &DK[CUR_DK].control.end, sizeof(SYSTEMTIME));
                      // ��������� ����. ������
                      TIME_PLUS(&DK[CUR_DK].control.start, &DK[CUR_DK].control.end, Tprom_len[CUR_DK]);
                      //  ������ �������
                      for (int i=0;i < DK[CUR_DK].PROJ->AmountDirects; i++)
                      {
                        DK[CUR_DK].control.prom_indx[i]=0;
                        DK[CUR_DK].control.prom_time[i]=0;
                      }
                      //
                      SET_PROM_STATE_LEDS();
                      DK[CUR_DK].control.STA = STA_PROM_TAKTS;
                   }
                   break;

                }
                //
                case STA_PROM_TAKTS:
                {
                     if (TIME_END())
                     {
                       // ���������
                       CUR_NEXT();
                       break;
                     }
                     //+1s
                     for (int i_n=0; i_n < DK[CUR_DK].PROJ->AmountDirects; i_n++)
                     {
                       DK[CUR_DK].control.prom_time[i_n]++;
                       //
                       prom_indx = DK[CUR_DK].control.prom_indx[i_n];
                       //
                       if ( DK[CUR_DK].control.prom_time[i_n]>=
                            prom_takts[CUR_DK][i_n][prom_indx].time)
                            {
                               DK[CUR_DK].control.prom_indx[i_n]++;
                               DK[CUR_DK].control.prom_time[i_n]=0;

                            }
                     }
                     //
                     SET_PROM_STATE_LEDS();
                     //
                     break;
                }
        }
return (0);
}
/*----------------------------------------------------------------------------*/
// ��������� ��� � 0.5�
unsigned short DK_MAIN()
{
      unsigned short fire_light;
      //char buf[128];
      //
        DS1390_TIME time;
        //
        BOOL true_time = GetTime_DS1390(&time);
        //
        CT.tm_hour = time.hour;
        CT.tm_mday = time.date;
        CT.tm_min = time.min;
        CT.tm_sec = time.sec;
        CT.tm_mon = time.month;
        CT.tm_year = time.year;
        //
        GREEN_PORT=0;
        RED_PORT=0;
        YEL_PORT=0;

        //
        for (int i_dk=0; i_dk<dk_num; i_dk++)
        {
           CUR_DK = i_dk;
           // ���������
           REQUESTS();
           // ������ ������ ��
           MODEL();
           // ����������� - ���������� ����������� ����
           CONTROL();
           //
           fire_light = Update_STATES(false);
        }
        //
        SET_OUTPUTS();
        //
        Clear_UDC_Arrays();
        //
        return (fire_light);
}
/*----------------------------------------------------------------------------*/
/* �������� ������� */
/*----------------------------------------------------------------------------*/
void DK_Service_OS(void)
{
    DK[CUR_DK].REQ.req[SERVICE].spec_prog = SPEC_PROG_OC;
    DK[CUR_DK].REQ.req[SERVICE].work = SPEC_PROG;
    DK[CUR_DK].REQ.req[SERVICE].source = SERVICE;
    DK[CUR_DK].REQ.req[SERVICE].presence = true;
}
/*----------------------------------------------------------------------------*/
void DK_Service_YF(void)
{
    DK[CUR_DK].REQ.req[SERVICE].spec_prog = SPEC_PROG_YF;
    DK[CUR_DK].REQ.req[SERVICE].work = SPEC_PROG;
    DK[CUR_DK].REQ.req[SERVICE].source = SERVICE;
    DK[CUR_DK].REQ.req[SERVICE].presence = true;
}
/*----------------------------------------------------------------------------*/
void DK_Service_undo(void)
{
    //memset(&(DK[CUR_DK].REQ.req[SERVICE]),0,sizeof(STATE));
    Clear_STATE(&(DK[CUR_DK].REQ.req[SERVICE]));
}
/*----------------------------------------------------------------------------*/
void DK_Service_KK(void)
{
    DK[CUR_DK].REQ.req[SERVICE].spec_prog = SPEC_PROG_KK;
    DK[CUR_DK].REQ.req[SERVICE].work = SPEC_PROG;
    DK[CUR_DK].REQ.req[SERVICE].source = SERVICE;
    DK[CUR_DK].REQ.req[SERVICE].presence = true;
}
/*----------------------------------------------------------------------------*/
void DK_Service_faza(unsigned long faz_i)
{
     DK[CUR_DK].REQ.req[SERVICE].work = SINGLE_FAZA;
     DK[CUR_DK].REQ.req[SERVICE].faza = faz_i;
     DK[CUR_DK].REQ.req[SERVICE].source = SERVICE;
     DK[CUR_DK].REQ.req[SERVICE].presence = true;
}