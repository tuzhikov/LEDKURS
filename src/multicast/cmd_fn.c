/*****************************************************************************
*
* � 2014 Cyber-SB. Written by Selyutin Alex.
*
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../stellaris.h"
#include "../tnkernel/tn.h"
#include "../lwip/lwiplib.h"
#include "../memory/firmware.h"
#include "../memory/memory.h"
#include "../pins.h"
#include "../pref.h"
#include "../adcc.h"
#include "../version.h"
#include "cmd_ch.h"
#include "cmd_fn.h"
#include "../mem_map.h"
#include "../crc32.h"
#include "../adcc.h"
#include "../gps/gps.h"
#include "../memory/ds1390.h"
#include "../light/light.h"
#include "../utime.h"
#include "../dk/dk.h"
#include "../event/evt_fifo.h"
#include "../utime.h"
#include "../debug/debug.h"

#ifdef KURS_DK
  #include "../dk/kurs.h"
#endif

#define ERR_STR_WRONG_PARAM     "ERR: Wrong parameters\n"

static struct eth_addr          g_hwaddr;
static enum net_mode            g_net_mode;
static struct ip_addr           g_ipaddr, g_ipmsk, g_ipgw, g_cmd_ip;
static unsigned                 g_cmd_port;

// local functions

static void process_get_cmd(struct cmd_raw* cmd_p, char const* par,int argc, char** argv);
static void process_set_cmd(struct cmd_raw* cmd_p, char const* par, char const* val);
static int  hex2buf(unsigned char* buf, unsigned len, char const* hex);
static int  buf2hex(char* hex, unsigned hex_len, unsigned char* buf, unsigned len);
static int  hex2char(int hex);
static void char2hex(char hex[2], int ch);

static err_t udp_send_success(struct cmd_raw* cmd_p);
static err_t udp_send_not_enough_mem(struct cmd_raw* cmd_p);
static err_t udp_send_wrong_par(struct cmd_raw* cmd_p);
static err_t udp_send_GPS_Info(struct cmd_raw* cmd_p);
static err_t udp_send_GPS_Time(struct cmd_raw* cmd_p);
static err_t udp_send_DS1390_time(struct cmd_raw* cmd_p);
static err_t udp_send_Light(struct cmd_raw* cmd_p);
static err_t udp_send_info(struct cmd_raw* cmd_p);
static err_t udp_send_tvp(struct cmd_raw* cmd_p);
static err_t udp_send_state(struct cmd_raw* cmd_p, int argc, char** argv);
static err_t udp_send_event(struct cmd_raw* cmd_p);
static err_t udp_send_ver(struct cmd_raw* cmd_p);
static err_t udp_send_lines(struct cmd_raw* cmd_p);
static void udp_send_sensors(struct cmd_raw* cmd_p);


static err_t udp_send_config(struct cmd_raw* cmd_p);

static struct cmd_raw debug_cmd_p;
static BOOL debugee_flag=false;
//
void debugee(char const* s)
{
    if (debugee_flag)
      udp_sendstr(&debug_cmd_p, s);
}
/*----------------------------------------------------------------------------*/
void cmd_debugee_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
  char buf[512];
  int i_page;
  //
  if (sscanf(argv[0], "%u", &i_page) == 1)
  {
    //memcpy(buf,&DBG_MEM[500*i_page],500);
    dbg_get_str(i_page, buf);
    //udp_sendstr(cmd_p, "DEBUGEE Set. DK WORK!!!...\n");
    udp_sendstr(cmd_p, buf);
    //tn_reset();
  }
}
/*----------------------------------------------------------------------------*/
void cmd_ifconfig_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    if (argc != 0)
    {
        udp_send_wrong_par(cmd_p);
        return;
    }

    get_hwaddr(&g_hwaddr);
    get_ipaddr(&g_ipaddr);
    get_ipmask(&g_ipmsk);
    get_ipgw(&g_ipgw);
    get_cmd_ch_ip(&g_cmd_ip);
    g_net_mode  = get_net_mode();
    g_cmd_port  = (u16_t)pref_get_long(PREF_L_CMD_PORT);

    udp_send_config(cmd_p);
}
/*----------------------------------------------------------------------------*/
void cmd_config_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    if (argc != 0)
    {
        udp_send_wrong_par(cmd_p);
        return;
    }

    if (!hwaddr_load(&g_hwaddr))
        memset(&g_hwaddr.addr, 0xFF, sizeof(g_hwaddr.addr));
    g_ipaddr.addr   = pref_get_long(PREF_L_NET_IP);
    g_ipmsk.addr    = pref_get_long(PREF_L_NET_MSK);
    g_ipgw.addr     = pref_get_long(PREF_L_NET_GW);
    g_cmd_ip.addr   = pref_get_long(PREF_L_CMD_SRV_IP);
    g_net_mode      = (enum net_mode)pref_get_long(PREF_L_NET_MODE);
    g_cmd_port      = pref_get_long(PREF_L_CMD_PORT);

    udp_send_config(cmd_p);
}

void cmd_get_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    if (argc < 1)
    {
        udp_send_wrong_par(cmd_p);
        return;
    }

    process_get_cmd(cmd_p, argv[0], argc, argv);
}

