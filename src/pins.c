/*****************************************************************************
*
* � 2014 Cyber-SB. Written by Selyutin Alex.
*
*****************************************************************************/

#include "stellaris.h"
#include "pins.h"
#include "adcc.h"


#include "debug/debug.h"
#include "version.h"
#include "tnkernel/tn.h"
#include "dk/dk.h"
#include "dk/kurs.h"
#include "event/evt_fifo.h"
//#include "pref.h"

//#define GPIO_PA3_GPIO       (GPIO_PA3_SSI0FSS & 0xFFFFFFF0)
//#define GPIO_PB2_GPIO       (GPIO_PB2_I2C0SCL & 0xFFFFFFF0)

#define TVP_LEVEL       3

static TN_TCB task_PINS_tcb;
#define PINS_REFRESH_INTERVAL        80
static unsigned int task_PINS_stk[TASK_PINS_STK_SZ];
//static unsigned char TVP_STATE[MaxTVP];


struct o_pin_nfo
{
    enum o_pin      pin_id;
    unsigned long   periph;
    unsigned long   port;
    unsigned long   pin;
};

struct i_pin_nfo
{
    enum i_pin      pin_id;
    unsigned long   periph;
    unsigned long   port;
    unsigned long   pin;
};
//------------------------------------------------------------------------------
// KURS PINS!!!!!!
// output pin's array GPIOIntTypeSet

static struct o_pin_nfo const g_opins[OPIN_CNT] =
{
    //PA5, 31 pin
    { OPIN_TEST2,     SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_5 },
    //PA4, 30 pin
    { OPIN_TEST1,     SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_4 },
    //PC7, 22 pin, HL1
    { OPIN_CPU_LED,       SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PIN_7 },
    //PH3, 83 pin
    { OPIN_POWER,	SYSCTL_PERIPH_GPIOH, GPIO_PORTH_BASE, GPIO_PIN_3 },
    //PC6, 23 pin, HL2, �� ����� ���� POWER
    { OPIN_ERR_LED,       SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PIN_6 },
    /////////////////////
    //PD2, 12 pin
    { OPIN_SIM900_ON,     SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, GPIO_PIN_2 },
    //PA6, 34 pin
    { OPIN_TR_EN,        SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_6 },
    //PA2, 28 pin
    { OPIN_485_PV,       SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_2 },
    //PF1, 61 pin
    { OPIN_485_RST,      SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_1 },
    //PH6, 62 pin
    { OPIN_DS1390_CS_CLOCK,        SYSCTL_PERIPH_GPIOH, GPIO_PORTH_BASE, GPIO_PIN_6 },
    //PH5, 62 pin
    { OPIN_SST25VFXXX_CS_FLASH,    SYSCTL_PERIPH_GPIOH, GPIO_PORTH_BASE, GPIO_PIN_5 },
    //PF5, 41 pin
    { OPIN_GPS_RESET,    SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_5 },
    //PA3, 29 pin
    { OPIN_Y_LOAD,       SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_3 },
    //PF4, 42 pin
    { OPIN_G_LOAD,       SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_4 },
    //PF0, 47 pin
    { OPIN_R_LOAD,       SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_0 },
    //PB3, 65 pin
    { OPIN_D3,          SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_3 },
    //PB0, 66 pin
    { OPIN_D0,          SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_0 },
    //PB1, 67 pin
    { OPIN_D1,          SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_1 },
    //PB2, 72 pin
    { OPIN_D2,          SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_2 },
    //PB7, 89 pin
    { OPIN_D7,          SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_7 },
    //PB6, 90 pin
    { OPIN_D6,          SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_6 },
    //PB5, 91 pin
    { OPIN_D5,          SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_5 },
    //PB4, 92 pin
    { OPIN_D4,          SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_4 }
};
//------------------------------------------------------------------------------
// input pin's array

static struct i_pin_nfo const g_ipins[IPIN_CNT] =
{

