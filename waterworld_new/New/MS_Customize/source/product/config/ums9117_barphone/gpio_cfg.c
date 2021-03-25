/******************************************************************************
 ** File Name:      gpio_cfg.c                                                *
 ** DATE:           2011.01.12                                                *
 ** Copyright:      2011 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 **                                                                           *
 **                                                                           *
 ******************************************************************************
 **---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/

#include "os_api.h"
#include "gpio_drv.h"
#include "gpio_prod_api.h"
#include "gpio_prod_cfg.h"
#include "analog_drv.h"
#include "wifi_drv.h"
#if defined(PLATFORM_UMS9117)
#include "eica_drvapi.h"
#endif

#define DEBUG_GPIO_PROD_CFG
#ifdef  DEBUG_GPIO_PROD_CFG

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C"
    {
#endif

#ifdef MODEM_CONTROL_SUPPORT_USB
extern PUBLIC uint32 Gx_UDC_USB_Resume(BOOLEAN param);
#endif
/**---------------------------------------------------------------------------*
 **                         Macro Definition                                  *
 **---------------------------------------------------------------------------*/
#define HEADSET_DETECT_SHAKING_TIME 	200
#define HEADSET_BUTTON_SHAKING_TIME     100
#define SDCARD_DETECT_SHAKING_TIME		300
#define FLIP_ON_SHAKING_TIME			200
#define BACKEND_IC0_INT_SHAKING_TIME	200
#define CAMERA_COVER_SHAKING_TIME		500
#define DOUBLE_KEY_ACTION_SHAKING_TIME  50
#define VBUS_DETECT_SHAKING_TIME 	40

extern PUBLIC SCI_TIMER_PTR g_headset_timer;

LOCAL uint32 _GPIO_OpenLCMBackLight(BOOLEAN is_open);
LOCAL uint32 _GPIO_OpenKeyPadBackLight( BOOLEAN is_open );
LOCAL uint32 _GPIO_OpenVibrator( BOOLEAN is_open );
LOCAL uint32 _GPIO_OpenSDPower( BOOLEAN is_open );
LOCAL uint32 _GPIO_IsUsbOrAdapter( BOOLEAN is_open );
LOCAL uint32 _GPIO_OpenPA(BOOLEAN is_open);
LOCAL uint32 _GPIO_OpenFlash(BOOLEAN is_open);

/**---------------------------------------------------------------------------*
 **                         Local Variables                                   *
 **---------------------------------------------------------------------------*/
