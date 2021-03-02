/*--------------------------------------------------------
  "THE BEER-WARE LICENSE" (Revision 42):
  Alex Kostyuk wrote this code. As long as you retain this
  notice you can do whatever you want with this stuff.
  If we meet some day, and you think this stuff is worth it,
  you can buy me a beer in return.
----------------------------------------------------------*/

#include "includes.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMMenus.h"

#define MENU_ITEM_NAME "FlyingCat USB I/O hub"
#define PLUGIN_NAME    "FlyingCat USB I/O hub plugin"
#define SIGNATURE      "usb.hub.xflyingcat"

   #define EVENT_TYPE_MASK   0xF000
   #define EVENT_CODE_MASK   0x0FFF

#ifdef TEST_BUILD
#include "dummies.h"
#endif // TEST_BUILD


void re_register_all(void);

SESSION_WORK_SPACE sws[SERIAL_PORTS_MAX];
int output_data[MAX_KEYS];
int input_data[MAX_KEYS];
void poll_dataref(void);
void poll_inputs(void);
void menu_create(void);
XPLMMenuID menu_;

unsigned long leds = 0;

void led_control(int pos, int state)
{
unsigned long mask = 1;
mask <<= pos;
    if(state)
       leds |= mask;
    else
       leds &= ~mask;
}

unsigned long get_leds_state(void)
{
  return leds;
}


void on_input_event(ULONG64 ev_container)
{
 //ULONG64 ev_container;
 EVENT_STRUCT *ev = (EVENT_STRUCT*)&ev_container;


 switch(ev->event_type)
 {
    case ROTARY_SWITCH:
    case ENCODER_PULSE:
       pass_to_sim(ev_container,-1);
       dirty_hack();
    break;

    case RISING_EDGE:
       pass_to_sim(ev_container,1);
    break;

    case FALLING_EDGE:
       pass_to_sim(ev_container,0);
    break;
 }

}

float main_callback(float elapsedPrev, float elapsedFlight, int counter, void *ptr)
{
static int cntr = 0;
ULONG64 event;
	if (elapsedPrev < 0) elapsedPrev = 0;

    if(++cntr == 2)
    {
      #ifdef TEST_BUILD
         void on_test_poll_dataref(void);
         on_test_poll_dataref();
      #else
         poll_dataref();
         poll_inputs();
      #endif

      cntr = 0;
    }

     if((recv_from_device(&event)))
     {
       #ifdef TEST_BUILD
          EVENT_STRUCT *ev = (EVENT_STRUCT*)&event;

          printf("type:%d id:%d port: %d val:%d\n",
          ev->event_type,
          ev->event_id,
          ev->port_id,
          ev->value);

          void on_test_event(unsigned long ev);
          on_test_event(event);
       #endif
       on_input_event(event);
     }
	return -1;  // -1 = run every frame
}


PLUGIN_API int XPluginStart(char *name, char *sig, char *desc)
{
    char path[MAX_PATH];
    strcpy(path,"Resources/plugins/");
    load_config(path);
    log_remove();
    log_write("Plugin start....");

	strcpy(name, PLUGIN_NAME);
	strcpy(sig,  SIGNATURE);
	strcpy(desc, "USB CDC I/O hub plugin rev."PLUGIN_REVISION" for X-Plane by xflyingcat");
  	menu_create();

	XPLMRegisterFlightLoopCallback(main_callback, -1, &sws);
    pipes_create();

	return 1;
}

PLUGIN_API void XPluginStop(void)
{
    XPLMClearAllMenuItems(menu_);
    XPLMDestroyMenu(menu_);
    log_write("XPluginStop(): unregister callback");
    XPLMUnregisterFlightLoopCallback(main_callback,NULL);
    pipes_destroy();
    log_write("Plugin stopped");
}

PLUGIN_API int XPluginEnable(void)
{
    log_write("XPluginEnable(): start serial");
    reload_datarefs();
    start_serial(sws);
    set_pipes_on();
    return 1;
}

PLUGIN_API void XPluginDisable(void)
{
     log_write("XPluginDisable(): stop serial");
     serial_stop(sws);
     log_write("XPluginDisable(): serial closed");
     set_pipes_off();
     log_write("Plugin disabled");
}