void cmd_set_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    if (argc != 2)
    {
        udp_send_wrong_par(cmd_p);
        return;
    }

    process_set_cmd(cmd_p, argv[0], argv[1]);
}
/*----------------------------------------------------------------------------*/
void cmd_light_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    ///
/*
    if (strcmp(argv[0], "OS") == 0) //
    {
        DK[CUR_DK].REQ.req[SERVICE].spec_prog = SPEC_PROG_OC;
        DK[CUR_DK].REQ.req[SERVICE].work = SPEC_PROG;
        DK[CUR_DK].REQ.req[SERVICE].source = SERVICE;

        DK[CUR_DK].REQ.req[SERVICE].presence = true;
        udp_send_success(cmd_p);

    }
    else if (strcmp(argv[0], "KK") == 0)
    {
        DK[CUR_DK].REQ.req[SERVICE].spec_prog = SPEC_PROG_KK;
        DK[CUR_DK].REQ.req[SERVICE].work = SPEC_PROG;
        DK[CUR_DK].REQ.req[SERVICE].source = SERVICE;
        //
        DK[CUR_DK].REQ.req[SERVICE].presence = true;
        REQUESTS();
        MODEL();
        //
           udp_send_success(cmd_p);
    }
    else if (strcmp(argv[0], "YF") == 0)
    {
        DK[CUR_DK].REQ.req[SERVICE].spec_prog = SPEC_PROG_YF;
        DK[CUR_DK].REQ.req[SERVICE].work = SPEC_PROG;
        DK[CUR_DK].REQ.req[SERVICE].source = SERVICE;
        //
        DK[CUR_DK].REQ.req[SERVICE].presence = true;
        REQUESTS();
        MODEL();
        //
           udp_send_success(cmd_p);
    }
    else if (strcmp(argv[0], "undo") == 0)
    {
          memset(&(DK[CUR_DK].REQ.req[SERVICE]),0,sizeof(STATE));
          udp_send_success(cmd_p);
    }
    else if (strcmp(argv[0], "faza") == 0)
    {

       unsigned long faz_i;
       //
       if (sscanf(argv[1], "%d", &faz_i) == 1)
       {
          DK[CUR_DK].REQ.req[SERVICE].work = SINGLE_FAZA;
          if (faz_i)
            faz_i--;
          //
            DK[CUR_DK].REQ.req[SERVICE].faza = faz_i;
          DK[CUR_DK].REQ.req[SERVICE].source = SERVICE;
          //
          DK[CUR_DK].REQ.req[SERVICE].presence = true;
          udp_send_success(cmd_p);
       }
    }
  */
    //
    udp_send_success(cmd_p);
    //process_light_cmd(cmd_p, argv[0], argv[1]);
}
/*----------------------------------------------------------------------------*/
void cmd_reboot_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    udp_sendstr(cmd_p, "SUCCESS: Rebooting...\n");
    tn_reset();
}
/*----------------------------------------------------------------------------*/
void cmd_prefReset_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
  if (!DK[0].tumbler)
  {
      udp_sendstr(cmd_p, "NO TUMBLER!!");
      return;
  }
  //
    if (argc == 1)
    {
        unsigned par;
        if ((sscanf(argv[0], "%u", &par) == 1) && par ==255)
        {
            udp_sendstr(cmd_p, "SUCCESS: FLASH was Erased...\n");
            flash_erase();
            return;
        }
    }
  //
    udp_send_wrong_par(cmd_p);
}
/*----------------------------------------------------------------------------*/
void cmd_event_func(struct cmd_raw* cmd_p, int argc, char** argv)
{

    char buf[128];
    struct evt_fifo_item_t  it;
    struct tm  time;
    BOOL res=FALSE;
    unsigned long pos;
    ///
    if (strcmp(argv[0], "get") == 0) //
    {
        if (argc==2)
        {
          // ������ ������
          if (sscanf(argv[1], "%u", &pos)==1)
            res = evt_fifo_get_direct(pos,&it);

        }
        else
        {
          res = evt_fifo_get(&it);
        }
        ///

        if (res==false)
        {
           strcpy(buf,"SUCCESS:+++NO events!!!");
           udp_sendstr(cmd_p, buf);
           return;
        }
         /////
        gmtime(it.ts, &time);
        it.crc=0;

        snprintf(buf, sizeof(buf), "SUCCESS:%02d.%02d.%d-%02d:%02d:%02d %s\n",
        time.tm_mday, time.tm_mon, time.tm_year,
        time.tm_hour, time.tm_min, time.tm_sec, it.dat);
        //
        udp_sendstr(cmd_p, buf);
        return;
    }
    else if (strcmp(argv[0], "init") == 0)
    {
        //evt_find_pointers();
        Event_Check_Pointers();

    }
    else if (strcmp(argv[0], "used") == 0)
    {
      int i = evt_fifo_used();
      snprintf(buf, sizeof(buf), "SUCCESS:%d",  i);
      udp_sendstr(cmd_p, buf);
      return;
    }
    else if (strcmp(argv[0], "erase") == 0)
    {
      strcpy(buf,"SUCCESS:START CLEAR FLASH");
      evt_fifo_clear();
      strcpy(buf,"SUCCESS:FLASH ERASE OK");
    }
    udp_send_success(cmd_p);
}
/*----------------------------------------------------------------------------*/
void cmd_test_func(struct cmd_raw* cmd_p, int argc, char** argv)
{

    if (argc != 1)
    {
        udp_send_wrong_par(cmd_p);
        return;
    }

    unsigned timeout;
    if (sscanf(argv[0], "%u", &timeout) == 1)
    {
        //pref_set_long(PREF_L_ORION_TX_TIMEOUT, timeout);
        //orion_reload_pref();
        udp_send_success(cmd_p);
    }
}
/*----------------------------------------------------------------------------*/
// ��������� ���������  �������
void cmd_chan_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    bool b_ch;
    ///
    if (!DK[0].test)
      return;
    ////
    if (strcmp(argv[2], "on") == 0)
      b_ch=true;
    else
    if (strcmp(argv[2], "off") == 0)
      b_ch=false;
    else
       return ;
    ////////
    unsigned group;
    unsigned char col = argv[1][0];
    unsigned char col_n;
    if (sscanf(argv[0], "%u", &group) == 1)
    {
        switch (col)
        {
          case 'r': col_n=0; break;
          case 'y': col_n=1; break;
          case 'g': col_n=2; break;
          default:  return; break;
        }
        ///
        Clear_LED();
        Set_LED(group,col_n,  b_ch);
        SET_OUTPUTS();
        //tn_task_sleep(1000);
        //
        //Set_LED(group,col_n,false);
        //SET_OUTPUTS();
        //tn_task_sleep(500);
        ///
        udp_send_success(cmd_p);
    }
}
/*----------------------------------------------------------------------------*/
void cmd_flashwr_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    static unsigned char buf[FLASH_MAX_PKT_LEN];
    static unsigned char buf1[FLASH_MAX_PKT_LEN];
    static unsigned long crc1, crc2;
    unsigned short p_try;

    unsigned long addr, len;
    int conv_len;

    if (argc != 3)
        goto err;

    if (!(sscanf(argv[0], "%08X", &addr) == 1 || sscanf(argv[0], "0x%08X", &addr) == 1))
        goto err;

    if (sscanf(argv[1], "%u", &len) != 1)
        goto err;

    conv_len = hex2buf(buf, sizeof(buf), argv[2]);
    if (len != conv_len)
        goto err;

    crc1 = crc32_calc((unsigned char*)buf, len);

    p_try = 0;
    while (p_try++ < 5)
    {
        flash_wr_direct(addr, buf, len);

        flash_rd_direct(addr, buf1, len);

        crc2 = crc32_calc((unsigned char*)buf1, len);

        if (crc1 == crc2)
            break;
    }
    if (p_try >= 5)
        goto err1;

    DK[0].flash = true;
    //
    udp_send_success(cmd_p);
    return;

