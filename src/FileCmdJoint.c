/*
 ============================================================================
 Name        : FileCmd.c
 Author      : LiuNing
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "md5.h"
#include "types.h"
#include "ini.h"

#define VERSION     "1.0.0"

#define BIN_FILE        "rtos.bin"
#define INFO_FILE        "README.txt"
#define INFO_STR          "--FILE INFO--"
#define UPDT_FILE_APP       "update.sp4"
#define BUF_SIZE            (1024)

#pragma pack(push, 1)
/** �����ļ���Ϣ */
typedef struct
{
    uint8 file_type;  /**< �����ļ����� */
    int8 file_name[24];     /**< �ļ��� */
    uint32 dest_addr;       /**< Ŀ���ַ */
    uint32 zip_len;         /**< ��������еĳ��ȣ�ѹ���� */
    uint32 raw_len;         /**< ԭʼ���ȣ�ѹ��ǰ�� */
    uint8 md5[16];          /**< ԭʼ�ļ�md5ֵ��ѹ��ǰ�� */
} file_info_t;

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
            char_t meter_ver[2];/**< ������汾��2�ֽڣ� */
            char_t oem[4];      /**< OEM��Ϣ��4�ֽڣ�����ĸ��ʾ,���Ϻ�����SHLN�� */
        };
        char_t t_info[11];      /**< �ն���Ϣ�� */
    };
    uint8 cs;           /**< ���������ݽ����ۼ���� */
} usb_update_info_t;

/** �������� */
typedef struct
{
    uint16 crc;             /**< CRC���� */
    uint32 len;             /**< ���ȣ�������crc��len */
    uint8 id[16];           /**< �����ʶ */
    uint8 is_zip;          /**< �Ƿ�ѹ�� */
    uint8 is_reboot;       /**< �Ƿ���Ҫ���� */
    uint8 dummy;            /**< ���� */
    uint8 file_count;       /**< �ļ����� */
    file_info_t file_info;  /**< �ļ���Ϣ */
} update_cmd_t;
#pragma pack(pop)


extern status_t
get_update_info_24bytes(char *pinfo, char *pfilename);

/**
 ******************************************************************************
 * @brief      CRCУ�����
 * @param[in]  int8 * ptr: У��������ʼ��ַ
 * @param[in]  uint16 count: У�����ݳ���
 * @retval     uint16: CRCУ����
 ******************************************************************************
 */
uint16
get_crc16(const int8 * pdata, uint16 count, uint16 crc)
{
    uint16 i;
    while (count > 0)
    {
        crc = crc ^ (uint16)((uint16)*pdata << 8u);
        pdata++;

        for (i = 0; i < 8; i++)
        {
            if ((crc & 0x8000u) != 0)
            {
                crc = ((uint16)(crc << 1)) ^ 0x1021u;
            }
            else
            {
                crc = (uint16)(crc << 1u);
            }
        }
        count--;
    }

    return (crc & 0xFFFFu);
}

/**
 ******************************************************************************
 * @brief   ����ڴ�����Ƿ�����ַ���
 * @param[in]  *paddr: �ڴ���ʼ��ַ
 * @param[in]  len   : �ڴ泤��
 * @param[in]  *pecho: �����ҵ��ַ���
 *
 * @retval  NULL : δ�ҵ�
 * @retval !NULL : �׵�ַ
 ******************************************************************************
 */
const char *
memstr(const void *paddr,
        int len,
        const char *pecho)
{
    int i;
    int chk_len = strlen(pecho);
    const char *pmem = (const char *)paddr;

    if (!pecho || (chk_len > len) || (chk_len <= 0))
    {
        return NULL;
    }

    for (i = 0; i <= (len - chk_len); i++)
    {
        if (!memcmp(pmem + i, pecho, chk_len))
        {
            return pmem + i;
        }
    }

    return NULL;
}

