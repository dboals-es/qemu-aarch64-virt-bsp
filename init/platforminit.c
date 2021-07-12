//==============================================================================
// Copyright 2020 Daniel Boals & Michael Boals
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
// THE SOFTWARE.
//==============================================================================

#include "types.h"
#include "../../rtos/include/process.h"
#include "memory.h"
#include "mmu.h"
#include "devices.h"
#include "platform.h"

#include "mailbox.h"
#include "framebuffer.h"
#include "gic400.h"

static status_t platformBuildTable(void);
void ParseTree(void *fdt);

typedef struct _fdt_header_t 
{
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
} fdt_header_t;


memoryDescriptor_t  platformMemory[3];
device_t            platformDeviceList[] 
= 
{
    // GIC400 Distributor
    {
        .driverInit         = Gic400DistributorInit,
        .mmio.memoryBase    = 0x4C0041000ULL,
        .mmio.memorySize    = 4*1024,
        .mmio.memoryType    = MEM_TYPE_DEVICE_MEMORY,
        .mem.memoryBase     = 0,
        .mem.memorySize     = 0,
        .mem.memoryType     = MEM_TYPE_NONE,
    },
   
    // GIC400 CPU
   {
        .driverInit         = Gic400CpuInit,
        .mmio.memoryBase    = 0x4C0042000ULL,
        .mmio.memorySize    = 4*1024,
        .mmio.memoryType    = MEM_TYPE_DEVICE_MEMORY,
        .mem.memoryBase     = 0,
        .mem.memorySize     = 0,
        .mem.memoryType     = MEM_TYPE_NONE,        
    },

    // EndOfListTag
    {
        .driverInit         = NULL,
        .mmio.memoryBase    = -1,
        .mmio.memorySize    = -1,
        .mmio.memoryType    = -1,
        .mem.memoryBase     = -1,
        .mem.memorySize     = -1,
        .mem.memoryType     = -1,
    },
};

