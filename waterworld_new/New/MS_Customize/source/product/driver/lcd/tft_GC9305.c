/******************************************************************************
 ** File Name:     cstn_pcf8833.c                                             *
 ** Description:															  *
 **    This file contains driver for color LCD.(CSTN)						  *
 ** Author:         Jim zhang                                                 *
 ** DATE:           03/01/2004                                                *
 ** Copyright:      2004 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 02/04/2004     louis.wei	    Create.
 ******************************************************************************/
#include "ms_customize_trc.h"
#include "os_api.h"
#include "dal_lcd.h"
#include "lcd_cfg.h"
#include "lcm_drvapi.h"
#include "spi_drvapi.h"

#ifndef LCD_DATA_WIDTH_8BIT
#define LCD_DATA_WIDTH_8BIT
#endif

#ifdef TRACE_INFO_SUPPORT
uint GC9305_mainlcd_id = 0x0;
#endif

/**---------------------------------------------------------------------------*
 **                            Macro Define
 **---------------------------------------------------------------------------*/

#ifdef   __cplusplus
    extern   "C" 
    {
#endif


#define LCD_CtrlWrite_GC9305( _cmd )\
				LCD_SendCmd( (_cmd ), 0 );

#define LCD_DataWrite_GC9305( _data )\
				LCD_SendData( (_data), 0 );

#define LCD_CtrlData_GC9305( _cmd, _data ) \
				LCD_SendCmdData( (_cmd ),  (_data),0 );  
			

LOCAL LCD_DIRECT_E s_lcd_direct = LCD_DIRECT_NORMAL;	



//uint test_val =0xB2;
#if 0
/******************************************************************************/
//  Description:   Clear color LCD with one color
//	Global resource dependence: 
//  Author:         louis.wei
//	Note:
/******************************************************************************/
LOCAL void GC9305_Clear(
						uint32 color	//color to fill the whole lcd.
						);
#endif						
  /******************************************************************************/
//  Description:   Close the lcd.(include sub lcd.)
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
LOCAL void GC9305_Close(void);

  /******************************************************************************/
//  Description:   Enter/Exit sleep mode .
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
LOCAL ERR_LCD_E  GC9305_EnterSleep(
	BOOLEAN is_sleep 	//SCI_TRUE: exter sleep mode;SCI_FALSE:exit sleep mode.
	);



/*********************************************************************/
//  Description:   Initialize color LCD : GC9305
//  Input:
//      None.
//  Return:
//      None.
//	Note:           
/*********************************************************************/
LOCAL ERR_LCD_E GC9305_Init(void);

/******************************************************************************/
//  Description:   invalidate a rectang of in LCD
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
LOCAL ERR_LCD_E GC9305_Invalidate(void);


/******************************************************************************/
//  Description:   Copy a retangle data from clcd_buffer to display RAM.
//                     then the rectangle display is to be refreshed
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:       
//     To improve speed, lcd is operate in HIGH SPEED RAM WRITE MODE(4
//     uint16 are write continuously always.) So, some dummy uint16 
//     should be inserted to satisfy this mode.   Please refer to spec.
/******************************************************************************/
LOCAL ERR_LCD_E GC9305_InvalidateRect(
	uint16 left, 	//the left value of the rectangel
	uint16 top, 	//top of the rectangle
	uint16 right, 	//right of the rectangle
	uint16 bottom	//bottom of the rectangle
	);
/******************************************************************************/
//  Description:   Set the windows address to display, in this windows
//                 color is  refreshed.
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
PUBLIC void GC9305_set_display_window(
	uint16 left, 	// start Horizon address
	uint16 right, 	// end Horizon address
	uint16 top, 		// start Vertical address
	uint16 bottom	// end Vertical address
	);


/**---------------------------------------------------------------------------*
 **                      Function  Definitions
 **---------------------------------------------------------------------------*/



/******************************************************************************/
//  Description:  Set LCD invalidate direction
//  Input:
//      is_invert: 0, horizontal; 1, vertical
//  Return:
//      None.
//	Note: Application should correct invalidate direction right after current
//	 	  image displayed
/******************************************************************************/
LOCAL ERR_LCD_E GC9305_SetDirection( LCD_DIRECT_E direct_type)
{
	switch(direct_type)
	{
		case LCD_DIRECT_NORMAL:			
			LCD_CtrlWrite_GC9305(0x36); // Memory Access Control
			LCD_DataWrite_GC9305 (0x48);	
			break;
		/*	
		case LCD_DIRECT_ROT_90:
			LCD_CtrlWrite_GC9305(0x36); // Memory Access Control
			LCD_DataWrite_GC9305 (0xc8);	
			break;
			
		case LCD_DIRECT_ROT_180:
			LCD_CtrlWrite_GC9305(0x36); // Memory Access Control
			LCD_DataWrite_GC9305 (0x88);	
			break;
			
		case LCD_DIRECT_ROT_270:
			LCD_CtrlWrite_GC9305(0x36); // Memory Access Control
			LCD_DataWrite_GC9305 (0x08);	
			break;

		case LCD_DIRECT_MIR_H:
			LCD_CtrlWrite_GC9305(0x36); // Memory Access Control
			LCD_DataWrite_GC9305 (0x58);	
			break;
			
		case LCD_DIRECT_MIR_V:
			LCD_CtrlWrite_GC9305(0x36); // Memory Access Control
			LCD_DataWrite_GC9305 (0x68);	
			break;
			
		case LCD_DIRECT_MIR_HV:
			LCD_CtrlWrite_GC9305(0x36); // Memory Access Control
			LCD_DataWrite_GC9305 (0x78);	
			break;*/
			
		default:			
#if defined CUSTOMER_CONFIG_P1918_TF2431
			LCD_CtrlWrite_GC9305(0x36); // Memory Access Control
			LCD_DataWrite_GC9305 (0x48);	
#else
			LCD_CtrlWrite_GC9305(0x36); // Memory Access Control
			LCD_DataWrite_GC9305 (0x08);	
#endif
		   //direct_type = LCD_DIRECT_NORMAL;
			break;
			
	}	
	s_lcd_direct = direct_type;	
	return ERR_LCD_NONE;
	
}
/******************************************************************************/
//  Description:   Set the windows address to display, in this windows
//                 color is  refreshed.
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
__inline void GC9305_set_display_window(
	uint16 left, 	// start Horizon address
	uint16 top, 	// end Horizon address
	uint16 right, 		// start Vertical address
	uint16 bottom	// end Vertical address
	)
{   
	//uint16 cmd;	
SCI_TRACE_LOW("GC9305_set_display_window");
	LCD_CtrlWrite_GC9305(0x2a);
	LCD_DataWrite_GC9305(left>>8); 
	LCD_DataWrite_GC9305(left&0xFF);
	LCD_DataWrite_GC9305(right>>8); 
	LCD_DataWrite_GC9305(right&0xFF);

	LCD_CtrlWrite_GC9305(0x2b);
	LCD_DataWrite_GC9305(top>>8); 
	LCD_DataWrite_GC9305(top&0xFF);
	LCD_DataWrite_GC9305(bottom>>8); 
	LCD_DataWrite_GC9305(bottom&0xFF);
  
		
	LCD_CtrlWrite_GC9305(0x2C);	

}


int i=0;
int j=0;

LOCAL void GC9305_driver(void)
{
#define LCD_DEFINE
	LCD_Reset();//reset lcd
	//************* Start Initial Sequence **********//GC9305_ivo2.8_chuangpu_SHX
	LCD_Delayms(120);

	//----------------------------------------end Reset Sequence---------------------------------------//
	//--------------------------------display control setting----------------------------------------//

	LCD_CtrlWrite_GC9305(0xfe);
	LCD_CtrlWrite_GC9305(0xef);
	LCD_CtrlWrite_GC9305(0x36);
	LCD_DataWrite_GC9305(0x48);
	LCD_CtrlWrite_GC9305(0x3a);
	LCD_DataWrite_GC9305(0x05);

	LCD_CtrlWrite_GC9305(0x35);
	LCD_DataWrite_GC9305(0x00);
	LCD_CtrlWrite_GC9305(0x44);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x60);
	
	//------end display control setting----//
	//------Power Control Registers Initial----//
	LCD_CtrlWrite_GC9305(0xa4);
	LCD_DataWrite_GC9305(0x44);
	LCD_DataWrite_GC9305(0x44);
	LCD_CtrlWrite_GC9305(0xa5);
	LCD_DataWrite_GC9305(0x42);
	LCD_DataWrite_GC9305(0x42);
	LCD_CtrlWrite_GC9305(0xaa);
	LCD_DataWrite_GC9305(0x88);
	LCD_DataWrite_GC9305(0x88);
	LCD_CtrlWrite_GC9305(0xe8);
	LCD_DataWrite_GC9305(0x11);
	LCD_DataWrite_GC9305(0x71);
	LCD_CtrlWrite_GC9305(0xe3);
	LCD_DataWrite_GC9305(0x01);
	LCD_DataWrite_GC9305(0x10);
	LCD_CtrlWrite_GC9305(0xff);
	LCD_DataWrite_GC9305(0x61);
	LCD_CtrlWrite_GC9305(0xAC);
	LCD_DataWrite_GC9305(0x00);

	LCD_CtrlWrite_GC9305(0xAe);
	LCD_DataWrite_GC9305(0x2b);//20161020

	LCD_CtrlWrite_GC9305(0xAd);
	LCD_DataWrite_GC9305(0x33);
	LCD_CtrlWrite_GC9305(0xAf);
	LCD_DataWrite_GC9305(0x55);
	LCD_CtrlWrite_GC9305(0xa6);
	LCD_DataWrite_GC9305(0x2a);
	LCD_DataWrite_GC9305(0x2a);
	LCD_CtrlWrite_GC9305(0xa7);
	LCD_DataWrite_GC9305(0x2b);
	LCD_DataWrite_GC9305(0x2b);
	LCD_CtrlWrite_GC9305(0xa8);
	LCD_DataWrite_GC9305(0x18);
	LCD_DataWrite_GC9305(0x18);
	LCD_CtrlWrite_GC9305(0xa9);
	LCD_DataWrite_GC9305(0x2a);
	LCD_DataWrite_GC9305(0x2a);
	//-----display window 240X320---------//
	LCD_CtrlWrite_GC9305(0x2a);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0xef);
	LCD_CtrlWrite_GC9305(0x2b);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x01);
	LCD_DataWrite_GC9305(0x3f);
	LCD_CtrlWrite_GC9305(0x2c);
	//--------end display window --------------//
	//------------gamma setting------------------//
	LCD_CtrlWrite_GC9305(0xf0);
	LCD_DataWrite_GC9305(0x02);
	LCD_DataWrite_GC9305(0x01);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x02);
	LCD_DataWrite_GC9305(0x09);
	
	LCD_CtrlWrite_GC9305(0xf1);
	LCD_DataWrite_GC9305(0x01);
	LCD_DataWrite_GC9305(0x02);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x11);
	LCD_DataWrite_GC9305(0x1c);
	LCD_DataWrite_GC9305(0x15);
	
	LCD_CtrlWrite_GC9305(0xf2);
	LCD_DataWrite_GC9305(0x0a);
	LCD_DataWrite_GC9305(0x07);
	LCD_DataWrite_GC9305(0x29);
	LCD_DataWrite_GC9305(0x04);
	LCD_DataWrite_GC9305(0x04);
	LCD_DataWrite_GC9305(0x38);//v43n  39
	
	LCD_CtrlWrite_GC9305(0xf3);
	LCD_DataWrite_GC9305(0x15);
	LCD_DataWrite_GC9305(0x0d);
	LCD_DataWrite_GC9305(0x55);
	LCD_DataWrite_GC9305(0x04);
	LCD_DataWrite_GC9305(0x03);
	LCD_DataWrite_GC9305(0x65);//v43p 66
	
	LCD_CtrlWrite_GC9305(0xf4);
	LCD_DataWrite_GC9305(0x0f);//v50n
	LCD_DataWrite_GC9305(0x1d);//v57n
	LCD_DataWrite_GC9305(0x1e);//v59n
	LCD_DataWrite_GC9305(0x0a);//v61n 0b
	LCD_DataWrite_GC9305(0x0d);//v62n 0d
	LCD_DataWrite_GC9305(0x0f);
	
	LCD_CtrlWrite_GC9305(0xf5);
	LCD_DataWrite_GC9305(0x05);//v50p
	LCD_DataWrite_GC9305(0x12);//v57p
	LCD_DataWrite_GC9305(0x11);//v59p
	LCD_DataWrite_GC9305(0x34);//v61p 35
	LCD_DataWrite_GC9305(0x34);//v62p 34
	LCD_DataWrite_GC9305(0x0f);
	//-------end gamma setting----//
	LCD_CtrlWrite_GC9305(0x11);
	LCD_Delayms(120); 
	LCD_CtrlWrite_GC9305(0x29);
	LCD_CtrlWrite_GC9305(0x2c);

	for(i=0;i<240;i++){
		for(j=0;j<320;j++){
		LCD_DataWrite_GC9305(0x00);
		LCD_DataWrite_GC9305(0x00);

		}
	}
	LCD_Delayms(50); 
