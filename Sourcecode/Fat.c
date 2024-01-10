
/*******************************************************************
 * Includes
 ********************************************************************/
#include <stdio.h>
#include <conio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "HAL.h"
#include "Fat.h"

/*******************************************************************
 * Defines
 ********************************************************************/
#define GET_2_BYTE(buff, index) (uint16_t)(((buff[(index) + 1u]) << 8u) | (buff[index]))

#define GET_4_BYTE(buff, index) (uint32_t)(((buff[(index) + 3u]) << 24u) | \
                                           ((buff[(index) + 2u]) << 16u) | \
                                           ((buff[(index) + 1u]) << 8u) |  \
                                           (buff[index]))

#define OFFSET_SIZE_SECTOR 0x0Bu
#define OFFSET_SIZE_CLUSTER 0x0Du
#define OFFSET_SIZE_BOOTSECTOR 0x0Eu
#define OFFSET_NUM_TABLE_FAT 0x10u
#define OFFSET_NUM_ENTRY_PER_RDET 0x11u
#define OFFSET_SIZE_VOLUME 0X13u
#define OFFSET_SIZE_VOLUME2 0X20u
#define OFFSET_SIZE_FAT 0X16u
#define OFFSET_SIZE_FAT32 0x24u
#define OFFSET_NAME_FAT12_16 0x36u
#define OFFSET_NAME_FAT32 0x52u
#define OFFSET_CLUSTER_START_32 0x2Cu

#define ENTRY_SIZE 32u
#define ENTRY_EMPTY 0u
#define ENTRY_DELETED 0xE5u
#define ENTRY_SUB 0x0Fu
#define ENTRY_OFFSET_SUB 0x0Bu
#define ENTRY_OFFSET_EXTEND 0x08u
#define ENTRY_OFFSET_ATTRIBUTE 0x0Bu
#define ENTRY_OFFSET_HIGH_CLUSTER 0x14u
#define ENTRY_OFFSET_SLOW_CLUSTER 0x1Au
#define ENTRY_OFFSET_HOUR_EDIT 0x16u
#define ENTRY_OFFSET_DATE_UPDATE 0x18u
#define ENTRY_OFFSET_SIZE 0x1Cu

#define MASK_HOUR 0xF800u
#define OFFSET_HOUR 0x0Bu
#define MASK_MINUTE 0x07E0u
#define OFFSET_MINUTE 0x05u
#define MASK_SECOND 0x001Fu
#define MASK_DATE 0x001Fu
#define MASK_MONTH 0x01E0u
#define OFFSET_MONTH 0x05u
#define MASK_YEAR 0xFE00u
#define OFFSET_YEAR 0x09u

#define END_OF_FAT12 0xFFFu
#define END_OF_FAT16 0xFFFFu
#define END_OF_FAT32 0x0FFFFFFFu
#define FAT12 12u
#define FAT16 16u
#define FAT32 32u

#define CLUSTER_RESERVE 0x00u
#define CLUSTER_START 2u


/*******************************************************************
 * Variables
 ********************************************************************/
static FATFS_BootApart_Struct_t g_bootsector;   /* apart of bootsector */
static uint16_t g_first_of_fat = 0u;            /* index first of fat */
static uint16_t g_first_of_rdet = 0u;           /* index first of rdet */
static uint16_t g_first_of_data = 0u;           /* index first of data */
static uint8_t *g_fatfs_table = NULL;           /* save buff of fat table */
static uint8_t g_type_fat = 0u;                 /* type of fat */
static uint32_t g_end_of_fat = 0u;              /* value end of in FAT table */
static uint32_t g_cluster_start_rdet_32 = 0u;   /* cluster start of rdet in fat32 */

/*******************************************************************
 * Prototypes
 ********************************************************************/
/*
 *@brief       - get name type fat or dir or file from buffer
 *@param       - name of fat or dir or file
 *@param       - buffer_hex is buffer of numbers hexa
 *@param       - length is length of buffer
 *@return      - return name of type fat or dir or file
 **/
