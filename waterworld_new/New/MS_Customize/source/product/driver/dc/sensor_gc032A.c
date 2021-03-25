/******************************************************************************
 ** File Name:    Sensor_GC032A.c                                         *
 ** Author:         Haydn_He                                                 *
 ** Date:           2012-11-16	                                         *
 ** Platform:       SP6530
 ** Copyright:    Spreadtrum All Rights Reserved.        *
 ** Description:   implementation of digital camera register interface       *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Log$
 ** DATE             NAME             DESCRIPTION                                 *
 ** 2012-11-16   Haydn            Fristly Released
 *******************************************************************************/
#ifndef _GC032A_C_

#define _GC032A_C_
#include "ms_customize_trc.h"
#include "sensor_cfg.h"
#include "sensor_drv.h"
//#include "i2c_drv.h"
#include "os_api.h"
#include "chip.h"
#include "dal_dcamera.h"


/**---------------------------------------------------------------------------*
 ** 						Compiler Flag									  *
 **---------------------------------------------------------------------------*/
#ifdef	 __cplusplus
	extern	 "C"
	{
#endif

/**---------------------------------------------------------------------------*
 **                         Const variables                                   *
 **---------------------------------------------------------------------------*/
#define GC032A_I2C_ADDR_W    			0x42
#define GC032A_I2C_ADDR_R    			0x43
#define GC032A_I2C_ACK					0x0
LOCAL BOOLEAN  	bl_GC_50Hz_GC032A		= FALSE;

/**---------------------------------------------------------------------------*
     GC032A SUPPORT 5 SERIAL MODE:
     CCIR601_8BIT, SPI, CCIR656_2BIT, CCIR656_4BIT, ONE2ONE(GC032A&GC6103)
 **---------------------------------------------------------------------------*/


// use next two interface
//#define GC032A_OUTPUT_MODE_CCIR656_2BIT
#define GC032A_OUTPUT_MODE_PACKET_DDR_2BIT 

//#define GC032A_SERIAL_LOAD_FROM_T_FLASH 

/**---------------------------------------------------------------------------*
 **                     Local Function Prototypes                             *
 **---------------------------------------------------------------------------*/
LOCAL uint32 GC032A_Power_On(uint32 power_on);
LOCAL uint32 GC032A_Identify(uint32 param);
LOCAL void   GC032A_WriteReg( uint8  subaddr, uint8 data );
LOCAL uint8  GC032A_ReadReg( uint8  subaddr);
LOCAL void   GC032A_Write_Group_Regs( SENSOR_REG_T* sensor_reg_ptr );
LOCAL void   GC032A_Delayms (uint32 ms);
//LOCAL uint32 Set_GC032A_Mirror(uint32 level);
//LOCAL uint32 Set_GC032A_Flip(uint32 level);
LOCAL uint32 Set_GC032A_Brightness(uint32 level);
LOCAL uint32 Set_GC032A_Contrast(uint32 level);
LOCAL uint32 Set_GC032A_Image_Effect(uint32 effect_type);
LOCAL uint32 Set_GC032A_Ev(uint32 level);
LOCAL uint32 Set_GC032A_Anti_Flicker(uint32 mode);
LOCAL uint32 Set_GC032A_Preview_Mode(uint32 preview_mode);
LOCAL uint32 Set_GC032A_AWB(uint32 mode);
LOCAL uint32 GC032A_AE_AWB_Enable(uint32 ae_enable, uint32 awb_enable);
LOCAL uint32 GC032A_Before_Snapshot(uint32 para);
LOCAL uint32 GC032A_After_Snapshot(uint32 para);
LOCAL uint32 Set_GC032A_Video_Mode(uint32 mode);
LOCAL uint32 GC032A_GetPclkTab(uint32 param);
LOCAL uint32 GC032A_set_flash_enable(uint32 enable);
LOCAL BOOLEAN NightMode_GC032A	= FALSE;

/**---------------------------------------------------------------------------*
 ** 						Local Variables 								 *
 **---------------------------------------------------------------------------*/
#ifdef GC032A_SERIAL_LOAD_FROM_T_FLASH  
#include "sfs.h"
#include "ffs.h"
LOCAL uint32 Load_GC032A_RegTab_From_T_Flash(void);
#endif


__align(4) const SENSOR_REG_T GC032A_YUV_640X480[] =
{
#if defined(GC032A_OUTPUT_MODE_CCIR656_2BIT)
	/*System*/
	{0xf3,0x83},
	{0xf5,0x0c},
	{0xf7,0x01},
	{0xf8,0x01},//PLL 01  //03
	{0xf9,0x4e},
	{0xfa,0x10},
	{0xfc,0x02},
	{0xfe,0x02},
	{0x81,0x03},

	{0xfe,0x00},
	{0x77,0x64},
	{0x78,0x40},
	{0x79,0x60},
	/*Analog&Cisctl*/
	{0xfe,0x00},
	{0x03,0x01},
	{0x04,0x20},
	{0x05,0x01},
	{0x06,0xaf},
	{0x07,0x00},
	{0x08,0x08},
	{0x0a,0x00},
	{0x0c,0x00},
	{0x0d,0x01},
	{0x0e,0xe8},
	{0x0f,0x02},
	{0x10,0x88},
	{0x17,0x54},
	{0x19,0x08},
	{0x1a,0x0a},
	{0x1f,0x40},
	{0x20,0x30},
	{0x2e,0x80},
	{0x2f,0x2b},
	{0x30,0x1a},
	{0xfe,0x02},
	{0x03,0x02},
	{0x06,0x60},
	{0x05,0xd7},
	{0x12,0x89},

	/*SPI*/
	{0xfe,0x03},
	{0x51,0x01},
	{0x52,0xd8},
	{0x53,0x25},//[7]crc [3:2]line_num
	{0x54,0x20},
	{0x55,0x00},//20
	{0x59,0x10},
	{0x5a,0x00},
	{0x5b,0x80},
	{0x5c,0x02},
	{0x5d,0xe0},
	{0x5e,0x01},
	{0x64,0x06}, //[1]sck always 06 04
	{0x65,0xff},  //head sync code
	{0x66,0x00},
	{0x67,0x00},

	//SYNC code
	{0x60,0xb6},  //frame_start -- not need
	{0x61,0x80},  //line sync start
	{0x62,0x9d},  //line sync end
	{0x63,0xb6},  //frame end */

	/*blk*/
	{0xfe,0x00},
	{0x18,0x02},

	{0xfe,0x02},
	{0x40,0x22},
	{0x45,0x00},
	{0x46,0x00},
	{0x49,0x20},
	{0x4b,0x3c},
	{0x50,0x20},
	{0x42,0x10},

	/*isp*/
	{0xfe,0x01},
	{0x0a,0xc5},
	{0x45,0x00},
	{0xfe,0x00},
	{0x40,0xff},
	{0x41,0x25},
	{0x42,0xcf},
	{0x43,0x10},
	{0x44,0x83},
	{0x46,0x22},
	{0x49,0x03},
	{0x52,0x02},
	{0x54,0x00},	
	{0xfe,0x02},
	{0x22,0xf6},
	
	/*Shading*/
	{0xfe,0x01},
	{0xc1,0x38},
	{0xc2,0x4c},
	{0xc3,0x00},
	{0xc4,0x32},
	{0xc5,0x24},
	{0xc6,0x16},
	{0xc7,0x08},
	{0xc8,0x08},
	{0xc9,0x00},
	{0xca,0x20},
	{0xdc,0x8a},
	{0xdd,0xa0},
	{0xde,0xa6},
	{0xdf,0x75},

	/*AWB*/ /*20170110*/
	{0xfe,0x01},
	{0x7c,0x09},
	{0x65,0x06},
	{0x7c,0x08},
	{0x56,0xf4},
	{0x66,0x0f},
	{0x67,0x84},
	{0x6b,0x80},
	{0x6d,0x12},
	{0x6e,0xb0}, 
	{0xfe,0x01},
	{0x90,0x00},
	{0x91,0x00},
	{0x92,0xf4},
	{0x93,0xd5},
	{0x95,0x0f},
	{0x96,0xf4},
	{0x97,0x2d},
	{0x98,0x0f},
	{0x9a,0x2d},
	{0x9b,0x0f},
	{0x9c,0x59},
	{0x9d,0x2d},
	{0x9f,0x67},
	{0xa0,0x59},
	{0xa1,0x00},
	{0xa2,0x00},
	{0x86,0x00},
	{0x87,0x00},
	{0x88,0x00},
	{0x89,0x00},
	{0xa4,0x00},
	{0xa5,0x00},
	{0xa6,0xd4},
	{0xa7,0x9f},
	{0xa9,0xd4},
	{0xaa,0x9f},
	{0xab,0xac},
	{0xac,0x9f},
	{0xae,0xd4},
	{0xaf,0xac},
	{0xb0,0xd4},
	{0xb1,0xa3},
	{0xb3,0xd4},
	{0xb4,0xac},
	{0xb5,0x00},
	{0xb6,0x00},
	{0x8b,0x00},
	{0x8c,0x00},
	{0x8d,0x00},
	{0x8e,0x00},
	{0x94,0x50},
	{0x99,0xa6},
	{0x9e,0xaa},
	{0xa3,0x0a},
	{0x8a,0x00},
	{0xa8,0x50},
	{0xad,0x55},
	{0xb2,0x55},
	{0xb7,0x05},
	{0x8f,0x00},
	{0xb8,0xb3},
	{0xb9,0xb6},

	/*CC*/ 
	{0xfe,0x01},
	{0xd0,0x40},
	{0xd1,0xf8},
	{0xd2,0x00},
	{0xd3,0xfa},
	{0xd4,0x45},
	{0xd5,0x02},
	{0xd6,0x30},
	{0xd7,0xfa},
	{0xd8,0x08},
	{0xd9,0x08},
	{0xda,0x58},
	{0xdb,0x02},
	{0xfe,0x00},	

	/*Gamma*/
	{0xfe,0x00},
	{0xba,0x00},
	{0xbb,0x04},
	{0xbc,0x0a},
	{0xbd,0x0e},
	{0xbe,0x22},
	{0xbf,0x30},
	{0xc0,0x3d},
	{0xc1,0x4a},
	{0xc2,0x5d},
	{0xc3,0x6b},
	{0xc4,0x7a},
	{0xc5,0x85},
	{0xc6,0x90},
	{0xc7,0xa5},
	{0xc8,0xb5},
	{0xc9,0xc2},
	{0xca,0xcc},
	{0xcb,0xd5},
	{0xcc,0xde},
	{0xcd,0xea},
	{0xce,0xf5},
	{0xcf,0xff},
	
	/*Auto Gamma*/                      
	{0xfe,0x00},
	{0x5a,0x08},
	{0x5b,0x0f},
	{0x5c,0x15},
	{0x5d,0x1c},
	{0x5e,0x28},
	{0x5f,0x36},
	{0x60,0x45},
	{0x61,0x51},
	{0x62,0x6a},
	{0x63,0x7d},
	{0x64,0x8d},
	{0x65,0x98},
	{0x66,0xa2},
	{0x67,0xb5},
	{0x68,0xc3},
	{0x69,0xcd},
	{0x6a,0xd4},
	{0x6b,0xdc},
	{0x6c,0xe3},
	{0x6d,0xf0},
	{0x6e,0xf9},
	{0x6f,0xff},
	
	/*Gain*/
	{0xfe,0x00},
	{0x70,0x50},

	/*AEC*/
	{0xfe,0x00},
	{0x4f,0x01},
	{0xfe,0x01},
	{0x0d,0x00},//08 add 20170110 	
	{0x12,0xa0},
	{0x13,0x3a},
	{0x44,0x04},
	{0x1f,0x30},
	{0x20,0x40},

	{0x3e,0x20},
	{0x3f,0x2d},
	{0x40,0x40},
	{0x41,0x5b},
	{0x42,0x82},
	{0x43,0xb7},
	{0x04,0x0a},
	{0x02,0x79},
	{0x03,0xc0},

	/*measure window*/
	{0xfe,0x01},
	{0xcc,0x08},
	{0xcd,0x08},
	{0xce,0xa4},
	{0xcf,0xec},

	/*DNDD*/
	{0xfe,0x00},
	{0x81,0xb8},
	{0x82,0x12},
	{0x83,0x0a},
	{0x84,0x01},
	{0x86,0x50},
	{0x87,0x18},
	{0x88,0x10},
	{0x89,0x70},
	{0x8a,0x20},
	{0x8b,0x10},
	{0x8c,0x08},
	{0x8d,0x0a},

	/*Intpee*/
	{0xfe,0x00},
	{0x8f,0xaa},
	{0x90,0x9c},
	{0x91,0x52},
	{0x92,0x03},
	{0x93,0x03},
	{0x94,0x08},
	{0x95,0x44},
	{0x97,0x00},
	{0x98,0x00},
	
	/*ASDE*/
	{0xfe,0x00},
	{0xa1,0x30},
	{0xa2,0x41},
	{0xa4,0x30},
	{0xa5,0x20},
	{0xaa,0x30},
	{0xac,0x32},

	/*YCP*/
	{0xfe,0x00},
	{0xd1,0x3c},
	{0xd2,0x3c},
	{0xd3,0x38},
	{0xd6,0xf4},
	{0xd7,0x1d},
	{0xdd,0x73},
	{0xde,0x84},
	
	/*Banding*/
	{0xfe,0x00},
	{0x05,0x01},//hb
	{0x06,0xe4},
	{0x07,0x00},//vb
	{0x08,0x08},
	
	{0xfe,0x01},
	{0x25,0x00},//step
	{0x26,0x24},

	{0x27,0x01},//7.1fps
	{0x28,0xb0},
	{0x29,0x01},//7.1fps
	{0x2a,0xb0},
	{0x2b,0x01},//7.1fps
	{0x2c,0xb0},
	{0x2d,0x01},//7.1fps
	{0x2e,0xb0},
	{0x2f,0x02},//6.24fps
	{0x30,0x40},
	{0x31,0x02},//4.99fps
	{0x32,0xd0},
	{0x33,0x03},//4.16fps
	{0x34,0x60},
	{0x3c,0x30},
	{0xfe,0x00},
		
#elif defined(GC032A_OUTPUT_MODE_PACKET_DDR_2BIT)
	/*System*/
	{0xf3,0x83}, //ff//1f//01 data output
	{0xf5,0x0f},
	{0xf7,0x01},
	{0xf8,0x01},
	{0xf9,0x4e},
	{0xfa,0x00},
	{0xfc,0x02},
	{0xfe,0x02},
	{0x81,0x03}, 

	{0xfe,0x00},
	{0x77,0x64},
	{0x78,0x40},
	{0x79,0x60},
	/*Analog&Cisctl*/
	{0xfe,0x00},
	{0x03,0x01},
	{0x04,0xf8},
	{0x05,0x01},
	{0x06,0xa8}, 
	{0x07,0x00},
	{0x08,0x10},

	{0x0a,0x00},
	{0x0c,0x00},
	{0x0d,0x01},
	{0x0e,0xe8},
	{0x0f,0x02},
	{0x10,0x88}, 
	{0x17,0x57},
	{0x19,0x08},
	{0x1a,0x0a},
	{0x1f,0x40},
	{0x20,0x30},
	{0x2e,0x80},
	{0x2f,0x2b},
	{0x30,0x1a},
	{0xfe,0x02},
	{0x03,0x02},
	{0x05,0xe7},//d7
	{0x06,0x60},
	{0x08,0x80},
	{0x12,0x89},

	/*SPI*/
	{0xfe,0x03},
	{0x52,0x38},
	{0x53,0x24},
	{0x54,0x20},
	{0x55,0x00},
	{0x59,0x10},
	{0x5a,0x40}, //00 //yuv 
	{0x5b,0x80},
	{0x5c,0x02},
	{0x5d,0xe0},
	{0x5e,0x01},
	{0x51,0x03},
	{0x64,0x06},
	{0xfe,0x00},

	/*blk*/
	{0xfe,0x00},
	{0x18,0x02},
	{0xfe,0x02},
	{0x40,0x22},
	{0x45,0x00},
	{0x46,0x00},
	{0x49,0x20},
	{0x4b,0x3c},
	{0x50,0x20},
	{0x42,0x10},

	/*isp*/
	{0xfe,0x01},
	{0x0a,0xc5},
	{0x45,0x00},
	{0xfe,0x00},
	{0x40,0xff},
	{0x41,0x25},
	{0x42,0xcf},
	{0x43,0x10},
	{0x44,0x83},
	{0x46,0x22},
	{0x49,0x03},
	{0x52,0x02},
	{0x54,0x00},	
	{0xfe,0x02},
	{0x22,0xf6},

	/*Shading*/ 
	{0xfe,0x01},
	{0xc1,0x38},
	{0xc2,0x4c},
	{0xc3,0x00},
	{0xc4,0x32},
	{0xc5,0x24},
	{0xc6,0x16},
	{0xc7,0x08},
	{0xc8,0x08},
	{0xc9,0x00},
	{0xca,0x20},
	{0xdc,0x8a},
	{0xdd,0xa0},
	{0xde,0xa6},
	{0xdf,0x75},

	/*AWB*/ /*20170110*/
	{0xfe,0x01},
	{0x7c,0x09},
	{0x65,0x0b},//06
	{0x66,0x10},//add 20200319,08
	{0x7c,0x08},
	{0x56,0xf4}, 
	{0x66,0x0f}, 
	{0x67,0x84}, 
	{0x6b,0x80},
	{0x6d,0x12},
	{0x6e,0xb0}, 
	{0xfe,0x01},
	{0x90,0x00},
	{0x91,0x00},
	{0x92,0xf4},
	{0x93,0xd5},
	{0x95,0x0f},
	{0x96,0xf4},
	{0x97,0x2d},
	{0x98,0x0f},
	{0x9a,0x2d},
	{0x9b,0x0f},
	{0x9c,0x59},
	{0x9d,0x2d},
	{0x9f,0x67},
	{0xa0,0x59},
	{0xa1,0x00},
	{0xa2,0x00},
	{0x86,0x00},
	{0x87,0x00},
	{0x88,0x00},
	{0x89,0x00},
	{0xa4,0x00},
	{0xa5,0x00},
	{0xa6,0xd4},
	{0xa7,0x9f},
	{0xa9,0xd4},
	{0xaa,0x9f},
	{0xab,0xac},
	{0xac,0x9f},
	{0xae,0xd4},
	{0xaf,0xac},
	{0xb0,0xd4},
	{0xb1,0xa3},
	{0xb3,0xd4},
	{0xb4,0xac},
	{0xb5,0x00},
	{0xb6,0x00},
	{0x8b,0x00},
	{0x8c,0x00},
	{0x8d,0x00},
	{0x8e,0x00},
	{0x94,0x50},
	{0x99,0xa6},
	{0x9e,0xaa},
	{0xa3,0x0a},
	{0x8a,0x00},
	{0xa8,0x50},
	{0xad,0x55},
	{0xb2,0x55},
	{0xb7,0x05},
	{0x8f,0x00},
	{0xb8,0xb3},
	{0xb9,0xb6},

	/*CC*/ 
	{0xfe,0x01},
	{0xd0,0x40},
	{0xd1,0xf8},
	{0xd2,0x00},
	{0xd3,0xfa},
	{0xd4,0x45},
	{0xd5,0x02},

	{0xd6,0x30},
	{0xd7,0xfa},
	{0xd8,0x08},
	{0xd9,0x08},
	{0xda,0x58},
	{0xdb,0x02},
	{0xfe,0x00},

	/*Gamma*/
	{0xfe,0x00},
	{0xba,0x00},
	{0xbb,0x04},
	{0xbc,0x0a},
	{0xbd,0x0e},
	{0xbe,0x22},
	{0xbf,0x30},
	{0xc0,0x3d},
	{0xc1,0x4a},
	{0xc2,0x5d},
	{0xc3,0x6b},
	{0xc4,0x7a},
	{0xc5,0x85},
	{0xc6,0x90},
	{0xc7,0xa5},
	{0xc8,0xb5},
	{0xc9,0xc2},
	{0xca,0xcc},
	{0xcb,0xd5},
	{0xcc,0xde},
	{0xcd,0xea},
	{0xce,0xf5},
	{0xcf,0xff},

	/*Auto Gamma*/
	{0xfe,0x00},
	{0x5a,0x08},
	{0x5b,0x0f},
	{0x5c,0x15},
	{0x5d,0x1c},
	{0x5e,0x28},
	{0x5f,0x36},
	{0x60,0x45},
	{0x61,0x51},
	{0x62,0x6a},
	{0x63,0x7d},
	{0x64,0x8d},
	{0x65,0x98},
	{0x66,0xa2},
	{0x67,0xb5},
	{0x68,0xc3},
	{0x69,0xcd},
	{0x6a,0xd4},
	{0x6b,0xdc},
	{0x6c,0xe3},
	{0x6d,0xf0},
	{0x6e,0xf9},
	{0x6f,0xff},

	/*Gain*/
	{0xfe,0x00}, 
	{0x70,0x50}, 

	/*AEC*/
	{0xfe,0x00},
	{0x4f,0x01},
	{0xfe,0x01},
	{0x0d,0x00},//08 add 20170110 	
	{0x12,0xa0},
	{0x13,0x35},//3a
	{0x44,0x04},
	{0x1f,0x30},
	{0x20,0x40},

	{0x3e,0x20},
	{0x3f,0x2d},
	{0x40,0x40},
	{0x41,0x5b},
	{0x42,0x82},
	{0x43,0xb7},
	{0x04,0x0a},
	{0x02,0x79},
	{0x03,0xc0},

	/*measure window*/
	{0xfe,0x01},
	{0xcc,0x08},
	{0xcd,0x08},
	{0xce,0xa4},
	{0xcf,0xec},

	/*DNDD*/
	{0xfe,0x00},
	{0x81,0xb8},
	{0x82,0x20},//12
	{0x83,0x15},//0a
	{0x84,0x01},
	{0x86,0x50},
	{0x87,0x18},
	{0x88,0x10},
	{0x89,0x70},
	{0x8a,0x20},
	{0x8b,0x10},
	{0x8c,0x08},
	{0x8d,0x0a},

	/*Intpee*/
	{0xfe,0x00},
	{0x8f,0xaa},
	{0x90,0x9c},
	{0x91,0x52},
	{0x92,0x03},
	{0x93,0x03},
	{0x94,0x08},
	{0x95,0x44},
	{0x97,0x00},
	{0x98,0x00},

	/*ASDE*/
	{0xfe,0x00},
	{0xa1,0x30},
	{0xa2,0x41},
	{0xa4,0x30},
	{0xa5,0x20},
	{0xaa,0x30},
	{0xac,0x32},

	/*YCP*/
	{0xfe,0x00},
	{0xd1,0x45},//40
	{0xd2,0x45},//40
	{0xd3,0x50},//38
	{0xd6,0xf4},
	{0xd7,0x1d},
	{0xdd,0x73},
	{0xde,0x84},

	/*Banding*/
	{0xfe,0x00},
	{0x05,0x01},
	{0x06,0xa8},
	{0x07,0x00},
	{0x08,0x10},

	{0xfe,0x01},
	{0x25,0x00},
	{0x26,0x54},

	{0x27,0x01},
	{0x28,0xf8},//16.6fps
	{0x29,0x02},
	{0x2a,0xa0},//12.5fps
	{0x2b,0x03},
	{0x2c,0x48},//10fps
	{0x2d,0x03},
	{0x2e,0xf0},//8.33fps
	{0x2f,0x05},
	{0x30,0x94},//5.88fps
	{0x31,0x07},
	{0x32,0x8c},//4.34fps
	{0x33,0x08},
	{0x34,0x34},//3.99fps
	{0x3c,0x30},
	{0xfe,0x00},

#endif
};

LOCAL SENSOR_REG_TAB_INFO_T s_GC032A_resolution_Tab_YUV[]=
{
#if defined(GC032A_OUTPUT_MODE_PACKET_DDR_2BIT)
    {ADDR_AND_LEN_OF_ARRAY(GC032A_YUV_640X480), 640, 480, 26, SENSOR_IMAGE_FORMAT_YUV422},
    // YUV422 PREVIEW 1
    {PNULL, 0, 640, 480, 26, SENSOR_IMAGE_FORMAT_YUV422},
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0},

#elif defined(GC032A_OUTPUT_MODE_CCIR656_2BIT)
    {ADDR_AND_LEN_OF_ARRAY(GC032A_YUV_640X480), 640, 480, 24, SENSOR_IMAGE_FORMAT_YUV422},
    // YUV422 PREVIEW 1
    {PNULL, 0, 640, 480, 24, SENSOR_IMAGE_FORMAT_YUV422},
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0},
#endif
     // YUV422 PREVIEW 2
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0},

};

