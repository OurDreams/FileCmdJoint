/**
 ******************************************************************************
 * @file       ini.h
 * @brief      API include file of ini.h.
 * @details    This file including all API functions's declare of ini.h.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef INI_H_
#define INI_H_

#ifdef __cplusplus             /* Maintain C++ compatibility */
extern "C" {
#endif /* __cplusplus */
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include "types.h"
#include "config.h"

/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
#define MAX_FILE_NAME_LEN   64

#define WordLength_8b                  ((uint16_t)0x0000)
#define WordLength_9b                  ((uint16_t)0x1000)

#define StopBits_1                     ((uint16_t)0x0000)
#define StopBits_0_5                   ((uint16_t)0x1000)
#define StopBits_2                     ((uint16_t)0x2000)
#define StopBits_1_5                   ((uint16_t)0x3000)

#define Parity_No                      ((uint16_t)0x0000)
#define Parity_Even                    ((uint16_t)0x0400)
#define Parity_Odd                     ((uint16_t)0x0600)

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
typedef struct
{
    char filename[MAX_FILE_NAME_LEN];
    int filemaxsize;
} file_t;

typedef struct
{
    /* cmd ini */
    char show_ver[6];   /**< 显示版本 */
    char bsp_ver[8];    /**< bsp版本 */
    char ker_ver[6];    /**< 内核版本（ASCII码，5字节） */
    int meter_ver;      /**< 计量库版本（2字节） */
    char oem[5];        /**< OEM信息（4字节，首字母表示,如上海联能SHLN） */

    /* joint ini */
    int files;
    int blank;
    char outfile[MAX_FILE_NAME_LEN];
    file_t file[DEFAULT_MAX_FILE];
} cmd_joint_ini_t;

/*-----------------------------------------------------------------------------
 Section: Globals
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
extern int
ini_get_info(cmd_joint_ini_t *pinfo);

#ifdef __cplusplus      /* Maintain C++ compatibility */
}
#endif /* __cplusplus */
#endif /* INI_H_ */
/*--------------------------End of ini.h-----------------------------*/