// GPIO product configure table for customer setting
LOCAL const GPIO_CFG_INFO_T s_gpio_prod_cfg_cus_table[] =
{
/*	ID									TYPE				VALID_LEVEL				NUM.				CALLBACK	*/
    {GPIO_PROD_LCD_BL_EN_ID,		             GPIO_PROD_TYPE_MAX, GPIO_PROD_HIGH_LEVEL,GPIO_PROD_NUM_INVALID,	_GPIO_OpenLCMBackLight},
    {GPIO_PROD_KEYPAD_BL_ID,		             GPIO_PROD_TYPE_MAX, GPIO_PROD_HIGH_LEVEL,GPIO_PROD_NUM_INVALID,	_GPIO_OpenKeyPadBackLight},
    {GPIO_PROD_VIBRATIOR_EN_ID,		      GPIO_PROD_TYPE_MAX, GPIO_PROD_HIGH_LEVEL,GPIO_PROD_NUM_INVALID,	_GPIO_OpenVibrator},
    {GPIO_PROD_FLASH_EN_ID,		             GPIO_PROD_TYPE_MAX, GPIO_PROD_HIGH_LEVEL,GPIO_PROD_NUM_INVALID,	_GPIO_OpenFlash},
    //{GPIO_PROD_HEADSET_DETECT_ID,		GPIO_PROD_TYPE_EIC_DBNC, GPIO_PROD_LOW_LEVEL,21,PNULL },
    //{GPIO_PROD_HEADSET_BUTTON_ID,		GPIO_PROD_TYPE_EIC_DBNC, GPIO_PROD_HIGH_LEVEL,20,PNULL },
    {GPIO_PROD_HEADSET_DETECT_ID,		GPIO_PROD_TYPE_EICA_DBNC, GPIO_PROD_HIGH_LEVEL,EICA_AUD_HEAD_INSERT_ALL,PNULL},
    {GPIO_PROD_HEADSET_BUTTON_ID,		GPIO_PROD_TYPE_EICA_DBNC, GPIO_PROD_HIGH_LEVEL,EICA_AUD_HEAD_BUTTON,PNULL},
    {GPIO_PROD_SPEAKER_PA_EN_ID,		       GPIO_PROD_TYPE_MAX, GPIO_PROD_HIGH_LEVEL,GPIO_PROD_NUM_INVALID,  _GPIO_OpenPA},
#ifdef MODEL_NAME_DATACARD_SPRDV100
    {GPIO_PROD_SDCARD_DETECT_ID,		GPIO_PROD_TYPE_BB0, GPIO_PROD_LOW_LEVEL,94,PNULL},//yintianci
#elif defined(MODEL_NAME_DATACARD_SPRDV110)
    {GPIO_PROD_SDCARD_DETECT_ID,		GPIO_PROD_TYPE_BB0, GPIO_PROD_LOW_LEVEL,26,PNULL},//yintianci
#elif defined(MODEL_NAME_DATACARD_ZTE01)
    {GPIO_PROD_SDCARD_DETECT_ID,		GPIO_PROD_TYPE_BB0, GPIO_PROD_LOW_LEVEL,86,PNULL},//yintianci
#endif

    {GPIO_PROD_CHARGE_PLUG_DETECT_ID,	GPIO_PROD_TYPE_EICA_DBNC, GPIO_PROD_HIGH_LEVEL, EICA_CHGR_INT, PNULL},
//    {GPIO_PROD_POWER_KEY_ID,			GPIO_PROD_TYPE_EIC_DBNC, GPIO_PROD_HIGH_LEVEL,	19,				PNULL},
    {GPIO_PROD_POWER_KEY_ID,				GPIO_PROD_TYPE_EICA_DBNC, GPIO_PROD_LOW_LEVEL,	EICA_PBINT,				PNULL},
    {GPIO_PROD_MENUCANCEL_KEY_ID,		GPIO_PROD_TYPE_EICA_DBNC, GPIO_PROD_HIGH_LEVEL,	EICA_RSTN,				PNULL},
    {GPIO_PROD_USB_DETECT_ID,		       GPIO_PROD_TYPE_MAX, GPIO_PROD_HIGH_LEVEL,   GPIO_PROD_NUM_INVALID, _GPIO_IsUsbOrAdapter },
#ifdef MODEM_CONTROL_SUPPORT_USB
    {GPIO_USB_RESUME_ID,                             GPIO_PROD_TYPE_BB0, GPIO_PROD_HIGH_LEVEL,    142,            Gx_UDC_USB_Resume},
#endif
    {GPIO_PROD_SENSOR_PWDN_ID,		       GPIO_PROD_TYPE_BB0, GPIO_PROD_HIGH_LEVEL,101,     PNULL   },
    {GPIO_PROD_SENSOR_PWDN_FRONT_ID,	GPIO_PROD_TYPE_BB0, GPIO_PROD_HIGH_LEVEL,69,     PNULL	},
    {GPIO_PROD_SENSOR_RESET_ID,		       GPIO_PROD_TYPE_BB0, GPIO_PROD_HIGH_LEVEL,68,     PNULL},


    {GPIO_PROD_BT_RESET_ID,			       GPIO_PROD_TYPE_BB0, GPIO_PROD_HIGH_LEVEL,105,PNULL   },
#if 0
    {GPIO_PROD_WIFI_PWD_ID,                        GPIO_PROD_TYPE_BB0, GPIO_PROD_LOW_LEVEL,    23 ,PNULL   },
    {GPIO_PROD_WIFI_RESET_ID,                     GPIO_PROD_TYPE_BB0, GPIO_PROD_LOW_LEVEL,    21 ,PNULL   },
    {GPIO_PROD_WIFI_INT_ID,                          GPIO_PROD_TYPE_BB0, GPIO_PROD_LOW_LEVEL,    22,(GPIO_CB)WIFI_IrqCallback},
#endif

#ifdef KEYPAD_TYPE_QWERTY_KEYPAD
    {GPIO_PROD_EXPAND_KEY0_ID,      GPIO_PROD_TYPE_BB0, GPIO_PROD_LOW_LEVEL,83,     PNULL   },
    {GPIO_PROD_EXPAND_KEY1_ID,      GPIO_PROD_TYPE_BB0, GPIO_PROD_LOW_LEVEL,84,     PNULL   },
    {GPIO_PROD_EXPAND_KEY2_ID,      GPIO_PROD_TYPE_BB0, GPIO_PROD_LOW_LEVEL,86,     PNULL   },
#endif

#ifdef SIM_PLUG_IN_SUPPORT
    {GPIO_PROD_SIM_PLUG_IN_ID,       GPIO_PROD_TYPE_BB0,    GPIO_PROD_LOW_LEVEL,32,  PNULL},
#endif

#if defined (FM_SUPPORT) && defined (FM_S_ANT_SUPPORT)
    {GPIO_PROD_FMLNA_ID,            GPIO_PROD_TYPE_BB0, GPIO_PROD_HIGH_LEVEL, 90,       PNULL},
#endif

    {GPIO_PROD_ID_MAX,					GPIO_PROD_TYPE_MAX, GPIO_PROD_HIGH_LEVEL,   GPIO_PROD_NUM_INVALID, PNULL},
};