/**
 ******************************************************************************
 * @brief   ����ڴ�����Ƿ�����ַ���
 * @param[in]  *paddr: �ڴ���ʼ��ַ
 * @param[in]  len   : �ڴ泤��
 * @param[in]  *pecho: �����ҵ��ַ���
 *
 * @retval  NULL : δ�ҵ�
 * @retval !NULL : �׵�ַ
 ******************************************************************************
 */
const char *
memstr_back(const void *paddr,
        int32_t len,
        const char *pecho)
{
    int32_t i;
    int32_t chk_len = strlen(pecho);
    const char *pmem = (const char *)paddr;

    if (!pecho || (chk_len > len) || (chk_len <= 0))
    {
        return NULL;
    }

    for (i = (len - chk_len - 1); i >= 0; i--)
    {
        if (!memcmp(pmem + i, pecho, chk_len))
        {
            return pmem + i;
        }
    }

    return NULL;
}

/**
 ******************************************************************************
 * @brief      bin�ļ������Ϣ��ĩβ
 * @retval     0���ɹ�
 ******************************************************************************
 */
int modify_bin_file(void)
{
    FILE* bin_file = fopen(BIN_FILE, "rb+");
    FILE* info_file = fopen(INFO_FILE, "rb");
    if (bin_file == NULL)
    {
        printf("�Ҳ���rtos.bin\n");
        goto __return_err;
    }
    if (info_file == NULL)
    {
        printf("�Ҳ���README.txt\n");
        goto __return_err;
    }

    cmd_joint_ini_t fji;
    if (0 != ini_get_info(&fji))
    {
        printf("ini get info err!\n");
        goto __return_err;
    }

    char buf[BUF_SIZE];
    fseek(bin_file, -BUF_SIZE, SEEK_END);
    fread(buf, 1, BUF_SIZE, bin_file);

    const char *find_info = memstr(buf, BUF_SIZE, INFO_STR);

    fseek(bin_file, 0, SEEK_END);
    int bin_file_size = ftell(bin_file);
    if (find_info != NULL)
    {
        int info_size = BUF_SIZE - (find_info - buf);
        bin_file_size -= info_size;

        /* set old info to 0xff */
        memset(buf, 0xff, BUF_SIZE);
        fseek(bin_file, -info_size, SEEK_END);
        fwrite(buf, 1, info_size, bin_file);
        printf("rtos.bin has already been modified!, info len:%d\n", info_size);
    }

    fseek(info_file, 0, SEEK_END);
    int info_file_size = ftell(info_file);
    int max_write_size = fji.file[1].filemaxsize - bin_file_size;
    if (max_write_size < 0)
    {
        printf("max_write_size error:%d, fji.file[1].filemaxsize:%d, bin_file_size:%d\n"
                , max_write_size, fji.file[1].filemaxsize, bin_file_size);
        goto __return_err;
    }
    if (info_file_size > BUF_SIZE - 1)
    {
        printf("README.txt is too large: %d > %d\n", info_file_size, BUF_SIZE - 1);
        goto __return_err;
    }

    int write_size = info_file_size < max_write_size ? info_file_size : max_write_size;
    fseek(info_file, 0, SEEK_SET);
    int info_len = fread(buf, 1, write_size, info_file);
    buf[write_size - 1] = 0x00;

    /* 4byte ���� */
    if (info_len % 4 != 0)
    {
        int fill_byte_size = 4 - (info_len % 4);
        int i = 0;
        printf("Kay, 4byte fill: %d\n", fill_byte_size);
        for (; i < fill_byte_size; i++)
        {
            buf[write_size + i] = 0xff;
            info_len += 1;
        }
    }

    /* д��info */
    fseek(bin_file, bin_file_size, SEEK_SET);
    int w_len = fwrite(buf, 1, info_len, bin_file);
    if (info_len != w_len)
    {
        printf("д�ļ�����!len:%d, wlen:%d\n", info_len, w_len);
        goto __return_err;
    }

    if (bin_file)fclose(bin_file);
    if (info_file)fclose(info_file);
    return OK;

__return_err:
    if (bin_file)fclose(bin_file);
    if (info_file)fclose(info_file);
    return ERROR;
}

