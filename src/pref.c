/*****************************************************************************
*
* © 2014 Cyber-SB. Written by Selyutin Alex.
*
*****************************************************************************/

#include <string.h> // memcpy()
#include "tnkernel/tn.h"
#include "lwip/lwiplib.h"
#include "memory/memory.h"
#include "crc32.h"
#include "pref.h"

#include "debug/debug.h"

static TN_MUTEX g_mutex;
static long     g_pref_cache[PREF_L_CNT];

#if PREF_L_CNT * 4 > EEPROM_PREF_SIZE/2 // PREF_L_CNT * sizeof(long)
#error Too many preferences data, see EEPROM_PREF_SIZE in memory/memory.h
#endif

// local functions

static BOOL pref_load();
static void pref_save();
static void pref_load_def();
static void lock();
static void unlock();
static int pref_b;

void pref_init()
{
    if (tn_mutex_create(&g_mutex, TN_MUTEX_ATTR_INHERIT, 0) != TERR_NO_ERR)
    {
        dbg_puts("tn_mutex_create(&g_mutex) != TERR_NO_ERR");
        dbg_trace();
        tn_halt();
    }

    lock();

    dbg_printf("Loading preferences...");
    //
    pref_b = pref_load();
    //pref_b = 0;
    if (pref_b)
    {
        dbg_puts("[done]");
    }
    else
    {
        pref_load_def();
        pref_save();
        dbg_puts("[done, defaults was loaded]");
    }
    //pref_load_def();
     
    
    unlock();
}

void pref_reset()
{
    lock();
    pref_load_def();
    pref_save();
    unlock();
}

long pref_get_long(enum pref_l_idx idx)
{
#ifdef DEBUG
    if ((unsigned)idx >= PREF_L_CNT)
    {
        dbg_printf("pref_get_long(idx == UNKNOWN[%d]) error", idx);
        tn_halt();
    }
#endif // DEBUG

    return g_pref_cache[idx];
}

void pref_set_long(enum pref_l_idx idx, long value)
{
#ifdef DEBUG
    if ((unsigned)idx >= PREF_L_CNT)
    {
        dbg_printf("pref_get_long(idx == UNKNOWN[%d]) error", idx);
        tn_halt();
    }
#endif // DEBUG

    lock();

    g_pref_cache[idx] = value;
    pref_save();

    unlock();
}

// local functions

static BOOL pref_load()
{
    unsigned char count = 5; // попытки чтения настроек
    unsigned long crc32;
    
    
/*    
    memset (g_pref_cache, 0xFF, sizeof(g_pref_cache));
    eeprom_wr(EEPROM_PREF, 0, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));    
    eeprom_wr(EEPROM_PREF, 256, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));    
    eeprom_wr(EEPROM_PREF, 512, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));    
    eeprom_wr(EEPROM_PREF, 512+256, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));    
    while (1);
*/    
    
    do
    {
        flash_rd(FLASH_PREF, 0, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));
        crc32 = crc32_calc((unsigned char*)g_pref_cache, sizeof(g_pref_cache)-sizeof(g_pref_cache[PREF_L_CRC32]));
        count--;
    }while (count && crc32 != g_pref_cache[PREF_L_CRC32]);
    
    if (crc32 != g_pref_cache[PREF_L_CRC32])
    {
        flash_wr(FLASH_PREF, 256, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));

        count = 5;
        do
        {
            flash_rd(FLASH_PREF, 512, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));
            crc32 = crc32_calc((unsigned char*)g_pref_cache, sizeof(g_pref_cache)-sizeof(g_pref_cache[PREF_L_CRC32]));
            count--;
        }while (count && crc32 != g_pref_cache[PREF_L_CRC32]);
        
        if (crc32 != g_pref_cache[PREF_L_CRC32])
        {
            flash_wr(FLASH_PREF, 512+256, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));
            return FALSE;
        }
        flash_wr(FLASH_PREF, 0, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));
    }
    
    return TRUE;
}

static void pref_save()
{
    g_pref_cache[PREF_L_CRC32] = crc32_calc((unsigned char*)g_pref_cache, sizeof(g_pref_cache)-sizeof(g_pref_cache[PREF_L_CRC32]));
    flash_wr(FLASH_PREF, 0, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));
    flash_wr(FLASH_PREF, 512, (unsigned char*)g_pref_cache, sizeof(g_pref_cache));
}

static void pref_load_def()
{
    struct ip_addr addr;

    g_pref_cache[PREF_L_NET_MODE]           = NET_MODE_STATIC_IP; //NET_MODE_DHCP
   ////
   #ifdef DEBUG
    //IP4_ADDR(&addr, 192, 168, 1, 10);
    IP4_ADDR(&addr, 169, 254, 16, 1);
   #else
    IP4_ADDR(&addr, 169, 254, 16, 1);
   #endif 
   //// 
    g_pref_cache[PREF_L_NET_IP]             = addr.addr;

    IP4_ADDR(&addr, 255, 255, 255, 0);
    g_pref_cache[PREF_L_NET_MSK]            = addr.addr;

    IP4_ADDR(&addr, 0, 0, 0, 0);
    g_pref_cache[PREF_L_NET_GW]             = addr.addr;
    
    IP4_ADDR(&addr, 0, 0, 0, 0);    
    g_pref_cache[PREF_L_CMD_SRV_IP] = addr.addr;
    g_pref_cache[PREF_L_CMD_PORT]           = 11990;
        
    
//    g_pref_cache[PREF_L_MAC_1]              = 0x001AB600;
//    g_pref_cache[PREF_L_MAC_2]              = 0;

    
    g_pref_cache[PREF_L_PWM_RGY]             = 0x623212;

    g_pref_cache[PREF_DELAY_LIGHT_ON]        = 0;
    g_pref_cache[PREF_DELAY_LIGHT_OFF]       = 0;
    
    g_pref_cache[PREF_RF_ADDR_PULT_HI]        = 0x00000000;
    g_pref_cache[PREF_RF_ADDR_PULT_LO]        = 0x00000000;
    
    
}

static void lock()
{
    if (tn_mutex_lock(&g_mutex, TN_WAIT_INFINITE) != TERR_NO_ERR)
    {
        dbg_puts("tn_mutex_lock(&g_mutex) != TERR_NO_ERR");
        dbg_trace();
        tn_halt();
    }
}

static void unlock()
{
    if (tn_mutex_unlock(&g_mutex) != TERR_NO_ERR)
    {
        dbg_puts("tn_mutex_unlock(&g_mutex) != TERR_NO_ERR");
        dbg_trace();
        tn_halt();
    }
}

void pref_data_wr(unsigned short addr, unsigned char* data, unsigned short len)
{
    lock();
    memcpy(((unsigned char*)g_pref_cache)+addr, data, len);
    pref_save();
    unlock();
}

void pref_data_rd(unsigned short addr, unsigned char* dest, unsigned short len)
{
    lock();
    memcpy(dest, ((unsigned char*)g_pref_cache)+addr, len);
    unlock();
}