/**---------------------------------------------------------------------------*
 **                         Constant Variables                                *
 **---------------------------------------------------------------------------*/


LOCAL uint32 _GPIO_OpenLCMBackLight(BOOLEAN is_open)
{
    if(is_open)
    {
        // Open lcm backlight
        ANA_SetDevValule(ANA_DEV_ID_LCM_BL, ANA_DEV_OPEN);
    }
    else
    {
        ANA_SetDevValule(ANA_DEV_ID_LCM_BL, ANA_DEV_CLOSE);
    }
	return TRUE;
}

LOCAL uint32 _GPIO_OpenKeyPadBackLight(BOOLEAN is_open)
{
    if(is_open)
    {
        // Set keypad backlight current to 1st level
        ANA_SetDevValule(ANA_DEV_ID_KPD_BL, ANA_DEV_VAL_HIGHER);
    }
    else
    {
        ANA_SetDevValule(ANA_DEV_ID_KPD_BL, ANA_DEV_VAL_MIN);
    }
	return TRUE;
}

LOCAL uint32 _GPIO_OpenVibrator(BOOLEAN is_open)
{
    if(is_open)
    {
        // Set vibrator current to 1st level
        ANA_SetDevValule(ANA_DEV_ID_VIBRATOR, ANA_DEV_VAL_HIGHER);
    }
    else
    {
        ANA_SetDevValule(ANA_DEV_ID_VIBRATOR, ANA_DEV_VAL_MIN);
    }
	return TRUE;
}
LOCAL uint32 _GPIO_OpenFlash(BOOLEAN is_open)
{
    if(is_open)
    {
        // Set vibrator current to 1st level
        ANA_SetDevValule(ANA_DEV_ID_FLASH_LIGHT, ANA_DEV_VAL_HIGHER);
    }
    else
    {
        ANA_SetDevValule(ANA_DEV_ID_FLASH_LIGHT, ANA_DEV_VAL_MIN);
    }
	return TRUE;
}

LOCAL uint32 _GPIO_OpenSDPower(BOOLEAN is_open)
{
	if(SC6600L == CHIP_GetChipType())
	{
		//for SC6600L1 chip, compatibly boards controlled by different sd power
		is_open = !is_open;

	    if(is_open)
		{
			ANA_SetDevValule(ANA_DEV_ID_SD_PWR, ANA_DEV_OPEN);
	   	}
		else
		{
			ANA_SetDevValule(ANA_DEV_ID_SD_PWR, ANA_DEV_CLOSE);
		}
	}
	return TRUE;
}
LOCAL uint32 _GPIO_IsUsbOrAdapter( BOOLEAN is_open )
{
    BOOLEAN ret_val = FALSE;

	if(CHIP_GetUsbDMValue())
	{
		ret_val = TRUE;
	}
	return ret_val;
}

PUBLIC uint32   AUDIO_PA_Ctl(uint32 id,uint32 is_open);

LOCAL uint32 _GPIO_OpenPA(BOOLEAN is_open)
{
#if 0
       SCI_TRACE_LOW("_GPIO_OpenPA is_open %d", is_open);
	if(is_open)   //
	{
		ANA_REG_SET(ANA_AUDIO_PA_CTRL0, 0x1F9);
		ANA_REG_SET(ANA_AUDIO_PA_CTRL1, 0x1542);
	}
	else
	{
		ANA_REG_SET(ANA_AUDIO_PA_CTRL1, 0x1642);
		ANA_REG_SET(ANA_AUDIO_PA_CTRL0, 0x18A);
	}
	return SCI_SUCCESS;
#else
    return AUDIO_PA_Ctl(0,is_open);
#endif
}