static void FATFS_GetNameFromBuffHex(uint8_t *name, uint8_t *buffer_hex, uint8_t length);

/*
 *@brief       - get all entry of RDET
 *@param       - buff save all byte of RDET
 *@param       - manage_entries is saved list all entry in RDET
 *@return      - return FATFS_CodeReturn_t
 */

static FATFS_CodeReturn_t FATFS_GetEntryRdet(FATFS_ManageEntry_Struct_t *manage_entries);

/*
 *@brief       - get all entry of SDET
 *@param       - buff save all byte of SDET
 *@param       - manage_entries is saved list all entry in SDET
 *@return      - return FATFS_CodeReturn_t
 */
static FATFS_CodeReturn_t FATFS_GetEntrySdet(const uint32_t cluster_start, FATFS_ManageEntry_Struct_t *manage_entries);

/*
 *@brief       - get next cluster of current cluster
 *@param       - curIndexCluster is cluster current
 *@return      - value of next cluster
 **/
static uint32_t FATFS_GetNextCluster(const uint32_t current_cluster);

/*******************************************************************
 * Codes
 ********************************************************************/

/* get name type fat or dir or file from buffer */
static void FATFS_GetNameFromBuffHex(uint8_t *name, uint8_t *buffer_hex, uint8_t length)
{
    uint8_t i = 0u;
    while (i < length)
    {
        if (*buffer_hex == 0x20u) /* 0x20 is charater space */
        {
            name[i] = '\0';
        }
        else
        {
            name[i] = *buffer_hex;
        }
        buffer_hex++;
        i++;
    }
}

/* function to get all entry of RDET */
static FATFS_CodeReturn_t FATFS_GetEntryRdet(FATFS_ManageEntry_Struct_t *manage_entries)
{
    FATFS_CodeReturn_t retcode = non_error;
    uint16_t i = 0;
    uint16_t index_entry = 0;
    bool end_of_entry = false;
    uint16_t num_sector_per_RDET = (g_bootsector.num_entries_per_rdet * ENTRY_SIZE) / g_bootsector.num_byte_per_sector;
    uint8_t* buff = (uint8_t*)calloc(num_sector_per_RDET * g_bootsector.num_byte_per_sector, sizeof(uint8_t));
    
    manage_entries->list_entries = calloc(ENTRY_SIZE * g_bootsector.num_entries_per_rdet, sizeof(uint8_t *));
    manage_entries->num_entries = 0;
    if ((NULL == buff) || (NULL == manage_entries->list_entries))
    {
        retcode = error_memory;
    }
    else
    {
        if ((num_sector_per_RDET * g_bootsector.num_byte_per_sector) != HAL_ReadMultiSector(g_first_of_rdet, num_sector_per_RDET, buff))
        {
            retcode = error_read_sector;
        }
        else
        {
            while ((i < g_bootsector.num_entries_per_rdet * ENTRY_SIZE) && (end_of_entry == false))
            {
                if ((buff[i] != ENTRY_DELETED) && (buff[i + ENTRY_OFFSET_SUB] != ENTRY_SUB))
                {
                    /* name dir is 8 bytes */
                    FATFS_GetNameFromBuffHex((uint8_t *)&manage_entries->list_entries[index_entry].name_dir, &buff[i], 8u);
                    /* name extend is 3 bytes */
                    FATFS_GetNameFromBuffHex((uint8_t *)&manage_entries->list_entries[index_entry].name_extend, &buff[i + ENTRY_OFFSET_EXTEND], 3u);
                    manage_entries->list_entries[index_entry].attribute = buff[i + ENTRY_OFFSET_ATTRIBUTE];
                    /* cluster start is 4 byte -> byte high << 16 bit */
                    manage_entries->list_entries[index_entry].cluster_start = GET_2_BYTE(buff, i + ENTRY_OFFSET_HIGH_CLUSTER) << 16;
                    manage_entries->list_entries[index_entry].cluster_start |= GET_2_BYTE(buff, i + ENTRY_OFFSET_SLOW_CLUSTER);
                    manage_entries->list_entries[index_entry].last_hour_edit.second = GET_2_BYTE(buff, i + ENTRY_OFFSET_HOUR_EDIT) & MASK_SECOND;
                    manage_entries->list_entries[index_entry].last_hour_edit.second *= 2;
                    manage_entries->list_entries[index_entry].last_hour_edit.minute = (GET_2_BYTE(buff, i + ENTRY_OFFSET_HOUR_EDIT) & MASK_MINUTE) >> OFFSET_MINUTE;
                    manage_entries->list_entries[index_entry].last_hour_edit.hour = (GET_2_BYTE(buff, i + ENTRY_OFFSET_HOUR_EDIT) & MASK_HOUR) >> OFFSET_HOUR;
                    manage_entries->list_entries[index_entry].last_date_update.date = GET_2_BYTE(buff, i + ENTRY_OFFSET_DATE_UPDATE) & MASK_DATE;
                    manage_entries->list_entries[index_entry].last_date_update.month = (GET_2_BYTE(buff, i + ENTRY_OFFSET_DATE_UPDATE) & MASK_MONTH) >> OFFSET_MONTH;
                    manage_entries->list_entries[index_entry].last_date_update.year = (GET_2_BYTE(buff, i + ENTRY_OFFSET_DATE_UPDATE) & MASK_YEAR) >> OFFSET_YEAR;
                    manage_entries->list_entries[index_entry].last_date_update.year += 1980; /* because value save in entry is minus 1980 */
                    manage_entries->list_entries[index_entry].size_directory = GET_4_BYTE(buff, i + ENTRY_OFFSET_SIZE);
                    index_entry++;
                    manage_entries->num_entries++;
                }
                if (buff[i] == ENTRY_EMPTY)
                {
                    end_of_entry = true;
                    manage_entries->num_entries--;
                    index_entry--;
                }
                i = i + 32;
            }            
        }
    }
    free(buff);
    return retcode;
}