/**
 ******************************************************************************
 * @brief      ���������ļ�
 * @retval     0���ɹ�
 ******************************************************************************
 */
int create_updt_file(void)
{
    char buf[BUF_SIZE];
    MD5_CTX ctx;
    update_cmd_t header;
    file_info_t *pfinfo = &header.file_info;
    int write_size = 0;
    int file_size = 0;
    size_t len = 0;

    char fileinfo[24];
    char dstfilename[32];

    FILE* fpdst = NULL;
    FILE* fpsrc = NULL;    //Ŀ���ļ� /Debug/rtos.bin

    //1. �������ļ����fileinfo��dstfilename
    memset(dstfilename, 0x00, sizeof(dstfilename));
    if (OK != get_update_info_24bytes(fileinfo, dstfilename))
    {
        printf("�������ļ������Ϣʧ��!����Ĭ���ļ���:%s\n", UPDT_FILE_APP);
        strncpy(dstfilename, UPDT_FILE_APP, sizeof(dstfilename));
        memset(fileinfo, 0x00, sizeof(fileinfo));
    }

    fpsrc = fopen(BIN_FILE, "rb");
    if (fpsrc == NULL)
    {
        printf("�Ҳ���rtos.bin\n");
        goto __return_err;
    }

    fseek(fpsrc, 0, SEEK_END);
    file_size = ftell(fpsrc);
    fseek(fpsrc, 0, SEEK_SET);


    fpdst = fopen(dstfilename, "w+b");
    if (fpdst == NULL)
    {
        printf("�����ļ�ʧ��!\n");
        goto __return_err;
    }
    fseek(fpdst, sizeof(update_cmd_t), SEEK_SET);
    /* md5У�� */
    MD5Init(&ctx);
    write_size = 0;
    while (write_size < file_size)
    {
        len = fread(buf, 1, BUF_SIZE, fpsrc);
        MD5Update(&ctx, (char*)buf, len);
        if (len != fwrite(buf, 1, len, fpdst))
        {
           printf("д�ļ�����!\n");
           write_size = -1;
           break;
        }
        write_size += len;
    }
    if (write_size == -1)
    {
        goto __return_err;
    }
    printf("�����ļ���С: %d �ֽ�\n", write_size);
    pfinfo->file_type = 0;  //Ӧ�ó���
    memcpy(pfinfo->file_name, fileinfo, sizeof(pfinfo->file_name));
    pfinfo->dest_addr = 0;  //Ŀ���ַ0
    pfinfo->zip_len = write_size;
    pfinfo->raw_len = write_size;

    MD5Final(pfinfo->md5, &ctx);

    memcpy(header.id, pfinfo->md5, 16);
    header.is_reboot = TRUE;
    header.is_zip = FALSE;
    header.len = write_size + sizeof(update_cmd_t) - 6;
    header.file_count = 1;
    header.dummy = 0;
    header.crc = get_crc16((int8*)&header.len, sizeof(header) - 2, 0);
    fflush(fpdst);
    fseek(fpdst, 0, SEEK_SET);
    if (sizeof(update_cmd_t) != fwrite((void*)&header, 1, sizeof(update_cmd_t), fpdst))
    {
        printf("д�ļ�����!\n");
        goto __return_err;
    }

    //���ļ�1k����
    fseek(fpdst, 0, SEEK_END);
    file_size = ftell(fpdst);
    write_size = (file_size % 1024) ? (1024 - (file_size % 1024)) : 0;
    if (write_size > 0)
    {
        memset(buf, 0xff, sizeof(buf));
        if (write_size != fwrite(buf, 1, write_size, fpdst))
        {
            printf("�����ļ�����!\n");
            goto __return_err;
        }
    }
    printf("���� U�������ļ�:%s, ���뵽 %d �ֽ�\n", dstfilename, file_size + write_size);

    if (fpdst != NULL)fclose(fpdst);
    if (fpsrc != NULL)fclose(fpsrc);
    return 0;

__return_err:
    if (fpdst != NULL)fclose(fpdst);
    if (fpsrc != NULL)fclose(fpsrc);
    return -1;
}