err:
    snprintf((char*)buf, sizeof(buf), ERR_STR_WRONG_PARAM "flashwr hexADDR decLEN hexDATA(max %u bytes)\n", FLASH_MAX_PKT_LEN);
    udp_sendstr(cmd_p, (char*)buf);
    return;

err1:
    snprintf((char*)buf, sizeof(buf), ERR_STR_WRONG_PARAM "can't write data to flash\n");
    udp_sendstr(cmd_p, (char*)buf);
    return;
}
/*----------------------------------------------------------------------------*/
void cmd_pflashwr_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    if (DK[0].tumbler)
    {
      cmd_flashwr_func(cmd_p, argc,argv);
    }
    else
      udp_sendstr(cmd_p, "NO TUMBLER!!");

}
/*----------------------------------------------------------------------------*/
// ���������������� �������
void cmd_proj_flashwr_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
  char* l_argv[3];
  char s_addr[10];
  int addr;
  //////////////
  if (!DK[0].tumbler)
  {
      udp_sendstr(cmd_p, "NO TUMBLER!!");
      return;
  }
  //////////////
  if (!(sscanf(argv[0], "%08X", &addr) == 1 || sscanf(argv[0], "0x%08X", &addr) == 1))
  {
    udp_sendstr(cmd_p, "Error: Incorrect address");
    return;
    //goto err;
  }
  ///
  // � addr  -������ ��������. ��������� ������� �����
  addr += get_region_start(FLASH_PROGRAM);
  ///////////////////
  snprintf((char*)s_addr, sizeof(s_addr), "%#X",addr);
  l_argv[0] = s_addr;
  //
  l_argv[1] = argv[1];
  l_argv[2] = argv[2];
  // ���� �����==0 - ������ ����
  if (argv[1][0]=='0')
  {
    udp_send_success(cmd_p);
  }
  else
  {
    cmd_flashwr_func(cmd_p, 3, l_argv);
  }
}
/*----------------------------------------------------------------------------*/
// ���������������� ��������
void cmd_progs_flashwr_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
  char* l_argv[3];
  char s_addr[10];
  int addr;
  //////////////
  if (!DK[0].tumbler)
    {
      udp_sendstr(cmd_p, "NO TUMBLER!!");
      return;
    }
    //////////////
   //////////////
  if (!(sscanf(argv[0], "%08X", &addr) == 1 || sscanf(argv[0], "0x%08X", &addr) == 1))
  {
    udp_sendstr(cmd_p, "Error: Incorrect address");
    return;
    //goto err;
  }
  ///
  // � addr  -������ ��������. ��������� ������� �����
  addr += get_region_start(FLASH_PROGS);
  ///////////////////
  snprintf((char*)s_addr, sizeof(s_addr), "%#X",addr);
  l_argv[0] = s_addr;
  //
  l_argv[1] = argv[1];
  l_argv[2] = argv[2];
  // ���� �����==0 - ������ ����
  if (argv[1][0]=='0')
  {
    udp_send_success(cmd_p);
  }
  else
  {
    cmd_flashwr_func(cmd_p, 3, l_argv);
  }
}
/*----------------------------------------------------------------------------*/
void cmd_flashrd_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    static unsigned char buf[FLASH_MAX_PKT_LEN];
    static char          bufx[FLASH_MAX_PKT_LEN * 2 + 1 + 8];
    unsigned long        addr, len;
    int                  conv_len;

    if (argc != 2)
        goto err;

    if (!(sscanf(argv[0], "%08X", &addr) == 1 || sscanf(argv[0], "0x%08X", &addr) == 1))
        goto err;

    if (sscanf(argv[1], "%u", &len) != 1 || len > FLASH_MAX_PKT_LEN )
        goto err;

    flash_rd_direct(addr, buf, len);
    //
    strcpy(bufx,"SUCCESS:");
    //
    conv_len = buf2hex(&bufx[8], sizeof(bufx), buf, len);
    if (conv_len != len * 2)
        goto err;

    //bufx[conv_len++] = '\n';
    udp_sendbuf(cmd_p, bufx, conv_len+8);
    return;