void show_msg(int msg)
{
    switch(msg)
    {
      case 101:  log_write("XPLM_MSG_PLANE_CRASHED"); break;
      case 102:  log_write("XPLM_MSG_PLANE_LOADED");  break;
      case 103:
          log_write("*** XPLM_MSG_AIRPORT_LOADED ****");
          re_register_all();
      break;
      case 104:  log_write("XPLM_MSG_SCENERY_LOADED"); break;
      case 105:  log_write("XPLM_MSG_AIRPLANE_COUNT_CHANGED"); break;
      case 106:  log_write("XPLM_MSG_PLANE_UNLOADED"); break;
      case 107:  log_write("XPLM_MSG_WILL_WRITE_PREFS"); break;
      case 108:  log_write("XPLM_MSG_LIVERY_LOADED");
          re_register_all();
      break;
      //      default:
      //log_write("XPLM_MSG: %d",msg);
    }
}



PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, long msg, void *param)
{

    show_msg(msg);
    if (from == XPLM_PLUGIN_XPLANE)
    {
        if (param == XPLM_PLUGIN_XPLANE && msg == XPLM_MSG_PLANE_LOADED)
        {
            re_register_all();
        }

    }
    else
    {
    }

}



void re_register_all(void)
{
char acf_file_name[300];
char acf_path[600],*p;
     log_write("Will start XPLMGetNthAircraftModel()");
     XPLMGetNthAircraftModel(0, acf_file_name, acf_path);
     p = strstr(acf_path,acf_file_name);
     if(p)
        *p = 0x00;
     log_write("Aircraft path: %s",acf_path);
     log_write("Aircraft file name: %s",acf_file_name);
     if(!load_config(acf_path))
      {
         strcpy(acf_path,"Resources/plugins/");
         if(!load_config(acf_path))
         {
            log_write("re_register_all(): ini file not found, stopped");
            exit(0);
         }
      }
     reload_datarefs();
}


void menu_CB(void * inMenuRef, void * inItemRef)
{
//int item_ref = inItemRef;
//if(item_ref == 1)
    log_write("menu_CB(): re_register_all()");
    re_register_all();

}

XPLMMenuID menu_;

void menu_create(void)
{
int menu = XPLMAppendMenuItem(XPLMFindPluginsMenu(),
                                       MENU_ITEM_NAME,
                                       NULL,
                                       1);

menu_ = XPLMCreateMenu( MENU_ITEM_NAME,
                        XPLMFindPluginsMenu(),
                        menu,
                        menu_CB,
                        NULL);

XPLMAppendMenuItem(menu_,
                   "Re-register all",
                   NULL,
                   1);


}



static XPLMCommandRef dirty_hack_cmd1;
static XPLMCommandRef dirty_hack_cmd2;

void reload_datarefs(void)
{
int i = 0;
int j = 0;
dirty_hack_cmd1 = XPLMFindCommand("sim/instruments/panel_bright_up");
dirty_hack_cmd2 = XPLMFindCommand("sim/instruments/panel_bright_down");

   log_write("reload_datarefs()");

   for(i=0;i<MAX_KEYS;i++)
   {
    if(key_config[i].mode <= 0)
                     break;

    switch(key_config[i].mode)
     {
       case MODE_NOP:
       break;

       case MODE_LOOP:
       case MODE_DREF:
       case MODE_POLL:
       case MODE_INPUT:
       case MODE_DREF_PUT:
            key_config[i].dref = XPLMFindDataRef(key_config[i].cmd0_dref);
            if(key_config[i].dref == NULL)
                log_write("Could not found dataref: %s",key_config[i].cmd0_dref);
            else
               j++;
       break;

       case MODE_COMMAND:
            key_config[i].cmd_ref0 = XPLMFindCommand(key_config[i].cmd0_dref);
            if(key_config[i].cmd_ref0 == NULL)
                log_write("Could not found command: %s",key_config[i].cmd0_dref);
            else
               j++;

       break;

       case MODE_COMMAND_TOGGLE:
            key_config[i].cmd_ref0 = XPLMFindCommand(key_config[i].cmd0_dref);

            if(key_config[i].cmd_ref0 == NULL)
                log_write("Could not found command: %s",key_config[i].cmd0_dref);
            else
               j++;

            key_config[i].cmd_ref1 = XPLMFindCommand(key_config[i].cmd1);

            if(key_config[i].cmd_ref1 == NULL)
                log_write("Could not found command: %s",key_config[i].cmd1);
            else
               j++;

       break;
     }

   }

   log_write("Total loaded keys: %d of %d",j,i);
}

const char* event_type[] = {
    "once",
    "end",
    "begin"
};

