/*--------------------------------------------------------
  "THE BEER-WARE LICENSE" (Revision 42):
  Alex Kostyuk wrote this code. As long as you retain this
  notice you can do whatever you want with this stuff.
  If we meet some day, and you think this stuff is worth it,
  you can buy me a beer in return.
----------------------------------------------------------*/

#include "includes.h"

CONFIG config;
static int cfg_item = -1;
static int ind_item = -1;
static int input_item = -1;
static int port = 0;

typedef struct {
const char *group;
const char *key;
char const *var;
int size;
} INIT_STRUCT;

static const DICT modes[] = {
  {"nop", MODE_NOP},
  {"datarefPoll", MODE_POLL},
  {"dataref-poll", MODE_POLL},
  {"loop", MODE_LOOP},
  {"dataref", MODE_DREF},
  {"command", MODE_COMMAND},
  {"command-toggle", MODE_COMMAND_TOGGLE},
  {"input", MODE_INPUT },
  {"dataref-put", MODE_DREF_PUT },
  {"",-1}
};

static const DICT types[] = {
  {"int",TYPE_INT},
  {"float",TYPE_FLOAT},
  {"",-1}
};


const char *printable_modes[] = {
  "NOP",
  "POLL",
  "LOOP",
  "DREF",
  "COMMAND",
  "COMMAND_TOGGLE",
  "INPUT",
  "DREF_PUT"
};

const char *printable_types[] = {
  "INT",
  "FLOAT"
};


void print_keys(void);

KEY_CONFIG key_config[MAX_KEYS];


int key_conversion(int ev, int port)
{
    return (port << 16) | (ev & 0xFFFF);
}


int find_key(const DICT *dct, char *key)
{
int i;

     for(i=0; dct[i].key[0];i++)
     {
       if(0 == strcmp(dct[i].key,key))
                       return dct[i].val;
     }

return -1;
}

#define ADD_ITEM(x,y,z)  {x,y,config.z,sizeof(config.z)}

static const INIT_STRUCT init_struct[] = {

ADD_ITEM("mode",       "log",   mode_log),
ADD_ITEM("debug",      "keyinfo",  debug_keyinfo),
ADD_ITEM("operation",  "reload_key",  reload_key),
ADD_ITEM("operation",   "indication",  indication),
{NULL,NULL,NULL,-1}
};

static int config_cb(int lineno, void* user, const char* section, const char* name,
                   const char* value)
{
int i;
int key_id,mode,type;
char pattern[100];
    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    for(i=0;init_struct[i].size!=-1;i++)
    {
      if(MATCH(init_struct[i].group, init_struct[i].key))
      {
       strncpy((char*)init_struct[i].var,value,init_struct[i].size);
      }
    }

    if(MATCH("device","port"))
    {
        port = atoi(value);
    }


    if(MATCH("device","numeric_outs_max"))
    {
        sws[port].numeric_outs_max = atoi(value);
    }

    if(MATCH("device","discrete_outs_max"))
    {
        sws[port].discrete_outs_max = atoi(value);
    }



    for(i=0;i<SERIAL_PORTS_MAX;i++)
    {
      sprintf(pattern,"port_%d",i);
      if(strcmp(section, "serial") == 0 && strcmp(name, pattern) == 0)
      {
         strcpy(sws[i].serial_port,value);

      }
    }

    if(
        (   section[0] == 'K'
         && section[1] == 'e'
         && section[2] == 'y') ||

        (   section[0] == 'L'
         && section[1] == 'E'
         && section[2] == 'D')  ||

        (   section[0] == 'I'
         && section[1] == 'N'
         && section[2] == 'D')  ||

        (   section[0] == 'A'
         && section[1] == 'x'
         && section[2] == 'i'
         && section[3] == 's')  ||

        (   section[0] == 'I'
         && section[1] == 'n'
         && section[2] == 'p')
      )

    {

     if(section[0] == 'K' && section[1] == 'e' && section[2] == 'y')
     {
         sscanf(&section[3],"%d",&key_id);
         key_id = key_conversion(key_id, port);
     } else
     if(section[0] == 'A' && section[1] == 'x' && section[2] == 'i'  && section[3] == 's')
     {
       input_item++;
       key_id = input_item;
     } else
     {
       ind_item++;
       key_id = ind_item;
     }

     if(0 == strcmp(name, "mode"))
     {  //printf("Key id: %X\n",key_id);
        cfg_item++;
        key_config[cfg_item].key_id = key_id;
        mode = find_key(modes,(char*)value);
        if(mode == -1)
        {
           key_config[cfg_item].port = -1;
           key_config[cfg_item].mode = -1;
           log_write("Error! Unknown mode: %s",value);
        }
        else
        {
           key_config[cfg_item].port = port;
           key_config[cfg_item].mode = mode;
        }

     } else
     if(0 == strcmp(name, "command"))
     {
       strcpy(key_config[cfg_item].cmd0_dref,value);
     } else
     if(0 == strcmp(name, "dataref"))
     {
       strcpy(key_config[cfg_item].cmd0_dref,value);
     } else
     if(0 == strcmp(name, "array"))
     {
       strcpy(key_config[cfg_item].cmd0_dref,value);
     } else
     if(0 == strcmp(name, "type"))
     {
        type = find_key(types,(char*)value);
        if(type == -1)
        log_write("Error! Unknown type: %s",type);
        else
        {
           key_config[cfg_item].type = type;
        }

     } else

     if((0 == strcmp(name, "devadr")) || (0 == strcmp(name, "axis")))
     {
       key_config[cfg_item].devadr = atoi(value);
     } else

     if(0 == strcmp(name, "mask"))
     {
       key_config[cfg_item].mask = atoi(value);
     } else

     if(0 == strcmp(name, "pos"))
     {
       key_config[cfg_item].pos = atoi(value);
     } else

     if(0 == strcmp(name, "port"))
     {
       key_config[cfg_item].port = atoi(value);
     } else

     if(0 == strcmp(name, "len"))
     {
       key_config[cfg_item].len = atoi(value);
     } else

     if(0 == strcmp(name, "factor"))
     {
       key_config[cfg_item].factor = atof(value);
     } else

     if(0 == strcmp(name, "offset"))
     {
       key_config[cfg_item].offset = atof(value);
     } else

     if(0 == strcmp(name, "opt"))
     {
       key_config[cfg_item].opt = atoi(value);
     } else


     if(0 == strcmp(name, "min"))
     {
       if(key_config[cfg_item].type == TYPE_INT)
       {
         key_config[cfg_item].min_int = atoi(value);
       } else
       if(key_config[cfg_item].type == TYPE_FLOAT)
       {
         key_config[cfg_item].min_float = atof(value);
       }
     } else
     if(0 == strcmp(name, "max"))
     {
       if(key_config[cfg_item].type == TYPE_INT)
       {
         key_config[cfg_item].max_int = atoi(value);
       } else
       if(key_config[cfg_item].type == TYPE_FLOAT)
       {
         key_config[cfg_item].max_float = atof(value);
       }
     } else
     if(0 == strcmp(name, "increment"))
     {
       if(key_config[cfg_item].type == TYPE_INT)
       {
         key_config[cfg_item].inc_int = atoi(value);
       } else
       if(key_config[cfg_item].type == TYPE_FLOAT)
       {
         key_config[cfg_item].inc_float = atof(value);
       }
     } else
     if(0 == strcmp(name, "index"))
     {
        key_config[cfg_item].index = 1 + atoi(value);
     } else
     if(0 == strcmp(name, "digits_number"))
     {
        key_config[cfg_item].max_int = atoi(value);
     } else
     if(0 == strcmp(name, "command_0"))
     {
        strcpy(key_config[cfg_item].cmd0_dref,value);
     } else
     if(0 == strcmp(name, "command_1"))
     {
        strcpy(key_config[cfg_item].cmd1,value);
     } else
     if(0 == strcmp(name, "0"))
     {
       if(key_config[cfg_item].type == TYPE_INT)
       {
         key_config[cfg_item].encoder_int[0] = atoi(value);
       } else
       if(key_config[cfg_item].type == TYPE_FLOAT)
       {
         key_config[cfg_item].encoder_float[0] = atof(value);
       }
     } else
     if(0 == strcmp(name, "1"))
     {
       if(key_config[cfg_item].type == TYPE_INT)
       {
         key_config[cfg_item].encoder_int[1] = atoi(value);
       } else
       if(key_config[cfg_item].type == TYPE_FLOAT)
       {
         key_config[cfg_item].encoder_float[1] = atof(value);
       }
     }

    }



return 1;
}


