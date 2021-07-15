#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "user_cryptsetup.h"
#include "cryptsetup_debug.h"

static void usage(void)
{
    printf("***********************************************\n");
    printf("usage:\n");
    printf("\tc & C\tThis action will change the password for FDE!\n");
    printf("\ta & A\tThis action will add a password for FDE <be carful:only for the first time> !\n");
    printf("\tq & Q\tExit action.\n");
    printf("***********************************************\n");
    printf("\nEnter your action please:");
}

static int fill_crypttab_info(char* line, crypttab_info* info)
{
    int i = 0, temp = 0;
    char temp_line[4][128] = {};

    if ((NULL == line) || (NULL == info)) {
        cryptsetup_debug(DEBUG_LOG_ERR, "line or info is NULL, check please");
        return EPARRAM;
    }

    cryptsetup_debug(DEBUG_LOG_DEBUG, "fill_crypttab_info enter: %s", line);
    for(i = 0; i < 4; i++)
        memset(temp_line[i], 0 , 128);

    i = 0;
    while(i != strlen(line) && i < LINE_CHAR){
        while(i != strlen(line) && i < LINE_CHAR && isspace(line[i]))
            i++;
        int j = i;
        while(j != strlen(line) && !isspace(line[j]))
            j++;
        if(i != j){
            strncpy(temp_line[temp], &line[i], j-i);
            cryptsetup_debug(DEBUG_LOG_DEBUG, "temp_line[%d] : %s", temp, temp_line[temp]);
            temp ++;
            i = j;
        }
    }

    if(NULL != temp_line[0]){
        strncpy(info->name, temp_line[0], strlen(temp_line[0]));
    }

    if(NULL != temp_line[1]){
        strncpy(info->encrypted_device, temp_line[1], strlen(temp_line[1]));
    }

    if(NULL != temp_line[2]){
        strncpy(info->options, temp_line[2], strlen(temp_line[2]));
    }

    if(NULL != temp_line[3]){
        strncpy(info->password, temp_line[3], strlen(temp_line[3]));
    }

    return NO_ERROR;
}

static int parse_file()
{
    FILE *fp = NULL;
    char line[LINE_CHAR] = {};
    int start = 0;
    int line_num = 0;

    cryptsetup_debug(DEBUG_LOG_DEBUG, "parse_file enter");
    if (!(access(FILE_CRYPTTAB, F_OK))) {
        fp = fopen(FILE_CRYPTTAB, "r");
        if (NULL == fp){
            cryptsetup_debug(DEBUG_LOG_ERR, "Open file error");
            return EOPENFAIL;
        }
        while ((fgets(line, LINE_CHAR, fp) != NULL) && (line_num < LINE_NUM)) {
            fill_crypttab_info(line, &cinfo[start]);
            start++;
            line_num++;
        }
    } else {
        cryptsetup_debug(DEBUG_LOG_ERR, "There is no crypttab, check it please");
        return ENOFILE;
    }

    cryptsetup_debug(DEBUG_LOG_DEBUG, "parse_file leave");
    return NO_ERROR;
}

static int do_exec(char* action, char* command)
{
    pid_t status = NO_ERROR;
    int ret = NO_ERROR;

    if(NULL == action || NULL == command){
        cryptsetup_debug(DEBUG_LOG_ERR, "action or command is NULL, check please");
        return EPARRAM;
    }

    status = system(command);
    if (-1 == status) {
        cryptsetup_debug(DEBUG_LOG_ERR, "<%s> fail beacuse system call fail", action);
        return status;
    } else {
        cryptsetup_debug(DEBUG_LOG_DEBUG, "system call return with status = [ 0x%x ]", status);
        if (WIFEXITED(status)) {
            if (0 == WEXITSTATUS(status)) {
                cryptsetup_debug(DEBUG_LOG_ERR, "exec <%s> successfully.", action);
                ret = WEXITSTATUS(status);
            } else { 
                cryptsetup_debug(DEBUG_LOG_ERR, \
                    "exec <%s> fail, cryptsetup exec fail with status = [ %d ]", \
                    action, WEXITSTATUS(status));
                ret = WEXITSTATUS(status);
            } 
        } else { 
                cryptsetup_debug(DEBUG_LOG_ERR, "exec <%s> fail with status = [ %d ]", action, WEXITSTATUS(status));
                ret = WEXITSTATUS(status);
            }
    }

    return ret;
}