void pass_to_sim(ULONG64 ev_container, int level)
{
    int tmp_int;
    float tmp_float;
    int key_code = 0,i;
    EVENT_STRUCT *ev = (EVENT_STRUCT*)&ev_container;

    key_code = ev->port_id;
    key_code <<= 16;
    key_code |= ev->event_id;
    int extra_value = ev->value;
#ifndef TEST_BUILD
   if(key_code == config.reload_code && level)
   {
     log_write("Reload key %04d has been pressed. Going to reload config)",key_code);
     re_register_all();
   }
#endif
   if(0==strcmp(config.debug_keyinfo,"on"))
   {
     log_write("Port: %d Key%03d %s", (key_code >> 8) & 0x0F,
                                      key_code & 0xFF,
                                      event_type[level+1]);
   }

   for(i=0;i<MAX_KEYS;i++)
   {
    if(key_config[i].mode <= 0)
    {
      break;
    }

    if(key_config[i].key_id == key_code && key_config[i].mode)
    {
     switch(key_config[i].mode)
     {
       case MODE_NOP:
       break;

       case MODE_LOOP:
           if(level == 0)
                 break;
           if(key_config[i].dref == NULL)
                           reload_datarefs();

           if(key_config[i].type == TYPE_INT)
           {
             if(key_config[i].dref)
             {
               if(key_config[i].index)
                    XPLMGetDatavi(key_config[i].dref,
                              &tmp_int,
                              key_config[i].index-1,1);
               else
                    tmp_int  =  XPLMGetDatai(key_config[i].dref);


                tmp_int += key_config[i].inc_int;
                if(key_config[i].inc_int > 0)
                {
                  if(tmp_int > key_config[i].max_int)
                     tmp_int = key_config[i].min_int;
                } else
                {
                  if(tmp_int < key_config[i].min_int)
                     tmp_int = key_config[i].max_int;
                }


               if(key_config[i].index)
                XPLMSetDatavi(key_config[i].dref,
                              &tmp_int,
                              key_config[i].index-1,1);
               else
                XPLMSetDatai(key_config[i].dref,tmp_int);
             }

           }
           else
           if(key_config[i].type == TYPE_FLOAT)
           {
               if(key_config[i].index)
                   XPLMGetDatavf(key_config[i].dref,
                                 &tmp_float,
                                 key_config[i].index-1,1);
               else
                   tmp_float  =  XPLMGetDataf(key_config[i].dref);

                tmp_float += key_config[i].inc_float;


                if(key_config[i].inc_float > 0)
                {
                  if(tmp_float > key_config[i].max_float)
                     tmp_float = key_config[i].min_float;
                } else
                {
                  if(tmp_float < key_config[i].min_float)
                     tmp_float = key_config[i].max_float;
                }

               if(key_config[i].index)
                   XPLMSetDatavf(key_config[i].dref,
                                 &tmp_float,
                                 key_config[i].index-1,1);
               else
                 XPLMSetDataf(key_config[i].dref,tmp_float);
           }

       break;

       case MODE_DREF:
           if(key_config[i].dref == NULL)
                           reload_datarefs();

           if(key_config[i].type == TYPE_INT)
           {
             if(key_config[i].dref)
             {
               tmp_int = key_config[i].encoder_int[level & 1];
               if(key_config[i].index)
                XPLMSetDatavi(key_config[i].dref,
                              &tmp_int,
                              key_config[i].index-1,1);
               else
                XPLMSetDatai(key_config[i].dref,
                             tmp_int);

             }
           }
           else
           if(key_config[i].type == TYPE_FLOAT)
           {
               tmp_float = key_config[i].encoder_float[level & 1];
               if(key_config[i].index)
                XPLMSetDatavf(key_config[i].dref,
                              &tmp_float,
                              key_config[i].index-1,1);
               else
                XPLMSetDataf(key_config[i].dref,
                             tmp_float);
           }
       break;

       case MODE_DREF_PUT:

           if(key_config[i].dref == NULL)
                           reload_datarefs();

           if(key_config[i].type == TYPE_INT)
           {
             if(key_config[i].dref)
             {
               tmp_int = (extra_value - key_config[i].offset) / key_config[i].factor;
               if(key_config[i].index)
                XPLMSetDatavi(key_config[i].dref,
                              &tmp_int,
                              key_config[i].index-1,1);
               else
                XPLMSetDatai(key_config[i].dref,
                             tmp_int);

             }
           }
           else
           if(key_config[i].type == TYPE_FLOAT)
           {
               tmp_float = 1.0 * (extra_value - key_config[i].offset);
               tmp_float = tmp_float * key_config[i].factor;
               if(key_config[i].index)
                XPLMSetDatavf(key_config[i].dref,
                              &tmp_float,
                              key_config[i].index-1,1);
               else
                XPLMSetDataf(key_config[i].dref,
                             tmp_float);
           }
       break;



       case MODE_COMMAND:

           if(key_config[i].cmd_ref0 == NULL)
                           reload_datarefs();

        if(key_config[i].cmd_ref0)
        {
          switch(level)
          {
            case 0:
              XPLMCommandEnd(key_config[i].cmd_ref0);
            break;

            case 1:
              XPLMCommandBegin(key_config[i].cmd_ref0);
            break;

            default:
              XPLMCommandOnce(key_config[i].cmd_ref0);
          }

        } //else
          //   log_write("dref/command is not assigned: %s",key_config[i].cmd0_dref);

       break;

       case MODE_COMMAND_TOGGLE:

          if(key_config[i].cmd_ref0 == NULL || key_config[i].cmd_ref1 == NULL)
                            reload_datarefs();

          if(key_config[i].cmd_ref0 == NULL)
          {
             //log_write("dref/command is not assigned: %s",key_config[i].cmd0_dref);
             break;
          }

          if(key_config[i].cmd_ref1 == NULL)
          {
             //log_write("dref/command is not assigned: %s",key_config[i].cmd1);
             break;
          }

          if(level)
              XPLMCommandOnce(key_config[i].cmd_ref1);
          else
              XPLMCommandOnce(key_config[i].cmd_ref0);

       break;
     }
    }
   }
}