#ifndef LCD_DEFINE
	#error "lcd must be defined"
#endif
}


/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
// Global resource dependence:
// Author: Jianping.wang
// Note:
/**************************************************************************************/
LOCAL ERR_LCD_E GC9305_Init(void)
{
	SCI_TRACE_LOW("GC9305_Init");	
 
	GC9305_driver();	
	
	return 0;
}

/******************************************************************************/
//  Description:   Enter/Exit sleep mode .
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
LOCAL ERR_LCD_E  GC9305_EnterSleep(
	BOOLEAN is_sleep 	//SCI_TRUE: enter sleep mode;SCI_FALSE:exit sleep mode.
	)
{
	SCI_TRACE_LOW("qinss LCD: in GC9305_EnterSleep, is_sleep = %d", is_sleep);
	if ( is_sleep ) // enter sleep mode.
	{
	
		LCD_CtrlWrite_GC9305(0x28);
		LCD_Delayms(120);
		LCD_CtrlWrite_GC9305(0x10);
		LCD_Delayms(120);
	}
	else 			// out sleep mode 
	{
		#ifdef TRACE_INFO_SUPPORT
		SCI_TRACE_LOW("qinss LCD: GC9305_mainlcd_id = %d", GC9305_mainlcd_id);
		#endif
	 	//GC9305_Init();

		LCD_CtrlWrite_GC9305(0x11);
		LCD_Delayms(120);
		LCD_CtrlWrite_GC9305(0x29);
		LCD_Delayms(120);
	}

	
	
	return ERR_LCD_NONE;

}
/******************************************************************************/
//  Description:   Close the lcd.(include sub lcd.)
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
LOCAL void GC9305_Close(void)
{
	SCI_TRACE_LOW("qinss LCD: in GC9305_Close");

	GC9305_EnterSleep( SCI_TRUE );	

}