/******************************************************************************/
// Description:
// Global resource dependence:
// Author:
// Note:
//
/******************************************************************************/
#ifdef GC032A_SERIAL_LOAD_FROM_T_FLASH
LOCAL SENSOR_IOCTL_FUNC_TAB_T s_GC032A_ioctl_func_tab =
{
// Internal
    PNULL,
    GC032A_Power_On,
    PNULL,
    GC032A_Identify,
    PNULL,
    PNULL,

    PNULL,
    GC032A_GetPclkTab,

// External
    PNULL,
    PNULL, //Set_GC032A_Mirror,
    PNULL, //Set_GC032A_Flip,

    PNULL,//Set_GC032A_Brightness,
    PNULL,//Set_GC032A_Contrast,
    PNULL,
    PNULL,
    Set_GC032A_Preview_Mode,

    PNULL,//Set_GC032A_Image_Effect,
    PNULL,//GC032A_Before_Snapshot,
    PNULL,//GC032A_After_Snapshot,

    PNULL,//GC032A_set_flash_enable,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,//Set_GC032A_AWB,
    PNULL,
    PNULL,
    PNULL,//Set_GC032A_Ev,
    PNULL,
    PNULL,
    PNULL,

    PNULL,
    PNULL,
    PNULL,//Set_GC032A_Anti_Flicker,
    PNULL,//Set_GC032A_Video_Mode,
    PNULL,
};
#else
LOCAL SENSOR_IOCTL_FUNC_TAB_T s_GC032A_ioctl_func_tab =
{
// Internal
    PNULL,
    GC032A_Power_On,
    PNULL,
    GC032A_Identify,
    PNULL,
    PNULL,

    PNULL,
    GC032A_GetPclkTab,

// External
    PNULL,
    PNULL, //Set_GC032A_Mirror,
    PNULL, //Set_GC032A_Flip,

    Set_GC032A_Brightness,
    Set_GC032A_Contrast,
    PNULL,
    PNULL,
    Set_GC032A_Preview_Mode,

    Set_GC032A_Image_Effect,
    PNULL,//GC032A_Before_Snapshot,
    PNULL,//GC032A_After_Snapshot,

    GC032A_set_flash_enable,//GC032A_set_flash_enable,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    PNULL,
    Set_GC032A_AWB,
    PNULL,
    PNULL,
    Set_GC032A_Ev,
    PNULL,
    PNULL,
    PNULL,

    PNULL,
    PNULL,
    Set_GC032A_Anti_Flicker,
    Set_GC032A_Video_Mode,
    PNULL,
    PNULL,	
};
#endif


