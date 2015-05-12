/***************************************************************************** 
* 
* File Name : wm_mem.c
* 
* Description: memory manager Module 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-12 
*****************************************************************************/ 

#include "wm_osal.h"
#include "wm_mem.h"
#include "list.h"
#include <string.h>

#if DEBUG

/**
 * This mutex is used to synchronize the list of allocated
 * memory blocks. This is a debug version only feature
 */
tls_os_sem_t    *mem_sem;

struct dl_list memory_used_list;
struct dl_list memory_free_list;
#define MEM_BLOCK_SIZE           200
MEMORY_BLOCK mem_blocks[MEM_BLOCK_SIZE];

/**
 * This variable is set if the memory mananger has been initialized.
 * This is available only for debug version of the driver
 */
bool         memory_manager_initialized = false;

u32 alloc_heap_mem_bytes = 0; 
u32 alloc_heap_mem_blk_cnt = 0;
u32 alloc_heap_mem_max_size = 0;

#define PRE_OVERSIZE        0
#define OVERSIZE        0

/**
 * This is a debug only function that performs memory management operations for us.
 * Memory allocated using this function is tracked, flagged when leaked, and caught for
 * overflows and underflows.
 *
 * \param size            The size in bytes of memory to
 *        allocate
 *
 * \param file            The full path of file where this
 *        function is invoked from
 * \param line            The line number in the file where this
 *        method was called from
 * \return Pointer to the allocated memory or NULL in case of a failure
 */
void * mem_alloc_debug(u32 size, char* file, int line)
{
    void *buf = NULL;
    u32 pad_len;
    int i =  0;
    u32  cpu_sr;
    //
    // If the memory manager has not been initialized, do so now
    //
    cpu_sr = tls_os_set_critical();
    if (!memory_manager_initialized) {
        tls_os_status_t os_status;
        memory_manager_initialized = true;
        //
        // NOTE: If two thread allocate the very first allocation simultaneously
        // it could cause double initialization of the memory manager. This is a
        // highly unlikely scenario and will occur in debug versions only.
        //
        os_status = tls_os_sem_create(&mem_sem, 1);
        if(os_status != TLS_OS_SUCCESS)
            printf("mem_alloc_debug: tls_os_sem_create mem_sem error\n");
        dl_list_init(&memory_used_list);
        dl_list_init(&memory_free_list);
        for(i = 0; i < MEM_BLOCK_SIZE; i++)
        {
            dl_list_add_tail(&memory_free_list, &mem_blocks[i].list);
        }
    }
    tls_os_release_critical(cpu_sr);

    tls_os_sem_acquire(mem_sem, 0);
    cpu_sr = tls_os_set_critical();
    //
    // Allocate the required memory chunk plus header and trailer bytes
    //
    pad_len = sizeof(u32) - (size & 0x3);
    buf = malloc(sizeof(MEMORY_PATTERN) + PRE_OVERSIZE + size + pad_len + OVERSIZE + sizeof(MEMORY_PATTERN));

    if (buf) {
        //
        // Memory allocation succeeded. Add information about the allocated
        // block in the list that tracks all allocations.
        //
        PMEMORY_PATTERN  mem_ptn_hd;
        PMEMORY_PATTERN  mem_ptn_tl;
        PMEMORY_BLOCK  mem_blk_hd1;

	 if(dl_list_empty(&memory_free_list))
	 {
	     printf("Memory blocks empty!\n");
		 tls_mem_alloc_info();
            free(buf);
            tls_os_release_critical(cpu_sr);
            tls_os_sem_release(mem_sem);
            return NULL;
	 }
	 mem_blk_hd1 = dl_list_first(&memory_free_list, MEMORY_BLOCK, list);
	 dl_list_del(&mem_blk_hd1->list);
	 dl_list_add_tail(&memory_used_list, &mem_blk_hd1->list);
        alloc_heap_mem_bytes += size+sizeof(MEMORY_PATTERN)+sizeof(MEMORY_PATTERN)+pad_len + PRE_OVERSIZE + OVERSIZE;
        alloc_heap_mem_blk_cnt++;
        if (alloc_heap_mem_bytes > alloc_heap_mem_max_size)
        {
            alloc_heap_mem_max_size = alloc_heap_mem_bytes;
            //printf("alloc_heap_mem_max_size=%d\n", alloc_heap_mem_max_size);
        }
		
        mem_blk_hd1->pad = pad_len;
        mem_blk_hd1->file = file;
        mem_blk_hd1->line = line;
        mem_blk_hd1->length = size;
        mem_blk_hd1->header_pattern = (u32)buf;
		
        // Fill in the memory header and trailer
        mem_ptn_hd = (PMEMORY_PATTERN)buf;
        mem_ptn_hd->pattern0= MEM_HEADER_PATTERN;
        /*mem_ptn_hd->pattern1= MEM_HEADER_PATTERN;
        mem_ptn_hd->pattern2= MEM_HEADER_PATTERN;
        mem_ptn_hd->pattern3= MEM_HEADER_PATTERN;*/

        mem_ptn_tl = (PMEMORY_PATTERN)(((u8 *)(buf))+size + sizeof(MEMORY_PATTERN)+pad_len + PRE_OVERSIZE + OVERSIZE);
        mem_ptn_tl->pattern0= MEM_TAILER_PATTERN;
        /*mem_ptn_tl->pattern1= MEM_TAILER_PATTERN;
        mem_ptn_tl->pattern2= MEM_TAILER_PATTERN;
        mem_ptn_tl->pattern3= MEM_TAILER_PATTERN;*/

        // Jump ahead by memory header so pointer returned to caller points at the right place
        buf = ((u8 *)buf) + sizeof (MEMORY_PATTERN) + PRE_OVERSIZE;

#if 0

        printf("==>Memory was allocated from %s at line %d with length %d\n",
                  mem_blk_hd->file,
                  mem_blk_hd->line,               
                  mem_blk_hd->length);
        printf("==>mem alloc ptr = 0x%x\n", buf);

#endif
    }
    else
    {
        printf("==>Memory was allocated from %s at line %d with length %d, allocated size %d, count %d\n",
                   file,
                   line,               
                   size, alloc_heap_mem_bytes, alloc_heap_mem_blk_cnt);
	 tls_mem_alloc_info();
    }
    tls_os_release_critical(cpu_sr);
    tls_os_sem_release(mem_sem);
    return buf;
}