/******************************************************************************/
//  Description:   invalidate a rectang of in LCD
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
LOCAL ERR_LCD_E GC9305_Invalidate(void)
{
	GC9305_set_display_window (0x0, 0X0, QVGA_LCD_WIDTH - 1, QVGA_LCD_HEIGHT -1);
	return ERR_LCD_NONE;
}
/******************************************************************************/
//  Description:   Copy a retangle data from clcd_buffer to display RAM.
//                     then the rectangle display is to be refreshed
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:       
//     To improve speed, lcd is operate in HIGH SPEED RAM WRITE MODE(4
//     uint16 are write continuously always.) So, some dummy uint16 
//     should be inserted to satisfy this mode.   Please refer to spec.
/******************************************************************************/
PUBLIC ERR_LCD_E GC9305_InvalidateRect(
	uint16 left, 	//the left value of the rectangel
	uint16 top, 	//top of the rectangle
	uint16 right, 	//right of the rectangle
	uint16 bottom	//bottom of the rectangle
	)
{
    {
        left 	= (left >= QVGA_LCD_WIDTH)    ? QVGA_LCD_WIDTH-1 : left;
        right 	= (right >= QVGA_LCD_WIDTH)   ? QVGA_LCD_WIDTH-1 : right;
        top 	= (top >= QVGA_LCD_HEIGHT)    ? QVGA_LCD_HEIGHT-1 : top;
        bottom 	= (bottom >= QVGA_LCD_HEIGHT) ? QVGA_LCD_HEIGHT-1 : bottom;
            
        if ( ( right < left ) || ( bottom < top ) )
        {
           	return ERR_LCD_OPERATE_FAIL;
        }   
	 
       GC9305_set_display_window(left, top, right, bottom); 
    }
  
    return ERR_LCD_NONE;
	
}