/**---------------------------------------------------------------------------*
 ** 						Global Variables								  *
 **---------------------------------------------------------------------------*/
PUBLIC SENSOR_INFO_T g_GC032A_yuv_info =
{
	GC032A_I2C_ADDR_W,				// salve i2c write address
	GC032A_I2C_ADDR_R, 				// salve i2c read address

	0,								// bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
									// bit2: 0: i2c register addr  is 8 bit, 1: i2c register addr  is 16 bit
									// other bit: reseved
	SENSOR_HW_SIGNAL_PCLK_P|\
	SENSOR_HW_SIGNAL_VSYNC_N|\

#if defined(GC032A_OUTPUT_MODE_PACKET_DDR_2BIT)
	SENSOR_HW_SIGNAL_HSYNC_N,	
#elif defined(GC032A_OUTPUT_MODE_CCIR656_2BIT)
	SENSOR_HW_SIGNAL_HSYNC_P,	
#endif
									// bit0: 0:negative; 1:positive -> polarily of pixel clock
									// bit2: 0:negative; 1:positive -> polarily of horizontal synchronization signal
									// bit4: 0:negative; 1:positive -> polarily of vertical synchronization signal
									// other bit: reseved

	// preview mode
	SENSOR_ENVIROMENT_NORMAL|\
	SENSOR_ENVIROMENT_NIGHT|\
	SENSOR_ENVIROMENT_SUNNY,

	// image effect
	SENSOR_IMAGE_EFFECT_NORMAL|\
	SENSOR_IMAGE_EFFECT_BLACKWHITE|\
	SENSOR_IMAGE_EFFECT_RED|\
	SENSOR_IMAGE_EFFECT_GREEN|\
	SENSOR_IMAGE_EFFECT_BLUE|\
	SENSOR_IMAGE_EFFECT_YELLOW|\
	SENSOR_IMAGE_EFFECT_NEGATIVE|\
	SENSOR_IMAGE_EFFECT_CANVAS,

	// while balance mode
	0,

	7,								// bit[0:7]: count of step in brightness, contrast, sharpness, saturation
									// bit[8:31] reseved

	SENSOR_LOW_PULSE_RESET,		// reset pulse level
	100,								// reset pulse width(ms)

	SENSOR_HIGH_LEVEL_PWDN,			// 1: high level valid; 0: low level valid

	2,								// count of identify code
	{{0xf0, 0x23},						// supply two code to identify sensor.
	{0xf1, 0x2a}},						// for Example: index = 0-> Device id, index = 1 -> version id

	SENSOR_AVDD_2800MV,				// voltage of avdd

	640,							// max width of source image
	480,							// max height of source image
	"GC032A",						// name of sensor

	SENSOR_IMAGE_FORMAT_YUV422,		// define in SENSOR_IMAGE_FORMAT_E enum,
									// if set to SENSOR_IMAGE_FORMAT_MAX here, image format depent on SENSOR_REG_TAB_INFO_T
	SENSOR_IMAGE_PATTERN_YUV422_YUYV,	// pattern of input image form sensor;

	s_GC032A_resolution_Tab_YUV,	// point to resolution table information structure
	&s_GC032A_ioctl_func_tab,		// point to ioctl function table

	PNULL,							// information and table about Rawrgb sensor
	PNULL,							// extend information about sensor
	SENSOR_AVDD_1800MV,                     // iovdd
	SENSOR_AVDD_1800MV,                      // dvdd
	0,                     // skip frame num before preview
	0,                      // skip frame num before capture
	0,                     // deci frame num during preview;
	2,                     // deci frame num during video preview;
	0,                     // threshold enable
	0,                     // threshold mode
	0,                     // threshold start postion
	0,                     // threshold end postion

#if defined(GC032A_OUTPUT_MODE_CCIR656_2BIT)
	SENSOR_OUTPUT_MODE_CCIR656_2BIT,
#elif defined(GC032A_OUTPUT_MODE_PACKET_DDR_2BIT)
    SENSOR_OUTPUT_MODE_PACKET_DDR_2BIT, 
#else
	SENSOR_OUTPUT_MODE_CCIR656_2BIT,
#endif
#if defined(GC032A_OUTPUT_MODE_CCIR656_2BIT)
	SENSOR_OUTPUT_ENDIAN_BIG
#elif defined(GC032A_OUTPUT_MODE_PACKET_DDR_2BIT)
    SENSOR_OUTPUT_ENDIAN_LITTLE
#else
	SENSOR_OUTPUT_ENDIAN_BIG

#endif
};

