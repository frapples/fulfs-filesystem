#include "mem_inode.h"

#include <stdbool.h>
#include "inode.h"

#include <assert.h>

/* 目前采用简单的数组保存和组织内存inode节点
   这会造成一些性能问题，后期应该重构成hash或平衡树来提高性能
*/

static struct {
    mem_inode_t mem_inode;
    bool is_free;
}mem_inodes[MAX_MEM_INODE_COUNT];

static bool mem_inodes_inited = false;


#define READ_ERROR -1
static int mem_inode_read(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t inode_no);
static void mem_inodes_init();

bool mem_inode_get(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t inode_no, mem_inode_t** p_result)
{
    if (!mem_inodes_inited) {
        mem_inodes_init();
        mem_inodes_inited = true;
    }

    for (int i = 0; i < MAX_MEM_INODE_COUNT; i++) {
        mem_inode_t* mem_inode = &(mem_inodes[i].mem_inode);
        if (!mem_inodes[i].is_free &&
            mem_inode->device == dev_inode_ctrl->device && mem_inode->inode_no == inode_no) {
            *p_result = mem_inode;
        }
    }

    int i = mem_inode_read(dev_inode_ctrl, inode_no);

    if (i != READ_ERROR) {
        *p_result = &(mem_inodes[i].mem_inode);
    } else {
        return false;
    }

    (*p_result)->ref_count++;
    return true;
}

bool mem_inode_put(dev_inode_ctrl_t* dev_inode_ctrl, mem_inode_t* mem_inode)
{
    mem_inode->ref_count--;

    if (mem_inode->ref_count <= 0) {
        bool success = inode_dump(dev_inode_ctrl, mem_inode->inode_no, &(mem_inode->inode));
        if (!success) {
            return false;
        }

        for (int i = 0; i < MAX_MEM_INODE_COUNT; i++) {
            if (&(mem_inodes[i].mem_inode) == mem_inode) {
                mem_inodes[i].is_free = true;
                return true;
            }
        }

        assert(false);

    } else {
        return true;
    }
}

static int mem_inode_read(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t inode_no)
{
    for (int i = 0; i < MAX_MEM_INODE_COUNT; i++) {
        if (mem_inodes[i].is_free) {
            bool success = inode_load(dev_inode_ctrl, inode_no, &(mem_inodes[i].mem_inode.inode));
            if (!success) {
                return READ_ERROR;
            }

            mem_inodes[i].is_free = false;
            mem_inodes[i].mem_inode.inode_no = inode_no;
            mem_inodes[i].mem_inode.ref_count = 0;
            mem_inodes[i].mem_inode.device = dev_inode_ctrl->device;
            return i;
        }
    }

    /* 超过了MAX_MEM_INODE_COUNT */
    return READ_ERROR;
}

static void mem_inodes_init()
{
    for (int i = 0; i < MAX_MEM_INODE_COUNT; i++) {
        mem_inodes[i].is_free = true;
    }
}
