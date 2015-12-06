/*****************************************************************************
*
* © 2014 Cyber-SB. Written by Selyutin Alex.
*
*****************************************************************************/

#ifndef __CMD_CH_H__
#define __CMD_CH_H__

#include "../lwip/lwiplib.h"

#define CMD_DQUE_SZ     32   // max cmd's fifo size
#define CMD_NFO_SZ      32  // max cmd's count
#define MAX_ARGV_SZ     8   // cmd's argument vector size (in one command)

struct cmd_raw
{
    struct udp_pcb* upcb;
    struct pbuf*    buf_p;
    struct ip_addr* raddr;
    u16_t           rport;
};

#define CMD_FL_ACT      0x0001 // cmd_nfo->cmd_fl

struct cmd_nfo
{
    struct cmd_nfo* next;
    unsigned int    cmd_fl;
    char*           cmd_txt;
    char*           cmd_help;
    void            (*cmd_func)(struct cmd_raw* cmd_p, int argc, char** argv);
};

extern struct cmd_nfo   g_cmd_nfo[CMD_NFO_SZ];
extern BOOL ETH_RECV_FLAG;

void        cmd_ch_init();
err_t       udp_sendstr(struct cmd_raw* cmd_p, char const* s);
err_t       udp_sendbuf(struct cmd_raw* cmd_p, char const* buf, int len);
err_t       udp_cmd_sendstr(char const* s);
err_t       udp_cmd_sendbuf(char const* buf, int len);
void        get_cmd_ch_ip(struct ip_addr* ipaddr);
unsigned    get_cmd_ch_port();
err_t       udp_debug_send_str(char const* s);



#endif // __CMD_CH_H__
