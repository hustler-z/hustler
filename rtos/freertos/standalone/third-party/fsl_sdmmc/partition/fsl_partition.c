/*
 * Copyright (c) 2023 Phytium Information Technology, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "fsl_sd.h"
#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
#include "fsl_mmc.h"
#endif
#include "fsl_partition.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char* TAG = "sdmmc_block";
static uintptr_t extended_partition_lba_offset; /* lba of DOS extend partition */

/*******************************************************************************
 * Code
 ******************************************************************************/
static status_t SDMMC_ReadBlocks(sdmmchost_t *host, uint8_t *buffer, uint32_t startBlock, uint32_t blockCount)
{
    assert(host != NULL);
    if ((kSDMMCHOST_CARD_TYPE_STANDARD_SD == host->config.cardType) ||
        (kSDMMCHOST_CARD_TYPE_MICRO_SD == host->config.cardType))
    {
        return SD_ReadBlocks(host->card, buffer, startBlock, blockCount);
    }
    else if (kSDMMCHOST_CARD_TYPE_EMMC ==  host->config.cardType)
    {
#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
        return MMC_ReadBlocks(host->card, buffer, startBlock, blockCount);
#endif
    }

    return kStatus_Fail;
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif

void SDMMC_GetGPTPartitionEntry(const struct gpt_entry *gpt, int index, sdmmc_partition_info *partInfo)
{
	static char buf[128];

	SDMMC_LOG("  [%3d] ", index);

	/* type */
	(void)snprintf(buf, sizeof(buf), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
			gpt->type.time_low, gpt->type.time_mid, gpt->type.time_hi_and_version, 
			gpt->type.clock_seq_hi, gpt->type.clock_seq_low,
			gpt->type.node[0], gpt->type.node[1], gpt->type.node[2],
			gpt->type.node[3], gpt->type.node[4], gpt->type.node[5]);
	SDMMC_LOG("%s", buf);
	if (strcmp(buf, GPT_DEFAULT_ENTRY_TYPE) == 0) {
		SDMMC_LOG("Linux native partition type");	
	}
	/* partition_guid */
	(void)snprintf(buf, sizeof(buf), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			gpt->partition_guid.time_low, gpt->partition_guid.time_mid, gpt->partition_guid.time_hi_and_version, 
			gpt->partition_guid.clock_seq_hi, gpt->partition_guid.clock_seq_low,
			gpt->partition_guid.node[0], gpt->partition_guid.node[1], gpt->partition_guid.node[2],
			gpt->partition_guid.node[3], gpt->partition_guid.node[4], gpt->partition_guid.node[5]);
	SDMMC_LOG(" %s", buf);

	/* lba_start */
	SDMMC_LOG("%10llu", gpt_partition_get_lab_start(gpt));
	/* lba_end */
	SDMMC_LOG("%16llu", gpt_partition_get_lba_end(gpt));
 
	SDMMC_LOG("%13llu    ", (gpt_partition_get_lba_end(gpt) - gpt_partition_get_lab_start(gpt) + 1) * 512);
	/* name */
	const uint16_t *p = gpt->name;	
	for ( ;; ) 
    {
		const char c = (const char )*p;	
		if (c == '\0') 
            break;
		SDMMC_LOG("%c", c);
		p++;
	}
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

status_t SDMMC_LookupGPTPartition(sdmmchost_t *host, uint8_t *mbr, sdmmc_partition_info *partInfo)
{
	SDMMC_LOGD(TAG, "Protective MBR:");
	SDMMC_LOGD(TAG, "  Type: GPT Partition");
    status_t ret = kStatus_Success;
    struct gpt_header *gptHeader = NULL;
    uint8_t *buf = NULL;
    size_t count;

    gptHeader = SDMMC_OSAMemoryAlignedAllocate(FSL_SDMMC_DEFAULT_BLOCK_SIZE, FSL_SDMMC_DEFAULT_BLOCK_SIZE);
    if (NULL == gptHeader)
    {
		SDMMC_LOGE(TAG, "No enought memory !!!");	
		return kStatus_Fail;        
    }

    /* block-1 parimary table */
    ret = SDMMC_ReadBlocks(host, (uint8_t *)gptHeader, FSL_GPT_HEADER_BLOCK_INDEX, 1U);
    if (kStatus_Success != ret)
    {
        SDMMC_LOGE(TAG, "Failed to read gpt header !!!");
        goto err_exit;
    }

    /* Check the GUID Partition Table signature */
	if (gptHeader->signature != GPT_HEADER_SIGNATURE) 
    {
		SDMMC_LOGE(TAG, "Wrong GPT signature");	
		ret = kStatus_Fail;
        goto err_exit;
    }

	SDMMC_LOGD(TAG, "GPT Header:");
	SDMMC_LOGD(TAG, "  Signature: EFI PART");
	SDMMC_LOGD(TAG, "  Version: 0x%08x", gptHeader->revision);
	SDMMC_LOGD(TAG, "  Hdr Size: %u", gptHeader->size);
	SDMMC_LOGD(TAG, "  Hdr Crc32: 0x%08x", gptHeader->crc32);
	SDMMC_LOGD(TAG, "  Reserved: 0x%08x", gptHeader->reserved1);
	SDMMC_LOGD(TAG, "  Hdr Start LBA: %llu", gptHeader->my_lba);
	SDMMC_LOGD(TAG, "  Backup Hdr Start LBA: %llu", gptHeader->alternative_lba);
	SDMMC_LOGD(TAG, "  Partition Start LBA: %llu", gptHeader->first_usable_lba);
	SDMMC_LOGD(TAG, "  Partition End LBA: %llu", gptHeader->last_usable_lba);
	SDMMC_LOGD(TAG, "  GUID: %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
			gptHeader->disk_guid.time_low, gptHeader->disk_guid.time_mid,
			gptHeader->disk_guid.time_hi_and_version, 
			gptHeader->disk_guid.clock_seq_hi, gptHeader->disk_guid.clock_seq_low,
			gptHeader->disk_guid.node[0], gptHeader->disk_guid.node[1],
			gptHeader->disk_guid.node[2], gptHeader->disk_guid.node[3],
			gptHeader->disk_guid.node[4], gptHeader->disk_guid.node[5]);
	SDMMC_LOGD(TAG, "  Partition Table Start LBA: %llu", gptHeader->partition_entry_lba);
	SDMMC_LOGD(TAG, "  Number of Partition Entry: %u", gptHeader->npartition_entries);
	SDMMC_LOGD(TAG, "  Size of Partition Entry: %u", gptHeader->sizeof_partition_entry);
	SDMMC_LOGD(TAG, "  Partition Table Crc32: 0x%08x", gptHeader->partition_entry_array_crc32);
	SDMMC_LOGD(TAG, "  Reserved2 must be all 0");
	
    /* GPT entries */
    count = gptHeader->npartition_entries * gptHeader->sizeof_partition_entry;
    buf = SDMMC_OSAMemoryAlignedAllocate(count * FSL_SDMMC_DEFAULT_BLOCK_SIZE, FSL_SDMMC_DEFAULT_BLOCK_SIZE);
    if (NULL == buf)
    {
		SDMMC_LOGE(TAG, "No enought memory !!!");	
		ret = kStatus_Fail;
        goto err_exit;        
    }

    ret = SDMMC_ReadBlocks(host, buf, gptHeader->partition_entry_lba, count);
    if (kStatus_Success != ret)
    {
        SDMMC_LOGE(TAG, "Failed to read gpt entries !!!");
        goto err_exit;
    }    

	SDMMC_LOG("Partition Table:\n");
	SDMMC_LOG("  [ Nr]        Partition Type GUID              Unique Partition Guid             Start(sector)    End(sector)    Size      Name\n");
	for (uint32_t i = 0; i < gptHeader->npartition_entries; i++) 
    {
		const struct gpt_entry *gpt = gpt_get_partition(buf, i);
		
        if (i >= FSL_PARTITION_MAX_NUM)
            break;

        if (memcmp(&gpt->type, &GPT_UNUSED_ENTRY_GUID, sizeof(struct gpt_guid)) == 0)	
            continue;
		
        partInfo->parts[i].blk_offset = gpt->lba_start;
        partInfo->parts[i].blk_count = gpt->lba_end - gpt->lba_start + 1U;
        SDMMC_GetGPTPartitionEntry(gpt, i, partInfo);
	}

err_exit:
    if (gptHeader)
        SDMMC_OSAMemoryFree(gptHeader);
    
    if (buf)
        SDMMC_OSAMemoryFree(buf);
    return ret;    
}

static status_t SDMMC_GetDosExtendPartition(sdmmchost_t *host, uintptr_t start_lba_sector, sdmmc_partition_info *partInfo)
{
    unsigned char *ebr = SDMMC_OSAMemoryAlignedAllocate(FSL_SDMMC_DEFAULT_BLOCK_SIZE, FSL_SDMMC_DEFAULT_BLOCK_SIZE);
    if (NULL == ebr)
    {
		SDMMC_LOGE(TAG, "No enought memory !!!");	
		return kStatus_Fail;        
    }

    status_t ret = SDMMC_ReadBlocks(host, ebr, start_lba_sector, 1U);
    if (kStatus_Success != ret)
    {
        SDMMC_LOGE(TAG, "Failed to read dos extend entries !!!");
        goto err_exit;
    }

	struct dos_partition *p = mbr_get_partition(ebr, 0);
	SDMMC_LOG(" %02x %4u/%3hhu/%2hhu  ~  %4u/%3hhu/%2hhu %8u %8u  %02x\n",
			p->boot_ind,
			chs_get_cylinder(p->bc, p->bs), p->bh, chs_get_sector(p->bs),
			chs_get_cylinder(p->ec, p->es), p->eh, chs_get_sector(p->es),
			dos_partition_get_start(p) , // + (unsigned int)start_lba_sector, /* 每个子扩展分区都把自己看成独立的硬盘 */
			dos_partition_get_size(p),
			p->sys_ind);

    /* read the 2nd extend partition */
    p = mbr_get_partition(ebr, 1);
	if (IS_EXTENDED(p->sys_ind)) 
    { 
		SDMMC_LOG("\tnext logic info: %02x %02x %u/%hhu/%hhu ~ %u/%hhu/%hhu %u(%u)\n",
					p->sys_ind, p->boot_ind,
					chs_get_cylinder(p->bc, p->bs), p->bh, chs_get_sector(p->bs),
					chs_get_cylinder(p->ec, p->es), p->eh, chs_get_sector(p->es),
					dos_partition_get_start(p),
					dos_partition_get_size(p));
	} 
    else 
    {
		SDMMC_LOG("no logic anymore\n");
        goto err_exit;
    }

    /* recursively find more extend artitions */
    ret = SDMMC_GetDosExtendPartition(host, 
                                      extended_partition_lba_offset + dos_partition_get_start(p),
                                      partInfo);

err_exit:
    SDMMC_OSAMemoryFree(ebr);
    return ret;        
}

static status_t SDMMC_LookupDosPartition(sdmmchost_t *host, uint8_t *mbr, sdmmc_partition_info *partInfo)
{
    status_t ret = kStatus_Success;
	SDMMC_LOG("Disk identifier: 0x%08x\r\n", mbr_get_id(mbr));
	SDMMC_LOG("Boot Start-C/H/S ~    End-C/H/S Start-LBA    Size Type\r\n");

    for (uint32_t i = 0; i < FSL_PARTITION_MAX_NUM; i++) 
    {
		struct dos_partition *p = mbr_get_partition(mbr, i);	
		if (dos_partition_get_start(p) == 0U)	
            continue;

        partInfo->parts[i].blk_offset = dos_partition_get_start(p);
        partInfo->parts[i].blk_count  = dos_partition_get_size(p);
		SDMMC_LOG(" %02x %4u/%3hhu/%2hhu  ~  %4u/%3hhu/%2hhu %8u %8u  %02x\r\n",
				p->boot_ind,
				chs_get_cylinder(p->bc, p->bs), p->bh, chs_get_sector(p->bs),
				chs_get_cylinder(p->ec, p->es), p->eh, chs_get_sector(p->es),
				dos_partition_get_start(p),
				dos_partition_get_size(p),
				p->sys_ind);

		if (IS_EXTENDED(p->sys_ind)) 
        { 	/* extended partition */
			extended_partition_lba_offset = dos_partition_get_start(p); 
			ret = SDMMC_GetDosExtendPartition(host, 
                                              extended_partition_lba_offset,
                                              partInfo);	/* 从extended_partition_lba_offset扇区处开始读扩展分区信息 */
            if (kStatus_Success != ret)
                break;
        }
    }

    return ret;
}

status_t SDMMC_LookupPartition(sdmmchost_t *host, sdmmc_partition_info *partInfo)
{
    assert(host && host->card);
    assert(partInfo);
    status_t ret = kStatus_Success;

    uint8_t *mbr = SDMMC_OSAMemoryAlignedAllocate(FSL_SDMMC_DEFAULT_BLOCK_SIZE, FSL_SDMMC_DEFAULT_BLOCK_SIZE);
    if (NULL == mbr)
    {
		SDMMC_LOGE(TAG, "No enought memory !!!");	
		return kStatus_Fail;        
    }    

    /* read block-0 for master boot record */
    ret = SDMMC_ReadBlocks(host, mbr, FSL_MBR_BLOCK_INDEX, 1U);
    if (kStatus_Success != ret)
    {
        SDMMC_LOGE(TAG, "Failed to read mbr !!!");
        return ret;
    }

#ifdef CONFIG_LOG_DEBUG
    SDMMC_LOGD(TAG, "Block-%d MBR", FSL_MBR_BLOCK_INDEX);
    FtDumpHexByte(mbr, 1 * FSL_SDMMC_DEFAULT_BLOCK_SIZE);
#endif

    /* check signature 0x55aa in block-0 content */
    if (mbr_is_valid_magic(mbr) == 0) 
    {
        SDMMC_LOGE(TAG, "Illegal mbr !!!");
        return kStatus_Fail;
    }

    /* check following paritions */
    struct dos_partition *p = mbr_get_partition(mbr, FSL_MBR_BLOCK_INDEX);
    if (p->sys_ind == MBR_GPT_PARTITION) 
    {
        SDMMC_LOGD(TAG, "GPT Partition");
        partInfo->type = p->sys_ind;
        ret = SDMMC_LookupGPTPartition(host, mbr, partInfo);
    }
    else
    {
        SDMMC_LOGD(TAG, "DOS Partition");
        partInfo->type = p->sys_ind;
        ret = SDMMC_LookupDosPartition(host, mbr, partInfo);
    }

    SDMMC_OSAMemoryFree(mbr);
    return ret;
}