err:
    udp_sendstr(cmd_p, ERR_STR_WRONG_PARAM "flashrd hexADDR decLEN\n");

}
/*----------------------------------------------------------------------------*/
// ������ �������
void cmd_proj_flashrd_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
  char* l_argv[3];
  char s_addr[10];
  int addr;
  //////////////
  if (!(sscanf(argv[0], "%08X", &addr) == 1 || sscanf(argv[0], "0x%08X", &addr) == 1))
  {
    udp_sendstr(cmd_p, "Error: Incorrect address");
    return;
    //goto err;
  }
  ///
  // � addr  -������ ��������. ��������� ������� �����
  addr += get_region_start(FLASH_PROGRAM);
  ///////////////////
  snprintf((char*)s_addr, sizeof(s_addr), "%#X",addr);
  l_argv[0] = s_addr;
  //
  l_argv[1] = argv[1];
  //l_argv[2] = argv[2];
  // ���� �����==0 - ������ ����
  if (argv[1][0]=='0')
  {
    udp_send_success(cmd_p);
  }
  else
  {
    cmd_flashrd_func(cmd_p, 2, l_argv);
  }
}
/*----------------------------------------------------------------------------*/
// ���������������� ��������
void cmd_progs_flashrd_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
  char* l_argv[3];
  char s_addr[10];
  int addr;
  //////////////
  if (!(sscanf(argv[0], "%08X", &addr) == 1 || sscanf(argv[0], "0x%08X", &addr) == 1))
  {
    udp_sendstr(cmd_p, "Error: Incorrect address");
    return;
    //goto err;
  }
  ///
  // � addr  -������ ��������. ��������� ������� �����
  addr += get_region_start(FLASH_PROGS);
  ///////////////////
  snprintf((char*)s_addr, sizeof(s_addr), "%#X",addr);
  l_argv[0] = s_addr;
  //
  l_argv[1] = argv[1];
  //l_argv[2] = argv[2];
  // ���� �����==0 - ������ ����
  if (argv[1][0]=='0')
  {
    udp_send_success(cmd_p);
  }
  else
  {
    cmd_flashrd_func(cmd_p, 2, l_argv);
  }
}
/*----------------------------------------------------------------------------*/
void cmd_flash_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    static unsigned char buf[FIRMWARE_MAX_PKT_LEN];
    static unsigned char buf1[FIRMWARE_MAX_PKT_LEN];
    static unsigned long crc1, crc2;
    unsigned short p_try;

    unsigned long addr, len;
    int conv_len;

    if (argc != 3)
        goto err;

    if (!(sscanf(argv[0], "%08X", &addr) == 1 || sscanf(argv[0], "0x%08X", &addr) == 1))
        goto err;

    if (sscanf(argv[1], "%u", &len) != 1)
        goto err;

    conv_len = hex2buf(buf, sizeof(buf), argv[2]);
    if (len != conv_len)
        goto err;

    crc1 = crc32_calc((unsigned char*)buf, len);

    p_try = 0;
    while (p_try++ < 5)
    {
        if (firmware_write(addr, buf, len) != FIRMWARE_ERR_NO_ERR)
            goto err;

        firmware_read(addr, buf1, len);

        crc2 = crc32_calc((unsigned char*)buf1, len);

        if (crc1 == crc2)
            break;
    }
    if (p_try >= 5)
        goto err;

    if (addr <= VTABLE_SZ && addr+len >= VTABLE_SZ)
    {
        if (!firmware_ver_chek())
            goto err1;
    }


    udp_send_success(cmd_p);
    return;

err:
    snprintf((char*)buf, sizeof(buf), ERR_STR_WRONG_PARAM "flash hexADDR decLEN hexDATA(max %u bytes)\n", FIRMWARE_MAX_PKT_LEN);
    udp_sendstr(cmd_p, (char*)buf);
    return;
err1:
    udp_sendstr(cmd_p, (char*)"ERR: Version major error");

}
/*----------------------------------------------------------------------------*/
void cmd_flashNFO_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    unsigned long crc32, fw_len;
    unsigned long crc32_real;

    if (argc != 2)
        goto err;

    if (!(sscanf(argv[0], "%08X", &crc32) == 1 || sscanf(argv[0], "0x%08X", &crc32) == 1))
        goto err;

    if (sscanf(argv[1], "%u", &fw_len) != 1)
        goto err;

    if (!firmware_ver_chek())
        goto err1;

    crc32_real = crc32_calc((unsigned char*)(MEM_FW_BUFFER_PTR + FIRMWARE_HDR_SIZE), fw_len);
    if (crc32_real !=  crc32)
        goto err;
    if (firmware_write_nfo(crc32, fw_len) != FIRMWARE_ERR_NO_ERR)
        goto err;

    udp_send_success(cmd_p);
    return;

err:
    udp_sendstr(cmd_p, ERR_STR_WRONG_PARAM "flashNFO hexCRC32 decLEN\n");
    return;
err1:
    udp_sendstr(cmd_p, (char*)"ERR: Version major error");
}
/*----------------------------------------------------------------------------*/
void udp_send_linePWM(struct cmd_raw* cmd_p)
{
    ADC_DATA adc;
    //
    struct pbuf* buf_p = pbuf_alloc(PBUF_RAW, 200, PBUF_RAM);
    if (buf_p == 0)
    {
        udp_send_not_enough_mem(cmd_p);
        return;
    }

    char* buf = buf_p->payload;
    //
    Get_real_pwm(&adc);
    //Check_Channels();
    //
    snprintf(buf, buf_p->len,
            "SUCCESS:"
            "%u "
            "%u "
            "%u "
            "%u "
            "%u "
            "%u "
            "%u "
            "%u",
            U_STAT[0], U_STAT[1], U_STAT[2],
            U_STAT[3], U_STAT[4], U_STAT[5],
            U_STAT[6], U_STAT[7] );
    /////////
    udp_sendstr(cmd_p, (char*)buf);

    pbuf_free(buf_p);

}
/*----------------------------------------------------------------------------*/
void udp_send_lineADC(struct cmd_raw* cmd_p)
{
    ADC_DATA adc;

    struct pbuf* buf_p = pbuf_alloc(PBUF_RAW, 200, PBUF_RAM);
    if (buf_p == 0)
    {
        udp_send_not_enough_mem(cmd_p);
        return;
    }

    char* buf = buf_p->payload;

    Get_real_adc(&adc);
    //
    //Check_Channels();
    //

    snprintf(buf, buf_p->len,
            "SUCCESS:"
            "%u "
            "%u "
            "%u "
            "%u "
            "%u "
            "%u "
            "%u "
            "%u",
            I_STAT[0], I_STAT[1], I_STAT[2],
            I_STAT[3], I_STAT[4], I_STAT[5],
            I_STAT[6], I_STAT[7] );
    /////
    udp_sendstr(cmd_p, (char*)buf);

    pbuf_free(buf_p);

}

