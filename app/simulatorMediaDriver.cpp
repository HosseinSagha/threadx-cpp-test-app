#include "simulatorMediaDriver.hpp"
#include "test.hpp"

FlashBlock norMem[TOTAL_BLOCKS];

void norFlashSimulatorMediaDriver(NorMedia &norMedia)
{
    ThreadX::Uchar *source_buffer;
    ThreadX::Uchar *destination_buffer;
    ThreadX::Ulong logical_sector;

    /* There are several useful/important pieces of information contained in the media
       structure, some of which are supplied by FileX and others are for the driver to
       setup. The following is a summary of the necessary FX_MEDIA structure members:
       
            FX_MEDIA Member                              Meaning
                                        
        fx_media_driver_request             FileX request type. Valid requests from FileX are 
                                            as follows:

                                                    FX_DRIVER_READ
                                                    FX_DRIVER_WRITE                 
                                                    FX_DRIVER_FLUSH
                                                    FX_DRIVER_ABORT
                                                    FX_DRIVER_INIT
                                                    FX_DRIVER_BOOT_READ
                                                    FX_DRIVER_RELEASE_SECTORS
                                                    FX_DRIVER_BOOT_WRITE
                                                    FX_DRIVER_UNINIT

        fx_media_driver_status              This value is RETURNED by the driver. If the 
                                            operation is successful, this field should be
                                            set to FX_SUCCESS for before returning. Otherwise,
                                            if an error occurred, this field should be set
                                            to FX_IO_ERROR. 

        fx_media_driver_buffer              Pointer to buffer to read or write sector data.
                                            This is supplied by FileX.

        fx_media_driver_logical_sector      Logical sector FileX is requesting.

        fx_media_driver_sectors             Number of sectors FileX is requesting.


       The following is a summary of the optional FX_MEDIA structure members:
       
            FX_MEDIA Member                              Meaning
                                        
        fx_media_driver_info                Pointer to any additional information or memory.
                                            This is optional for the driver use and is setup
                                            from the fx_media_open call. The RAM disk uses
                                            this pointer for the RAM disk memory itself.

        fx_media_driver_write_protect       The DRIVER sets this to FX_TRUE when media is write 
                                            protected. This is typically done in initialization, 
                                            but can be done anytime.

        fx_media_driver_free_sector_update  The DRIVER sets this to FX_TRUE when it needs to 
                                            know when clusters are released. This is important
                                            for FLASH wear-leveling drivers.

        fx_media_driver_system_write        FileX sets this flag to FX_TRUE if the sector being
                                            written is a system sector, e.g., a boot, FAT, or 
                                            directory sector. The driver may choose to use this
                                            to initiate error recovery logic for greater fault
                                            tolerance.

        fx_media_driver_data_sector_read    FileX sets this flag to FX_TRUE if the sector(s) being
                                            read are file data sectors, i.e., NOT system sectors.

        fx_media_driver_sector_type         FileX sets this variable to the specific type of 
                                            sector being read or written. The following sector
                                            types are identified:

                                                    FX_UNKNOWN_SECTOR 
                                                    FX_BOOT_SECTOR
                                                    FX_FAT_SECTOR
                                                    FX_DIRECTORY_SECTOR
                                                    FX_DATA_SECTOR  
    */

    norMedia.fx_media_driver_status = FX_SUCCESS;

    /* Process the driver request specified in the media control block.  */
    switch (norMedia.fx_media_driver_request)
    {
    case FX_DRIVER_READ:

        /* Setup the destination buffer and logical sector.  */
        logical_sector = norMedia.fx_media_driver_logical_sector;
        destination_buffer = norMedia.fx_media_driver_buffer;

        /* Loop to read sectors from flash.  */
        for (ThreadX::Ulong i{}; i < norMedia.fx_media_driver_sectors; i++)
        {

            /* Read a sector from NOR flash.  */
            if (norMedia.m_norFlash.readSector(logical_sector, destination_buffer) != LevelX::Error::success)
            {
                norMedia.fx_media_driver_status = FX_IO_ERROR;
                return;
            }

            /* Move to the next entries.  */
            ++logical_sector;
            destination_buffer += norMedia.fx_media_bytes_per_sector;
        }

        return;

    case FX_DRIVER_WRITE:
        /* Setup the source buffer and logical sector.  */
        logical_sector = norMedia.fx_media_driver_logical_sector;
        source_buffer = norMedia.fx_media_driver_buffer;

        /* Loop to write sectors to flash.  */
        for (ThreadX::Ulong i{}; i < norMedia.fx_media_driver_sectors; i++)
        {

            /* Write a sector to NOR flash.  */
            if (norMedia.m_norFlash.writeSector(logical_sector, source_buffer) != LevelX::Error::success)
            {
                norMedia.fx_media_driver_status = FX_IO_ERROR;
                return;
            }

            /* Move to the next entries.  */
            logical_sector++;
            source_buffer = source_buffer + norMedia.fx_media_bytes_per_sector;
        }

        return;

    case FX_DRIVER_RELEASE_SECTORS:

        /* Setup the logical sector.  */
        logical_sector = norMedia.fx_media_driver_logical_sector;

        /* Release sectors.  */
        for (ThreadX::Ulong i{}; i < norMedia.fx_media_driver_sectors; i++)
        {

            /* Release NOR flash sector.  */
            if (norMedia.m_norFlash.releaseSector(logical_sector) != LevelX::Error::success)
            {
                norMedia.fx_media_driver_status = FX_IO_ERROR;
                return;
            }

            /* Move to the next entries.  */
            logical_sector++;
        }

        return;

    case FX_DRIVER_INIT:
        /* FLASH drivers are responsible for setting several fields in the 
               media structure, as follows:

                    media -> fx_media_driver_free_sector_update
                    media -> fx_media_driver_write_protect

               The fx_media_driver_free_sector_update flag is used to instruct
               FileX to inform the driver whenever sectors are not being used.
               This is especially useful for FLASH managers so they don't have 
               maintain mapping for sectors no longer in use.

               The fx_media_driver_write_protect flag can be set anytime by the
               driver to indicate the media is not writable.  Write attempts made
               when this flag is set are returned as errors.  */

        /* Perform basic initialization here... since the boot record is going
               to be read subsequently and again for volume name requests.  */

        /* With flash wear leveling, FileX should tell wear leveling when sectors
               are no longer in use.  */
        norMedia.fx_media_driver_free_sector_update = FX_TRUE;

        /* Open the NOR flash simulation.  */
        if (norMedia.m_norFlash.open() != LevelX::Error::success)
        {
            norMedia.fx_media_driver_status = FX_IO_ERROR;
        }

        return;

    case FX_DRIVER_UNINIT:
        /* There is nothing to do in this case for the RAM driver.  For actual
               devices some shutdown processing may be necessary.  */

        /* Close the NOR flash simulation.  */
        if (norMedia.m_norFlash.close() != LevelX::Error::success)
        {
            norMedia.fx_media_driver_status = FX_IO_ERROR;
        }

        return;

    case FX_DRIVER_BOOT_READ:
        /* Read the boot record and return to the caller.  */

        /* Setup the destination buffer.  */
        destination_buffer = norMedia.fx_media_driver_buffer;

        /* Read boot sector from NOR flash.  */
        if (norMedia.m_norFlash.readSector(0, destination_buffer) != LevelX::Error::success)
        {
            norMedia.fx_media_driver_status = FX_IO_ERROR;
            return;
        }

        /* For NOR driver, determine if the boot record is valid.  */
        if (destination_buffer[0] != 0xEB || destination_buffer[1] != 0x34 || destination_buffer[2] != 0x90)
        {
            /* Invalid boot record, return an error!  */
            norMedia.fx_media_driver_status = FX_MEDIA_INVALID;
        }

        return;

    case FX_DRIVER_BOOT_WRITE:
        /* Make sure the media bytes per sector equals to the LevelX logical sector size.  */
        if (norMedia.fx_media_bytes_per_sector != LevelX::norSectorSizeInWord * ThreadX::wordSize)
        {
            /* Sector size mismatch, return error.  */
            norMedia.fx_media_driver_status = FX_IO_ERROR;
            return;
        }

        /* Write the boot record and return to the caller.  */

        /* Setup the source buffer.  */
        source_buffer = norMedia.fx_media_driver_buffer;

        /* Write boot sector to NOR flash.  */
        if (norMedia.m_norFlash.writeSector(0, source_buffer) != LevelX::Error::success)
        {
            norMedia.fx_media_driver_status = FX_IO_ERROR;
        }

        return;

    case FX_DRIVER_FLUSH:
    case FX_DRIVER_ABORT:
        return;

    default:
        /* Invalid driver request.  */
        norMedia.fx_media_driver_status = FX_IO_ERROR;
        return;
    }
}