/**---------------------------------------------------------------------------*
 ** 							Function  Definitions
 **---------------------------------------------------------------------------*/
LOCAL void GC032A_WriteReg( uint8  subaddr, uint8 data )
{

	#ifndef	_USE_DSP_I2C_
		uint8 cmd[2];
		cmd[0]	=	subaddr;
		cmd[1]	=	data;
		//I2C_WriteCmdArr(GC032A_I2C_ADDR_W, cmd, 2, SCI_TRUE);
		Sensor_WriteReg(subaddr, data);
	#else
		DSENSOR_IICWrite((uint16)subaddr, (uint16)data);
	#endif

	//SCI_TRACE_LOW("SENSOR: GC032A_WriteReg reg/value(%x,%x) !!", subaddr, data);

}

LOCAL uint8 GC032A_ReadReg( uint8  subaddr)
{
	uint8 value = 0;

	#ifndef	_USE_DSP_I2C_
		//I2C_WriteCmdArr(GC032A_I2C_ADDR_W, &subaddr, 1, SCI_TRUE);
		//I2C_ReadCmd(GC032A_I2C_ADDR_R, &value, SCI_TRUE);
		value =Sensor_ReadReg( subaddr);
	#else
		value = (uint16)DSENSOR_IICRead((uint16)subaddr);
	#endif

	//SCI_TRACE_LOW("SENSOR: GC032A_ReadReg reg/value(%x,%x) !!", subaddr, value);

	return value;
}


