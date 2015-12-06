#ifndef kursH
#define kursH

#include "dk.h"


void  Update_STATES_KURS(bool flash);
void  Collect_Tramvays();
void  Set_LED(int group, int gr_col, bool stat);
void  SET_OUTPUTS();
void  New_Project_KURS();
void Clear_LED();

extern     BYTE     GREEN_PORT_TRAM;
extern     BYTE     YEL_PORT_TRAM;

extern     BYTE     GREEN_PORT;
extern     BYTE     RED_PORT;
extern     BYTE     YEL_PORT;

extern     BYTE     GREEN_PORT_CONF;
extern     BYTE     RED_PORT_CONF;
extern     BYTE     YEL_PORT_CONF;

extern     BYTE     GREEN_PORT_ERR;
extern     BYTE     RED_PORT_ERR;

extern     TPROGRAM     PROG_PLAN;
extern     BYTE     tram_green_drive_line[8];
extern     BYTE     tram_green_drive_line_num[8];

/*----------------------------------------------------------------------------*/
void Prepare_KURS_Structures(void);

#endif