/**
 ******************************************************************************
 * @brief      ���ɺϳ��ļ�
 * @retval     0���ɹ�
 ******************************************************************************
 */
int create_joint_file(void)
{
    cmd_joint_ini_t fji;
    char buf[BUF_SIZE];
    FILE* fdest = NULL;
    FILE* fcur = NULL;
    int file_size = 0;
    int write_size = 0;
    int len = 0;
    int i;

    /* 0. ��ʼ������ */
    if (0 != ini_get_info(&fji))
    {
        printf("ini get info err!\n");
        goto __return_err;
    }
    //todo: log

    /* 1. У���ļ����� */
    for (i = 0; i < fji.files; i++)
    {
        fcur = fopen(fji.file[i].filename, "rb");
        if (fcur == NULL)
        {
            printf("%s������!\n", fji.file[i].filename);
            goto __return_err;
        }
        fseek(fcur, 0, SEEK_END);
        file_size = ftell(fcur);
        if (file_size > fji.file[i].filemaxsize)
        {
            printf("%s:size[%d > %d]\n", fji.file[i].filename,
                    file_size, fji.file[i].filemaxsize);
            goto __return_err;
        }
        (void)fclose(fcur);
        fcur = NULL;
    }

    /* 2. ����Ŀ���ļ� */
    fdest = fopen(fji.outfile, "w+b");
    if (fdest == NULL)
    {
        printf("�����ļ�%sʧ��!\n", fji.outfile);
        goto __return_err;
    }

    /* 3. д������ */
    for (i = 0; i < fji.files; i++)
    {
        fcur = fopen(fji.file[i].filename, "rb");
        if (fcur == NULL)
        {
            printf("%s������!\n", fji.file[i].filename);
            goto __return_err;
        }
        fseek(fcur, 0, SEEK_END);
        file_size = ftell(fcur);
        write_size = 0;
        fseek(fcur, 0, SEEK_SET);
        while (write_size < file_size)
        {
            len = fread(buf, 1, BUF_SIZE, fcur);
            if ((len <= 0) || (len != fwrite(buf, 1, len, fdest)))
            {
               printf("д�ļ�����[L%d]!\n", __LINE__);
               goto __return_err;
            }
            write_size += len;
        }
        (void)fclose(fcur);
        fcur = NULL;
        //����ȱ�ռ�
        memset(buf, fji.blank, sizeof(buf));
        write_size = fji.file[i].filemaxsize - file_size;
        while (write_size > 0)
        {
            len = (write_size > BUF_SIZE) ? BUF_SIZE : write_size;
            if (len != fwrite(buf, 1, len, fdest))
            {
               printf("д�ļ�����[L%d]!\n", __LINE__);
               goto __return_err;
            }
            write_size -= len;
        }
    }

    if (fdest != NULL)fclose(fdest);
    if (fcur != NULL)fclose(fcur);
    return 0;

__return_err:
    if (fdest != NULL)fclose(fdest);
    if (fcur != NULL)fclose(fcur);
    return -1;
}

int main(int argc, char**argv)
{
    int ret = 0;
    printf("FileCmdJoint v%s\n", VERSION);

    printf("\n1.file modify\n");
    if (modify_bin_file() != OK)
    {
        printf("failed!\n");
    }
    else printf("ok\n");

    printf("\n2.fileCmd\n");
	if (create_updt_file() != 0)
	{
	    ret |= 1;
	    printf("failed!\n");
	}
    else printf("ok\n");

	printf("\n3.FileJoint\n");
	if (create_joint_file() != 0)
    {
	    ret |= 2;
	    printf("failed!\n");
    }
    else printf("ok\n");

    printf("FileCmdJoint Done:%d\n", ret);
	return ret;
}