/**************************************************************************************/
// Description: refresh a rectangle of lcd
// Global resource dependence:
// Author: Jianping.wang
// Note:
//		left - the left value of the rectangel
//   	top - the top value of the rectangel
// 		right - the right value of the rectangel
//		bottom - the bottom value of the rectangel
/**************************************************************************************/
LOCAL ERR_LCD_E GC9305_RotationInvalidateRect(uint16 left,uint16 top,uint16 right,uint16 bottom,LCD_ANGLE_E angle)
{
	//int32 error;
	

	switch(angle)
	{
		case LCD_ANGLE_0:
			GC9305_set_display_window(left, top, right, bottom);
			break;
		case LCD_ANGLE_90:
			GC9305_set_display_window(left, top, bottom,right);
			break;
		case LCD_ANGLE_180:
			GC9305_set_display_window(left, top, right, bottom);
			break;
		case LCD_ANGLE_270:
			GC9305_set_display_window(left, top, bottom,right);
			break;
		default:
			GC9305_set_display_window(left, top, right, bottom);
			break;			
	}
		

	return ERR_LCD_NONE;
}//en of GC9305_VerticalInvalidateRect	


/******************************************************************************/
//  Description:  set the contrast value 
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
LOCAL ERR_LCD_E   GC9305_SetContrast(
	uint16  contrast	//contrast value to set
	)
{
	return ERR_LCD_FUNC_NOT_SUPPORT;
} 