// last function
/*----------------------------------------------------------------------------*/
void udp_send_sensors(struct cmd_raw* cmd_p)
{
    char buf[30];
    //
    snprintf(buf, 30,
            "SUCCESS:"
            "%u "
            "%u "
            "%u "
            "%u "
            "%u "
            "%u "
            "%u "
            "%u",
            DK[0].U_sens_fault_log[0], DK[0].U_sens_fault_log[1],
           DK[0].U_sens_fault_log[2], DK[0].U_sens_fault_log[3],
           DK[0].U_sens_fault_log[4],DK[0].U_sens_fault_log[5],
           DK[0].U_sens_fault_log[6],DK[0].U_sens_fault_log[7]);
    /////
    udp_sendstr(cmd_p, (char*)buf);


}

// last function
/*----------------------------------------------------------------------------*/
void cmd_help_func(struct cmd_raw* cmd_p, int argc, char** argv)
{
    char const title[] = "Cmd's list:\n";

    int buf_sz = 0;

    buf_sz += strlen(title) + 1; // + '\0' char

    struct cmd_nfo* nfo_p = g_cmd_nfo;
    while (nfo_p != 0)
    {
        buf_sz += strlen(nfo_p->cmd_txt) + strlen(nfo_p->cmd_help)+2; // + '\n' char
        nfo_p = nfo_p->next;
    }

    struct pbuf* buf_p = pbuf_alloc(PBUF_RAW, buf_sz, PBUF_RAM);

    if (buf_p == 0)
    {
        udp_send_not_enough_mem(cmd_p);
        return;
    }

    char* buf = buf_p->payload;

    strcpy(buf, title);

    nfo_p = g_cmd_nfo;
    while (nfo_p != 0)
    {
        strcat(buf, nfo_p->cmd_txt);
        strcat(buf, nfo_p->cmd_help);
        strcat(buf, "\r\n");
        nfo_p = nfo_p->next;
    }
    udp_sendstr(cmd_p, buf);
    pbuf_free(buf_p);
}