//-----------------------------------------------------------------------------
// platform Init.   Platform Specific Initialization
//-----------------------------------------------------------------------------
void platformInit(void)
{
    platformBuildTable();

    fdt_header_t* ftd = (fdt_header_t*)(0x40000000);
    void * pDevTree = (void*)(ftd->off_dt_struct + ((uintptr_t) ftd));

    ParseTree(pDevTree);

     MmuInit((((uint64_t)(1024LL*1024LL*1024LL) + (uint64_t)(16*1024*1024)) - 1) - ((1) * (1024) * (1024)) + 1, ((1) * (1024) * (1024))); 
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static status_t platformBuildTable(void)
{
    // fill in the platform memory table

    platformMemory[0].memoryBase    = _OS_VIRT_ROM_AREA_BASE;
    platformMemory[0].memorySize    = _OS_VIRT_ROM_AREA_SIZE;
    platformMemory[0].memoryType    = MEM_TYPE_NORMAL_MEMORY;

    platformMemory[1].memoryBase    = ((1024) * (1024) * (1024));  // TODO clean up these magic numbers
    platformMemory[1].memorySize    = ((((uint64_t)(1024LL*1024LL*1024LL) + (uint64_t)(16*1024*1024)) - 1) + 1) - ((1024) * (1024) * (1024));
    platformMemory[1].memoryType    = MEM_TYPE_NORMAL_MEMORY;

    platformMemory[2].memoryBase    = -1;
    platformMemory[2].memorySize    = -1;
    platformMemory[2].memoryType    = -1;

    return STATUS_SUCCESS;
}

typedef enum _fdt_token_t
{
    FDT_BEGIN_NODE = 0x00000001,
    FDT_END_NODE   = 0x00000002,
    FDT_PROP       = 0x00000003,
    FDT_NOP        = 0x00000004,
    FDT_END        = 0x00000009,
} ftd_token_t;

typedef struct _deviceNode_t
{
    struct _deviceNode_t *  parentNode;
    struct _deviceNode_t *  nextPeerNode;
    struct _deviceNode_t *  prevPeerNode;
    struct _deviceNode_t *  firstChildNode;
    struct _deviceNode_t *  lastChildNode;
    uint32_t        interruptNumber;
    uintptr_t       baseAddress;
    uintptr_t       size;
    uintptr_t       offset;
    // function pointers to various driver related functions.
} deviceNode_t;

deviceNode_t    fdtDeviceList[256];
deviceNode_t*   fdtDeviceTreeRoot;
uint32_t        fdtCurrentIndex = 0;
deviceNode_t*   fdtCurrentNode;




//-------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------
static deviceNode_t* fdtGetFreeDeviceNode()
{
    fdtCurrentIndex++;
    fdtCurrentNode = &fdtDeviceList[fdtCurrentIndex];

    fdtCurrentNode->firstChildNode  = NULL;
    fdtCurrentNode->lastChildNode   = NULL;
    fdtCurrentNode->parentNode      = NULL;
    fdtCurrentNode->nextPeerNode    = NULL;
    fdtCurrentNode->baseAddress     = 0;
    fdtCurrentNode->offset          = 0;
    fdtCurrentNode->interruptNumber = -1;

    return fdtCurrentNode;
}

static void fdtProcessNode(deviceNode_t* node)
{
    //find and init driver
    
}

//-------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------
static uint32_t fdtGetNextToken(unsigned char **pCurrentLocation)
{
    unsigned char * nextToken = *pCurrentLocation;
    uint32_t beToken = (uint32_t) *nextToken;
    uint32_t token = __builtin_bswap32(beToken);
    pCurrentLocation += sizeof(uint32_t);
    return token;
}

//-------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------
static void fdtCollectName(uint8_t **pCurrentLocation, deviceNode_t * devNode)
{
    while (**pCurrentLocation != '\0')
    {
        (*pCurrentLocation)++;
    }

    // move the pointer to a 32bit aligned address 
    uintptr_t alignment = (uintptr_t)*pCurrentLocation;
    alignment += 3;
    alignment &=  ~(uintptr_t)(0x3);
    *pCurrentLocation = (uint8_t*)alignment;
}

//-------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------
void fdtCollectRelativeOffset(uint8_t **pCurrentLocation, deviceNode_t * devNode)
{
    uint64_t offset = *((uint64_t*)(*pCurrentLocation));
    offset = __builtin_bswap64(offset);
    *pCurrentLocation += sizeof(uint64_t);
}


//-------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------
static void fdtSetParentDevNode(deviceNode_t * parentNode, deviceNode_t * childNode)
{
    childNode->parentNode = parentNode;

    // Add node to parent child list
    childNode->nextPeerNode = parentNode = parentNode->firstChildNode;
    parentNode->firstChildNode = childNode;     
}

//-------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------
static deviceNode_t * fdtGetParentNode(deviceNode_t * node)
{
    return node->parentNode;
}

//-------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------
void FdtParseTree(void *fdt)
{
    uint8_t * pCurrentLocation;

	ftd_token_t token;
    deviceNode_t* newNode;
    deviceNode_t* currentNode;    

    while ((token = fdtGetNextToken(&pCurrentLocation)) != FDT_END)
    {
        switch(token)
        {
            case FDT_BEGIN_NODE:
                newNode = fdtGetFreeDeviceNode();
                fdtSetParentDevNode(currentNode, newNode);
                fdtCollectName(&pCurrentLocation, currentNode);
                fdtCollectRelativeOffset(&pCurrentLocation, currentNode);
                break;

            case FDT_END_NODE:
                fdtProcessNode(currentNode);
                currentNode = fdtGetParentNode(currentNode);
                break;

            case FDT_PROP:
            {
//                CollectProperty Name Offset
//                CollectPropertyLen
//                if (PropertyNameIsOneWeCareAbout(PropName))
                {
//                    CollectProperty() // Note we are only interested in some Properties
                }
//                MovePastProp()
            }

            case FDT_NOP:
            {
                break;
            }

            case FDT_END:

                break;
        }
    }
}