/*****************************************************************************/
//  Description:    Set the brightness of LCD.
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/*****************************************************************************/
LOCAL ERR_LCD_E   GC9305_SetBrightness(
	uint16 brightness	//birghtness to set
	)
{
	return ERR_LCD_FUNC_NOT_SUPPORT;
}

/*****************************************************************************/
//  Description:    Enable lcd to partial display mode, so can save power.
//	Global resource dependence: 
//  Author:         Jim.zhang
//  Return:         SCI_TRUE:SUCCESS ,SCI_FALSE:failed.
//	Note:           If all input parameters are 0, exit partial display mode.
/*****************************************************************************/
LOCAL ERR_LCD_E GC9305_SetDisplayWindow(
	uint16 left, 		//left of the window
	uint16 top,			//top of the window
	uint16 right,		//right of the window
	uint16 bottom		//bottom of the window
	)
{
	GC9305_set_display_window(left, top, right, bottom);

	return ERR_LCD_NONE;
}

#ifdef HX_DRV_LCD_ESD_RECOVER
static uint32 s_lcd_red_value  = 0x0;
LOCAL uint32 LCD_ESD_RECOVER_Init(void)
{

	volatile uint8 readid1 = 0, readid2 = 0, readid3 = 0, readid4 = 0;
	uint16 lcd_id=0;
	LCD_SendCmd(0x09, lcd_id);	  
	readid1 = LCM_Read(lcd_id,  DATA_LEVEL); 
	readid1 = LCM_Read(lcd_id,  DATA_LEVEL)&0xfe; 
	readid2 = LCM_Read(lcd_id,  DATA_LEVEL)&0x7f; 
	readid3 = LCM_Read(lcd_id,  DATA_LEVEL)&0x07; 
	readid4 = LCM_Read(lcd_id,  DATA_LEVEL)&0xe0; 

	s_lcd_red_value = ((readid1<<24)|(readid2<<16)|(readid3<<8)|(readid4));
   
  
}
LOCAL uint32 lcd_ESD_Init_check(void)
{

	 volatile uint8 readid1 = 0, readid2 = 0, readid3 = 0, readid4 = 0;
	 uint16 lcd_id=0;
	 LCD_SendCmd(0x09, lcd_id);    
	 readid1 = LCM_Read(lcd_id,  DATA_LEVEL); 
	 readid1 = LCM_Read(lcd_id,  DATA_LEVEL)&0xfe; //fe
	 readid2 = LCM_Read(lcd_id,  DATA_LEVEL)&0x7f; //7f
	 readid3 = LCM_Read(lcd_id,  DATA_LEVEL)&0x07; //07
	 readid4 = LCM_Read(lcd_id,  DATA_LEVEL)&0xe0; //e0


     if(s_lcd_red_value ==((readid1<<24)|(readid2<<16)|(readid3<<8)|(readid4)))
     	{
	LCD_CtrlWrite_GC9305(0xFE);
    LCD_CtrlWrite_GC9305(0xEF);

	LCD_CtrlWrite_GC9305(0x36);
	LCD_DataWrite_GC9305(0x48);

	LCD_CtrlWrite_GC9305(0x3a);
	LCD_DataWrite_GC9305(0x05);

	LCD_CtrlWrite_GC9305(0x20);

	LCD_CtrlWrite_GC9305(0xE0);
	LCD_DataWrite_GC9305(0x0F);
	
	LCD_CtrlWrite_GC9305(0xE1);
	LCD_DataWrite_GC9305(0x10);
	LCD_DataWrite_GC9305(0x26);
	
	LCD_CtrlWrite_GC9305(0xDE);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xE3);
	LCD_DataWrite_GC9305(0x01);
	LCD_DataWrite_GC9305(0x10);

	LCD_CtrlWrite_GC9305(0xFF);
	LCD_DataWrite_GC9305(0x61);
	
	LCD_CtrlWrite_GC9305(0xE4);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xE5);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xE6);
	LCD_DataWrite_GC9305(0x00);
	
	
	LCD_CtrlWrite_GC9305(0xE7);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xDF);
	LCD_DataWrite_GC9305(0x08);
	
	LCD_CtrlWrite_GC9305(0xE8);
	LCD_DataWrite_GC9305(0x11);
	LCD_DataWrite_GC9305(0x71);
	
	LCD_CtrlWrite_GC9305(0xEA);
	LCD_DataWrite_GC9305(0x51);
	LCD_DataWrite_GC9305(0x95);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xEB);
	LCD_DataWrite_GC9305(0x30);

	
	LCD_CtrlWrite_GC9305(0xEE);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xEC);
	LCD_DataWrite_GC9305(0x33);
	LCD_DataWrite_GC9305(0x22);
	LCD_DataWrite_GC9305(0x88);
	
	LCD_CtrlWrite_GC9305(0xED);
	LCD_DataWrite_GC9305(0x18);
	LCD_DataWrite_GC9305(0x08);
	
	LCD_CtrlWrite_GC9305(0xA0);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xA1);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xA2);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xA3);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xA4);
	LCD_DataWrite_GC9305(0x44);
	LCD_DataWrite_GC9305(0x44);

	LCD_CtrlWrite_GC9305(0xA6);
	LCD_DataWrite_GC9305(0x2a);
	LCD_DataWrite_GC9305(0x2a);
	LCD_CtrlWrite_GC9305(0xA7);
	LCD_DataWrite_GC9305(0x2b);
	LCD_DataWrite_GC9305(0x2b);
	LCD_CtrlWrite_GC9305(0xA8);
	LCD_DataWrite_GC9305(0x18);
	LCD_DataWrite_GC9305(0x18);
	LCD_CtrlWrite_GC9305(0xA9);
	LCD_DataWrite_GC9305(0x2a);
	LCD_DataWrite_GC9305(0x2a);

	LCD_CtrlWrite_GC9305(0xAA);
	LCD_DataWrite_GC9305(0x88);
	LCD_DataWrite_GC9305(0x88);
	
	LCD_CtrlWrite_GC9305(0xAB);
	LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xAC);
	LCD_DataWrite_GC9305(0x00);
	
	LCD_CtrlWrite_GC9305(0xAD);
	LCD_DataWrite_GC9305(0x33);
	
	LCD_CtrlWrite_GC9305(0xAE);
	LCD_DataWrite_GC9305(0x2b);//4b
	
	LCD_CtrlWrite_GC9305(0xAF);
	LCD_DataWrite_GC9305(0x55);
	
	LCD_CtrlWrite_GC9305(0xB6);
    LCD_DataWrite_GC9305(0x00);
	LCD_DataWrite_GC9305(0x80);
	LCD_DataWrite_GC9305(0x27);

		      return 0;
     	}
	 else
	 	{
	 	    GC9305_Init();
	 	    return 1;
	 	}
}
#endif
/******************************************************************************/
// Description: Invalidate Pixel
// Global resource dependence: 
// Author: 
// Note:
/******************************************************************************/
LOCAL void GC9305_InvalidatePixel(uint16 x, uint16 y, uint32 data)
{
	GC9305_InvalidateRect(x,y,x,y);
	LCD_DataWrite_GC9305(data);
}