// local functions
/*----------------------------------------------------------------------------*/
static void process_get_cmd(struct cmd_raw* cmd_p, char const* par,
                            int argc, char** argv)
{
    if (strcmp(par, "ver") == 0)
    {
        udp_send_ver(cmd_p);
    }else
    if (strcmp(par, "event") == 0)
    {
        udp_send_event(cmd_p);
    }else
    if (strcmp(par, "sensors") == 0)
    {
        udp_send_sensors(cmd_p);
    }else
    if (strcmp(par, "lines") == 0)
    {
        udp_send_lines(cmd_p);
    }else
    if (strcmp(par, "state") == 0)
    {
        udp_send_state(cmd_p, argc, argv);
    }else
    if (strcmp(par, "gps_time") == 0)
    {
        udp_send_GPS_Time(cmd_p);
    }else
    if (strcmp(par, "gps_info") == 0)
    {
        udp_send_GPS_Info(cmd_p);
    }else
    if (strcmp(par, "ds1390_time") == 0)
    {
        udp_send_DS1390_time(cmd_p);
    }else
    if (strcmp(par, "light") == 0)
    {
        udp_send_Light(cmd_p);
    }else
    if (strcmp(par, "info") == 0)
    {
        udp_send_info(cmd_p);
    }
    else
    if (strcmp(par, "lineADC") == 0)
    {
        udp_send_lineADC(cmd_p);
    }else
    if (strcmp(par, "linePWM") == 0)
    {
        udp_send_linePWM(cmd_p);
    }else

    if (strcmp(par, "tvp") == 0)
    {
        udp_send_tvp(cmd_p);
    }else

        udp_send_wrong_par(cmd_p);
}
/*----------------------------------------------------------------------------*/
static void process_set_cmd(struct cmd_raw* cmd_p, char const* par, char const* val)
{
    if (strcmp(par, "ip") == 0) // IPv4 adress
    {
        struct in_addr ip;
        if (inet_aton(val, &ip))
        {
            pref_set_long(PREF_L_NET_IP, ip.s_addr);
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "mask") == 0) // IPv4 mask
    {
        struct in_addr msk;
        if (inet_aton(val, &msk))
        {
            pref_set_long(PREF_L_NET_MSK, msk.s_addr);
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "gw") == 0) // IPv4 gateway
    {
        struct in_addr gw;
        if (inet_aton(val, &gw))
        {
            pref_set_long(PREF_L_NET_GW, gw.s_addr);
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "cmdSrvIP") == 0) // IPv4 command server addr
    {
        struct in_addr cmd_srv;
        if (inet_aton(val, &cmd_srv))
        {
            pref_set_long(PREF_L_CMD_SRV_IP, cmd_srv.s_addr);
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "cmdPort") == 0) // IPv4 command server addr
    {
        unsigned port;
        if (sscanf(val, "%u", &port) == 1 && port <= 0xFFFF)
        {
            pref_set_long(PREF_L_CMD_PORT, port);
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "tvp") == 0) // Set TVP
    {
        unsigned port;
        if (sscanf(val, "%u", &port) == 1 && port <= 0x03)
        {
            //pref_set_long(PREF_L_CMD_PORT, port);
            //DK[CUR_DK].REQ.req[TVP].presence=true;
            //DK[CUR_DK].tvps.pres=port;
            //
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "dhcp") == 0)
    {
        if (strcmp(val, "on") == 0)
        {
            pref_set_long(PREF_L_NET_MODE, NET_MODE_DHCP);
            udp_send_success(cmd_p);
        }
        else if (strcmp(val, "off") == 0)
        {
            pref_set_long(PREF_L_NET_MODE, NET_MODE_STATIC_IP);
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "test") == 0)
    {
        if (strcmp(val, "on") == 0)
        {
            if (DK[0].tumbler)
            {
               DK[0].test = true;
               //DK_Service_OS();
               ///

               //
               SIGNAL_OFF();
               //

               udp_send_success(cmd_p);
            }
        }
        else if (strcmp(val, "off") == 0)
        {
            DK[0].test = false;
            DK[0].test_init=false;
            //Set_Test();
            //
            //DK_Service_undo();
            //
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "init") == 0)
    {
            evt_fifo_clear();
            //
            udp_send_success(cmd_p);

    }
    else if (strcmp(par, "rele") == 0)
    {
        if (strcmp(val, "on") == 0)
        {
            pin_on(OPIN_POWER);
            //pref_set_long(PREF_L_NET_MODE, NET_MODE_DHCP);
            udp_send_success(cmd_p);
        }
        else if (strcmp(val, "off") == 0)
        {
            pin_off(OPIN_POWER);
            //pref_set_long(PREF_L_NET_MODE, NET_MODE_STATIC_IP);
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "hwaddr") == 0)
    {
        unsigned hw[6];
        if (sscanf(val, "%02X:%02X:%02X:%02X:%02X:%02X", &hw[0], &hw[1], &hw[2], &hw[3], &hw[4], &hw[5]) == 6)
        {
/*
            struct eth_addr hwaddr;
            for (int i = 0; i < sizeof(hwaddr.addr); ++i)
                hwaddr.addr[i] = hw[i];
*/          if ( (hw[0] == 0xFF && hw[1] == 0xFF && hw[2] == 0xFF && hw[3] == 0xFF && hw[4] == 0xFF && hw[5] == 0xFF)
                 || ((hw[0]|hw[1]|hw[2]|hw[3]|hw[4]|hw[5])== 0))
                 {
                     udp_send_wrong_par(cmd_p);
                 }else
                 {
                     pref_set_long(PREF_L_MAC_1, hw[0] | (hw[1]<<8) | (hw[2]<<16) | (hw[3]<<24));
                     pref_set_long(PREF_L_MAC_2, hw[4] | (hw[5]<<8));
                     udp_sendstr(cmd_p, "SUCCESS: hwaddr was written to flash\n");
                 }
/*            if (hwaddr_save(&hwaddr))
                udp_sendstr(cmd_p, "SUCCESS: hwaddr was written to flash user registers.\n");
            else
                udp_sendstr(cmd_p, "ERR: hwaddr is already programmed.\n");
*/
        }
        else
            udp_send_wrong_par(cmd_p);
    }
    else if (strcmp(par, "pwm_rgy") == 0)
    {
        unsigned PWM_r, PWM_g, PWM_y;
        if (sscanf(val, "%u,%u,%u", &PWM_r, &PWM_g, &PWM_y) == 3 && (PWM_r <= 100) && (PWM_g <= 100)&& (PWM_y <= 100))
        {
            pref_set_long(PREF_L_PWM_RGY, (PWM_r<<16)|(PWM_g<<8)|PWM_y);
            SetPWM((PWM_r<<16)|(PWM_g<<8)|PWM_y);
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);

    }else if (strcmp(par, "work") == 0)
    {
        if (strcmp(val, "start") == 0)
        {
            if (GetLightMachineWork())
            {
                udp_sendstr(cmd_p, "ERR: Light_machine is working\n");
            }else
            {
                SetLightMachineWork(TRUE);
                udp_sendstr(cmd_p, "SUCCESS: Light_machine is started\n");
            }
        }
        else if (strcmp(val, "stop") == 0)
        {
            if (GetLightMachineWork())
            {
                SetLightMachineWork(FALSE);
                udp_sendstr(cmd_p, "SUCCESS: Light_machine is stoped\n");
            }else
            {
                udp_sendstr(cmd_p, "ERR: Light_machine is stop\n\n");
            }
        }
        else
            udp_send_wrong_par(cmd_p);
    }else if (strcmp(par, "light_r") == 0)
    {
        if (strcmp(val, "on") == 0)
        {
            //SetLight ( RED | (RED<<4) | (RED<<8) | (RED<<12));
            udp_send_success(cmd_p);
        }
        else if (strcmp(val, "off") == 0)
        {
            //SetLight ( 0x0000 );
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }else if (strcmp(par, "light_y") == 0)
    {
        if (strcmp(val, "on") == 0)
        {
            //SetLight ( YELLOW  | (YELLOW<<8) );
            udp_send_success(cmd_p);
        }
        else if (strcmp(val, "off") == 0)
        {
            //SetLight ( 0x0000 );
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }else if (strcmp(par, "light_g") == 0)
    {
        if (strcmp(val, "on") == 0)
        {
           // SetLight ( GREEN | (GREEN<<4) | (GREEN<<8) | (GREEN<<12) );
            udp_send_success(cmd_p);
        }
        else if (strcmp(val, "off") == 0)
        {
            //SetLight ( 0x0000 );
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }else if (strcmp(par, "dark") == 0)
    {
        if (strcmp(val, "on") == 0)
        {
           //SetLight ( 0x0000 );
            udp_send_success(cmd_p);
        }
        else
            udp_send_wrong_par(cmd_p);
    }else if (strcmp(par, "ds1390_time") == 0)
    {
        DS1390_TIME t;
        unsigned i[6];
        struct tm tp;

        if (sscanf(val, "%u.%u.%u-%u:%u:%u", &i[0], &i[1], &i[2], &i[3], &i[4], &i[5]) == 6)
        {
            t.msec = 0;
            t.sec = (unsigned char)i[5];
            t.min = (unsigned char)i[4];
            t.hour = (unsigned char)i[3];
            t.year = (unsigned short)i[2];
            t.month = (unsigned char)i[1];
            t.date = (unsigned char)i[0];

            if (SetTime_DS1390(&t))
                udp_send_success(cmd_p);
            else
                udp_send_wrong_par(cmd_p);

            tp.tm_sec = (unsigned char)i[5];
            tp.tm_min = (unsigned char)i[4];
            tp.tm_hour = (unsigned char)i[3];
            tp.tm_mday = (unsigned char)i[0];
            tp.tm_mon = (unsigned char)i[1];
            tp.tm_year = (unsigned short)i[2];
            if (tp.tm_year < 100)
               tp.tm_year+=2000;
            if (!tm_is_valid(&tp))
                udp_send_wrong_par(cmd_p);

            time_t t = mktime(&tp);
            stime(&t);

        }
        else
            udp_sendstr(cmd_p, "set ds1390_time XX.XX.XX-XX:XX:XX\n");
    }else
        udp_send_wrong_par(cmd_p);
}
/*static void process_light_cmd(struct cmd_raw* cmd_p, char const* par, char const* val)
{

}*/
/*----------------------------------------------------------------------------*/
static int hex2buf(unsigned char* buf, unsigned len, char const* hex)
{
    int conv_len = 0;

    while (len-- && *hex != '\0')
    {
        if (*(hex + 1) == '\0')
            break;

        int hi = hex2char(*hex++);
        int lo = hex2char(*hex++);

        if (hi == -1 || lo == -1)
            break;

        *buf++ = (unsigned char)((hi << 4) + lo);
        conv_len++;
    }

    return conv_len;
}

static int buf2hex(char* hex, unsigned hex_len, unsigned char* buf, unsigned len)
{
    int conv_len = 0;
    char hex_buf[2];

    if (hex_len < len * 2 + 1)
        return conv_len;

    while (len--)
    {
        char2hex(hex_buf, *buf++);
        hex[conv_len++] = hex_buf[0];
        hex[conv_len++] = hex_buf[1];
    }

    hex[conv_len] = '\0';
    return conv_len;
}

static int hex2char(int hex)
{
    if (hex >= '0' && hex <= '9')
        return hex - '0';
    else if (hex >= 'A' && hex <= 'F')
        return hex - 'A' + 10;
    else if (hex >= 'a' && hex <= 'f')
        return hex - 'a' + 10;
    else
        return -1;
}

static void char2hex(char hex[2], int ch)
{
    hex[0] = ch >> 4 & 0x0F;
    hex[1] = ch & 0x0F;

    hex[0] = hex[0] > 9 ? 'A' + hex[0] - 10 : '0' + hex[0];
    hex[1] = hex[1] > 9 ? 'A' + hex[1] - 10 : '0' + hex[1];
}

static err_t udp_send_success(struct cmd_raw* cmd_p)
{
    return udp_sendstr(cmd_p, "SUCCESS: Normal completion\n");
}

static err_t udp_send_not_enough_mem(struct cmd_raw* cmd_p)
{
    return udp_sendstr(cmd_p, "ERR: Not enough memory\n");
}

static err_t udp_send_wrong_par(struct cmd_raw* cmd_p)
{
    return udp_sendstr(cmd_p, ERR_STR_WRONG_PARAM);
}


static err_t udp_send_GPS_Info(struct cmd_raw* cmd_p)
{
    char buf[192];

    GPS_INFO gps;

    Get_gps_info(&gps);

    snprintf(buf, sizeof(buf), "GPS health - %s, fix - %s\n"
                               "GPS Time: %02d.%02d.%02d-%02d:%02d:%02d\n"
                               "GPS Position: Latitude %f, Longitude %f, Altitude %.1f\n",
             gps.health?"TRUE":"FALSE", gps.fix_valid?"TRUE":"FALSE",
             gps.time_pack.date, gps.time_pack.month, gps.time_pack.year,
             gps.time_pack.hour, gps.time_pack.min, gps.time_pack.sec,
             gps.Position.Latitude,gps.Position.Longitude,gps.Position.Altitude
             );
    return udp_sendstr(cmd_p, buf);
}

static err_t udp_send_GPS_Time(struct cmd_raw* cmd_p)
{
    char buf[128];

    GPS_INFO gps;

    Get_gps_info(&gps);

    snprintf(buf, sizeof(buf), "GPS Time %02d.%02d.%02d-%02d:%02d:%02d-%s\n",
             gps.time_pack.date, gps.time_pack.month, gps.time_pack.year,
             gps.time_pack.hour, gps.time_pack.min, gps.time_pack.sec,gps.fix_valid?"TRUE":"FALSE");
    return udp_sendstr(cmd_p, buf);
}
/*----------------------------------------------------------------------------*/
static err_t udp_send_DS1390_time(struct cmd_raw* cmd_p)
{
    char buf[128];
    DS1390_TIME time;
    GetTime_DS1390(&time);
    snprintf(buf, sizeof(buf), "SUCCESS:"
                               "%02d.%02d.%d-%02d:%02d:%02d\n",
             time.date, time.month, time.year,
             time.hour, time.min, time.sec);//, time.day-1>6?"err":days[time.day-1]);
    return udp_sendstr(cmd_p, buf);
}
/*----------------------------------------------------------------------------*/
static err_t udp_send_Light(struct cmd_raw* cmd_p)
{
    char buf[128];
    GetLightText((char*)buf);
    return udp_sendstr(cmd_p, buf);
}
//Set_Test
/*----------------------------------------------------------------------------*/
static err_t udp_send_tvp(struct cmd_raw* cmd_p)
{
    char buf[128];
    //
    strcpy(buf,"TVP1 is ");
    if (pin_rd(IPIN_TVP0))
      strcat(buf,"off");
    else
      strcat(buf,"on");
    //
    strcat(buf,". TVP2 is ");
    if (pin_rd(IPIN_TVP1))
      strcat(buf,"off\n");
    else
      strcat(buf,"on\n");

    return udp_sendstr(cmd_p, buf);
}
/*----------------------------------------------------------------------------*/
static err_t udp_send_event(struct cmd_raw* cmd_p)
{
    char buf[128];
    struct evt_fifo_item_t  it;
    struct tm  time;
    //
    BOOL res = evt_fifo_get(&it);
    //cache_update();
    if (res==false)
    {
      strcpy(buf,"SUCCESS:+++EMPTY");
      return udp_sendstr(cmd_p, buf);
    }
    //
    gmtime(it.ts, &time);

    snprintf(buf, sizeof(buf), "SUCCESS:%02d.%02d.%d-%02d:%02d:%02d %s\n",
        time.tm_mday, time.tm_mon, time.tm_year,
        time.tm_hour, time.tm_min, time.tm_sec, it.dat);

    return udp_sendstr(cmd_p, buf);
}
/*----------------------------------------------------------------------------*/
static err_t udp_send_ver(struct cmd_raw* cmd_p)
{
    char buf[32];
    //struct evt_fifo_item_t  it;
    //struct tm  time;
    //unsigned long ver;
    //
    //ver = VER_MAJOR*100 + VER_MINOR*10 + VER_MINORER;
    strcpy(buf,"SUCCESS:%u");
      return udp_sendstr(cmd_p, buf);

}
/*----------------------------------------------------------------------------*/
// bb - byte to convert
void Byte_to_Bin(unsigned char bb, char *str)
{
   for (int i=0; i<8; i++)
     if (bb & (1<<i))
       strcat(str,"1");
     else
       strcat(str,"0");
}
/*----------------------------------------------------------------------------*/
static err_t udp_send_lines(struct cmd_raw* cmd_p)
{
    char buf[128];
    ///
    strcpy(buf,"SUCCESS:RED_CHANNEL=");
    Byte_to_Bin(RED_PORT, buf);
    strcat(buf," YEL_CHANNEL=");
    Byte_to_Bin(YEL_PORT, buf);
    strcat(buf," GREEN_CHANNEL=");
    Byte_to_Bin(GREEN_PORT, buf);
    //
    return udp_sendstr(cmd_p, buf);

}
/*----------------------------------------------------------------------------*/
static err_t udp_send_state(struct cmd_raw* cmd_p,int argc, char** argv)
{
    char buf[128];
    int curr_dk=0;
    //////////////////
    if (argc==2)
    {
       if (sscanf(argv[1], "%u", &curr_dk) == 1)
       {
          if (curr_dk>DK_N)
            curr_dk=0;
          //
          if (curr_dk)
            curr_dk--;
       }

    }
        if (DK[curr_dk].CUR.work == PROG_FAZA)
        {
          snprintf(buf, sizeof(buf), "SUCCESS:WORK=PROGRAM PROGRAM=%d PROG_FAZA=%d",
             (DK[curr_dk].CUR.prog+1),(DK[curr_dk].CUR.prog_faza+1));

        }
        //
        if (DK[curr_dk].CUR.work == SINGLE_FAZA)
        {
           snprintf(buf, sizeof(buf), "SUCCESS:WORK=SINGLE_FAZA SINGLE_FAZA=%d",
             (DK[curr_dk].CUR.faza+1));

           //S_W("Oaae. oaca=" + IntToStr(DK[CUR_DK].CUR.faza+1));
        }
        //
        if (DK[curr_dk].CUR.work == SPEC_PROG)
        {
          //snprintf(buf, sizeof(buf), "SUCCESS:WORK=SPEC_PROG SPEC_PROG=%d",
          //   (DK[CUR_DK].CUR.spec_prog));
          strcpy(buf,"SUCCESS:WORK=SPEC_PROG SPEC_PROG=");
          if (DK[curr_dk].CUR.spec_prog==1)
            strcat(buf,"FLASH");
          ///
          if (DK[curr_dk].CUR.spec_prog==2)
            strcat(buf,"OFF");
          ///
          if (DK[curr_dk].CUR.spec_prog==0)
            strcat(buf,"KK");
          ///
        }
        strcat(buf," LEVEL=");
        if (DK[curr_dk].CUR.source==ALARM) strcat(buf,"ALARM");
        if (DK[curr_dk].CUR.source==TUMBLER) strcat(buf,"TUMBLER");

        if (DK[curr_dk].CUR.source==SERVICE) strcat(buf,"SERVICE");
        if (DK[curr_dk].CUR.source==VPU) strcat(buf,"VPU");
        if (DK[curr_dk].CUR.source==TVP) strcat(buf,"TVP");
        if (DK[curr_dk].CUR.source==PLAN) strcat(buf,"PLAN");

    return udp_sendstr(cmd_p, buf);
}
/*----------------------------------------------------------------------------*/
static err_t udp_send_info(struct cmd_raw* cmd_p)
{
    char buf[96];
    char buf1[128];
    snprintf (buf, sizeof (buf),
        "SUCCESS:%s ver.%u.%u.%u \r\n "
        "%s \r\n "
        "%s \r\n ",
        APP_NAME,VER_MAJOR,VER_MINOR,VER_MINORER,VER_SVN_DATE,APP_COPYRIGHT);
    ///
    //get_version(buf, sizeof(buf));
    snprintf (buf1, sizeof(buf1),"SUCCESS:%s\n"
        "CPU: LM3S9B96\n"
        "Clk: %u MHz\n\n",
        buf, hw_sys_clock()
        );
    return udp_sendstr(cmd_p, buf);
}

static err_t udp_send_config(struct cmd_raw* cmd_p)
{
    char buf[320];
    snprintf(buf, sizeof(buf),
        "IP      : %u.%u.%u.%u\n"
        "Mask    : %u.%u.%u.%u\n"
        "GW      : %u.%u.%u.%u\n"
        "HwAddr  : %02X:%02X:%02X:%02X:%02X:%02X\n"
        "Mode    : %s\n"
        "CmdEP   : %u.%u.%u.%u:%u\n",
        (unsigned)ip4_addr1(&g_ipaddr), (unsigned)ip4_addr2(&g_ipaddr), (unsigned)ip4_addr3(&g_ipaddr), (unsigned)ip4_addr4(&g_ipaddr),
        (unsigned)ip4_addr1(&g_ipmsk), (unsigned)ip4_addr2(&g_ipmsk), (unsigned)ip4_addr3(&g_ipmsk), (unsigned)ip4_addr4(&g_ipmsk),
        (unsigned)ip4_addr1(&g_ipgw), (unsigned)ip4_addr2(&g_ipgw), (unsigned)ip4_addr3(&g_ipgw), (unsigned)ip4_addr4(&g_ipgw),
        (unsigned)g_hwaddr.addr[0], (unsigned)g_hwaddr.addr[1], (unsigned)g_hwaddr.addr[2], (unsigned)g_hwaddr.addr[3], (unsigned)g_hwaddr.addr[4], (unsigned)g_hwaddr.addr[5],
        g_net_mode == NET_MODE_DHCP ? "DHCP" : "Static IP",

        (unsigned)ip4_addr1(&g_cmd_ip), (unsigned)ip4_addr2(&g_cmd_ip), (unsigned)ip4_addr3(&g_cmd_ip), (unsigned)ip4_addr4(&g_cmd_ip), g_cmd_port
        );

    return udp_sendstr(cmd_p, buf);
}
/*----------------------------------------------------------------------------*/