/**---------------------------------------------------------------------------*
 **                         Constant Variables                                *
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                     Public Function Prototypes                            *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function is used to initialize customer configure table
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC GPIO_CFG_INFO_T_PTR GPIO_CFG_GetCusTable(void)
{
	return (GPIO_CFG_INFO_T_PTR)s_gpio_prod_cfg_cus_table;
}

/*****************************************************************************/
//  Description:    This function is used to initialize GPIO  about customer setting
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_CustomizeInit(void)
{
    int32 val=0;
    if (g_headset_timer == NULL)
    {
        g_headset_timer = SCI_CreateTimer ("headset_int_timer",
                                              GPIO_HeadsetIntTimerCallback,
                                              NULL,
                                              HEADSET_POLETYPE_SHAKING_TIME,
                                              SCI_NO_ACTIVATE);
    }

    //SCI_TRACE_LOW:"GPIO_CustomizeInit() Start !"
    //SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_CFG_197_112_2_18_0_27_1_155,(uint8*)"");

    	//SCI_TRACE_LOW("zgt GPIO_CustomizeInit  ");
	// Init Product GPIO configure
	GPIO_PROD_InitCfgTable();

  // headset plug detect
	GPIO_PROD_RegGpio(
		GPIO_PROD_HEADSET_DETECT_ID,
		SCI_FALSE,
		SCI_FALSE,
		SCI_TRUE,
		HEADSET_DETECT_SHAKING_TIME,
		(GPIO_PROD_CALLBACK)GPIO_HeadsetDetectIntHandler
		);


   // Headset button
	GPIO_PROD_RegGpio(
		GPIO_PROD_HEADSET_BUTTON_ID,
		SCI_FALSE,
		SCI_TRUE,
		SCI_TRUE,
		HEADSET_BUTTON_SHAKING_TIME,
		(GPIO_PROD_CALLBACK)GPIO_HeadsetButtonIntHandler
		);

	// Charge plug detect
	GPIO_PROD_RegGpio(
		GPIO_PROD_CHARGE_PLUG_DETECT_ID,
		SCI_FALSE,
		SCI_FALSE,
		SCI_TRUE,
		VBUS_DETECT_SHAKING_TIME,
		(GPIO_PROD_CALLBACK)GPIO_ChargeIntHandler
		);

    // handle power key interrupt
    GPIO_PROD_RegGpio(
		GPIO_PROD_POWER_KEY_ID,
		SCI_FALSE,
		SCI_FALSE,
		SCI_TRUE,
		GPIO_DEFAULT_SHAKING_TIME,
		(GPIO_PROD_CALLBACK)GPIO_PowerKeyIntHandler
		);
	//handle Menu cancel Key interrupt
    GPIO_PROD_RegGpio(
		GPIO_PROD_MENUCANCEL_KEY_ID,
		SCI_FALSE,
		SCI_FALSE,
		SCI_TRUE,
		GPIO_DEFAULT_SHAKING_TIME,
		(GPIO_PROD_CALLBACK)GPIO_MENUCANCELKeyIntHandler
		);
#ifdef USE_NEW_USB_FRAMEWORK
        // SDCARD plug detect yintianci
        GPIO_PROD_RegGpio(
            GPIO_PROD_SDCARD_DETECT_ID,
            SCI_FALSE,
            SCI_FALSE,
            SCI_TRUE,
            SDCARD_DETECT_SHAKING_TIME,
            (GPIO_PROD_CALLBACK)GPIO_SdcardDetectIntHandler
        );
#endif
#ifdef MODEM_CONTROL_SUPPORT_USB
        GPIO_PROD_RegGpio(
		    GPIO_USB_RESUME_ID,
		    SCI_FALSE,
		    SCI_FALSE,
		    SCI_FALSE,
		    0,
		    (GPIO_PROD_CALLBACK)GPIO_USBResumeHandler
		);
#endif
#ifdef KEYPAD_TYPE_QWERTY_KEYPAD
       KPD_Reg_Expand_Key(5,  GPIO_GetExpandKeyHandler);
#endif

#ifdef SIM_PLUG_IN_SUPPORT
            GPIO_PROD_RegGpio(
                GPIO_PROD_SIM_PLUG_IN_ID,
                SCI_FALSE,
                SCI_TRUE,
                SCI_TRUE,
                GPIO_DEFAULT_SHAKING_TIME*4,
                 (GPIO_PROD_CALLBACK)GPIO_SIMIntHandler);
#endif

    GPIO_EXT_Init();
	//SCI_TRACE_LOW:"GPIO_CustomizeInit() End !"
	//SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_CFG_270_112_2_18_0_27_1_156,(uint8*)"");

}

#ifdef   __cplusplus
    }
#endif  // end of gpio_cfg.c

#endif  // End of DEBUG_GPIO_CFG