static int do_add_password()
{
    cryptsetup_debug(DEBUG_LOG_DEBUG, "Add new password for FDE!");
    int ret = NO_ERROR, i = 0;
    char temp_cmd[LINE_CHAR] = {};
    char* action = "Add new password for FDE!";
    char* command0 = "mv /etc/crypttab /etc/crypttab_bak";
    char* command1 = "/sbin/cryptsetup luksChangeKey";
    char* command2 = "echo '";
    char* command3 = "rm ";
    char* command4 = "mv /etc/crypttab_bak /etc/crypttab";
        
    //do_exec(action, command0);/*backup cryptatb file*/
    while(i < LINE_NUM){
        memset(temp_cmd, 0, LINE_CHAR);
        if(!(access(cinfo[i].password, F_OK))){
            do_exec(action, command0);/*backup cryptatb file*/
            sprintf(temp_cmd, "%s\t%s\t%s%s",command1, cinfo[i].encrypted_device, "--key-file=", cinfo[i].password);
            cryptsetup_debug(DEBUG_LOG_DEBUG, "temp_cmd[%d]: %s", i, temp_cmd);
            ret = do_exec(action, temp_cmd);
            if(ret){
                cryptsetup_debug(DEBUG_LOG_ERR, "Remove default key file fail");
                do_exec(action, command4);
                return ret;
            }
            memset(temp_cmd, 0, LINE_CHAR);
            sprintf(temp_cmd, "%s%s\t%s\t%s%s", command2, cinfo[i].name, \
                     cinfo[i].encrypted_device, cinfo[i].options, "' >> /etc/crypttab");
            cryptsetup_debug(DEBUG_LOG_DEBUG, "temp_cmd[%d]: %s", i, temp_cmd);
            ret = do_exec(action, temp_cmd);
            if(!(access(cinfo[i].password,F_OK))){
                memset(temp_cmd, 0, LINE_CHAR);
                sprintf(temp_cmd, "%s%s", command3, cinfo[i].password);
                cryptsetup_debug(DEBUG_LOG_DEBUG, "temp_cmd[%d]: %s", i, temp_cmd);
                ret = do_exec(action, temp_cmd);
            }
        } else{
            if(0 != strlen(cinfo[i].name))
                cryptsetup_debug(DEBUG_LOG_ERR, "You have added a password, maybe you want to chang it...");
        }
        i++;
    }
    return ret;
}

static int do_change_password()
{
    cryptsetup_debug(DEBUG_LOG_DEBUG, "Change password for FDE!");
    char* action = "Change password for FDE!";
    char temp_cmd [512] = {0};
    char* command = "/sbin/cryptsetup luksChangeKey ";
    int i = 0, ret = NO_ERROR;

    while(i < LINE_NUM){
        if(0 != strlen(cinfo[i].name)){
            memset(temp_cmd, 0, 512);
            sprintf(temp_cmd, "%s\t%s",command, cinfo[i].encrypted_device);
            cryptsetup_debug(DEBUG_LOG_DEBUG, "command: %s", temp_cmd);
            ret |= do_exec(action, temp_cmd);
        }
        i++;
    }

    return ret;
}

static int do_exit()
{
    cryptsetup_debug(DEBUG_LOG_DEBUG, "Exit!");
    return NO_ERROR;
}

static int do_default(void)
{
    cryptsetup_debug(DEBUG_LOG_DEBUG, "Unsupport action, be careful!");
    return EUNSUPPORT;
}

static int do_action(char action){
    int ret = NO_ERROR;
    cryptsetup_debug(DEBUG_LOG_DEBUG, "action: %c", action);
    switch(action){
        case 'a':
        case 'A':
            ret = do_add_password();
            break;
        case 'c':
        case 'C':
            ret = do_change_password();
            break;
        case 'q':
        case 'Q':
            ret = do_exit();
            break;
        default:
            ret = do_default();
            break;
    }
}

int main(int argc, char* argv[]){
    char clear, action = ' ';
    int ret = NO_ERROR;
    parse_file();
    do{
        action = ' ';
        usage();
        fflush(stdin);
        fflush(stdout);
        scanf("%c", &action);
        while((clear = getchar()) != '\n' && clear != EOF);/*clear 'Enter'*/
        ret = do_action(action);
        if(ret)
            break;
    }while(action != 'q' && action != 'Q');

    return ret;
}