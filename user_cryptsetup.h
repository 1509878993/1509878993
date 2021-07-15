#ifndef USER_CRYPTSETUP_H
#define USER_CRYPTSETUP_H

#define FILE_CRYPTTAB   ("/etc/crypttab")
//#define FILE_KEY        ("/etc/init.d/fde_default.bin")

#define LINE_CHAR   (512) /*Max char num of a line read from crypttab*/
#define INFO_CHAR   (128) /*Max num of every param in a line*/
#define LINE_NUM    (10) /*Max line num of crypttab*/

#define NO_ERROR    (0)
#define ENOFILE     (-0x1)
#define EOPENFAIL   (-0x2)
#define EPARRAM     (-0x3)
#define EUNSUPPORT  (-0xFF)

typedef struct crypttab_info_t{
    char name[INFO_CHAR];
    char encrypted_device[INFO_CHAR];
    char password[INFO_CHAR];
    char options[INFO_CHAR];
}crypttab_info;

static void usage(void);
static int fill_crypttab_info(char* line, crypttab_info* info);
static int parse_file(void);
static int do_action(char action);
static int do_exit(void);
static int do_change_password(void);
static int do_add_password(void);
static int do_default(void);
static int do_exec(char* action, char* command);

crypttab_info cinfo[10] = {};

#endif /*USER_CRYPTSETUP*/