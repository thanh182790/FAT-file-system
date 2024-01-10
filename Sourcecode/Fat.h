
#ifndef _DRIVER_H
#define _DRIVER_H
/*******************************************************************
 * Defines
 ********************************************************************/
#define IS_FOLDER 0x10u /* value pf attribute folder */

/* struct describe a part of bootsector 12,16,32 */
typedef struct
{
    uint16_t num_byte_per_sector;       /* number bytes per a sector */
    uint8_t num_sector_per_cluster;     /* number sectors per a cluster */
    uint16_t num_sector_per_bootsector; /* number sectors on BOOTSECTOR */
    uint8_t num_fat;                    /* number tables FAT */
    uint16_t num_entries_per_rdet;      /* number entrys ( 1 entry = 32 bytes) on RDET */
    uint32_t num_sector_per_vol;        /* number sectors per a volume */
    uint32_t num_sector_per_fat;        /* number sectors per a FAT */
    uint8_t type_fat[8];                /* type of FAT (12/16/32) */
} FATFS_BootApart_Struct_t;

/* struct describe time */
typedef struct
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t milisecond;
} FATFS_Time_Struct_t;

/* struct describe date */
typedef struct
{
    uint8_t date;
    uint8_t month;
    uint16_t year;
} FATFS_Date_Struct_t;

/* struct describe a part of entry */
typedef struct
{
    uint8_t name_dir[8];                  /* name of dir or file */
    uint8_t name_extend[3];               /* name extend of dir */
    uint8_t attribute;                    /* attribute of dir */
    FATFS_Time_Struct_t hour_create;      /* hour created dir */
    FATFS_Date_Struct_t date_create;      /* date created dir */
    FATFS_Date_Struct_t date_access;      /* date access dir */
    uint32_t cluster_start;               /* cluster start of dir */
    FATFS_Time_Struct_t last_hour_edit;   /* hour edit lastest */
    FATFS_Date_Struct_t last_date_update; /* date update lastest */
    uint32_t size_directory;              /* size of dir */
} FATFS_Entry_Struct_t;

/* struct to manage all entry of a directory */
typedef struct
{
    FATFS_Entry_Struct_t *list_entries; /* pointer to manage a array of all entry in directory */
    uint16_t num_entries;
} FATFS_ManageEntry_Struct_t;

/* enum return code */
typedef enum
{
    non_error = 0x00,        /* prgram non error */
    error_memory = 0x01,     /* program erro memory */
    error_open_image = 0x02, /* program error open image */
    error_read_sector = 0x03 /* program error read sector */
} FATFS_CodeReturn_t;

/*******************************************************************
 * API
 ********************************************************************/

/*
 *@brief       - Init to find parameters of bootsector
 *@param       - path is path of file image
 *@param       - boot is bootsector to send to Application
 *@param       - return one of Code in enum
 */
FATFS_CodeReturn_t FATFS_Init(const uint8_t *path, FATFS_BootApart_Struct_t *boot);

/*
 *@brief       - Read a directory
 *@param       - cluster_start is cluster start of a directory
 *@param       - list to manage a array entrys of directory
 *@param       - return one of Code in enum
 */
FATFS_CodeReturn_t FATFS_ReadFolder(const uint32_t cluster_start, FATFS_ManageEntry_Struct_t *manage_entries);

/*
 *@brief       - Read a file
 *@param       - cluster_start is cluster start of a file
 *@param       - buff_of_file is buffer is save content of file
 *@param       - return one of Code in enum
 */
FATFS_CodeReturn_t FATFS_ReadFile(uint8_t *buff_of_file,const uint32_t cluster_start);

#endif /* _DRIVER_H */
/*******************************************************************
* EOF
********************************************************************/

