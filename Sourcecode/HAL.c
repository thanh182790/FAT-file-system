/*******************************************************************
 * Includes
 *******************************************************************/
#include <stdio.h>
#include <conio.h>
#include <stdint.h>
#include <stdbool.h>
#include "HAL.h"

/*******************************************************************
 * Defines
 *******************************************************************/
#define BYTE_PER_SECTOR_DEFAULT 512u

/*******************************************************************
 * Variables
 *******************************************************************/
static FILE *file_img = NULL;    /* file image to read */
static uint16_t size_sector = 0; /* size of sector to read in funtion HAL_ReadSector and HAL_ReadMultiSector*/

/*******************************************************************
 * Codes
 *******************************************************************/
/* init open image */
bool HAL_OpenFileImage(const uint8_t *path)
{
    bool ret_val = false;
    file_img = fopen(path, "rb");
    if (file_img != NULL)
    {
        ret_val = true;
        size_sector = BYTE_PER_SECTOR_DEFAULT;
    }
    return ret_val;
}

/* Read sector at index into the buffer */
int32_t HAL_ReadSector(uint32_t index, uint8_t *buff)
{
    int32_t num_bytes = 0;
    if (0 == fseek(file_img, index * size_sector, SEEK_SET))
    {
        num_bytes = fread(buff, 1u, size_sector, file_img);
    }
    return num_bytes;
}

/* Read num sectors consecutively, start at index into  the buffer */
int32_t HAL_ReadMultiSector(uint32_t index, uint16_t num, uint8_t *buff)
{
    int32_t num_bytes = 0;
    if (0 == fseek(file_img, index * size_sector, SEEK_SET))
    {
        num_bytes = fread(buff, 1u, num * size_sector, file_img);
    }
    return num_bytes;
}

/* close file image */
void HAL_CloseFileImage(void)
{
    fclose(file_img);
    file_img = NULL;
}

/* update sizesector from driver to HAL */
void HAL_UpdateSizeSector(const uint16_t new_size)
{
    size_sector = new_size;
}

/*******************************************************************
 * EOF
 *******************************************************************/

