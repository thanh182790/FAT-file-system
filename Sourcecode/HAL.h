
/*************************************************************************************
 * Defines
 ************************************************************************************/
#ifndef _HAL_H
#define _HAL_H
/************************************************************************************
 * API
 ************************************************************************************/
/*
 *@brief           init open image
 *@param           no
 *@return          true if open succsss, false if fail
 */
bool HAL_OpenFileImage(const uint8_t *path);
/*
 *@brief           Read sector at index into the buffer
 *@param           index is posisition want read sector
 *@param           buff is a buffer save sector
 *@return          number of bytes read
 */
int32_t HAL_ReadSector(uint32_t index, uint8_t *buff);

/*
 *@brief           Read num sectors consecutively, start at index into  the buffer
 *@param           index is posisition start want read multi sector
 *@param           buff is a buffer save num sector
 *@param           num is number sector wanna read
 *@return          number of bytes read
 */
int32_t HAL_ReadMultiSector(uint32_t index, uint16_t num, uint8_t *buff);

/*
 *@brief           Close file image
 *@param           no
 *@return          no
 */
void HAL_CloseFileImage(void);

/*
 *@brief            update sizesector from driver to HAL
 *@param            new_size is new size need to update
 *@return           no
 */
void HAL_UpdateSizeSector(const uint16_t new_size);

#endif /* _HAL_H */
/*******************************************************************
 * EOF
 ********************************************************************/