LOCAL void GC032A_Write_Group_Regs( SENSOR_REG_T* sensor_reg_ptr )
{
    uint32 i;

    for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value) ; i++)
    {
        Sensor_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
    }

}

LOCAL void GC032A_Delayms (uint32 ms)
{
	uint32 t1, t2;

	t1 = t2 = SCI_GetTickCount ();

	do{
		t2 = SCI_GetTickCount ();
	}while (t2 < (t1+ms));
}


/**---------------------------------------------------------------------------*
 **                         Function Definitions                              *
 **---------------------------------------------------------------------------*/
LOCAL uint32 GC032A_Power_On(uint32 power_on)
{
	SENSOR_AVDD_VAL_E		dvdd_val=g_GC032A_yuv_info.dvdd_val;
	SENSOR_AVDD_VAL_E		avdd_val=g_GC032A_yuv_info.avdd_val;
	SENSOR_AVDD_VAL_E		iovdd_val=g_GC032A_yuv_info.iovdd_val;
	BOOLEAN 				power_down=g_GC032A_yuv_info.power_down_level;
	BOOLEAN 				reset_level=g_GC032A_yuv_info.reset_pulse_level;
	uint32 				reset_width=g_GC032A_yuv_info.reset_pulse_width;
      SCI_TRACE_LOW("set_GC032A_Power_On:%d",power_on);
	if(SCI_TRUE==power_on)
	{
		Sensor_SetVoltage(dvdd_val, avdd_val, iovdd_val);

		//GPIO_SetSensorPwdn(!power_down);
		#if 1
		 if(SENSOR_MAIN==Sensor_GetCurId())
		{
			GPIO_SetFrontSensorPwdn(power_down);
			GC032A_Delayms(2);
			GPIO_SetSensorPwdn(!power_down);
			GC032A_Delayms(2);
			GPIO_SetSensorPwdn(power_down);
			GC032A_Delayms(2);
			GPIO_SetSensorPwdn(!power_down);
            	}
		else
		{
			GPIO_SetSensorPwdn(power_down);
			GC032A_Delayms(2);
			GPIO_SetFrontSensorPwdn(!power_down);
			GC032A_Delayms(2);
			GPIO_SetFrontSensorPwdn(power_down);
			GC032A_Delayms(2);
			GPIO_SetFrontSensorPwdn(!power_down);
		}
        #endif

		// Open Mclk in default frequency
		Sensor_SetMCLK(SENSOR_DEFALUT_MCLK);
		//Sensor_SetResetLevel(!reset_level);
		//GC032A_Delayms(100);
		//Sensor_SetResetLevel(reset_level);
		//SCI_Sleep(reset_width);
		//Sensor_SetResetLevel(!reset_level);

	}
	else
	{
		GPIO_SetSensorPwdn(power_down);

		Sensor_SetMCLK(SENSOR_DISABLE_MCLK);

		Sensor_SetVoltage(SENSOR_AVDD_CLOSED, SENSOR_AVDD_CLOSED, SENSOR_AVDD_CLOSED);
	}

	SCI_TRACE_LOW("set_GC032A_Power_On");
	return SCI_SUCCESS;
}




/******************************************************************************/
// Description: sensor probe function
// Author:     benny.zou
// Input:      none
// Output:     result
// Return:     0           successful
//             others      failed
// Note:       this function only to check whether sensor is work, not identify
//              whitch it is!!
/******************************************************************************/
LOCAL uint32 GC032A_Identify(uint32 param)
{
#define GC032A_PID_VALUE    0xf0
#define GC032A_PID_ADDR     0x23
#define GC032A_VER_VALUE    0xf1
#define GC032A_VER_ADDR     0x2a

	uint32 i;
	uint32 nLoop;
	uint8 ret;
	uint32 err_cnt = 0;
	uint8 reg[2]    = {0xf0, 0xf1};
	uint8 value[2]  = {0x23, 0x2a};
	GC032A_Power_On(TRUE);

	SCI_TRACE_LOW("GC032A_Identify");
//	SCI_TRACE_ID(TRACE_TOOL_CONVERT,SENSOR_GC032A_725_112_2_18_0_30_46_781,(uint8*)"");
	for(i = 0; i<2; )
	{
		nLoop = 1000;
		ret = GC032A_ReadReg(reg[i]);
		if( ret != value[i])
		{
			err_cnt++;
			if(err_cnt>3)
			{
				SCI_TRACE_LOW("It is not GC032A");
			//	SCI_TRACE_ID(TRACE_TOOL_CONVERT,SENSOR_GC032A_735_112_2_18_0_30_46_782,(uint8*)"");
				return SCI_ERROR;
			}
			else
			{
				//Masked by frank.yang,SCI_Sleep() will cause a  Assert when called in boot precedure
				//SCI_Sleep(10);
				while(nLoop--)
				    { ;}
				continue;
			}
		}
		err_cnt = 0;
		i++;
	}

	SCI_TRACE_LOW("GC032A_Identify: it is GC032A");
	//SCI_TRACE_ID(TRACE_TOOL_CONVERT,SENSOR_GC032A_753_112_2_18_0_30_46_783,(uint8*)"");

	return (uint32)SCI_SUCCESS;

}


LOCAL SENSOR_TRIM_T s_GC032A_Pclk_Tab[]=
{
	// COMMON INIT
	{0, 0, 0, 0,  6},

	// YUV422 PREVIEW 1
	{0, 0, 0, 0,  6},
	{0, 0, 0, 0,  6},
	{0, 0, 0, 0,  6},
	{0, 0, 0, 0,  6},

	// YUV422 PREVIEW 2
	{0, 0, 0, 0,  0},
	{0, 0, 0, 0,  0},
	{0, 0, 0, 0,  0},
	{0, 0, 0, 0,  0}
};

LOCAL uint32 GC032A_GetPclkTab(uint32 param)
{
    return (uint32)s_GC032A_Pclk_Tab;
}