void poll_dataref(void)
{
int tmp_int;
float tmp_float;
int key_code = 0,i;

   for(i=0;i<MAX_KEYS;i++)
   {
    if(key_config[i].mode <= 0)
    {
      break;
    }
    if(key_config[i].mode == MODE_POLL)
    {
       key_code = key_config[i].key_id;
       if(key_config[i].dref != NULL)
       {
           if(key_config[i].factor == 0.0)
                    key_config[i].factor = 1.0;

           if(key_config[i].type == TYPE_INT)
           {

               if(key_config[i].index)
                  XPLMGetDatavi(key_config[i].dref,
                              &tmp_int,
                              key_config[i].index-1,1);
               else
                  tmp_int  =  XPLMGetDatai(key_config[i].dref);

             if(key_config[i].mask)
             {
               if(tmp_int & key_config[i].mask)
                   output_data[key_code] = 1;
               else
               {
                  output_data[key_code] = 0;
               }
             } else
             output_data[key_code] = tmp_int * key_config[i].factor + key_config[i].offset;

           }
           else if(key_config[i].type == TYPE_FLOAT)
           {
                     if(key_config[i].index)
                           XPLMGetDatavf(key_config[i].dref,
                                 &tmp_float,
                                 key_config[i].index-1,1);
                     else
                           tmp_float  =  XPLMGetDataf(key_config[i].dref);

             if(key_config[i].len)
             {
               output_data[key_code] = tmp_float * key_config[i].factor + key_config[i].offset;
             } else
                {
                  if(tmp_float > 0.5)
                       output_data[key_code] = 1;
                  else
                       output_data[key_code] = 0;
                }
           }
       }
       else
       {
           //log_write("!!! dref is not assigned: %s",key_config[i].cmd0_dref);
       }
    }
   }
}


void poll_inputs(void)
{
int tmp_int;
float tmp_float;
int key_code = 0,port = 0, i;

   //for(i=0;i<SERIAL_PORTS_MAX*16;i++)
   //{
   //  log_write("%d ",input_data[i]);
   //}

   for(i=0;i<MAX_KEYS;i++)
   {
    if(key_config[i].mode <= 0)
    {
      break;
    }
    if(key_config[i].mode == MODE_INPUT)
    {
       port = key_config[i].port;
       key_code = key_config[i].devadr;
       if(key_config[i].dref != NULL)
       {
           if(key_config[i].factor == 0.0)
                    key_config[i].factor = 1.0;

           if(key_config[i].type == TYPE_INT)
           {
               tmp_int = (input_data[port * 16 + key_code] - key_config[i].offset) / key_config[i].factor;
               if(key_config[i].index)
                  XPLMSetDatavi(key_config[i].dref,
                                 &tmp_int,
                                  key_config[i].index-1,1);
               else
                  XPLMSetDatai(key_config[i].dref,tmp_int);

           }
           else if(key_config[i].type == TYPE_FLOAT)
           {
                    tmp_float = 1.0 * (input_data[port * 16 + key_code] - key_config[i].offset);
                    tmp_float = tmp_float * key_config[i].factor;

                     if(key_config[i].index)
                        XPLMSetDatavf(key_config[i].dref,
                                      &tmp_float,
                                      key_config[i].index-1,1);
                     else
                        XPLMSetDataf(key_config[i].dref,tmp_float);

           }
       }
       else
       {
           //log_write("!!! dref is not assigned: %s",key_config[i].cmd0_dref);
       }
    }
   }


}


void dirty_hack(void)
{
 XPLMCommandOnce(dirty_hack_cmd1);
 XPLMCommandOnce(dirty_hack_cmd2);
}