    //PE0, 74 pin
    { IPIN_TVP0,        SYSCTL_PERIPH_GPIOE, GPIO_PORTE_BASE, GPIO_PIN_0 },
    //PE1, 75 pin
    { IPIN_TVP1,        SYSCTL_PERIPH_GPIOE, GPIO_PORTE_BASE, GPIO_PIN_1 },
    //PG7, 36 pin
    { IPIN_GPS_PPS_INT,  SYSCTL_PERIPH_GPIOG, GPIO_PORTG_BASE, GPIO_PIN_7 },
    //PA7, 35 pin
    { IPIN_DS1390_INT,   SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_7 },
    //PC5, 24 pin
    { IPIN_Y_BLINK,   SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PIN_5 },
    //PC4, 25 pin
    { IPIN_OFF,       SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PIN_4 },
    //////////////
    //PJ0, 14 pin
    { IPIN_UC1,     SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, GPIO_PIN_0 },
    //PJ0, 14 pin
    { IPIN_UC2,     SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, GPIO_PIN_1 },
    //PJ0, 14 pin
    { IPIN_UC3,     SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, GPIO_PIN_2 },
    //PJ0, 14 pin
    { IPIN_UC4,     SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, GPIO_PIN_3 },
    //PJ0, 14 pin
    { IPIN_UC5,     SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, GPIO_PIN_4 },
    //PJ0, 14 pin
    { IPIN_UC6,     SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, GPIO_PIN_5 },
    //PJ0, 14 pin
    { IPIN_UC7,     SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, GPIO_PIN_6 },
    //PJ0, 14 pin
    { IPIN_UC8,     SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, GPIO_PIN_7 },
    /// IPIN_PW_CONTR
    { IPIN_PW_CONTR,  SYSCTL_PERIPH_GPIOH, GPIO_PORTH_BASE, GPIO_PIN_7 }
};

// local functions

static void pins_pre_configure();
static void pins_post_configure();