LevelX::Error norFlashSimulatorRead(ThreadX::Ulong *flash_address, ThreadX::Ulong *destination, ThreadX::Ulong words)
{
    /* Loop to read flash.  */
    while (words--)
    {
        /* Copy word.  */
        *destination++ = *flash_address++;
    }

    return LevelX::Error::success;
}

LevelX::Error norFlashSimulatorWrite(ThreadX::Ulong *flash_address, ThreadX::Ulong *source, ThreadX::Ulong words)
{
    /* Loop to write flash.  */
    while (words--)
    {
        /* Copy word.  */
        *flash_address++ = *source++;
    }

    return LevelX::Error::success;
}

LevelX::Error norFlashSimulatorBlockErase(ThreadX::Ulong block, [[maybe_unused]] ThreadX::Ulong erase_count)
{
    ThreadX::Ulong *pointer;
    ThreadX::Ulong words;

    /* Setup pointer.  */
    pointer = reinterpret_cast<ThreadX::Ulong *>(&norMem[block]);

    /* Loop to erase block.  */
    words = sizeof(FlashBlock) / sizeof(ThreadX::Ulong);
    while (words--)
    {

        /* Erase word of block.  */
        *pointer++ = 0xFFFFFFFFUL;
    }

    return LevelX::Error::success;
}

LevelX::Error norFlashSimulatorEraseAll()
{
    ThreadX::Ulong *pointer;
    ThreadX::Ulong words;

    /* Setup pointer.  */
    pointer = reinterpret_cast<ThreadX::Ulong *>(&norMem[0]);

    /* Loop to erase block.  */
    words = sizeof(norMem) / (sizeof(ThreadX::Ulong));
    while (words--)
    {
        /* Erase word of block.  */
        *pointer++ = 0xFFFFFFFFUL;
    }

    return LevelX::Error::success;
}

LevelX::Error norFlashSimulatorBlockErasedVerify(ThreadX::Ulong block)
{
    ThreadX::Ulong *word_ptr;
    ThreadX::Ulong words;

    /* Determine if the block is completely erased.  */

    /* Pickup the pointer to the first word of the block.  */
    word_ptr = &norMem[block].eraseCount;

    /* Calculate the number of words in a block.  */
    words = sizeof(FlashBlock) / sizeof(ThreadX::Ulong);

    /* Loop to check if the block is erased.  */
    while (words--)
    {

        /* Is this word erased?  */
        if (*word_ptr++ != 0xFFFFFFFF)
            return LevelX::Error::error;
    }

    /* Return success.  */
    return LevelX::Error::success;
}