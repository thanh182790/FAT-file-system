
/*******************************************************************
 * Includes
 ********************************************************************/
#include <stdio.h>
#include <conio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Fat.h"

/*******************************************************************
 * Defines
 ********************************************************************/
#define PATH "floppy1.img"

/*******************************************************************
 * Prototypes
 ********************************************************************/

/**
 *@brief           show directory
 *@param           manage_entries manage list entries of directory
 *@return          no
 **/
void API_ShowDirectory(FATFS_ManageEntry_Struct_t *manage_entries);

/*
 *@brief            show erro or successful
 *@param            retcode is code return
 *@return           no
 */
void API_ShowResult(FATFS_CodeReturn_t retcode);

/*******************************************************************
 * Codes
 ********************************************************************/

/* show directory */
void API_ShowDirectory(FATFS_ManageEntry_Struct_t *manage_entries)
{
    uint8_t i = 0u;
    uint8_t j = 0u;

    FATFS_Entry_Struct_t *entry = manage_entries->list_entries;
    printf("\n\t%-10s\t%-15s\t%-10s\t%-15s\t\t%-10s\t\t%-15s\n", "STT", "NAME", "EXTEND", " DATE", " TIME", " SIZE");
    printf("\t----------------------------------------------------------------------------------------------------\n");
    while (i < manage_entries->num_entries)
    {
        printf("\t%-10d\t%-15s\t", (i + 1), entry[i].name_dir);
        for (j = 0u; j < 3u; j++)
        {
            printf("%c", entry[i].name_extend[j]);
        }
        printf("\t\t");
        printf(" %.2d/%.2d/%-14.4d\t %-.2d:%.2d:%-10.2d\t %-15d\n", entry[i].last_date_update.date,
               entry[i].last_date_update.month, entry[i].last_date_update.year,
               entry[i].last_hour_edit.hour, entry[i].last_hour_edit.minute, entry[i].last_hour_edit.second,
               entry[i].size_directory);
        i++;
    }
    printf("\t----------------------------------------------------------------------------------------------------\n");
}

/* Manage program */
FATFS_CodeReturn_t API_ManageProgram(void)
{
    uint32_t option = 0u;
    uint8_t character_check = '0'; /* use to check input is number iterger or not */
    uint8_t *buff_file = NULL;
    uint32_t i = 0u;
    uint32_t cout = 0u;
    uint32_t size_allocate = 0u; /* size is roundup to cluster when read file */
    FATFS_CodeReturn_t retcode = non_error;

    FATFS_ManageEntry_Struct_t manage_entries = {NULL, 0u};
    FATFS_BootApart_Struct_t bootsector;
    retcode = FATFS_Init(PATH, &bootsector);
    if(non_error == retcode)
    {
        retcode = FATFS_ReadFolder(0u, &manage_entries);  /* Read Root Directory */
        if(non_error == retcode)
        {
            API_ShowDirectory(&manage_entries);
            do
            {
                do
                {
                    printf("\n ENTER YOUR OPTION ( PRESS 0 TO EXIT ): ");
                    fflush(stdin);
                }
                while ((scanf("%d%c", &option, &character_check) != 2u) || (character_check != '\n'));
                if ((option > 0u) && (option <= manage_entries.num_entries))
                {
                    system("cls");
                    if (manage_entries.list_entries[option - 1u].attribute == IS_FOLDER) /* read folder */
                    {
                        retcode = FATFS_ReadFolder(manage_entries.list_entries[option - 1].cluster_start, &manage_entries);
                        if(non_error == retcode)
                        {
                            printf("\n\t\t-----> READ FOLDER SUCCEESSFULL!!!!!!\n");
                        }
                    }
                    else /* read file */
                    {
                        size_allocate = (manage_entries.list_entries[option - 1u].size_directory) / (bootsector.num_byte_per_sector * bootsector.num_sector_per_cluster);
                        size_allocate = (size_allocate + 1) * (bootsector.num_byte_per_sector * bootsector.num_sector_per_cluster);
                        buff_file = (uint8_t*)calloc(size_allocate, sizeof(uint8_t));
                        retcode = FATFS_ReadFile(buff_file, manage_entries.list_entries[option - 1u].cluster_start);
                        for (i = 0u; i < manage_entries.list_entries[option - 1].size_directory; i++)
                        {
                            printf("%c", buff_file[i]);
                        }
                        if(non_error == retcode)
                        {
                            printf("\n\t\t-----> READ FILE SUCCEESSFULL!!!!!!\n");
                        }
                        free(buff_file);
                        buff_file = NULL;
                    }
                    API_ShowDirectory(&manage_entries);
                }
            }
            while ((option != 0u) && (retcode == non_error));
        }
    }
    if(non_error != retcode)
    {
        API_ShowResult(retcode);
    }
    else if(0 == option)
    {
        printf("\n\t\t---> EXIT PROGRAM SUCEESSFULL ");
    }

    return retcode;
}

void API_ShowResult(const FATFS_CodeReturn_t retcode)
{
    switch (retcode)
    {
        case error_memory:
            printf("\n\t\t-----> ERROR MEMORY!!!!!!\n");
            break;
        case error_open_image:
            printf("\n\t\t-----> ERROR OPEN FILE IMAGE!!!!!!\n");
            break;
        case error_read_sector:
            printf("\n\t\t-----> ERROR OPEN FILE IMAGE!!!!!!\n");
            break;
        default:
            printf("\n\t\t-----> SUCCEESSFULL!!!!!!\n");
            break;
    }
}
/*******************************************************************
 * EOF
 ********************************************************************/

