/**
 ******************************************************************************
 * @file      filename.c
 * @brief     C Source file of filename.c.
 * @details   This file including all API functions's 
 *            implement of filename.c.	
 *
 * @copyright Copyrigth(C), 2008-2012,Sanxing Electric Co.,Ltd.
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "ini.h"

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
#pragma pack(push, 1)

typedef struct
{
    uint32 magic;       /**< ħ����:0xa1b2c3d4��ʾu������ */
    char_t show_ver[4]; /**< ��ʾ�汾 */
    char_t bsp_ver[4];  /**< bsp�汾 */
    union
    {
        struct
        {
            char_t ker_ver[5];  /**< �ں˰汾��ASCII�룬5�ֽڣ� */
            uint16 meter_ver;   /**< ������汾��2�ֽڣ� */
            char_t oem[4];      /**< OEM��Ϣ��4�ֽڣ�����ĸ��ʾ,���Ϻ�����SHLN�� */
        };
        char_t t_info[11];      /**< �ն���Ϣ�� */
    };
    uint8 cs;           /**< ���������ݽ����ۼ���� */
} my_usb_update_info_t;
#pragma pack(pop)

/*-----------------------------------------------------------------------------
 Section: Constant Definitions
 ----------------------------------------------------------------------------*/
#define PRODUCT_TYPE_STRING     "PRODUCT_TYPE"
#define PRD_AREA_STRING         "PRD_AREA"

#define RTUCFG_FILENAME         "./app/rtucfg.h"

#define BUF_SIZE                (512)

/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
static char the_filename[8 + 1 + 3 + 1];

/*-----------------------------------------------------------------------------
 Section: Local Function Prototypes
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Global Function Prototypes
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/
static int
have_string(FILE *pfd, const char *pstr)
{
    char buf[BUF_SIZE];

    fseek(pfd, 0, SEEK_SET);

    while (fgets(buf, sizeof(buf), pfd))
    {
        if (strstr(buf, pstr) != NULL)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static int
get_value_by_define(FILE *pfd, const char *pdefine)
{
    int i;
    int j;
    char *p;
    char define_char[128];
    char buf[BUF_SIZE];

    fseek(pfd, 0, SEEK_SET);
    snprintf(define_char, sizeof(define_char), "#define %s", pdefine);
    memset(buf, 0x00, sizeof(buf));
    while (fgets(buf, sizeof(buf), pfd))
    {
        p = strstr(buf, define_char);
        if (p != NULL)
        {
            p += strlen(define_char);
            for (i = p - buf; i < sizeof(buf); i++)
            {
                if (buf[i] == ' ')
                {
                    continue;
                }
                if ((buf[i] >= '0') && (buf[i] <= '9'))
                {
                    return atoi(buf + i);
                }
                else if (((buf[i] >= 'a') && (buf[i] <= 'z'))
                        || ((buf[i] >= 'A') && (buf[i] <= 'Z')))
                {
                    for (j = i + 1; j < sizeof(buf); j++)
                    {
                        if ((buf[j] == ' ') || (buf[j] == '/')
                                || (buf[j] == '\r') || (buf[j] == '\n'))
                        {
                            buf[j] = '\0';
                            break;
                        }
                    }
                    if (j >= sizeof(buf))
                    {
                        break;
                    }

                    return get_value_by_define(pfd, buf + i);
                }
            }
        }
        memset(buf, 0x00, sizeof(buf));
    }

    return -1;
}

/**
 ******************************************************************************
 * @brief   ���������ļ�����ļ���
 * @param[in]  *pcfgname : rtucfg.h
 * @param[out] *pfilename: �ļ���
 *
 * @retval  OK    : �ɹ�
 * @retval  ERROR : ʧ��
 ******************************************************************************
 */
static status_t
try_get_filename(const char *pcfgname,
        char *pfilename)
{
    status_t ret = ERROR;
    char gn = 'g';      //����g����n
    int product_type;   //��Ʒ����
    int prd_area;       //�汾��������
    int hard_ver;       //Ӳ���汾
    FILE *pfd = fopen(pcfgname, "r");

    do
    {
        if (!pfd)
        {
            printf("can't find file: \"%s\"\n", pcfgname);
            break;
        }

        //����g����n
        if (TRUE == have_string(pfd, "����"))
        {
            gn = 'n';
        }

        //��Ʒ����
        product_type = get_value_by_define(pfd, PRODUCT_TYPE_STRING);
        printf("product_type:%d\n", product_type);
        if ((product_type < 0) || (product_type > 9))
        {
            break;
        }

        //�汾��������
        prd_area = get_value_by_define(pfd, PRD_AREA_STRING);
        printf("prd_area:%d\n", prd_area);
        if ((prd_area < 0) || (prd_area > 99))
        {
            break;
        }

        //todo: Ӳ���汾
        hard_ver = 0;

        sprintf(pfilename, "up%c%d%02d%02d.sp4", gn,
                product_type, prd_area, hard_ver);
        ret = OK;
    } while (0);

    if (pfd)
    {
        fclose(pfd);
    }
    return OK;
}