/* function to get all entry of SDET */
static FATFS_CodeReturn_t FATFS_GetEntrySdet(const uint32_t cluster_start, FATFS_ManageEntry_Struct_t *manage_entries)
{
    FATFS_CodeReturn_t retcode = non_error;
    uint16_t number_byte_per_cluster = g_bootsector.num_sector_per_cluster * g_bootsector.num_byte_per_sector;
    uint32_t next_cluster = cluster_start;
    uint32_t sector_startof_cluster = 0;
    uint16_t count_cluster = 0;
    uint16_t index_entry = 0;
    uint16_t i = 0;
    bool end_of_entry = false;
    uint8_t* buff = (uint8_t*) calloc(number_byte_per_cluster, sizeof(uint8_t));
    
    while (next_cluster != g_end_of_fat)
    {
        count_cluster++;
        next_cluster = FATFS_GetNextCluster(next_cluster);
    }
    next_cluster = cluster_start; /* update next_cluster is cluster start to start read */
    manage_entries->list_entries = calloc(count_cluster * number_byte_per_cluster, sizeof(uint8_t));
    manage_entries->num_entries = 0;
    if ((NULL == buff) || (NULL == manage_entries->list_entries))
    {
        retcode = error_memory;
    }
    else
    {
        while (next_cluster != g_end_of_fat)
        {
            sector_startof_cluster = (next_cluster - 2u) * g_bootsector.num_sector_per_cluster + g_first_of_data;
            if (HAL_ReadMultiSector(sector_startof_cluster, g_bootsector.num_sector_per_cluster, buff) != number_byte_per_cluster)
            {
                retcode = error_read_sector;
            }
            else
            {
                i = 0; /* update i to read entries at cluster next */
                while ((i < number_byte_per_cluster) && (end_of_entry == false))
                {
                    if ((buff[i] != ENTRY_DELETED) && (buff[i + ENTRY_OFFSET_SUB] != ENTRY_SUB))
                    {
                        /* name dir is 8 bytes */
                        FATFS_GetNameFromBuffHex((uint8_t *)&manage_entries->list_entries[index_entry].name_dir, &buff[i], 8u);
                        /* name extend is 3 bytes */
                        FATFS_GetNameFromBuffHex((uint8_t *)&manage_entries->list_entries[index_entry].name_extend, &buff[i + ENTRY_OFFSET_EXTEND], 3u);
                        manage_entries->list_entries[index_entry].attribute = buff[i + ENTRY_OFFSET_ATTRIBUTE];
                        /* cluster start is 4 byte -> byte high << 16 bit */
                        manage_entries->list_entries[index_entry].cluster_start = GET_2_BYTE(buff, i + ENTRY_OFFSET_HIGH_CLUSTER) << 16;
                        manage_entries->list_entries[index_entry].cluster_start |= GET_2_BYTE(buff, i + ENTRY_OFFSET_SLOW_CLUSTER);
                        manage_entries->list_entries[index_entry].last_hour_edit.second = GET_2_BYTE(buff, i + ENTRY_OFFSET_HOUR_EDIT) & MASK_SECOND;
                        manage_entries->list_entries[index_entry].last_hour_edit.second *= 2;
                        manage_entries->list_entries[index_entry].last_hour_edit.minute = (GET_2_BYTE(buff, i + ENTRY_OFFSET_HOUR_EDIT) & MASK_MINUTE) >> OFFSET_MINUTE;
                        manage_entries->list_entries[index_entry].last_hour_edit.hour = (GET_2_BYTE(buff, i + ENTRY_OFFSET_HOUR_EDIT) & MASK_HOUR) >> OFFSET_HOUR;
                        manage_entries->list_entries[index_entry].last_date_update.date = GET_2_BYTE(buff, i + ENTRY_OFFSET_DATE_UPDATE) & MASK_DATE;
                        manage_entries->list_entries[index_entry].last_date_update.month = (GET_2_BYTE(buff, i + ENTRY_OFFSET_DATE_UPDATE) & MASK_MONTH) >> OFFSET_MONTH;
                        manage_entries->list_entries[index_entry].last_date_update.year = (GET_2_BYTE(buff, i + ENTRY_OFFSET_DATE_UPDATE) & MASK_YEAR) >> OFFSET_YEAR;
                        manage_entries->list_entries[index_entry].last_date_update.year += 1980; /* because value save in entry is minus 1980 */
                        manage_entries->list_entries[index_entry].size_directory = GET_4_BYTE(buff, i + ENTRY_OFFSET_SIZE);
                        index_entry++;
                        manage_entries->num_entries++;
                    }
                    if (buff[i] == ENTRY_EMPTY)
                    {
                        end_of_entry = true;
                        manage_entries->num_entries--;
                        index_entry--;
                    }
                    i = i + 32;
                }
            }
            next_cluster = FATFS_GetNextCluster(next_cluster);
        }
    }
    free(buff);

    return retcode;
}