__align(4) const SENSOR_REG_T GC032A_brightness_tab[][2]=
{
	{{0xd5, 0xd0},	{0xff,0xff}},
	{{0xd5, 0xe0},	{0xff,0xff}},
	{{0xd5, 0xf0},	{0xff,0xff}},
	{{0xd5, 0x00},	{0xff,0xff}},
	{{0xd5, 0x20},	{0xff,0xff}},
	{{0xd5, 0x30},	{0xff,0xff}},
	{{0xd5, 0x40},	{0xff,0xff}},
};

LOCAL uint32 Set_GC032A_Brightness(uint32 level)
{

    SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GC032A_brightness_tab[level];

    SCI_ASSERT(PNULL != sensor_reg_ptr);

    GC032A_Write_Group_Regs(sensor_reg_ptr);

    SCI_TRACE_LOW("set_GC032A_brightness: level = %d", level);

    return 0;
}



__align(4) const SENSOR_REG_T GC032A_contrast_tab[][2]=
{
	{{0xd3,0x30}, {0xff,0xff}},
	{{0xd3,0x34}, {0xff,0xff}},
	{{0xd3,0x38}, {0xff,0xff}},
	{{0xd3,0x40}, {0xff,0xff}},
	{{0xd3,0x44}, {0xff,0xff}},
	{{0xd3,0x48}, {0xff,0xff}},
	{{0xd3,0x50}, {0xff,0xff}},
};

LOCAL uint32 Set_GC032A_Contrast(uint32 level)
{

    SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GC032A_contrast_tab[level];

    SCI_ASSERT(PNULL != sensor_reg_ptr);

    GC032A_Write_Group_Regs(sensor_reg_ptr);

    SCI_TRACE_LOW("set_GC032A_contrast: level = %d", level);

    return 0;
}


__align(4) const SENSOR_REG_T GC032A_image_effect_tab[][4]=
{
		// effect normal
			{
				{0x43,0x10},
				{0xda,0x00},
				{0xdb,0x00},
				{0xff,0xff}
			},
			//effect BLACKWHITE
			{
				{0x43,0x12},
				{0xda,0x00},
				{0xdb,0x00},
				{0xff,0xff}
			},
			// effect RED pink
			{
				{0x43,0x12},
				{0xda,0x10},
				{0xdb,0x50},
				{0xff,0xff},
			},
			// effect GREEN
			{
				{0x43,0x12},
				{0xda,0xc0},
				{0xdb,0xc0},
				{0xff,0xff},
			},
			// effect  BLUE
			{
				{0x43,0x12},
				{0xda,0x50},
				{0xdb,0xe0},
				{0xff,0xff}
			},
			// effect  YELLOW
			{
				{0x43,0x12},
				{0xda,0x80},
				{0xdb,0x20},
				{0xff,0xff}
			},
			// effect NEGATIVE
			{
				{0x43,0x11},
				{0xda,0x00},
				{0xdb,0x00},
				{0xff,0xff}
			},
			//effect ANTIQUE
			{
				{0x43,0x12},
				{0xda,0xd2},
				{0xdb,0x28},
				{0xff,0xff}
			},
};

LOCAL uint32 Set_GC032A_Image_Effect(uint32 effect_type)
{

    SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GC032A_image_effect_tab[effect_type];

    SCI_ASSERT(PNULL != sensor_reg_ptr);

    GC032A_Write_Group_Regs(sensor_reg_ptr);

    SCI_TRACE_LOW("set_GC032A_image_effect: effect_type = %d", effect_type);

    return 0;
}



__align(4) const SENSOR_REG_T GC032A_ev_tab[][4]=
{
    {{0xfe, 0x01}, {0x13, 0x40}, {0xfe, 0x00}, {0xff, 0xff}},
    {{0xfe, 0x01}, {0x13, 0x48}, {0xfe, 0x00}, {0xff, 0xff}},
    {{0xfe, 0x01}, {0x13, 0x4b}, {0xfe, 0x00}, {0xff, 0xff}},
    {{0xfe, 0x01}, {0x13, 0x5b}, {0xfe, 0x00}, {0xff, 0xff}},// level zero
    {{0xfe, 0x01}, {0x13, 0x70}, {0xfe, 0x00}, {0xff, 0xff}},
    {{0xfe, 0x01}, {0x13, 0x78}, {0xfe, 0x00}, {0xff, 0xff}},
    {{0xfe, 0x01}, {0x13, 0x80}, {0xfe, 0x00}, {0xff, 0xff}},
};
__align (4) const SENSOR_REG_T GC032A_ev_nigtmode_tab[][4] =
 {
	{{0xfe, 0x01}, {0x13, 0x50}, {0xfe, 0x00}, {0xff, 0xff}},
	{{0xfe, 0x01}, {0x13, 0x60}, {0xfe, 0x00}, {0xff, 0xff}},
	{{0xfe, 0x01}, {0x13, 0x70}, {0xfe, 0x00}, {0xff, 0xff}},
	{{0xfe, 0x01}, {0x13, 0x80}, {0xfe, 0x00}, {0xff, 0xff}},// level zero
	{{0xfe, 0x01}, {0x13, 0x88}, {0xfe, 0x00}, {0xff, 0xff}},
	{{0xfe, 0x01}, {0x13, 0x90}, {0xfe, 0x00}, {0xff, 0xff}},
	{{0xfe, 0x01}, {0x13, 0x98}, {0xfe, 0x00}, {0xff, 0xff}},
};
LOCAL uint32 Set_GC032A_Ev(uint32 level)
{

	SENSOR_REG_T *sensor_reg_ptr;
	if(NightMode_GC032A)
		 sensor_reg_ptr = (SENSOR_REG_T *) GC032A_ev_nigtmode_tab[level];
	else
		sensor_reg_ptr = (SENSOR_REG_T *) GC032A_ev_tab[level];

    SCI_ASSERT(PNULL != sensor_reg_ptr);
    SCI_ASSERT(level < 7);

    GC032A_Write_Group_Regs(sensor_reg_ptr );

    SCI_TRACE_LOW("set_GC032A_ev: level = %d", level);

    return 0;
}