/**
 ******************************************************************************
 * @brief   �ֶ�����
 * @param[out] *pfilename: �ļ���
 *
 * @retval  OK    : �ɹ�
 * @retval  ERROR : ʧ��
 ******************************************************************************
 */
status_t
manual_filename(char *pfilename)
{
    return OK;
}

/**
 ******************************************************************************
 * @brief   ��ȡ�����ļ���
 * @retval  �����ļ���
 ******************************************************************************
 */
const char *
get_update_filename(void)
{
    strncpy(the_filename, "update.sp4", sizeof(the_filename));

    if (OK != try_get_filename(RTUCFG_FILENAME, the_filename))
    {
        //todo: �ֹ�����
        (void)manual_filename(the_filename);
    }

    return the_filename;
}

uint8
get_cs(const uint8 * pfbuf, uint16 len)
{
    uint8 cs = 0u;
    int32 i;

    for (i = 0; i < len; i++)
    {
        cs = cs + pfbuf[i];
    }

    return cs;
}

/**
 ******************************************************************************
 * @brief   ��ȡ������Ϣ24�ֽ�
 * @param[out] *pinfo: ������Ϣ
 *
 * @retval  OK    : �ɹ�
 * @retval  ERROR : ʧ��
 ******************************************************************************
 */
status_t
get_update_info_24bytes(char *pinfo, char *pfilename)
{
    cmd_joint_ini_t uui;
    my_usb_update_info_t *p = (my_usb_update_info_t*)pinfo;

    if (0 != ini_get_info(&uui))
    {
        printf("ini get info err!\n");
        goto __return_err;
    }

    //������
    if ((strlen(uui.show_ver) != 5) || (uui.show_ver[2] != '.'))
    {
        printf("�������ʾ�汾(%s)����ȷ!�ο�:CV.03\n", uui.show_ver);
        goto __return_err;
    }
    else
    {
        p->show_ver[0] = uui.show_ver[0];
        p->show_ver[1] = uui.show_ver[1];
        p->show_ver[2] = uui.show_ver[3];
        p->show_ver[3] = uui.show_ver[4];
    }

    if ((strlen(uui.bsp_ver) != 7) || (uui.bsp_ver[1] != '.')
            || (uui.bsp_ver[3] != '.') || (uui.bsp_ver[5] != '.'))
    {
        printf("�����Ӳ���汾(%s)����ȷ!�ο�:3.4.0.3\n", uui.bsp_ver);
        goto __return_err;
    }
    else
    {
        p->bsp_ver[0] = uui.bsp_ver[0];
        p->bsp_ver[1] = uui.bsp_ver[2];
        p->bsp_ver[2] = uui.bsp_ver[4];
        p->bsp_ver[3] = uui.bsp_ver[6];
    }

    if ((strlen(uui.ker_ver) != 5) || (uui.ker_ver[1] != '.')
            || (uui.ker_ver[3] != '.'))
    {
        printf("������ں˰汾(%s)����ȷ!�ο�:1.0.9\n", uui.ker_ver);
        goto __return_err;
    }
    else
    {
        p->ker_ver[0] = uui.ker_ver[0];
        p->ker_ver[1] = uui.ker_ver[1];
        p->ker_ver[2] = uui.ker_ver[2];
        p->ker_ver[3] = uui.ker_ver[3];
        p->ker_ver[4] = uui.ker_ver[4];
    }

    if ((uui.meter_ver > 0xffff) || (uui.meter_ver < 0))
    {
        printf("����ļ����汾(%x)����ȷ!�ο�:0x010d,��ͨ��shell->vm����鿴\n",
                uui.meter_ver);
        goto __return_err;
    }
    else
    {
        p->meter_ver = (uint16)uui.meter_ver;
    }

    if (strlen(uui.oem)!=0 && (strlen(uui.oem) != 4))
    {
        printf("�����OEM��Ϣ(%s)����ȷ!��4�ֽڣ�����ĸ��ʾ,���Ϻ�����SHLN,�ɲ��\n",
                uui.oem);
        goto __return_err;
    }
    else
    {
        p->oem[0] = uui.oem[0];
        p->oem[1] = uui.oem[1];
        p->oem[2] = uui.oem[2];
        p->oem[3] = uui.oem[3];
    }

    p->magic = 0xa1b2c3d4;
    p->cs = get_cs((const uint8 *)p, sizeof(*p) - 1);

    sprintf(pfilename, "%c%c%c%c%c%c.sp4", uui.show_ver[0], uui.show_ver[1],
            p->bsp_ver[0], p->bsp_ver[1], p->bsp_ver[2], p->bsp_ver[3]);
    return OK;

__return_err:
    return ERROR;
}

/*-------------------------------filename.c----------------------------------*/
