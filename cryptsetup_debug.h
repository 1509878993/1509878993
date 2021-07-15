#ifndef CRYPTSETUP_DEBUG_H
#define CRYPTSETUP_DEBUG_H

/*Linux message level*/
#define DEBUG_LOG_EMERG       (0)
#define DEBUG_LOG_ALERT       (1)
#define DEBUG_LOG_CRIT        (2) 
#define DEBUG_LOG_ERR         (3) 
#define DEBUG_LOG_WARNING     (4)
#define DEBUG_LOG_NOTICE      (5)
#define DEBUG_LOG_INFO        (6)
#define DEBUG_LOG_DEBUG       (7)

/*
*If you want to get more log, 
*modifing the "DEBUG_LOG_CURRENT" ,please.
*/
#define DEBUG_LOG_CURRENT	(DEBUG_LOG_DEBUG)

#define LOG_TAG "CRYPTSETUP"

#define cryptsetup_debug(level, fmt, ...) \
	if(level <= DEBUG_LOG_CURRENT){ \
		printf("\033[32m%s: func:%s->line:%d,\033[0m " fmt, LOG_TAG, __func__, __LINE__, ##__VA_ARGS__); \
		printf(" \n"); \
	}else { \
		do{}while(0); \
	}

#endif//CRYPTSETUP_DEBUG_H
