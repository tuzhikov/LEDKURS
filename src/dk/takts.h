//---------------------------------------------------------------------------

#ifndef taktsH
#define taktsH
//---------------------------------------------------------------------------
#include "structures.h"
//#include "../tnkernel/tn.h"
#include "../light/light.h"

#ifndef  __cplusplus
#define bool  unsigned char
#define true  1
#define false 0
#endif 

  typedef struct __TAKT_PROP
  {
     STATES_LIGHT  col;
     BYTE          time;
  } TAKT_PROP;
  //
  typedef  TAKT_PROP     prom_takts_faz_type[MaxDirects][TaktsN];
  //typedef  TAKT_PROP   osn_takts_faz_type[MaxDirects];
  typedef  STATES_LIGHT  osn_takts_faz_type[MaxDirects];


int  Get_Red_Green_Type_Fasa(int i_prog, int i_napr, int i_faz);
int  Init_TAKTS(TPROJECT  *proj);
void Build_Takts(int c_prog, int n_prog, int  cur_faz,int  next_faz);
void Load_Progs(int c_prog, int n_prog);
extern int   cur_prog[DK_N];  ///������� ���������
//extern       takts_faz_type  takts_faz; // ����� ����� ����
  extern   prom_takts_faz_type      prom_takts[DK_N]; // ����. ����� ����� ����
  extern   osn_takts_faz_type  osn_takt[DK_N]; // �������� �����  ����
  extern   int       Tprom_len[DK_N];
  extern   int       osn_takt_time[DK_N];  //����� ��������� �����
  extern TPROGRAM  PROG_NEXT; //��������� ��������� ���������



#endif
