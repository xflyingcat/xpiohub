/*--------------------------------------------------------
  "THE BEER-WARE LICENSE" (Revision 42):
  Alex Kostyuk wrote this code. As long as you retain this
  notice you can do whatever you want with this stuff.
  If we meet some day, and you think this stuff is worth it,
  you can buy me a beer in return.
----------------------------------------------------------*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <windows.h>

//#define ARDUINO_LOG    "xpio_arduino.log"
//#define FLYINGCAT_LOG  "xpio_flyingcat.log"
//#define ARDUINO_INI    "xpio_arduino.ini"
//#define FLYINGCAT_INI  "xpio_flyingcat.ini"
//#define SWITCH_PANEL_LOG "swpanel.log"
//#define SWITCH_PANEL_INI "swpanel.ini"
#define MCP_INI "xpiohub.ini"
#define MCP_LOG "xpiohub.log"


typedef struct {

char mode_log[MAX_PATH];
char debug_keyinfo[50];
char reload_key[50];
int  reload_code;
char indication[50];
} CONFIG;

extern CONFIG config;

int load_config(char *path);


#endif