/******************************************************************************/
// Description: anti 50/60 hz banding flicker
// Global resource dependence:
// Author:
// Note:
//
/******************************************************************************/
LOCAL uint32 Set_GC032A_Anti_Flicker(uint32 mode)
{
	switch(mode)
	{
	case DCAMERA_FLICKER_50HZ:

		bl_GC_50Hz_GC032A = TRUE;

#if defined(GC032A_OUTPUT_MODE_PACKET_DDR_2BIT)

		GC032A_WriteReg(0x05,0x01);
		GC032A_WriteReg(0x06,0xa8); // ad
		GC032A_WriteReg(0x07,0x00);
		GC032A_WriteReg(0x08,0x10);

		GC032A_WriteReg(0xfe,0x01);
		GC032A_WriteReg(0x25,0x00);//step
		GC032A_WriteReg(0x26,0x54);
		
		GC032A_WriteReg(0x27,0x01);
		GC032A_WriteReg(0x28,0xf8);//16.6fps
		GC032A_WriteReg(0x29,0x02);
		GC032A_WriteReg(0x2a,0xa0);//12.5fps
		GC032A_WriteReg(0x2b,0x03);
		GC032A_WriteReg(0x2c,0x48);//10fps
		GC032A_WriteReg(0x2d,0x03);
		GC032A_WriteReg(0x2e,0x48);//8.33fps3f0
		GC032A_WriteReg(0x2f,0x03);
		GC032A_WriteReg(0x30,0x48);//5.88fps594
		GC032A_WriteReg(0x31,0x03);
		GC032A_WriteReg(0x32,0x48);//4.34fps78c
		GC032A_WriteReg(0x33,0x03);
		GC032A_WriteReg(0x34,0x48);//3.99fps834
		GC032A_WriteReg(0x3c,0x30);
		GC032A_WriteReg(0xfe,0x00);

#elif defined(GC032A_OUTPUT_MODE_CCIR656_2BIT)

		GC032A_WriteReg(0xfe,0x00);
		GC032A_WriteReg(0x05,0x01);//hb
		GC032A_WriteReg(0x06,0xe4);
		GC032A_WriteReg(0x07,0x00);//vb
		GC032A_WriteReg(0x08,0x08);

		GC032A_WriteReg(0xfe,0x01);
		GC032A_WriteReg(0x25,0x00);//step
		GC032A_WriteReg(0x26,0x24);

		GC032A_WriteReg(0x27,0x01);//7.1fps
		GC032A_WriteReg(0x28,0xb0);
		GC032A_WriteReg(0x29,0x01);//7.1fps
		GC032A_WriteReg(0x2a,0xb0);
		GC032A_WriteReg(0x2b,0x01);//7.1fps
		GC032A_WriteReg(0x2c,0xb0);
		GC032A_WriteReg(0x2d,0x01);//7.1fps
		GC032A_WriteReg(0x2e,0xb0);
		GC032A_WriteReg(0x2f,0x02);//6.24fps
		GC032A_WriteReg(0x30,0x40);
		GC032A_WriteReg(0x31,0x02);//4.99fps
		GC032A_WriteReg(0x32,0xd0);
		GC032A_WriteReg(0x33,0x03);//4.16fps
		GC032A_WriteReg(0x34,0x60);
		GC032A_WriteReg(0x3c,0x30);
		GC032A_WriteReg(0xfe,0x00);
#endif
		
	break;
	case DCAMERA_FLICKER_60HZ:

		bl_GC_50Hz_GC032A = FALSE;
#if defined(GC032A_OUTPUT_MODE_PACKET_DDR_2BIT)

		GC032A_WriteReg(0xfe,0x00);
		GC032A_WriteReg(0x05,0x01);
		GC032A_WriteReg(0x06,0xa8);
		GC032A_WriteReg(0x07,0x00);
		GC032A_WriteReg(0x08,0x0e);
		
		GC032A_WriteReg(0xfe,0x01);
		GC032A_WriteReg(0x25,0x00);
		GC032A_WriteReg(0x26,0x46);
		
		GC032A_WriteReg(0x27,0x01);//16.5fps
		GC032A_WriteReg(0x28,0xea);
		GC032A_WriteReg(0x29,0x02);//13.3fps
		GC032A_WriteReg(0x2a,0x76);
		GC032A_WriteReg(0x2b,0x03);//10.9fps
		GC032A_WriteReg(0x2c,0x02);
		GC032A_WriteReg(0x2d,0x03);//8.56fps3d4
		GC032A_WriteReg(0x2e,0x02);
		GC032A_WriteReg(0x2f,0x03);//7.05fps4a6
		GC032A_WriteReg(0x30,0x02);
		GC032A_WriteReg(0x31,0x03);//5.45fps604
		GC032A_WriteReg(0x32,0x02);
		GC032A_WriteReg(0x33,0x03);//4.61fps71c
		GC032A_WriteReg(0x34,0x02);
		GC032A_WriteReg(0x3c,0x30);
		GC032A_WriteReg(0xfe,0x00);		
#elif defined(GC032A_OUTPUT_MODE_CCIR656_2BIT)
		GC032A_WriteReg(0xfe,0x00);
		GC032A_WriteReg(0x05,0x01);
		GC032A_WriteReg(0x06,0xe2);
		GC032A_WriteReg(0x07,0x00);
		GC032A_WriteReg(0x08,0x0e);
		
		GC032A_WriteReg(0xfe,0x01);
		GC032A_WriteReg(0x25,0x00);
		GC032A_WriteReg(0x26,0x1e);
		
		GC032A_WriteReg(0x27,0x01);//7fps
		GC032A_WriteReg(0x28,0x68);
		GC032A_WriteReg(0x29,0x01);//7fps
		GC032A_WriteReg(0x2a,0x68);
		GC032A_WriteReg(0x2b,0x01);//7fps
		GC032A_WriteReg(0x2c,0x68);
		GC032A_WriteReg(0x2d,0x01);//7fps
		GC032A_WriteReg(0x2e,0x68);
		GC032A_WriteReg(0x2f,0x02);//5fps
		GC032A_WriteReg(0x30,0xd0);
		GC032A_WriteReg(0x31,0x03);//4.45fps
		GC032A_WriteReg(0x32,0x2a);
		GC032A_WriteReg(0x33,0x03);//4fps
		GC032A_WriteReg(0x34,0x84);
		GC032A_WriteReg(0x3c,0x30);
		GC032A_WriteReg(0xfe,0x00);
#endif
	break;

	    default:
	    break;
    }

    SCI_TRACE_LOW("set_GC032A_anti_flicker-mode=%d",mode);
    OS_TickDelay(100);

    return 0;
}
LOCAL uint32 GC032A_set_flash_enable(uint32 enable)
{
#ifdef DC_FLASH_SUPPORT
    GPIO_SetFlashLight(enable);
#endif
    return 0;
}

LOCAL uint32 Set_GC032A_Preview_Mode(uint32 preview_mode)
{
	switch (preview_mode)
	{
		case DCAMERA_ENVIRONMENT_NORMAL:
		GC032A_WriteReg(0xfe, 0x01);
		GC032A_WriteReg(0x3c, 0x40);
	    GC032A_WriteReg(0xfe, 0x00);

	    NightMode_GC032A=TRUE;

	    break;
		case DCAMERA_ENVIRONMENT_NIGHT:
		GC032A_WriteReg(0xfe, 0x01);
		GC032A_WriteReg(0x3c, 0x30);
		GC032A_WriteReg(0xfe, 0x00);

		NightMode_GC032A=FALSE;
		break;
		default:
			break;
	}

	GC032A_WriteReg(0xfe,0x00);
	if(SENSOR_MAIN==Sensor_GetCurId())
	{
		GC032A_WriteReg(0x17,0x57);
	}
	else
	{
		GC032A_WriteReg(0x17,0x55);
	}

#ifdef GC032A_SERIAL_LOAD_FROM_T_FLASH  
	 Load_GC032A_RegTab_From_T_Flash();
#endif	

	SCI_TRACE_LOW("set_GC032A_preview_mode: level = %d", preview_mode);
	return 0;
}


__align(4) const SENSOR_REG_T GC032A_awb_tab[][5]=
{
	//AUTO
	{{0x77, 0x57},{0x78, 0x4d},{0x79, 0x45},{0x42, 0xcf},{0xff, 0xff}},
	//INCANDESCENCE:
	{{0x42, 0xcd},{0x77, 0x48},{0x78, 0x40},{0x79, 0x5c},{0xff, 0xff}},
	//U30
	{{0x42, 0xcd},{0x77, 0x40},{0x78, 0x54},{0x79, 0x70},{0xff, 0xff}},
	//CWF  //
	{{0x42, 0xcd},{0x77, 0x40},{0x78, 0x54},{0x79, 0x70},{0xff, 0xff}},
	//FLUORESCENT:
	{{0x42, 0xcd},{0x77, 0x40},{0x78, 0x42},{0x79, 0x50},{0xff, 0xff}},
	//SUN:
	{{0x42, 0xcd},{0x77, 0x50},{0x78, 0x45},{0x79, 0x40},{0xff, 0xff}},
	//CLOUD:
	{{0x42, 0xcd},{0x77, 0x5a},{0x78, 0x42},{0x79, 0x40},{0xff, 0xff}},
};

LOCAL uint32 Set_GC032A_AWB(uint32 mode)
{

	SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GC032A_awb_tab[mode];

	SCI_ASSERT(mode < 7);
	SCI_ASSERT(PNULL != sensor_reg_ptr);

	GC032A_Write_Group_Regs(sensor_reg_ptr);

	SCI_TRACE_LOW("set_GC032A_awb_mode: mode = %d", mode);

	return 0;
}