LOCAL uint32 GC9305_ReadID(uint16 lcd_cs, uint16 lcd_cd, uint16 lcd_id)
{
	uint16 lcm_dev_id = 0;	
	uint32 lcm_id0=0,lcm_id1=0,dummy=0;
	
	LCD_CtrlWrite_GC9305(0x04);
	dummy = LCM_Read(0,  DATA_LEVEL);  //param0:dummy
	
	dummy = LCM_Read(0,  DATA_LEVEL);  //param1
	
	lcm_id0 = LCM_Read(0,  DATA_LEVEL);	
	
	lcm_id1 = LCM_Read(0,  DATA_LEVEL);

	lcm_dev_id=(lcm_id0<<8)|(lcm_id1);
	
	return lcm_dev_id;
}


/******************************************************************************/
//  Description:   Close the lcd.(include sub lcd.)
//	Global resource dependence: 
//  Author:         Jim.zhang
//	Note:
/******************************************************************************/
const LCD_OPERATIONS_T GC9305_operations = 
{
	GC9305_Init,
	GC9305_EnterSleep,
	GC9305_SetContrast,
	GC9305_SetBrightness,
	GC9305_SetDisplayWindow,
	GC9305_InvalidateRect,
	GC9305_Invalidate,
	GC9305_Close,
	GC9305_RotationInvalidateRect,
	GC9305_SetDirection,
	NULL,
	GC9305_ReadID,
#if defined(HX_DRV_LCD_ESD_RECOVER)
	LCD_ESD_RECOVER_Init,
	lcd_ESD_Init_check,
#endif
};