/* function to get next cluster */
static uint32_t FATFS_GetNextCluster(const uint32_t current_cluster)
{
    uint32_t next_cluster = 0u;
    uint32_t index_fat = 0u;

    if (FAT16 == g_type_fat)
    {
        index_fat = 2u * current_cluster;
        next_cluster = GET_2_BYTE(g_fatfs_table, index_fat);
    }
    else if (FAT12 == g_type_fat)
    {
        index_fat = current_cluster * 3u / 2u;
        if (current_cluster % 2u != 0u)
        {
            next_cluster = (uint32_t)(g_fatfs_table[index_fat] >> 4u) | ((uint16_t)(g_fatfs_table[index_fat + 1u] << 4u));
        }
        else
        {
            next_cluster = (uint32_t)(g_fatfs_table[index_fat]) | ((uint16_t)(g_fatfs_table[index_fat + 1] & 0x0Fu) << 8u);
        }
    }
    else /* FAT32 */
    {
        index_fat = 4u * current_cluster;
        next_cluster = GET_4_BYTE(g_fatfs_table, index_fat);
    }

    return next_cluster;
}

/* Fatfs init */
FATFS_CodeReturn_t FATFS_Init(const uint8_t *path, FATFS_BootApart_Struct_t *boot)
{
    uint32_t num_byte_per_FAT = 0u;
    FATFS_CodeReturn_t retCode = non_error;
    uint8_t *buff_bootsector = calloc(512u, sizeof(uint8_t)); /* read 512 byte first to have parameter bootsector*/
    uint32_t num_sector_per_data = 0u;

    if (buff_bootsector == NULL)
    {
        retCode = error_memory;
    }
    else
    {
        if (!HAL_OpenFileImage(path))
        {
            retCode = error_open_image;
        }
        else if (HAL_ReadSector(0u, buff_bootsector) != 512u)
        {
            retCode = error_read_sector;
        }
        else
        {
            g_bootsector.num_byte_per_sector = GET_2_BYTE(buff_bootsector, OFFSET_SIZE_SECTOR);
            g_bootsector.num_sector_per_cluster = buff_bootsector[OFFSET_SIZE_CLUSTER];
            g_bootsector.num_sector_per_bootsector = GET_2_BYTE(buff_bootsector, OFFSET_SIZE_BOOTSECTOR);
            g_bootsector.num_fat = buff_bootsector[OFFSET_NUM_TABLE_FAT];
            g_bootsector.num_entries_per_rdet = GET_2_BYTE(buff_bootsector, OFFSET_NUM_ENTRY_PER_RDET);
            g_bootsector.num_sector_per_vol = GET_2_BYTE(buff_bootsector, OFFSET_SIZE_VOLUME);
            if (0 == g_bootsector.num_sector_per_vol) /* number_secter_per_vol wil save at offset OFFSET_SIZE_VOLUME2 */
            {
                g_bootsector.num_sector_per_vol = GET_4_BYTE(buff_bootsector, OFFSET_SIZE_VOLUME2);
            }

            /* find type FAT */
            
            num_sector_per_data = g_bootsector.num_sector_per_vol - g_bootsector.num_sector_per_bootsector 
                                - g_bootsector.num_fat * g_bootsector.num_sector_per_fat 
                                - (g_bootsector.num_entries_per_rdet * ENTRY_SIZE) / g_bootsector.num_byte_per_sector;
            if (num_sector_per_data / g_bootsector.num_sector_per_cluster <= 4085u) /* 4085 is max cluster of data FAT12*/
            {
                g_type_fat = FAT12;
                g_end_of_fat = END_OF_FAT12;
            }
            else if ((num_sector_per_data / g_bootsector.num_sector_per_cluster > 4085u) && (num_sector_per_data / g_bootsector.num_sector_per_cluster <= 65525u) )
            {
            	/* 4086 <= max cluster of data FAT16 <= 65525*/
                g_type_fat = FAT16;
                g_end_of_fat = END_OF_FAT16;
            }
            else
            {
            	/* FAT32 */
                g_type_fat = FAT32;
                g_end_of_fat = END_OF_FAT32;
                g_cluster_start_rdet_32 = GET_4_BYTE(buff_bootsector, OFFSET_CLUSTER_START_32);
			}

            if (FAT32 == g_type_fat)
            {
                FATFS_GetNameFromBuffHex((uint8_t *)&g_bootsector.type_fat, &buff_bootsector[OFFSET_NAME_FAT32], 8u);  /* type of fat32 at offset 0x52 */
            }
            else
            {
                FATFS_GetNameFromBuffHex((uint8_t *)&g_bootsector.type_fat, &buff_bootsector[OFFSET_NAME_FAT12_16], 8u); /* type of fat12,16 at offset 0x36 */
            }
            /* determine start of field */
            g_first_of_fat = g_bootsector.num_sector_per_bootsector;
            if ((g_type_fat == FAT12) || (g_type_fat == FAT16))
            {
                g_bootsector.num_sector_per_fat = GET_2_BYTE(buff_bootsector, OFFSET_SIZE_FAT);
                g_first_of_rdet = g_first_of_fat + g_bootsector.num_sector_per_fat * 2u;
                g_first_of_data = g_first_of_rdet + (g_bootsector.num_entries_per_rdet * ENTRY_SIZE) / g_bootsector.num_byte_per_sector;
            }
            else
            { 
                g_bootsector.num_sector_per_fat = GET_4_BYTE(buff_bootsector, OFFSET_SIZE_FAT32);    /* FAT32 Root is start of field DATA */
                g_first_of_data = g_first_of_fat + g_bootsector.num_sector_per_fat * 2u;
            }

            /* read fatfs table */
            HAL_UpdateSizeSector(g_bootsector.num_byte_per_sector); /* update to HAL */
            num_byte_per_FAT = g_bootsector.num_byte_per_sector * g_bootsector.num_sector_per_fat;
            g_fatfs_table = (uint8_t *)calloc(num_byte_per_FAT, sizeof(uint8_t));
            if (NULL == g_fatfs_table )
            {
                retCode = error_memory;
            }
            else if (num_byte_per_FAT != HAL_ReadMultiSector(g_first_of_fat, g_bootsector.num_sector_per_fat, g_fatfs_table))
            {
                retCode = error_read_sector;
            }
            *boot = g_bootsector;
        }
    }
    free(buff_bootsector);

    return retCode;
}

