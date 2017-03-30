/**
 ******************************************************************************
 * @file      ini.c
 * @brief     C Source file of ini.c.
 * @details   This file including all API functions's 
 *            implement of ini.c.	
 ******************************************************************************
 */
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "iniparser.h"
#include "ini.h"

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/
/**
 ******************************************************************************
 * @brief   创建默认配置文件
 * @return  None
 ******************************************************************************
 */
static void
create_example_ini_file(void)
{
    FILE * ini;

    ini = fopen(DEFAULT_INI_FILE, "w");
    fprintf(ini,
            "[cfg]\n"
            "#######U盘升级配置#######\n"
            "#显示版本(5字符，例如CV.03)\n"
            "ShowVer     = CV.03\n"
            "#BSP硬件版本(7字符，例如3.4.0.3)\n"
            "BspVer     = 3.4.0.3\n"
            "#内核版本（ASCII码，5字符，例如1.0.9）\n"
            "KernelVer  = 1.0.9\n"
            "#计量版本（2字节,十六进制）,通过shell->vm命令查看\n"
            "MeterVer  = 0x010d\n"
            "#OEM信息（4字节，首字母表示,如上海联能SHLN）\n"
            "OemVer  = \n"
            "\n\n"

            "#######文件拼接配置#######\n"
            "#文件总数量(一般填2,最大5)\n"
            "Files = 2\n"
            "#空白填充数字(0~255,大于255直接为255)\n"
            "Blank = 255\n"
            "#输出文件名\n"
            "OutFile = ./FLASH.bin\n"
            "\n"
            "#文件1\n"
            "#0x800-->32K\n"
            "[f1]\n"
            "FileName    = ./boot/boot(FKGA23_4_0_3).bin\n"
            "FileMaxSize = 0x8000\n"
            "\n"
            "#文件2\n"
            "#0xf8000-->992K\n"
            "#0xb8000-->736K\n"
            "#0x7f800-->510K\n"
            "[f2]\n"
            "FileName    = ./rtos.bin\n"
            "FileMaxSize = 0xb8000\n"
    );
    fclose(ini);
}

/**
 ******************************************************************************
 * @brief   从配置文件中获取文件合并信息
 * @param[out] *pinfo   : 返回info
 *
 * @retval      0 成功
 ******************************************************************************
 */
int
ini_get_info(usb_update_ini_t *pinfo)
{
    dictionary  *   ini ;
    char *pstr = NULL;

    memset(pinfo, 0x00, sizeof(*pinfo));

    ini = iniparser_load(DEFAULT_INI_FILE);
    if (NULL == ini)
    {
        create_example_ini_file();
        ini = iniparser_load(DEFAULT_INI_FILE);
        if (ini == NULL)
        {
            return -1;
        }
    }

    iniparser_dump(ini, NULL);//stderr

    /*************************fileCmd**********************/
    //显示版本
    pstr = iniparser_getstring(ini, "cfg:ShowVer", NULL);
    if (pstr == NULL)
    {
        iniparser_freedict(ini);
        return -1;
    }
    strncpy(pinfo->show_ver, pstr, sizeof(pinfo->show_ver));

    //BSP硬件版本
    pstr = iniparser_getstring(ini, "cfg:BspVer", NULL);
    if (pstr == NULL)
    {
        iniparser_freedict(ini);
        return -1;
    }
    strncpy(pinfo->bsp_ver, pstr, sizeof(pinfo->bsp_ver));

    //内核版本
    pstr = iniparser_getstring(ini, "cfg:KernelVer", NULL);
    if (pstr == NULL)
    {
        iniparser_freedict(ini);
        return -1;
    }
    strncpy(pinfo->ker_ver, pstr, sizeof(pinfo->ker_ver));

    //计量版本
    pinfo->meter_ver = iniparser_getint(ini, "cfg:MeterVer", -1);
    if (pinfo->meter_ver == -1)
    {
        iniparser_freedict(ini);
        return -1;
    }

    //OEM信息
    pstr = iniparser_getstring(ini, "cfg:OemVer", NULL);
    if (pstr == NULL)
    {
        iniparser_freedict(ini);
        return -1;
    }
    strncpy(pinfo->oem, pstr, sizeof(pinfo->oem));

    /*************************fileJoint**********************/
    pinfo->files = iniparser_getint(ini, "cfg:Files", -1);
    if ((pinfo->files > DEFAULT_MAX_FILE) || (pinfo->files <= 0))
    {
        iniparser_freedict(ini);
        return -1;
    }
    pinfo->blank = iniparser_getint(ini, "cfg:Blank", -1);
    if (pinfo->blank < 0)
    {
        iniparser_freedict(ini);
        return -1;
    }
    if (pinfo->blank > 255)
    {
        pinfo->blank = 255;
    }
    pstr = iniparser_getstring(ini, "cfg:OutFile", NULL);
    if (pstr == NULL)
    {
        strncpy(pinfo->outfile, DEFAULT_DEST_FILE, sizeof(pinfo->outfile));
    }
    else
    {
        strncpy(pinfo->outfile, pstr, sizeof(pinfo->outfile));
    }

    /* 扫描所以文件 */
    char ftmp[32];
    int i = 0;
    for (i = 0; i < pinfo->files; i++)
    {
        sprintf(ftmp, "f%d:FileName", i + 1);
        pstr = iniparser_getstring(ini, ftmp, NULL);
        if (pstr == NULL)
        {
            iniparser_freedict(ini);
            return -1;
        }
        strncpy(pinfo->file[i].filename, pstr, sizeof(pinfo->file[i].filename));

        sprintf(ftmp, "f%d:FileMaxSize", i + 1);
        pinfo->file[i].filemaxsize = iniparser_getint(ini, ftmp, -1);
        if (pinfo->file[i].filemaxsize == -1)
        {
            iniparser_freedict(ini);
            return -1;
        }
    }

    iniparser_freedict(ini);
    return 0;
}
/*----------------------------ini.c--------------------------------*/