LOCAL uint32 GC032A_AE_AWB_Enable(uint32 ae_enable, uint32 awb_enable)
{
	uint16 ae_value = 0 , awb_value=0;

	awb_value = GC032A_ReadReg(0x42);

	if(0x01==ae_enable)
	{
		ae_value |= 0x01;
	}
	else if(0x00==ae_enable)
	{
		ae_value &=0xfe;
	}

	if(0x01==awb_enable)
	{
		awb_value |=0x02;
	}
	else if(0x00==awb_enable)
	{
		awb_value &= 0xfd;
	}

	GC032A_WriteReg(0xa4, ae_value);
	GC032A_WriteReg(0x42, awb_value);

       SCI_TRACE_LOW("SENSOR: set_ae_awb_enable: ae_enable = %d  awb_enable = %d", ae_enable ,awb_enable);
	return 0;
}


LOCAL uint32 GC032A_Before_Snapshot(uint32 para)
{
	uint8 reg_val = 0;
	uint16 exposal_time=0x00;

	GC032A_AE_AWB_Enable(0x00,0x00);   // close aec awb

	reg_val = GC032A_ReadReg(0x04);
	exposal_time=reg_val&0x00ff;
	reg_val = GC032A_ReadReg(0x03);
	exposal_time|=((reg_val&0x00ff)<<0x08);

	if(exposal_time<1)
	{
		exposal_time=1;
	}

	reg_val=exposal_time&0x00ff;
	GC032A_WriteReg(0x04, reg_val);
	reg_val=(exposal_time&0xff00)>>8;
	GC032A_WriteReg(0x03, reg_val);

	GC032A_Delayms(400);

	SCI_TRACE_LOW("SENSOR_GC032A: Before Snapshot");

	return 0;
}



LOCAL uint32 GC032A_After_Snapshot(uint32 para)
{
	GC032A_AE_AWB_Enable(0x01,0x01);   // Open aec awb

	GC032A_Delayms(400);

	SCI_TRACE_LOW("SENSOR_GC032A: After Snapshot");

	return 0;
}



/******************************************************************************/
// Description: set video mode
// Global resource dependence:
// Author:
// Note:
//
/******************************************************************************/
LOCAL uint32 Set_GC032A_Video_Mode(uint32 mode)
{

	SCI_ASSERT(mode <=DCAMERA_MODE_MAX);

	if(1==mode ||2==mode )
	{
		if(bl_GC_50Hz_GC032A == TRUE)  // is 50hz in video
		{

		}
		else  // is 60hz in video mode
		{

		}
	SCI_TRACE_LOW("set_GC032A_video_mode=%d",mode);
	}

	return 0;
}

#ifdef GC032A_SERIAL_LOAD_FROM_T_FLASH  
__align(4) static SENSOR_REG_T GC032A_YUV_Init_Reg[1000] = {{0x00,0x00},}; 
//#define READ_BUFFER_SIZE  (274*12)    //must be divisible by 12
LOCAL uint32 Load_GC032A_RegTab_From_T_Flash(void)
{
    SFS_HANDLE    file_handle = 0;
    FFS_ERROR_E   ffs_err = FFS_NO_ERROR;
  //  char *file_name = "C:\\GC032A_SERIAL_Initialize_Setting.Bin";
   	static wchar unicode_file_name[256] = {0};
    int regNum = 0; //uint32 regNo = 0;
    int i = 0;  //uint32 i = 0;
    uint8 *curr_ptr = NULL;
    uint8 *buffer_ptr = NULL;  //char buffer_ptr[READ_BUFFER_SIZE] = {0};
    uint32 read_size = 0;
	uint32 file_size = 0;
    uint8 func_ind[3] = {0};  /* REG or DLY */
   // for(i=0;i<25;i++)
       // unicode_file_name[i] = file_name[i];

	SCI_MEM16CPY(unicode_file_name,L"E:\\GC032A_SERIAL_Initialize_Setting.Bin",sizeof(L"E:\\GC032A_SERIAL_Initialize_Setting.Bin"));
   

	/* Search the setting file in all of the user disk. */
	#if 0
	curr_ptr = (uint8 *)unicode_file_name;
	while (file_handle == 0)
    {
        if ((*curr_ptr >= 'c' && *curr_ptr <= 'z') || (*curr_ptr >= 'C' && *curr_ptr <= 'Z'))
        {
			file_handle = SFS_CreateFile((const uint16 *)unicode_file_name, 0x0030|SFS_MODE_READ, NULL, NULL);	//	FFS_MODE_OPEN_EXISTING
            if (file_handle > 0)
            {
                break; /* Find the setting file. */
            }
            *curr_ptr = *curr_ptr + 1;
        }
        else
        {
            break ;
        }
    }
	#else
	file_handle = SFS_CreateFile((const uint16 *)unicode_file_name, 0x0030|SFS_MODE_READ, NULL, NULL);	//	FFS_MODE_OPEN_EXISTING
	#endif
	
	if(file_handle == 0) //read file error
    {
		SCI_TRACE_LOW("!!! Warning, Can't find the initial setting file!!!");
		return SCI_ERROR;
    }


	SFS_GetFileSize(file_handle,&file_size);
	if(file_size < 10)
	{
		SCI_TRACE_LOW("!!! Warning, Invalid setting file!!!");
		return SCI_ERROR;
	}
	
	buffer_ptr =SCI_ALLOCA(file_size);///
	
	if (buffer_ptr == NULL)
	{
    	SCI_TRACE_LOW("!!! Warning, Memory not enoughed...");
    	return SCI_ERROR;        /* Memory not enough */
	}
    ffs_err = SFS_ReadFile(file_handle, buffer_ptr, file_size, &read_size, NULL);
	curr_ptr = buffer_ptr;
    if(SFS_NO_ERROR == ffs_err)
    {
		while(curr_ptr < (buffer_ptr + read_size))
        {     
        	while ((*curr_ptr == ' ') || (*curr_ptr == '\t')) /* Skip the Space & TAB */
        		curr_ptr++;
			if (((*curr_ptr) == '/') && ((*(curr_ptr + 1)) == '*'))
   			{
   	    		 while (!(((*curr_ptr) == '*') && ((*(curr_ptr + 1)) == '/')))
       		 	{
            		curr_ptr++;    /* Skip block comment code. */
       		 	}
        		while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
        		{
            		curr_ptr++;
        		}
        		curr_ptr += 2;            /* Skip the enter line */
        		continue ;
    		}
			if (((*curr_ptr) == '/') && ((*(curr_ptr+1)) == '/'))   /* Skip // block comment code. */
    		{
        		while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
        		{
            		curr_ptr++;
        		}
        		curr_ptr += 2;            /* Skip the enter line */
        		continue ;
   			}

			/* This just content one enter line. */
    		if (((*curr_ptr) == 0x0D) && ((*(curr_ptr + 1)) == 0x0A))
    		{
        		curr_ptr += 2;
        		continue ;
    		}
			SCI_MEMCPY(func_ind, curr_ptr,3);
			curr_ptr += 4;  
			if (strcmp((const char *)func_ind, "REG") == 0)    /* REG */
			{
            GC032A_YUV_Init_Reg[regNum].reg_addr = (uint16)strtol(curr_ptr, NULL, 16);
            curr_ptr += 5;
            GC032A_YUV_Init_Reg[regNum].reg_value = (uint16)strtol(curr_ptr, NULL, 16);
            //strNum += 5;
			
			}
			regNum++;	
			/* Skip to next line directly. */
    		while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
    		{
        		curr_ptr++;
    		}		
            curr_ptr += 2;    
        }
    }
	GC032A_YUV_Init_Reg[regNum].reg_addr = 0xff;
	GC032A_YUV_Init_Reg[regNum].reg_value =0xff;
	
	SCI_TRACE_LOW("%d register read...", i);
 
	SCI_FREE(buffer_ptr);
 	buffer_ptr = NULL;	
    ffs_err = SFS_CloseFile( file_handle);
	file_handle = NULL;

	SCI_TRACE_LOW("Start apply initialize setting.");
    /* Start apply the initial setting to sensor. */
	SCI_ASSERT(PNULL != GC032A_YUV_Init_Reg);
    GC032A_Write_Group_Regs(GC032A_YUV_Init_Reg);

	return SCI_SUCCESS;
}
#endif

#endif