/**
 * This routine is called to free memory which was previously allocated using MpAllocateMemory function.
 * Before freeing the memory, this function checks and makes sure that no overflow or underflows have
 * happened and will also try to detect multiple frees of the same memory chunk.
 *
 * \param p    Pointer to allocated memory
 */
void mem_free_debug(void *p,  char* file, int line)
{
    PMEMORY_PATTERN  mem_ptn_hd;
    PMEMORY_PATTERN  mem_ptn_tl;
    PMEMORY_BLOCK  mem_blk_hd1;
    u8              needfree = 0;
    u32  cpu_sr;

    // Jump back by memory header size so we can get to the header
    mem_ptn_hd = (PMEMORY_PATTERN) (((u8 *)p) - sizeof(MEMORY_PATTERN)  - PRE_OVERSIZE);
    tls_os_sem_acquire(mem_sem, 0);
    cpu_sr = tls_os_set_critical();
    dl_list_for_each(mem_blk_hd1, &memory_used_list, MEMORY_BLOCK, list){
        if(mem_blk_hd1->header_pattern == (u32)mem_ptn_hd)
        {
            needfree = 1;
            break;
        }
    }
    if(needfree)
    {
        dl_list_del(&mem_blk_hd1->list);
        dl_list_add_tail(&memory_free_list, &mem_blk_hd1->list);
        alloc_heap_mem_bytes -= mem_blk_hd1->length + sizeof(MEMORY_PATTERN) + sizeof(MEMORY_PATTERN) + PRE_OVERSIZE + OVERSIZE +
            mem_blk_hd1->pad;
        alloc_heap_mem_blk_cnt--;
    }
    if(needfree == 0)
    {
	 printf("Memory Block %p was deallocated from %s at line %d \n", mem_ptn_hd, file, line);
	 printf("Memory %p has been deallocated!\n", p);
	 dl_list_for_each_reverse(mem_blk_hd1, &memory_free_list, MEMORY_BLOCK, list){
            if(mem_blk_hd1->header_pattern == (u32)mem_ptn_hd)
            {
                printf("Memory Block %p has been put free list!\n", mem_ptn_hd);
                break;
            }
        }
        tls_mem_alloc_info();
        tls_os_release_critical(cpu_sr);
        tls_os_sem_release(mem_sem);
	 return;
    }
#if 0
    if(mem_blk_hd1->line == 976 || mem_blk_hd1->line == 983)
    {
	 printf("Memory Block %p can not deallocated from %s at line %d \n", mem_ptn_hd, file, line);
	 printf("Memory %p has been deallocated!\n", p);
        tls_mem_alloc_info();
    }
#endif
    mem_ptn_tl = (PMEMORY_PATTERN) ((u8 *)p + mem_blk_hd1->length + mem_blk_hd1->pad + OVERSIZE);
    //
    // Check that header was not corrupted
    //
    if (mem_ptn_hd->pattern0 != MEM_HEADER_PATTERN /*|| mem_ptn_hd->pattern1 != MEM_HEADER_PATTERN 
		|| mem_ptn_hd->pattern2 != MEM_HEADER_PATTERN || mem_ptn_hd->pattern3 != MEM_HEADER_PATTERN*/) 
    {
        printf("Memory %p was deallocated from %s at line %d \n", p, file, line);
        printf("Memory header corruption due to underflow detected at memory block %p\n",
 	            mem_ptn_hd);
        printf("Header pattern 0(0x%x)\n",//, 1(0x%x), 2(0x%x), 3(0x%x)
			mem_ptn_hd->pattern0/*,
			mem_ptn_hd->pattern1,
			mem_ptn_hd->pattern2,
			mem_ptn_hd->pattern3*/);
        //printf("Dumping information about memory block. "
        //        "This information may itself have been "
         //       "corrupted and could cause machine to bugcheck.\n");
        printf("Memory was allocated from %s at line %d with length %d\n",
                mem_blk_hd1->file,
                mem_blk_hd1->line,
                mem_blk_hd1->length);
        tls_mem_alloc_info();
    }

#if 0
    printf("<==free memory allocated from %s at line %d with length %d\n",
            mem_blk_hd->file,
            mem_blk_hd->line,
            mem_blk_hd->length);
    printf("<==free memory 0x%x\n", (u8 *)mem_blk_hd+sizeof(*mem_blk_hd));
#endif

    //
    // Check that trailer was not corrupted
    //
    if(mem_ptn_tl->pattern0 != MEM_TAILER_PATTERN /*|| mem_ptn_tl->pattern1 != MEM_TAILER_PATTERN 
		|| mem_ptn_tl->pattern2 != MEM_TAILER_PATTERN || mem_ptn_tl->pattern3 != MEM_TAILER_PATTERN*/) {
	 printf("Memory %p was deallocated from %s at line %d \n", p, file, line);
        printf("Memory tailer corruption due to overflow detected at %p\n", mem_ptn_hd);
        printf("Tailer pattern 0(0x%x)\n",//, 1(0x%x), 2(0x%x), 3(0x%x)
			mem_ptn_tl->pattern0/*,
			mem_ptn_tl->pattern1,
			mem_ptn_tl->pattern2,
			mem_ptn_tl->pattern3*/);
        //printf("Dumping information about memory block. "
        //       "This information may itself have been "
        //        "corrupted and could cause machine to bugcheck.\n");
        printf("Memory was allocated from %s at line %d with length %d\n",
                mem_blk_hd1->file, mem_blk_hd1->line, mem_blk_hd1->length);
        tls_mem_alloc_info();
    }
    if(needfree){
        free(mem_ptn_hd);
    }
    
    tls_os_release_critical(cpu_sr);
    tls_os_sem_release(mem_sem);
}