int load_config(char *path)
{
    int i;
    memset(key_config,0,sizeof(key_config));
    for(i=0;i<MAX_KEYS;i++)
        key_config[i].mode = -1;

    cfg_item = -1;
    ind_item = -1;
    input_item = -1;
    port = 0;

    char ini_path[MAX_PATH];
    strcpy(ini_path,path);

    strcat(ini_path, MCP_INI);

    if(ini_parse(ini_path, config_cb, &config) < 0)
    {
        log_write("could not load config file %s",ini_path);
        return 0;
    }
    if(config.reload_key[0])
        sscanf(config.reload_key,"%d",&config.reload_code);
    else
        config.reload_code = -1;


    print_keys();
    return 1;
}



void print_keys(void)
{
   int i;
   for(i=0;i<MAX_KEYS && key_config[i].mode != -1;i++)
   {
      printf("********\n");
      printf("* %d \n",i);
      printf("********\n");

      printf("port:.........%d\n",key_config[i].port);
      printf("mode:.........%s\n",printable_modes[key_config[i].mode & 3]);
      printf("type:.........%s\n",printable_types[key_config[i].type & 1]);
      printf("key_id:.......%08X\n",key_config[i].key_id);

      printf("cmd_ref0:.....%08X\n",key_config[i].cmd_ref0);
      printf("cmd_ref1:.....%08X\n",key_config[i].cmd_ref1);
      printf("dref:.........%s\n",key_config[i].dref);
      printf("cmd0_dref:....%s\n",key_config[i].cmd0_dref);
      printf("cmd1:.........%s\n",key_config[i].cmd1);

      printf("min_float:....%f\n",key_config[i].min_float);
      printf("max_float:....%f\n",key_config[i].max_float);
      printf("inc_float:....%f\n",key_config[i].inc_float);
      printf("state_float:..%f\n",key_config[i].state_float);
      printf("min_int:......%d\n",key_config[i].min_int);
      printf("max_int:......%d\n",key_config[i].max_int);
      printf("inc_int:......%d\n",key_config[i].inc_int);

      printf("encoder_int:..{%d, %d}\n",key_config[i].encoder_int[0],key_config[i].encoder_int[1]);
      printf("encoder_float:{%f, %f}\n",key_config[i].encoder_float[0],key_config[i].encoder_float[1]);
      printf("index:........%d\n",key_config[i].index);
      printf("devadr:.......%d\n",key_config[i].devadr);
      printf("pos:..........%d\n",key_config[i].pos);
      printf("len:..........%d\n",key_config[i].len);
      printf("float.factor:.%f\n",key_config[i].factor);
      printf("offset:.......%f\n",key_config[i].offset);
      printf("opt:..........%d\n",key_config[i].opt);
      printf("mask:.........%X\n",key_config[i].mask);
     }
     printf("\n");
}