/* fucntion to read folder */
FATFS_CodeReturn_t FATFS_ReadFolder(const uint32_t cluster_start, FATFS_ManageEntry_Struct_t *manage_entries)
{
    FATFS_CodeReturn_t retcode = non_error;

    if (cluster_start == 0) // read root folder
    {
        if ((FAT12 == g_type_fat) || (FAT16 == g_type_fat))
        {
            FATFS_GetEntryRdet(manage_entries);   /* read root FAT12 16 */
        }
        else
        {
            FATFS_GetEntrySdet(g_cluster_start_rdet_32,manage_entries);  /* read root FAT32 */
        }
    }
    else if (cluster_start >= CLUSTER_START) // read sub directory or file
    {
        /* free list before read entry of folder */
        free(manage_entries->list_entries);
        retcode = FATFS_GetEntrySdet(cluster_start, manage_entries);
    }

    return retcode;
}

/* read file */
FATFS_CodeReturn_t FATFS_ReadFile(uint8_t *buff_of_file,const uint32_t cluster_start)
{
    FATFS_CodeReturn_t retcode = non_error;
    uint32_t next_cluster = cluster_start;
    uint32_t sector_startof_cluster = 0u;
    uint16_t count_cluster = 0u;
    uint32_t num_byte_per_cluster = 0u;
    uint32_t i = 0u;

    if (cluster_start != CLUSTER_RESERVE)
    {
        num_byte_per_cluster = g_bootsector.num_sector_per_cluster * g_bootsector.num_byte_per_sector;
        while (next_cluster != g_end_of_fat)
        {
            /* number 2 is minus 2 position 0 and 1 of cluster don't used */
            sector_startof_cluster = (next_cluster - 2u) * g_bootsector.num_sector_per_cluster + g_first_of_data;
            if (num_byte_per_cluster != HAL_ReadMultiSector(sector_startof_cluster, g_bootsector.num_sector_per_cluster, &buff_of_file[num_byte_per_cluster * i]))
            {
                retcode = error_read_sector;
            }
            else
            {
                i++;
                next_cluster = FATFS_GetNextCluster(next_cluster);
            }
        }
    }

    return retcode;
}
/*******************************************************************
 * EOF
 ********************************************************************/