static unsigned char tvp_count[MaxTVP];
//-----------------------------------------------------------------------------
// ������������ ���
void  TVP_IN(BOOL pin_sta, int tvp_num)
{
        if (pin_sta)
        {
           //TVP off
           if (tvp_count[tvp_num])
             tvp_count[tvp_num]--;
           ///
        }
        else
        {
            //TVP ON
           if (tvp_count[tvp_num]<TVP_LEVEL)
           {
               tvp_count[tvp_num]++;
               if (tvp_count[tvp_num]==TVP_LEVEL)
               {
                 for (int i_dk=0; i_dk<DK_N; i_dk++)
                 if (DK[i_dk].REQ.req[TVP].presence==false)
                 {
                    if (DK[i_dk].CUR.source>=TVP)
                    if (DK[i_dk].TVP.enabled)
                    if (DK[i_dk].tvp_en[tvp_num])
                    if (DK[i_dk].CUR.prog_faza!=DK[i_dk].tvp_faza[tvp_num])
                    {
                      if (PROJ[i_dk].jornal.inputs)
                      {
                        if (tvp_num==0)
                         Event_Push_Str("���=1");
                        //
                        if (tvp_num==1)
                         Event_Push_Str("���=2");

                      }
                      //
                      DK[i_dk].REQ.req[TVP].presence =true;
                      DK[i_dk].tvps[tvp_num].pres=1;
                      //DK[CUR_DK].tvp[0].pres=1;
                    }
                 }
                 else
                 {
                   if (DK[i_dk].tvps[tvp_num].pres==0)
                       DK[i_dk].tvps[tvp_num].pres=1;

                 }
                 ////////////
               }
           }

        }
}
//-----------------------------------------------------------------------------
static void task_PINS_func(void* param)
{

    static unsigned char yf_count=0;
    static unsigned char oc_count=0;
    for (int i=0;i<MaxTVP; i++) tvp_count[i]=0;
    //
    for (;;)
    {
        tn_task_sleep(PINS_REFRESH_INTERVAL);
        ////////////////////////////


        ////////////////////////////
        if (!pin_rd(IPIN_Y_BLINK))
        {
           if (yf_count)
             yf_count--;
        }
        else
        {
           if (yf_count<TVP_LEVEL)
           {
               yf_count++;
           }
        }
        //
        if (yf_count>=TVP_LEVEL)
        {
          for (int i_dk=0; i_dk<DK_N; i_dk++)
            DK[i_dk].YF = true;
        }
        else
        {
          for (int i_dk=0; i_dk<DK_N; i_dk++)
            DK[i_dk].YF = false;
        }
        ////////////////////////////
        ////////////////////////////
        if (!pin_rd(IPIN_OFF))
        {
           if (oc_count)
             oc_count--;
        }
        else
        {
           if (oc_count<TVP_LEVEL)
           {
               oc_count++;
           }
        }
        //
        if (oc_count==TVP_LEVEL)
        {
          for (int i_dk=0; i_dk<DK_N; i_dk++)
           DK[i_dk].OS = true;
        }
        else
        {
          for (int i_dk=0; i_dk<DK_N; i_dk++)
           DK[i_dk].OS = false;
        }
        ///////////////////////////////
      for (int i_dk=0; i_dk<DK_N; i_dk++)
      {
          if (DK[i_dk].YF || DK[i_dk].OS)
            DK[i_dk].tumbler=true;
          else
            DK[i_dk].tumbler = false;
          ////////////////////
          if (DK[i_dk].tumbler)
          {
            // ������ �������
            DK[i_dk].tvps[0].pres = false;
            DK[i_dk].tvps[1].pres = false;
            DK[i_dk].REQ.req[TVP].presence =false;
            //////
            if (DK[i_dk].OS)
            {
              DK[i_dk].REQ.req[TUMBLER].spec_prog = SPEC_PROG_OC;
              DK[i_dk].REQ.req[TUMBLER].work = SPEC_PROG;
              DK[i_dk].REQ.req[TUMBLER].source = TUMBLER;
              DK[i_dk].REQ.req[TUMBLER].presence = true;
              //pin_off(OPIN_POWER);

              //DK_Service_OS();
            }
            else
              if (DK[i_dk].YF)
              {
                DK[i_dk].REQ.req[TUMBLER].spec_prog = SPEC_PROG_YF;
                DK[i_dk].REQ.req[TUMBLER].work = SPEC_PROG;
                DK[i_dk].REQ.req[TUMBLER].source = TUMBLER;
                DK[i_dk].REQ.req[TUMBLER].presence = true;
                //DK_Service_YF();
              }
          }
          else
          {
             DK[i_dk].REQ.req[TUMBLER].presence = false;
             //
            //DK_Service_undo();
          }
        }
      /////////////////////


        ////////////////////////////////IPIN_OFF
        TVP_IN(pin_rd(IPIN_TVP0),0);
        TVP_IN(pin_rd(IPIN_TVP1),1);
        /////////////////
          for (int i_dk=0; i_dk<DK_N; i_dk++)
          {
            if (DK[i_dk].CUR.source==PLAN)
            if (DK[i_dk].tvps[0].pres ||
                DK[i_dk].tvps[1].pres)
             DK[i_dk].REQ.req[TVP].presence =true;
           }
          ////////////////

        ////////////////
        /*
        if (pin_rd(IPIN_TVP0))
        //if (pin_rd(IPIN_OFF))
        {
           //TVP off
           if (tvp_count[0])
             tvp_count[0]--;
           ///
        }
        else
        {
            //TVP ON
           if (tvp_count[0]<TVP_LEVEL)
           {
               tvp_count[0]++;
               if (tvp_count[0]==TVP_LEVEL)
               {
                 for (int i_dk=0; i_dk<DK_N; i_dk++)
                 if (DK[i_dk].REQ.req[TVP].presence==false)
                 {
                    //if (DK[CUR_DK].CUR.source==PLAN)
                    if (DK[i_dk].TVP.enabled)
                    if (DK[i_dk].CUR.prog_faza!=DK[i_dk].tvp1_faza)
                    {
                      if (PROJ[i_dk].jornal.inputs)
                        Event_Push_Str("���=1");
                      //
                      DK[i_dk].REQ.req[TVP].presence =true;
                      DK[i_dk].tvps[0].pres=1;
                      //DK[CUR_DK].tvp[0].pres=1;
                    }
                 }
                 else
                 {
                   if (DK[i_dk].tvps[0].pres==0)
                     DK[i_dk].tvps[0].pres=1;

                 }
                 ////////////
               }
           }

        }
        ////////////////////////////////
        if (pin_rd(IPIN_TVP1))
        {
           //TVP off
           if (tvp_count[1])
             tvp_count[1]--;
           ///
        }
        else
        {
            //TVP ON
           if (tvp_count[1]<TVP_LEVEL)
             tvp_count[1]++;
           {
              if (tvp_count[1]==TVP_LEVEL)
              {
                for (int i_dk=0; i_dk<DK_N; i_dk++)
                if (DK[i_dk].REQ.req[TVP].presence==false)
                {
                  //if (DK[CUR_DK].CUR.source==PLAN)
                  if (DK[i_dk].TVP.enabled)
                  if (DK[i_dk].CUR.prog_faza!=DK[i_dk].tvp2_faza)
                  {
                    if (PROJ[i_dk].jornal.inputs)
                     Event_Push_Str("���=2");
                    //
                    DK[i_dk].REQ.req[TVP].presence=true;
                    //DK[CUR_DK].tvps.pres=2;
                    DK[i_dk].tvps[1].pres=2;
                  }
                }
                else
                 {
                   if (DK[i_dk].tvps[1].pres==0)
                     DK[i_dk].tvps[1].pres=2;

                 }
                 ////////////
              }
           }
        }
        */
        ///////////////////////////////
        // ����������� TVP Req
        /*
        for (int i_dk=0; i_dk<DK_N; i_dk++)
        if (DK[i_dk].CUR.source==PLAN)
        if ((DK[i_dk].tvps[0].pres) || (DK[i_dk].tvps[1].pres))
        {
              DK[i_dk].REQ.req[TVP].presence=true;
        }
        */
        /////////////////
        ///////////////

    }
}
//-----------------------------------------------------------------------------
void pins_init()
{
    dbg_printf("Initializing GPIO...");
    pins_pre_configure();

    // init output pin's
    for (int i = 0; i < OPIN_CNT; ++i)
    {
        struct o_pin_nfo const* const p = &g_opins[i];
        MAP_SysCtlPeripheralEnable(p->periph);

        MAP_GPIOPinTypeGPIOOutput(p->port, p->pin);
        pin_off(p->pin_id);

    }

    // init input pin's
    for (int i = 0; i < IPIN_CNT; ++i)
    {
        struct i_pin_nfo const* const p = &g_ipins[i];
        MAP_SysCtlPeripheralEnable(p->periph);
        MAP_GPIOPinTypeGPIOInput(p->port, p->pin);
    }

    pins_post_configure(); // set default states for pins
    dbg_puts("[done]");
    //
    Clear_LED();
    SET_OUTPUTS();
    ///
    if (tn_task_create(&task_PINS_tcb, &task_PINS_func, TASK_PINS_PRI,
        &task_PINS_stk[TASK_PINS_STK_SZ - 1], TASK_PINS_STK_SZ, 0,
        TN_TASK_START_ON_CREATION) != TERR_NO_ERR)
    {
        dbg_puts("tn_task_create(&task_PINS_tcb) error");
        goto err;
    }

     return;
err:
    dbg_trace();
    tn_halt();


}
//------------------------------------------------------------------------------
void Write_OUT(BYTE dat)
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
// ����� � �������
void  SET_OUTPUTS_ZERO()
{
  //OPIN_TR_EN

  pin_off(OPIN_R_LOAD);
  pin_off(OPIN_G_LOAD);
  pin_off(OPIN_Y_LOAD);

    //OPIN_R_LOAD
    Write_OUT(0);
    //
    pin_on(OPIN_R_LOAD );
    //wait
    pin_off(OPIN_R_LOAD );
    ///////////////////////
    Write_OUT(0);
    //
    pin_on(OPIN_Y_LOAD );
    //wait
    pin_off(OPIN_Y_LOAD );
    //////////////////////
    Write_OUT(0);
    //
    pin_on(OPIN_G_LOAD );
    //wait
    pin_off(OPIN_G_LOAD );
    ///


}

