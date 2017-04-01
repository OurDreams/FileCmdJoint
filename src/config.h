/**
 ******************************************************************************
 * @file       config.h
 * @brief      �����ļ�
 * @details    This file including all API functions's declare of config.h.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef CONFIG_H_
#define CONFIG_H_ 

/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
#define DEFAULT_INI_FILE        "./FileCmdJoint.ini"     /**< Ĭ�������ļ� */


#define DEFAULT_MAX_FILE        (5u)                /**< ����ļ��ϲ����� */
#define DEFAULT_DEST_FILE       "./FLASH.bin"       /**< �ϲ����ļ� */
#define DEFAULT_BOOT_SIZE       (32 * 1024)         /**< boot��С */
#define DEFAULT_APP_SIZE        ((1024 - 32) * 1024)/**< app��С */


#define DEFAULT_LOG_ON          (1u)                /**< Ĭ�ϴ���־��¼ */
#define DEFAULT_LOG_FILE_NAME   "./FileCmdJoint.log"     /**< Ĭ����־�ļ� */

#endif /* CONFIG_H_ */
/*-----------------------------End of config.h-------------------------------*/