void * mem_realloc_debug(void *mem_address, u32 size, char* file, int line)
{
	void * mem_re_addr;
	u32 cpu_sr;
    cpu_sr = tls_os_set_critical();
	if ((mem_re_addr = mem_alloc_debug(size,  file, line)) == NULL){
		printf("mem_realloc_debug failed(size=%d).\n", size);
		return NULL;
	}
	if(mem_address != NULL)
	{
		memcpy(mem_re_addr, mem_address, size);
		mem_free_debug(mem_address, file, line);
	}
	//printf("mem_realloc_debug mem_address=%p, mem_re_addr=%p, size=%d, file=%s, line=%d\n", mem_address, mem_re_addr, size, file, line);
    tls_os_release_critical(cpu_sr);
	return mem_re_addr;
}

void tls_mem_alloc_info(void)
{
    int i;
    MEMORY_BLOCK * pos;
    u32 cpu_sr;

    //tls_os_sem_acquire(mem_sem, 0);
    //cpu_sr = tls_os_set_critical();
    i = 1;
    dl_list_for_each(pos, &memory_used_list, MEMORY_BLOCK, list){
        printf("Block(%2d): addr<%p>, file<%s>, line<%d>, length<%d>\n",
                i, pos->header_pattern, pos->file, pos->line, pos->length);
        i++;
    }
    //tls_os_release_critical(cpu_sr);
    //tls_os_sem_release(mem_sem);
    
}