LOCAL const LCD_TIMING_U s_GC9305_spitiming = {
	#ifdef FPGA_VERIFICATION
	26000000,
	#else
	96000000,
	#endif
	0, 0,
	/*	SPI_CLK_48MHZ,        // clk frequency support (unit:MHz)
		SPI_CLK_IDLE_LOW,     // CPOL: 0--SPI_CLK_IDLE_LOW, 1--SPI_CLK_IDLE_HIGH
		SPI_SAMPLING_RISING,  // CPHA: 0--SPI_SAMPLING_RISING,  1--SPI_SAMPLING_FALLING
	*/
	8,			// tx bit length: 8/16bits refer to lcm driver ic
	0,
	0
};

LOCAL const LCD_TIMING_U s_GC9305_lcmtiming = {
	// LCM_CYCLE_U start(ns)
	30,		// CS setup time for LCM read (optional)  new
	150,	// low pulse width for LCM read (according spec)
	150,	// high pulse width for LCM read (according spec)
	40,		// CS setup time for LCM write  (optional)
	50,		// low pulse width for LCM write (according spec)
	50,		// high pulse width for LCM write (according spec)
	// LCM_CYCLE_U end(ns)
};

#ifdef MAINLCM_INTERFACE_SPI
const LCD_SPEC_T g_lcd_GC9305 = {
	LCD_WIDTH,
	LCD_HEIGHT,
	SCI_NULL,
	SCI_NULL,
	WIDTH_8,
	(LCD_TIMING_U *) &s_GC9305_spitiming,
	(LCD_OPERATIONS_T *) &GC9305_operations,
	1,
	0
};
#else
const LCD_SPEC_T g_lcd_GC9305 = {
	LCD_WIDTH,
	LCD_HEIGHT,
	LCD_MCU,
	BUS_MODE_8080,
	WIDTH_8,
	(LCD_TIMING_U *) &s_GC9305_lcmtiming,
	(LCD_OPERATIONS_T *) &GC9305_operations,
	1,
	0
};
#endif
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif

