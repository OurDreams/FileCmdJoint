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
 * @brief   ����Ĭ�������ļ�
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
            "#######U����������#######\n"
            "#��ʾ�汾(5�ַ�������CV.03)\n"
            "ShowVer     = CV.03\n"
            "#BSPӲ���汾(7�ַ�������3.4.0.3)\n"
            "BspVer     = 3.4.0.3\n"
            "#�ں˰汾��ASCII�룬5�ַ�������1.0.9��\n"
            "KernelVer  = 1.0.9\n"
            "#�����汾��2�ֽ�,ʮ�����ƣ�,ͨ��shell->vm����鿴\n"
            "MeterVer  = 0x010d\n"
            "#OEM��Ϣ��4�ֽڣ�����ĸ��ʾ,���Ϻ�����SHLN��\n"
            "OemVer  = \n"
            "\n\n"

            "#######�ļ�ƴ������#######\n"
            "#�ļ�������(һ����2,���5)\n"
            "Files = 2\n"
            "#�հ��������(0~255,����255ֱ��Ϊ255)\n"
            "Blank = 255\n"
            "#����ļ���\n"
            "OutFile = ./FLASH.bin\n"
            "\n"
            "#�ļ�1\n"
            "#0x800-->32K\n"
            "[f1]\n"
            "FileName    = ./boot/boot(FKGA23_4_0_3).bin\n"
            "FileMaxSize = 0x8000\n"
            "\n"
            "#�ļ�2\n"
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
 * @brief   �������ļ��л�ȡ�ļ��ϲ���Ϣ
 * @param[out] *pinfo   : ����info
 *
 * @retval      0 �ɹ�
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
    //��ʾ�汾
    pstr = iniparser_getstring(ini, "cfg:ShowVer", NULL);
    if (pstr == NULL)
    {
        iniparser_freedict(ini);
        return -1;
    }
    strncpy(pinfo->show_ver, pstr, sizeof(pinfo->show_ver));

    //BSPӲ���汾
    pstr = iniparser_getstring(ini, "cfg:BspVer", NULL);
    if (pstr == NULL)
    {
        iniparser_freedict(ini);
        return -1;
    }
    strncpy(pinfo->bsp_ver, pstr, sizeof(pinfo->bsp_ver));

    //�ں˰汾
    pstr = iniparser_getstring(ini, "cfg:KernelVer", NULL);
    if (pstr == NULL)
    {
        iniparser_freedict(ini);
        return -1;
    }
    strncpy(pinfo->ker_ver, pstr, sizeof(pinfo->ker_ver));

    //�����汾
    pinfo->meter_ver = iniparser_getint(ini, "cfg:MeterVer", -1);
    if (pinfo->meter_ver == -1)
    {
        iniparser_freedict(ini);
        return -1;
    }

    //OEM��Ϣ
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

    /* ɨ�������ļ� */
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
