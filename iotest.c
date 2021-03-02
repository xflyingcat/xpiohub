/*--------------------------------------------------------
  "THE BEER-WARE LICENSE" (Revision 42):
  Alex Kostyuk wrote this code. As long as you retain this
  notice you can do whatever you want with this stuff.
  If we meet some day, and you think this stuff is worth it,
  you can buy me a beer in return.
----------------------------------------------------------*/

#include "includes.h"

int XPluginStart(char *name, char *sig, char *desc);
int XPluginEnable(void);
float main_callback(float elapsedPrev, float elapsedFlight, int counter, void *ptr);

void indication_config(void)
{
    return;
int i,j;
memset(key_config,0,sizeof(key_config));

#define config_output_7segdisp(p,a,b,c,d) do {\
key_config[a].mode = MODE_POLL; \
key_config[a].key_id = a; \
key_config[a].port = p; \
key_config[a].len = b; \
key_config[a].devadr = c; \
key_config[a].pos = d; \
} while(0)

#define config_output_single_led(a,b) do {\
key_config[a].mode = MODE_POLL; \
key_config[a].key_id = a; \
key_config[a].port = 0; \
key_config[a].len = 0; \
key_config[a].devadr = 0; \
key_config[a].pos = b; \
} while(0)

//                             l d p
    config_output_7segdisp(0,0,3,0,2); /* crs   */
    config_output_7segdisp(0,1,3,1,0); /* speed */
    config_output_7segdisp(0,2,3,1,5); /* magnetic heading */
    config_output_7segdisp(0,3,5,2,3); /* vertical velocity */
    config_output_7segdisp(0,4,5,3,1); /* altitude */

    key_config[0].opt = 1;
    key_config[2].opt = 1;

    config_output_7segdisp(2,5+22+0,5,0,0); /* com1(5)   */
    config_output_7segdisp(2,5+22+1,5,1,0); /* com1_stb(5) */
    config_output_7segdisp(2,5+22+2,5,2,0); /* com2(5)   */
    config_output_7segdisp(2,5+22+3,5,3,0); /* com2_stb(5) */
    config_output_7segdisp(2,5+22+4,5,4,0); /* nav1(5)   */
    config_output_7segdisp(2,5+22+5,5,5,0); /* nav1_stb(5) */
    config_output_7segdisp(2,5+22+6,5,6,0); /* nav2(5)   */
    config_output_7segdisp(2,5+22+7,5,7,0); /* nav2_stb(5) */
    config_output_7segdisp(2,5+22+8,4,8,0); /* adf1(4)   */
    config_output_7segdisp(2,5+22+9,5,9,0); /* dme_freq(5) */
    config_output_7segdisp(2,5+22+10,4,10,0); /* dme_dist(4) */
    config_output_7segdisp(2,5+22+11,4,11,0); /* dme1 dist */
    config_output_7segdisp(2,5+22+12,4,12,0); /* press   */

    config_output_7segdisp(2,5+22+13,4,8,4); /* adf2(4) */
    config_output_7segdisp(2,5+22+14,5,10,4); /* xpnd(4)   */
    config_output_7segdisp(2,5+22+15,3,4,5); /* crs1(3) */
    config_output_7segdisp(2,5+22+16,3,6,5); /* crs2(3)   */
    config_output_7segdisp(2,5+22+17,5,11,4); /* dme2_dist(4) */

    output_data[5+22+0] = 11111;
    output_data[5+22+1] = 22222;
    output_data[5+22+2] = 33333;
    output_data[5+22+3] = 44444;
    output_data[5+22+4] = 55555;
    output_data[5+22+5] = 66666;
    output_data[5+22+6] = 77777;
    output_data[5+22+7] = 88888;
    output_data[5+22+8] = 99999;
    output_data[5+22+9] = 99999;
    output_data[5+22+10] = 11111;
    output_data[5+22+11] = 22222;
    output_data[5+22+12] = 33333;
    output_data[5+22+13] = 44444;
    output_data[5+22+14] = 55555;
    output_data[5+22+15] = 66666;
    output_data[5+22+16] = 77777;
    output_data[5+22+17] = 88888;


    for(i=5,j=0;j<22;j++,i++)
    {
     config_output_single_led(i,j);
    }


}


int main(int argc, char **argv)
{
char name[MAX_PATH];
char sig[MAX_PATH];
char desc[MAX_PATH];
XPluginStart(name, sig, desc);
printf("\n\
************************************************************\n\
 %s\n %s\n %s\n\
************************************************************\n",name,sig,desc);
XPluginEnable();


indication_config();

while(1)
{
  main_callback(0.0, 0.0, 0, NULL);
  Sleep(1);
}
return 0;
}


void on_test_poll_dataref(void)
{


}

static int button_table[] = {
         /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
   /* 0 */ -1,-1, 0, 1, 2, 3,-1,-1, 4,-1, 5, 6,-1, 7, 9, 8,
   /* 1 */ 10,12,13,14,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   /* 2 */ -1,15,16,17,18,19,20,21,22,-1,-1,-1,-1,-1,-1,-1,
   /* 3 */ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};


void on_test_event(int event)
{
  switch(event)
  {
    /* CRS */
    case 0x2048:
       output_data[0]++;
       if(output_data[0] > 359)
          output_data[0] = 0;
    break;

    case 0x2049:
       output_data[0]--;
       if(output_data[0] < 0)
          output_data[0] = 359;
    break;

    /* IAS */
    case 0x204A:
       output_data[1]++;
       if(output_data[1] > 999)
          output_data[1] = 999;
    break;

    case 0x204B:
       output_data[1]--;
       if(output_data[1] < 0)
          output_data[1] = 0;
    break;

    /* HDG */
    case 0x204D:
       output_data[2]++;
       if(output_data[2] > 359)
          output_data[2] = 0;
    break;

    case 0x204C:
       output_data[2]--;
       if(output_data[2] < 0)
          output_data[2] = 359;
    break;


    /* HDG */
    case 0x204F:
       output_data[3] += 100;
       if(output_data[3] > 9900)
          output_data[3] = 9900;
    break;

    case 0x204E:
       output_data[3] -= 100;
       if(output_data[3] < -9900)
          output_data[3] = -9900;
    break;

    /* ALT */
    case 0x2050:
       output_data[4] += 100;
       if(output_data[4] > 49000)
          output_data[4] = 49000;
    break;

    case 0x2051:
       output_data[4] -= 100;
       if(output_data[4] < 0)
          output_data[4] = 0;
    break;
    default:

        if((event & 0xF000) == 0x8000 )
        {
         if((event & 0xFFF) < 64)
         {
          if(button_table[event & 0xFFF] != -1)
          {
            output_data[5+button_table[event & 0xFFF]] ^= 1;
            output_data[5+button_table[event & 0xFFF]] &= 1;
          }
         }
        }
  }
}