int is_safe_addr_debug(void* p, u32 len, char* file, int line)
{
    int i;
    MEMORY_BLOCK * pos;
    u32 cpu_sr;

    if(((u32)p) >= (u32)0x60000)
    {
        return 1;
    }
    tls_os_sem_acquire(mem_sem, 0);
    cpu_sr = tls_os_set_critical();
    i = 1;
    dl_list_for_each(pos, &memory_used_list, MEMORY_BLOCK, list){
        if((pos->header_pattern + sizeof (MEMORY_PATTERN)  + PRE_OVERSIZE) <= ((u32)p) && ((u32)p) <= ((u32)(pos->header_pattern + sizeof(MEMORY_PATTERN) + PRE_OVERSIZE + pos->length)))
        {
            if(((u32)p) + len > ((u32)(pos->header_pattern + sizeof(MEMORY_PATTERN) + PRE_OVERSIZE + pos->length)))
            {
                printf("==>Memory oversize. Block(%2d): addr<%p>, file<%s>, line<%d>, length<%d>\n",
                    i, pos->header_pattern, pos->file, pos->line, pos->length);
                break;
            }
            else
            {
                tls_os_release_critical(cpu_sr);
                tls_os_sem_release(mem_sem);
                return 1;
            }
        }
        else if(((u32)p) < pos->header_pattern)
        {
            tls_os_release_critical(cpu_sr);
            tls_os_sem_release(mem_sem);
            return 1;
        }
        i++;
    }
    tls_os_release_critical(cpu_sr);
    tls_os_sem_release(mem_sem);
    printf("==>Memory is not safe addr<%p>, file<%s>, line<%d>.\n",p, file, line);
    return 0;
}

#else /* DEBUG */

void * mem_alloc_debug(u32 size)
{
	void * buffer = NULL;
	u32 cpu_sr;
	
	cpu_sr = tls_os_set_critical();
	buffer = malloc(size);
	tls_os_release_critical(cpu_sr);	
    return buffer;
}

void mem_free_debug(void *p)
{
	u32 cpu_sr;
	cpu_sr = tls_os_set_critical();
	free(p);
	tls_os_release_critical(cpu_sr);	
}

void * mem_realloc_debug(void *mem_address, u32 size)
{
	void * mem_re_addr;
	u32 cpu_sr;
	cpu_sr = tls_os_set_critical();
	mem_re_addr = realloc(mem_address, size);
	tls_os_release_critical(cpu_sr);
	return mem_re_addr;
}

#endif /* DEBUG */