//------------------------------------------------------------------------------

void opins_init()
{
    // init output pin's
    for (int i = 0; i < OPIN_CNT; ++i)
    {
        struct o_pin_nfo const* const p = &g_opins[i];
        MAP_SysCtlPeripheralEnable(p->periph);
        MAP_GPIOPinTypeGPIOOutput(p->port, p->pin);
        pin_off(p->pin_id);
    }
    ///
   //SET_OUTPUTS_ZERO();
    pin_on(OPIN_TR_EN );


}



void opins_free()
{
    // free output pin's
    for (int i = 0; i < OPIN_CNT; ++i)
    {
        struct o_pin_nfo const* const p = &g_opins[i];
        MAP_SysCtlPeripheralDisable(p->periph);
    }
}

void pin_on(enum o_pin pin)
{
    struct o_pin_nfo const* const p = &g_opins[pin];
    MAP_GPIOPinWrite(p->port, p->pin, p->pin);
}

void pin_off(enum o_pin pin)
{
    struct o_pin_nfo const* const p = &g_opins[pin];
    MAP_GPIOPinWrite(p->port, p->pin, 0);
}

BOOL pin_rd(enum i_pin pin)
{
    struct i_pin_nfo const* const p = &g_ipins[pin];
    return (MAP_GPIOPinRead(p->port, p->pin) & p->pin) != 0;
}
// local functions
static void pins_pre_configure()
{
  // ��� B7 - ���������! ��� ������������� ��� ���� ���������!
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    HWREG(GPIO_PORTB_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
    HWREG(GPIO_PORTB_BASE + GPIO_O_CR) = GPIO_PIN_7;
}

static void pins_post_configure()
{
    //struct i_pin_nfo const* ip;
    ///
    //static int j;
    //pin_on(OPIN_POWER);
    pin_on(OPIN_SST25VFXXX_CS_FLASH);
    pin_on(OPIN_DS1390_CS_CLOCK);
    //pin_on(OPIN_DIGI_CS_868);
    //pin_on(OPIN_DIGI_CS_ZB);
    //////////////
    // input pin's
    for (int i = 0; i < IPIN_CNT; ++i)
    {
        struct i_pin_nfo const*  ip = &g_ipins[i];
        MAP_SysCtlPeripheralEnable(ip->periph);
        GPIOPinTypeGPIOInput(ip->port, ip->pin);
        MAP_GPIOPadConfigSet(ip->port, ip->pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    }
    ///
    /*
    MAP_IntEnable(INT_GPIOJ);
    //// ������ INT
    for (int i = IPIN_UC1; i < IPIN_CNT; ++i)
    {
        struct i_pin_nfo const*  p = &g_ipins[i];
        //
        GPIOIntTypeSet(p->port, p->pin, GPIO_BOTH_EDGES);
        GPIOPinIntEnable(p->port, p->pin);
        //GPIOPortIntRegister(p->port, IntGPIO_U);
    }
    //
     MAP_IntPrioritySet(INT_GPIOJ,0x42); // low priority
    ///
    MAP_IntEnable(INT_GPIOH);
    //
    GPIOIntTypeSet(GPIO_PORTH_BASE, GPIO_PIN_7, GPIO_BOTH_EDGES);
    GPIOPinIntEnable(GPIO_PORTH_BASE, GPIO_PIN_7);
    //
    MAP_IntPrioritySet(INT_GPIOH,0x35); // low priority
    */
    //////////////////
    /*
    // button IPIN_TVP0 set pull-up
    ip = &g_ipins[IPIN_TVP0];
    MAP_GPIOPadConfigSet(ip->port, ip->pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // button IPIN_TVP1 set pull-up
    ip = &g_ipins[IPIN_TVP1];
    MAP_GPIOPadConfigSet(ip->port, ip->pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    ip = &g_ipins[IPIN_Y_BLINK];
    MAP_GPIOPadConfigSet(ip->port, ip->pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    ip = &g_ipins[IPIN_OFF];
    MAP_GPIOPadConfigSet(ip->port, ip->pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    //ip = &g_ipins[IPIN_TEST1];
    //MAP_GPIOPadConfigSet(ip->port, ip->pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    ip = &g_ipins[IPIN_GPS_PPS_INT];
    MAP_GPIOPadConfigSet(ip->port, ip->pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    //

    ip = &g_ipins[IPIN_DS1390_INT];
    MAP_GPIOPadConfigSet(ip->port, ip->pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    //
    */

}
