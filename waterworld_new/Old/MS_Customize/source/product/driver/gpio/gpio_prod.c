/******************************************************************************
 ** File Name:      gpio_prod.c                                               *
 ** Author:         Richard.Yang                                              *
 ** DATE:           03/08/2004                                                *
 ** Copyright:      2004 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the pin usage and initiate value of     *
 **                 SP7100B                                                   *
 **                GPIO used information( SP7100B ), please see following     *
 **                GPIO define                                                *
 **                                                                           *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 03/08/2004     Richard.Yang     Create.                                   *
 ** 03/18/2004     Lin.liu          Modify for SP7100B                        *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "ms_customize_trc.h"
#include "gpio_drv.h"
#include "cmddef.h"
#include "tb_dal.h"
#include "gpio_prod_api.h"
#include "gpio_prod_cfg.h"
#include "gpio_ext_drv.h"
#include "scm_api.h"
#include "lcd_backlight.h"
#include "deep_sleep.h"
#include "ref_outport.h"
#include "vb_drv.h"
#include "pinmap.h"
//#include "pwm_drvapi.h"
#include "boot_drvapi.h"/*lint -esym(766, chip_drv\export\inc\boot_drvapi.h)*/
#include "analog_drv.h"
#if defined(PLATFORM_SC6530) || defined(PLATFORM_SC8501C) || defined(PLATFORM_SC7702)
#include "eic_hal.h"
#endif
#if defined(PLATFORM_UMS9117)
#include "eica_drvapi.h"
#endif

#ifdef DUAL_BATTERY_SUPPORT
#include "dualbat_drvapi.h"
#endif //DUAL_BATTERY_SUPPORT
#include "power_cfg.h"
#include "dal_keypad.h"
#include "adc_parameter.h"
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
extern   "C"
{
#endif


static irq_handler_t GPIO_VBUS_handler = NULL;
/**---------------------------------------------------------------------------*
 **                         local Variables                                  *
 **---------------------------------------------------------------------------*/
typedef struct HEADSET_Tag
{
	uint32 status;
	uint32 button_status;
	uint32 time;
}HEADSET_T;

#define  HEADSET_DISCONNECT  0
#define  HEADSET_CONNECT     1
//because samsung's earphone has three keys to read by ADC CHANNEL7
#ifdef HEADSET_ADC_CFG
#define KEY_SEND_MIN_VOL     0
#define KEY_SEND_MAX_VOL     20
#define KEY_VOLUP_MIN_VOL    25
#define KEY_VOLUP_MAX_VOL    46
#define KEY_VOLDOWN_MIN_VOL  55
#define KEY_VOLDOWN_MAX_VOL  106
#define KEY_RELEASE_MIN_VOL  184
#define KEY_RELEASE_MAX_VOL  668
#define KEY_WATER_BOTTOM	 751

// Previously 2.3 V MIC_BIAS was used.
// But now for all projects EAR_MIC_BIAS 2.8V is used. So below macro not needed
//#if defined(BOARD_ERICPLUS_CFG_VER03) || defined(BOARD_KIWI_CFG_VER02)
 #define KEY_3POLE_MIN_VOL	 0
 #define KEY_3POLE_MAX_VOL	 204
 #define KEY_4POLE_MIN_VOL	 225
 #define KEY_4POLE_MAX_VOL	 812

#else
#define KEY_3POLE_MIN_VOL	 0
 #define KEY_3POLE_MAX_VOL	 950

//
#define KEY_SEND_MIN_VOL    0
 #define KEY_SEND_MAX_VOL    496 // micbias 2.8v little scale 1.25v

 #define KEY_VOLUP_MIN_VOL   600
 #define KEY_VOLUP_MAX_VOL   1019
 #define KEY_VOLDOWN_MIN_VOL 1212
 #define KEY_VOLDOWN_MAX_VOL 2165

 #define KEY_RELEASE_MIN_VOL 3184 // 4_pole
 #define KEY_RELEASE_MAX_VOL 4095 // 4_pole
 #define KEY_WATER_BOTTOM	 2700//2500
 //#define KEY_DISABLE_MIN_VOL 30
#endif

#define CYCLE_COUNT_FORWATER    5
#define CYCLE_POLE_TYPE    20
#define COUNT_4POLE_KEY		 15

PUBLIC SCI_TIMER_PTR g_headset_timer = NULL;
LOCAL  HEADSET_T  s_headset_ctl;
LOCAL BOOLEAN headset_flag = SCI_FALSE;
LOCAL uint32 CountHead_forwater = 0;
LOCAL uint32 CountHeadPole_forwater = 0;
LOCAL uint32 var_diff_key_3pole = 0;
LOCAL BOOLEAN var_4pole_reg = SCI_FALSE;
LOCAL EARJACK_TYPE var_pole_type = EAR_ADC_0_POLE;
//LOCAL BOOLEAN var_3pole_type = SCI_FALSE;

#define  MAX_HEADSET_BUTTON_READY_TIME 1000//CYCLE_COUNT_FORWATER*HEADSET_DETECT_TIME+ CYCLE_POLE_TYPE*HEADSET_POLETYPE_TIME
#define MAX_HEADSET_BUTTON_READY_TIME_FOR_4POLE   2000
#define CHIP_REG_SET(reg_addr, value)   (*(volatile uint32 *)(reg_addr)  = (uint32)(value))

LOCAL uint8 headsetDetectCnt = 0;
TB_MSG  gpio_detect_msg = {0};

//LOCAL BOOLEAN is_headset_button_masked = SCI_FALSE;

LOCAL GPIO_CFG_INFO_T s_gpio_prod_cfg_full_table[GPIO_PROD_ID_MAX] = {0};

LOCAL uint32 adc_water_value = 0;
//Invoked in demod interrupt handler.
#if defined(DEMOD_HW_SIANO) || defined(DEMOD_HW_INNOFIDEI)
extern void DemodSpiIsr(void* context);
#endif

#ifdef JTAG_SUPPORT
BOOLEAN KeyPadBackLight_onoff = SCI_TRUE;
PUBLIC BOOLEAN  GPIO_PROD_GetKeyPadBackLightStatus (void)
{
	return KeyPadBackLight_onoff;
}
#endif

#ifdef SS_JIG_SUPPORT_GPIO
/*****************************************************************************/
//  Description:    This function is used to check jig-box gpio value
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC uint32  GPIO_PROD_GetJIGConnectStatus (void)
{
	unsigned char level = 0;
	uint16 gpio_num = 0;
	GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_JIG_STATE_ID);

	gpio_num = cfg_ptr->gpio_num;
	level = GPIO_GetValue(gpio_num);

	return (level?(SCI_FALSE):(SCI_TRUE));
}
#endif

/*****************************************************************************/
//  Description:    This function is used to set product gpio value
//  Author:         Liangwen.Zhen
//  Note:           First call callback funtion, second set gpio value
/*****************************************************************************/
BOOLEAN _GPIO_PROD_SetVal (GPIO_PROD_ID_E id, BOOLEAN value)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (id);

    if (PNULL != cfg_ptr)
    {
        if (!cfg_ptr->valid_level)
        {
            value = !value;
        }

        // have Call back,execute call back function
        if (PNULL != cfg_ptr->gpio_cb_fun)
        {
            cfg_ptr->gpio_cb_fun (value);
            //return SCI_TRUE;
        }

        if (GPIO_PROD_NUM_INVALID != cfg_ptr->gpio_num)
        {
            switch (cfg_ptr->gpio_type)
            {
                case GPIO_PROD_TYPE_BB0:

                    HAL_SetGPIOVal (cfg_ptr->gpio_num, value);
                    break;

                case GPIO_PROD_TYPE_EXT:
                    GPIO_EXT_SetValue (cfg_ptr->gpio_num, value);
                    break;

                default :
                    //SCI_PASSERT(0, ("_GPIO_PROD_RegGpio : type %d", cfg_ptr->gpio_type));
                    break;
            }
        }

        return SCI_TRUE;
    }
    else
    {
        return SCI_FALSE;
    }



}
/*****************************************************************************/
//  Description:    This function is used to get product gpio value
//  Author:         Liangwen.Zhen
//  Note:           First to check if there is the valid hardware GPIO,
//                  or call call-back function
/*****************************************************************************/
LOCAL BOOLEAN _GPIO_PROD_GetVal (GPIO_PROD_ID_E  id, BOOLEAN *value_ptr)
{

    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (id);

    if (PNULL != cfg_ptr)
    {
        if (GPIO_PROD_NUM_INVALID != cfg_ptr->gpio_num)
        {
            switch (cfg_ptr->gpio_type)
            {
                case GPIO_PROD_TYPE_BB0:
                    *value_ptr = HAL_GetGPIOVal (cfg_ptr->gpio_num);
                    break;

                case GPIO_PROD_TYPE_EXT:
                    *value_ptr = GPIO_EXT_GetValue (cfg_ptr->gpio_num);
                    break;

#if defined(PLATFORM_SC6530) || defined(PLATFORM_SC8501C)|| defined(PLATFORM_SC7702)
                case GPIO_PROD_TYPE_EIC_DBNC:
                    *value_ptr = EIC_HAL_GetValue (cfg_ptr->gpio_num);
                    break;
#endif
#if defined(PLATFORM_UMS9117)
                case GPIO_PROD_TYPE_EICA_DBNC:
                    *value_ptr = EICA_HAL_GetValue (cfg_ptr->gpio_num);
                    break;
#endif
                default :
                    //SCI_PASSERT(0, ("_GPIO_PROD_RegGpio : type %d", cfg_ptr->gpio_type));
                    break;
            }
        }

        else if ( (PNULL != cfg_ptr->gpio_cb_fun)
                  && (GPIO_PROD_TYPE_MAX == cfg_ptr->gpio_type))
        {
            // Call back
            *value_ptr = cfg_ptr->gpio_cb_fun (0);
        }
        else
        {
            return SCI_FALSE;
        }

        if (!cfg_ptr->valid_level)
        {
            *value_ptr = (! (*value_ptr));
        }

        return SCI_TRUE;
    }
    else
    {
        return SCI_FALSE;
    }
}

/*****************************************************************************/
//  Description:    This function is used to set gpio interrupt sense
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
LOCAL void _GPIO_PROD_SetInterruptSense (GPIO_PROD_ID_E  id, GPIO_PROD_INT_TYPE type)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (id);

    if (PNULL != cfg_ptr)
    {
        if (GPIO_PROD_TYPE_BB0 ==  cfg_ptr->gpio_type)
        {
            HAL_SetGPIOInterruptSense (cfg_ptr->gpio_num, type);
        }
#if defined(PLATFORM_SC6530) || defined(PLATFORM_SC8501C) || defined(PLATFORM_SC7702)
	else if(GPIO_PROD_TYPE_EIC_DBNC ==  cfg_ptr->gpio_type)
	{
            EIC_HAL_SetIntSense(cfg_ptr->gpio_num, type);
	}
#endif
#if defined(PLATFORM_UMS9117)
	else if(GPIO_PROD_TYPE_EICA_DBNC ==  cfg_ptr->gpio_type)
	{
            EICA_HAL_SetIntSense(cfg_ptr->gpio_num, type);
	}
#endif
	else
	{

	}
    }


}


/*****************************************************************************/
//  Description:    This function is used to set gpio direction
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
LOCAL void _GPIO_PROD_SetDirection (GPIO_PROD_ID_E  id, BOOLEAN is_output)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (id);

    if (PNULL != cfg_ptr)
    {
        if (GPIO_PROD_TYPE_BB0 ==  cfg_ptr->gpio_type)
        {
            HAL_SetGPIODirection (cfg_ptr->gpio_num, is_output);
        }
    }


}

/*****************************************************************************/
//  Description:    This function check if speaker pa and headset pa share one PA or not.
//   return  val:    SCI_TRUE  :   share
//                       SCI_FALSE :   no share
//  Author:          hyy
//  Note:
/*****************************************************************************/
LOCAL BOOLEAN  _GPIO_Check_Amplifier_Share (void)
{
    GPIO_CFG_INFO_T_PTR speaker_pa_cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_SPEAKER_PA_EN_ID);
    GPIO_CFG_INFO_T_PTR headset_pa_cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_HEADSET_PA_EN_ID);

    if ( (PNULL == speaker_pa_cfg_ptr) || (PNULL == headset_pa_cfg_ptr))
    {
        return SCI_FALSE;
    }

    if (speaker_pa_cfg_ptr->gpio_num == headset_pa_cfg_ptr->gpio_num)
    {
        return SCI_TRUE;
    }

    return SCI_FALSE;

}

/*****************************************************************************/
//  Description:    This function works when speaker pa and headset pa share one PA .
//  pa_type:        SCI_FALSE: speaker pa         SCI_TRUE:headset pa
//  b_enable:      enable selected pa
//  Author:          hyy
//  Note:
/*****************************************************************************/
LOCAL void  _GPIO_Control_Amplifier_Shared (BOOLEAN pa_type, BOOLEAN b_enable)
{
    if (!_GPIO_Check_Amplifier_Share())
    {
        return;  // no share
    }

    //when share
#define _SHARED_PA_STATE_X_X_   0   //speaker dis,  headset dis
#define _SHARED_PA_STATE_S_X_   1   //SPEAKER EN,   headset dis
#define _SHARED_PA_STATE_X_H_   2   //speaker dis,  HEADSER EN,
#define _SHARED_PA_STATE_S_H_   3   //SPEAKER EN,   HEADSER EN,

    {
        static uint8 shared_pa_state = _SHARED_PA_STATE_X_X_;

        switch (shared_pa_state)
        {
            case _SHARED_PA_STATE_X_X_:

                if (b_enable)
                {
                    if (pa_type)   //headset
                    {
                        _GPIO_PROD_SetVal (GPIO_PROD_HEADSET_PA_EN_ID, SCI_TRUE);
                        _GPIO_PROD_SetVal (GPIO_PROD_SHARED_PA_SW_ID,SCI_TRUE);  // SCI_TRUE MEANS :SHARED PA OUT = HEADSET
                        shared_pa_state = _SHARED_PA_STATE_X_H_;
                    }
                    else            //speaker
                    {
                        _GPIO_PROD_SetVal (GPIO_PROD_SPEAKER_PA_EN_ID, SCI_TRUE);
                        _GPIO_PROD_SetVal (GPIO_PROD_SHARED_PA_SW_ID,SCI_FALSE);  // SCI_FALSE MEANS :SHARED PA OUT = ALL
                        shared_pa_state = _SHARED_PA_STATE_S_X_;
                    }
                }

                break;

            case _SHARED_PA_STATE_S_X_:

                if (!pa_type)     //speaker
                {
                    if (!b_enable)
                    {
                        _GPIO_PROD_SetVal (GPIO_PROD_SPEAKER_PA_EN_ID, SCI_FALSE);
                        shared_pa_state = _SHARED_PA_STATE_X_X_;
                    }
                }
                else                //headset
                {
                    if (b_enable)
                    {
                        _GPIO_PROD_SetVal (GPIO_PROD_HEADSET_PA_EN_ID, SCI_TRUE);
                        _GPIO_PROD_SetVal (GPIO_PROD_SHARED_PA_SW_ID,SCI_FALSE);  // SCI_TRUE MEANS :SHARED PA OUT = ALL
                        shared_pa_state = _SHARED_PA_STATE_S_H_;
                    }
                }

                break;

            case _SHARED_PA_STATE_X_H_:

                if (pa_type)    //headset
                {
                    if (!b_enable)
                    {
                        _GPIO_PROD_SetVal (GPIO_PROD_HEADSET_PA_EN_ID, SCI_FALSE);
                        shared_pa_state = _SHARED_PA_STATE_X_X_;
                    }
                }
                else                 //speaker
                {
                    if (b_enable)
                    {
                        _GPIO_PROD_SetVal (GPIO_PROD_SPEAKER_PA_EN_ID, SCI_TRUE);
                        _GPIO_PROD_SetVal (GPIO_PROD_SHARED_PA_SW_ID,SCI_FALSE);  // SCI_TRUE MEANS :SHARED PA OUT = ALL
                        shared_pa_state = _SHARED_PA_STATE_S_H_;
                    }
                }

                break;

            case _SHARED_PA_STATE_S_H_:

                if (!b_enable)
                {
                    if (pa_type)   //headset
                    {
                        _GPIO_PROD_SetVal (GPIO_PROD_SHARED_PA_SW_ID,SCI_FALSE);  // SCI_TRUE MEANS :SHARED PA OUT = ALL
                        shared_pa_state = _SHARED_PA_STATE_S_X_;
                    }
                    else            //speaker
                    {
                        _GPIO_PROD_SetVal (GPIO_PROD_SHARED_PA_SW_ID,SCI_TRUE);  // SCI_FALSE MEANS :SHARED PA OUT = HEADSET
                        shared_pa_state = _SHARED_PA_STATE_X_H_;
                    }
                }

                break;
            default:
                break;
        }
    }
}


/**---------------------------------------------------------------------------*
 **                     Public Function Prototypes                             *
 **---------------------------------------------------------------------------*/
#ifdef MOTION_SENSOR_TYPE_OPTICAL_SCM013
PUBLIC void GPIO_ProximitySensorTXEn (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal(GPIO_PROD_PROXSENSOR_TX_ID,b_on);
}

PUBLIC void GPIO_ProximitySensorRXEn (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal(GPIO_PROD_PROXSENSOR_RX_ID,b_on);
}
#endif
/*****************************************************************************/
//  Description:    This function is used to set udc driver reg isr handler
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_VBUS_RegIntHandler(irq_handler_t irq_handler)
{
	if(irq_handler != NULL)
	{
		GPIO_VBUS_handler = irq_handler;
	}
}

/*****************************************************************************/
//  Description:    This function is used to call udc Callback function
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_VBUS_Callback(uint32 gpio_state)
{
    if(GPIO_VBUS_handler != NULL)
    {
        GPIO_VBUS_handler(gpio_state);
    }
}

/*****************************************************************************/
//  Description:    This function is used to handle charge int
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_ChargeIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    CHGMNG_ChargerPlugInHandler (gpio_id, gpio_state);
    GPIO_VBUS_Callback(gpio_state);

    if (gpio_state)
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_CHARGE_PLUG_DETECT_ID, GPIO_INT_LEVEL_LOW); //plug out det
    }
    else
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_CHARGE_PLUG_DETECT_ID, GPIO_INT_LEVEL_HIGH); //plug in det
    }
}
#ifdef MOTION_SENSOR_TYPE
/*****************************************************************************/
//  Description:    This function is used to handler g-sensor interrupt
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_GSensorIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    //Because Reading registers via IIC is very slow, so it CANNOT be done
    //in interrupt context.
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_GSENSOR_INT_ID);

    if (gpio_state)
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_GSENSOR_INT_ID, GPIO_INT_LEVEL_LOW);
    }
    else
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_GSENSOR_INT_ID, GPIO_INT_LEVEL_HIGH);
    }

    if (cfg_ptr && (cfg_ptr->valid_level == gpio_state) && (cfg_ptr->gpio_cb_fun))
    {
        cfg_ptr->gpio_cb_fun (GPIO_PROD_GSENSOR_INT_ID);
    }

}
#endif

#ifdef DUAL_BATTERY_SUPPORT
/*****************************************************************************/
//  Description:    Aux Battery Detect Int Handler.
//  Author:         Mingwei.Zhang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_DBAT_AuxBatDetectIntHandler(uint32 gpio_id, uint32 gpio_state)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo( GPIO_PROD_DBAT_ABAT_INT_ID );

    if(PNULL != cfg_ptr)
    {
        if (!gpio_state)
        {
            //SCI_TRACE_LOW:"[DBAT]GPIO_DBAT_AuxBatDetectIntHandler: Aux battery plug OUT !"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_461_112_2_18_0_33_1_1521,(uint8*)"");
            _GPIO_PROD_SetInterruptSense(GPIO_PROD_DBAT_ABAT_INT_ID, GPIO_INT_LEVEL_HIGH);
            DBAT_SendMsgToDBatTask (DBAT_A_BAT_PLUGOUT_MSG);
        }
        else
        {
            //SCI_TRACE_LOW:"[DBAT]GPIO_DBAT_AuxBatDetectIntHandler: Aux battery plug IN !"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_467_112_2_18_0_33_1_1522,(uint8*)"");
            _GPIO_PROD_SetInterruptSense(GPIO_PROD_DBAT_ABAT_INT_ID, GPIO_INT_LEVEL_LOW);
            DBAT_SendMsgToDBatTask (DBAT_A_BAT_PLUGIN_MSG);
        }
    }
}

/*****************************************************************************/
//  Description:    Main Battery Detect Int Handler.
//  Author:         Mingwei.Zhang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_DBAT_MainBatDetectIntHandler(uint32 gpio_id, uint32 gpio_state)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo( GPIO_PROD_DBAT_MBAT_INT_ID );

    if(PNULL != cfg_ptr)
    {
        if (!gpio_state)
        {
            //SCI_TRACE_LOW:"[DBAT]GPIO_DBAT_MainBatDetectIntHandler: Main battery plug OUT !"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_485_112_2_18_0_33_1_1523,(uint8*)"");
            _GPIO_PROD_SetInterruptSense(GPIO_PROD_DBAT_MBAT_INT_ID, GPIO_INT_LEVEL_HIGH);
            DBAT_SendMsgToDBatTask (DBAT_M_BAT_PLUGOUT_MSG);
        }
        else
        {
            //SCI_TRACE_LOW:"[DBAT]GPIO_DBAT_MainBatDetectIntHandler: Main battery plug IN !"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_491_112_2_18_0_33_1_1524,(uint8*)"");
            _GPIO_PROD_SetInterruptSense(GPIO_PROD_DBAT_MBAT_INT_ID, GPIO_INT_LEVEL_LOW);
            DBAT_SendMsgToDBatTask (DBAT_M_BAT_PLUGIN_MSG);
        }
    }
}

#endif  //DUAL_BATTERY_SUPPORT

/*****************************************************************************/
//  Description:    GPIO extend intrrupt handler function.
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_GpioExtendINTHandler (uint32 gpio_id, uint32 gpio_state)
{
    BOOLEAN gpio_value;
    HAL_SetGPIOInterruptSense (gpio_id, GPIO_INT_LEVEL_LOW);

    gpio_value = GPIO_GetValue (gpio_id);

    if (SCI_FALSE == gpio_value)
    {
        GPIO_EXT_ISR (gpio_id);
    }
}


/*****************************************************************************/
//  Description:    Micbas handler function.
//  Author:          Jackey.ling
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_TurnOnHeadsetMicbias(BOOLEAN open)
{
    if(!open)
   	{
    	if(ADC_GetJIGConnectStatus()== ADC_JIG_UART)
    	{
    	    return;
    	}
    }
	//Power_SetEarjackLDO(open);
	__sprd_codec_headset_micbias_en(open);
}

PUBLIC void Headset_SetFlag(BOOLEAN flag)
{
    headset_flag = flag;
    CountHeadPole_forwater = 0;
    var_diff_key_3pole = 0;
}

PUBLIC BOOLEAN Headset_GetFlag(void)
{
    return headset_flag;
}

/*****************************************************************************/
//  Description:   The timer handler for the uart break detect                            *
//  Global resource dependence : NONE                                                             *
//  Author:        Tao.Zhou                                                                               *
//  Note:          NONE                                                                                   *
/*****************************************************************************/
PUBLIC void GPIO_HeadsetIntTimer (void)
{
    TB_MSG  gpio_msg;

    if (g_headset_timer != NULL )
    {
        SCI_DeactiveTimer (g_headset_timer);
    }

    adc_water_value = GetHeadsetStatusVoltage();
    if(Headset_GetFlag())
    {
        if(adc_water_value < KEY_WATER_BOTTOM)
        {
            if((adc_water_value >= KEY_RELEASE_MIN_VOL) && (adc_water_value <= KEY_RELEASE_MAX_VOL))
            {
                CountHeadPole_forwater++;
                if(CountHeadPole_forwater < CYCLE_POLE_TYPE && !var_4pole_reg)
                {
                    SCI_ChangeTimer(g_headset_timer, GPIO_HeadsetIntTimer, HEADSET_POLETYPE_TIME);
                    SCI_ActiveTimer(g_headset_timer);//loop timer when mic haven't connect.
                }
		  		var_pole_type = EAR_ADC_4_POLE;

            }
            else if((adc_water_value >= 0) && (adc_water_value <= KEY_3POLE_MAX_VOL))// 3_pole
            {
                var_diff_key_3pole ++;
		        var_4pole_reg = SCI_TRUE;
		        if(var_diff_key_3pole > COUNT_4POLE_KEY/*(HEADSET_POLETYPE_TIME*CYCLE_POLE_TYPE/200)*/)
                {
           			var_pole_type = EAR_ADC_3_POLE;
           			GPIO_TurnOnHeadsetMicbias(SCI_FALSE);
		   	    }
                if((CountHeadPole_forwater < CYCLE_POLE_TYPE) && (var_diff_key_3pole <= COUNT_4POLE_KEY))
		        {
		            SCI_ChangeTimer(g_headset_timer, GPIO_HeadsetIntTimer, HEADSET_POLETYPE_TIME);
                    SCI_ActiveTimer(g_headset_timer);//loop timer when mic haven't connect.
		        }
            }
	        if(CountHeadPole_forwater <=1 && var_diff_key_3pole <=1 )
	        {
                gpio_msg.message = TB_KPD_PRESSED;
                s_headset_ctl.status = HEADSET_CONNECT;
                gpio_msg.wparam  = SCI_VK_HEADSET_DETECT;
                gpio_msg.lparam  = TB_NULL;
            }
            else
                return;
        }
        else
        {
            SCI_TRACE_LOW("Headset water %d\n",CountHead_forwater);
            if(CountHead_forwater < CYCLE_COUNT_FORWATER)
            {
                SCI_ChangeTimer(g_headset_timer, GPIO_HeadsetIntTimer, HEADSET_DETECT_TIME);
                SCI_ActiveTimer(g_headset_timer);//loop timer when mic haven't connect.
            }
            else
            {
                GPIO_TurnOnHeadsetMicbias(SCI_FALSE);
            }
            CountHead_forwater ++;
            return;
        }
    }
    else
    {
        if(s_headset_ctl.status == HEADSET_CONNECT)
        {
        	var_pole_type = EAR_ADC_0_POLE;
            gpio_msg.message = TB_KPD_RELEASED;
            s_headset_ctl.status = HEADSET_DISCONNECT;
            gpio_msg.wparam  = SCI_VK_HEADSET_DETECT;
            gpio_msg.lparam  = TB_NULL;
        }
        else
            return;
    }
    if(HEADSET_CONNECT == s_headset_ctl.status)
    {
         s_headset_ctl.time = SCI_GetTickCount();
    }
    else
    {
	  s_headset_ctl.time = 0;
    }
#ifdef MICIN_BIAS_UNSTABLE
    if(SCI_IsTimerActive(g_headbutt_down_timer))
    {
    	SCI_DeactiveTimer(g_headbutt_down_timer);
    }
#endif
    DRV_Callback (TB_GPIO_INT,&gpio_msg);
}
#ifdef MICIN_BIAS_UNSTABLE
TB_MSG gpio_butt_down_msg = 0;
TB_MSG gpio_butt_up_msg = 0;

PUBLIC void GPIO_HeadButtDownIntTimer(void)
{
    if(g_headbutt_down_timer != NULL)
    {
    	SCI_DeactiveTimer(g_headbutt_down_timer);
    }
    else
       return;

    DRV_Callback (TB_GPIO_INT,&gpio_butt_down_msg);
}

PUBLIC void GPIO_HeadButtUpIntTimer(void)
{
    if(g_headbutt_up_timer != NULL)
    {
    	SCI_DeactiveTimer(g_headbutt_up_timer);
    }
    else
       return;

    DRV_Callback (TB_GPIO_INT,&gpio_butt_up_msg);
}
#endif

LOCAL uint32 key_send_handler(uint32 adc_value)
{
    LOCAL uint32 key_value = 0;
    uint32 value = 0;

    if((adc_value >= KEY_SEND_MIN_VOL) && (adc_value <= KEY_SEND_MAX_VOL))
    {
         key_value = value = SCI_VK_HEADSET_BUTTOM;
    }
    else if((adc_value >= KEY_VOLUP_MIN_VOL) && (adc_value <=KEY_VOLUP_MAX_VOL))
    {
        key_value = value = SCI_VK_VOL_UP;//SCI_VK_INVALID_KEY;//
    }
    else if((adc_value >= KEY_VOLDOWN_MIN_VOL) && (adc_value <= KEY_VOLDOWN_MAX_VOL))
    {
        key_value = value = SCI_VK_VOL_DOWN;//SCI_VK_INVALID_KEY; //SCI_VK_SIDE_DOWN;
    }
    else if((adc_value >= KEY_RELEASE_MIN_VOL) && (adc_value <= KEY_RELEASE_MAX_VOL))
    {
        value = key_value ;
        key_value = 0;
    }
    return value;
}

/*****************************************************************************/
//  Description:   The timer handler for the uart break detect                            *
//  Global resource dependence : NONE                                                             *
//  Author:        Tao.Zhou                                                                               *
//  Note:          NONE                                                                                   *
/*****************************************************************************/

LOCAL void GPIO_SentHeadsetIsConnectMsg(BOOLEAN connect)
{
    TB_MSG  gpio_msg;

    SCI_TRACE_LOW("headset _GPIO_SentHeadsetIsConnectMsg connect= %d",connect);

	if(connect == SCI_TRUE)
	{
		gpio_msg.message = TB_KPD_PRESSED;
		s_headset_ctl.status = HEADSET_CONNECT;
		gpio_msg.wparam  = SCI_VK_HEADSET_DETECT;
		gpio_msg.lparam  = TB_NULL;
	}
	else
	{
		gpio_msg.message = TB_KPD_RELEASED;
		s_headset_ctl.status = HEADSET_DISCONNECT;
		gpio_msg.wparam  = SCI_VK_HEADSET_DETECT;
		gpio_msg.lparam  = TB_NULL;
	}

	if(HEADSET_CONNECT == s_headset_ctl.status)
	{
	       s_headset_ctl.time = SCI_GetTickCount();
	}
	else
	{
	  	s_headset_ctl.time = 0;
	}
    //__sprd_codec_headset_set_connection_status(s_headset_ctl.status);
	DRV_Callback (TB_GPIO_INT,&gpio_msg);
}
PUBLIC void GPIO_HeadsetIntTimerCallback (void)
{
    TB_MSG  gpio_msg;

    if (g_headset_timer != NULL )
    {
    	SCI_DeactiveTimer (g_headset_timer);
    }

    //adc_water_value = GetHeadsetStatusVoltage();

    //headset_select_audio_to_adc(HEADSET_ADC_HEAD_DISABLE);	// disble AUXADC_REG for power saving

    SCI_TRACE_LOW("GPIO_HeadsetIntTimerCallback");

    if(Headset_GetFlag())
    {
    	if(s_headset_ctl.status == HEADSET_DISCONNECT)
    	{
    		GPIO_SentHeadsetIsConnectMsg(SCI_TRUE);
    	}
    }
    else
    {
    	if(s_headset_ctl.status == HEADSET_CONNECT)
    	{
    		//var_pole_type = ear_status_type = EAR_ADC_0_POLE;
    //			sprd_inter_headphone_pa_pre(0);
    		GPIO_SentHeadsetIsConnectMsg(SCI_FALSE);
    	}
    }
        __sprd_codec_headset_set_connection_status(s_headset_ctl.status);
    if(HEADSET_CONNECT==s_headset_ctl.status)
    {
        var_pole_type = __sprd_codec_headset_type_detect();
    }
    //GPIO_TurnOnHeadsetMicbias(SCI_FALSE);//
    return;
}

PUBLIC void GPIO_HeadsetDetectIntHandler (uint32 gpio_id, uint32 gpio_state)   //only for samsung
{
	GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_HEADSET_DETECT_ID);

	SCI_TRACE_LOW("_GPIO_HeadsetDetectIntHandler_GPIO gpio_state = %d valid_level = %d\n", gpio_state,cfg_ptr->valid_level);

    if (PNULL != cfg_ptr)
    {
        if (gpio_state)
        {
            if (cfg_ptr->valid_level)
            {
                Headset_SetFlag(SCI_TRUE);
                //GPIO_TurnOnHeadsetMicbias(SCI_TRUE);// TODO: set open mic power reg

			//headset_select_audio_to_adc(HEADSET_ADC_HEADMIC_IN);	//switch headmic_in into AUXADC for 3_pole/4_pole judgement
            }
            else
            {
                Headset_SetFlag(SCI_FALSE);
                //GPIO_TurnOnHeadsetMicbias(SCI_FALSE);// TODO:
            }
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_DETECT_ID, GPIO_INT_LEVEL_LOW);
        }
        else
        {
            if (!cfg_ptr->valid_level)
            {
                Headset_SetFlag(SCI_TRUE);
                //GPIO_TurnOnHeadsetMicbias(SCI_TRUE);
			//headset_select_audio_to_adc(HEADSET_ADC_HEADMIC_IN);	//switch headmic_in into AUXADC for 3_pole/4_pole judgement
            }
            else
            {
                Headset_SetFlag(SCI_FALSE);
               // GPIO_TurnOnHeadsetMicbias(SCI_FALSE);
            }
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_DETECT_ID, GPIO_INT_LEVEL_HIGH);
        }

		if (SCI_IsTimerActive(g_headset_timer))
		{
		    SCI_DeactiveTimer(g_headset_timer);
		}

		if(Headset_GetFlag())
		    SCI_ChangeTimer(g_headset_timer, GPIO_HeadsetIntTimerCallback, HEADSET_DETECT_TIME);
		else
		  SCI_ChangeTimer(g_headset_timer, GPIO_HeadsetIntTimerCallback, 2);

		SCI_ActiveTimer(g_headset_timer);
    }
}


PUBLIC void GPIO_HeadsetButtonIntHandler(uint32 gpio_id, uint32 gpio_state)
{
	uint32 adc_value = 0;
	BOOLEAN gpio_val = SCI_FALSE;

	LOCAL uint32 key_value = SCI_VK_INVALID_KEY;
	TB_MSG  gpio_msg;


	GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_HEADSET_BUTTON_ID);

	adc_value = __sprd_codec_get_headbutton_value();


	SCI_TRACE_LOW("GPIO_HeadsetButtonIntHandler  gpio_state %d, adc_value = %d",gpio_state,adc_value);

	if (PNULL != cfg_ptr)
	{
        	if (!_GPIO_PROD_GetVal (GPIO_PROD_HEADSET_DETECT_ID, &gpio_val))
        	{
        		//return;
        	}
	        SCI_TRACE_LOW("GPIO_HeadsetButtonIntHandler  gpio_id %d, gpio_val = %d",gpio_id,gpio_val);

	    /*   The available condition of Headset Button:
		    1. 	Headset had connected
		    2.  The time of headset button interrupt raised should be lated more than 1000ms
		        after headset detected interrupt raised
		    3.  The pin status of headset button is available
	   */

		if (gpio_state)
		{
			_GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_BUTTON_ID, GPIO_INT_LEVEL_LOW);
		}
		else
		{
			_GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_BUTTON_ID, GPIO_INT_LEVEL_HIGH);
		}
	        SCI_TRACE_LOW("GPIO_HeadsetButtonIntHandler  valid_level %d, gpio_val = %d",cfg_ptr->valid_level,gpio_val);
		if ((HEADSET_CONNECT == s_headset_ctl.status) &&
		(SCI_GetTickCount() > (s_headset_ctl.time + MAX_HEADSET_BUTTON_READY_TIME)) &&
		gpio_val)
		{
			if (gpio_state)
			{
				if (cfg_ptr->valid_level)
				{
					gpio_msg.message = TB_KPD_PRESSED;
					key_value = key_send_handler(adc_value);  //send differ key
				}

				else
				{
					gpio_msg.message = TB_KPD_RELEASED;
				}

				gpio_msg.wparam  = key_value & 0xffff;
				gpio_msg.lparam  = TB_NULL;
				_GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_BUTTON_ID, GPIO_INT_LEVEL_LOW);
			}
			else
			{
				if (!cfg_ptr->valid_level)
				{
					gpio_msg.message = TB_KPD_PRESSED;
					key_value = key_send_handler(adc_value);  //send differ key
				}
				else
				{
					gpio_msg.message = TB_KPD_RELEASED;
				}

				gpio_msg.wparam  = key_value;
				gpio_msg.lparam  = TB_NULL;
				_GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_BUTTON_ID, GPIO_INT_LEVEL_HIGH);
			}
	            SCI_TRACE_LOW("GPIO_HeadsetButtonIntHandler  gpio_id %d, key_value = %d",gpio_id,key_value);

#if 0
			if(!poweron_headsetbutton_detect)
			{
				DRV_Callback (TB_GPIO_INT,&gpio_msg);
			}
			else
			{
				poweron_headsetbutton_detect = 0;
			}
#endif
                    DRV_Callback (TB_GPIO_INT,&gpio_msg);
		}
	}
}


PUBLIC void GPIO_HeadsetButtonKeyIntHandler(uint32 gpio_id, uint32 gpio_state)
{
    TB_MSG  gpio_msg;
    uint32 adc_value = 0,key_value;

    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_HEADSET_BUTTON_ID);

#ifdef HEADSET_ADC_CFG
    adc_value = ADC_CheckHeadsetStatus();
#else
    adc_value = GetHeadsetStatusVoltage();
#endif

    if (PNULL != cfg_ptr)
    {
        BOOLEAN gpio_val = SCI_FALSE;

        if (!_GPIO_PROD_GetVal (GPIO_PROD_HEADSET_DETECT_ID, &gpio_val))
        {
            return;
        }

        /*   The available condition of Headset Button:
		    1. 	Headset had connected
		    2.  The time of headset button interrupt raised should be lated more than 1000ms
		        after headset detected interrupt raised
		    3.  The pin status of headset button is available
       */

        if (gpio_state)
        {
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_BUTTON_ID, GPIO_INT_LEVEL_LOW);
        }
        else
        {
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_BUTTON_ID, GPIO_INT_LEVEL_HIGH);
        }

        if ((HEADSET_CONNECT == s_headset_ctl.status) &&
			((SCI_GetTickCount() > (s_headset_ctl.time + MAX_HEADSET_BUTTON_READY_TIME)) ||
			((SCI_GetTickCount() > (s_headset_ctl.time + MAX_HEADSET_BUTTON_READY_TIME_FOR_4POLE)) && (GPIO_GetHeadsetTypeStatus() == EAR_ADC_4_POLE))) &&
			gpio_val)
	{
            if (gpio_state)
            {
                if (cfg_ptr->valid_level)
                {
                    gpio_msg.message = TB_KPD_PRESSED;
                    key_value = key_send_handler(adc_value);
                }
                else
                {
                    gpio_msg.message = TB_KPD_RELEASED;
                    key_value = key_send_handler(adc_value);
                }

                gpio_msg.wparam  = key_value & 0xffff;
                gpio_msg.lparam  = TB_NULL;
                _GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_BUTTON_ID, GPIO_INT_LEVEL_LOW);
            }
            else
            {
                if (!cfg_ptr->valid_level)
                {
                    gpio_msg.message = TB_KPD_PRESSED;
                    key_value = key_send_handler(adc_value);
                }
                else
                {
                    gpio_msg.message = TB_KPD_RELEASED;
                    key_value = key_send_handler(adc_value);
                }

                gpio_msg.wparam  = key_value;
                gpio_msg.lparam  = TB_NULL;
                _GPIO_PROD_SetInterruptSense (GPIO_PROD_HEADSET_BUTTON_ID, GPIO_INT_LEVEL_HIGH);
            }
#ifdef MICIN_BIAS_UNSTABLE
            if(gpio_msg.message == TB_KPD_PRESSED)
	        {
	    	    if(SCI_IsTimerActive(g_headbutt_down_timer))
                {
    	            SCI_DeactiveTimer(g_headbutt_down_timer);
                }
                gpio_butt_down_msg.message = TB_KPD_PRESSED;
                gpio_butt_down_msg.lparam = gpio_msg.lparam;
                gpio_butt_down_msg.wparam = gpio_msg.wparam;
                SCI_ChangeTimer(g_headbutt_down_timer, GPIO_HeadButtDownIntTimer, HEADBUTT_DETECT_TIME);
                SCI_ActiveTimer(g_headbutt_down_timer);
	        }
	        else
	        {
	    	    if(SCI_IsTimerActive(g_headbutt_up_timer))
                {
    	            SCI_DeactiveTimer(g_headbutt_up_timer);
                }
                gpio_butt_up_msg.message = TB_KPD_RELEASED;
                gpio_butt_up_msg.lparam = gpio_msg.lparam;
                gpio_butt_up_msg.wparam = gpio_msg.wparam;
                SCI_ChangeTimer(g_headbutt_up_timer, GPIO_HeadButtUpIntTimer, HEADBUTT_DETECT_TIME);
                SCI_ActiveTimer(g_headbutt_up_timer);
	        }
#else
            DRV_Callback (TB_GPIO_INT,&gpio_msg);
#endif
        }
   }
}
/*****************************************************************************/
//  Description:    FlipOn detection handler function.
//  Author:         BenjaminWang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_FlipOnIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    TB_MSG  gpio_msg;

    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_FLIP_ON_ID);

    if (PNULL != cfg_ptr)
    {

        if (gpio_state)
        {
            if (cfg_ptr->valid_level)
            {
                gpio_msg.message = TB_KPD_PRESSED;
            }
            else
            {
                gpio_msg.message = TB_KPD_RELEASED;
            }

            gpio_msg.wparam  = SCI_VK_FLIP;
            gpio_msg.lparam  = TB_NULL;
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_FLIP_ON_ID, GPIO_INT_LEVEL_LOW);
        }
        else
        {
            if (!cfg_ptr->valid_level)
            {
                gpio_msg.message = TB_KPD_PRESSED;
            }
            else
            {
                gpio_msg.message = TB_KPD_RELEASED;
            }

            gpio_msg.wparam  = SCI_VK_FLIP;
            gpio_msg.lparam  = TB_NULL;
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_FLIP_ON_ID, GPIO_INT_LEVEL_HIGH);
        }

        DRV_Callback (TB_GPIO_INT,&gpio_msg);
    }
}

/*****************************************************************************/
//  Description:    PowerKey detection handler function.
//  Author:         BenjaminWang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_PowerKeyIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    TB_MSG  gpio_msg;
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_POWER_KEY_ID);

    if (gpio_state == cfg_ptr->valid_level)
    {
        gpio_msg.message = TB_KPD_PRESSED;
        gpio_msg.wparam  = SCI_VK_POWER;
        gpio_msg.lparam  = TB_NULL;
    }
    else
    {
        gpio_msg.message = TB_KPD_RELEASED;
        gpio_msg.wparam  = SCI_VK_POWER;
        gpio_msg.lparam  = TB_NULL;
    }

    if (gpio_state)
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_POWER_KEY_ID, GPIO_INT_LEVEL_LOW);
    }
    else
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_POWER_KEY_ID, GPIO_INT_LEVEL_HIGH);
    }

    DRV_Callback (TB_GPIO_INT,&gpio_msg);
}

/*****************************************************************************/
//  Description:    MENUCANCELKey detection handler function.
//  Author:         BenjaminWang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_MENUCANCELKeyIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    TB_MSG  gpio_msg;
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_MENUCANCEL_KEY_ID);

    if (gpio_state == cfg_ptr->valid_level) {
        gpio_msg.message = TB_KPD_PRESSED;
        gpio_msg.wparam  = SCI_VK_MENU_CANCEL;
        gpio_msg.lparam  = TB_NULL;
    } else {
        gpio_msg.message = TB_KPD_RELEASED;
        gpio_msg.wparam  = SCI_VK_MENU_CANCEL;
        gpio_msg.lparam  = TB_NULL;
    }

    if (gpio_state)
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_MENUCANCEL_KEY_ID, GPIO_INT_LEVEL_LOW);
    else
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_MENUCANCEL_KEY_ID, GPIO_INT_LEVEL_HIGH);

    DRV_Callback (TB_GPIO_INT,&gpio_msg);
}

/*****************************************************************************/
//  Description:    Touch panel detection handler function.
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_TPDetectIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_TPXL_DETECT_ID);

    if (PNULL != cfg_ptr)
    {
        if (!cfg_ptr->valid_level)
        {
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_TPXL_DETECT_ID, GPIO_INT_LEVEL_LOW);
        }
        else
        {
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_TPXL_DETECT_ID, GPIO_INT_LEVEL_HIGH);
        }

        if (PNULL != cfg_ptr->gpio_cb_fun)
        {
            if (!cfg_ptr->valid_level)
            {
                gpio_state = !gpio_state;
            }

            cfg_ptr->gpio_cb_fun (gpio_state);
        }

    }

}


/*****************************************************************************/
//  Description:    PowerKey detection handler function.
//  Author:         BenjaminWang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_BackendICIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    //Interrupt_Processing(gpio_id, gpio_state);
}

#ifndef MODEM_PLATFORM
/*****************************************************************************/
//  Description:    SDCard detection handler function.
//  Author:         juan.zhang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SdcardDetectIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_SDCARD_DETECT_ID);
    SCM_SLOT_NAME_E slot_name = SCM_SLOT_0;
#ifdef SD_HOTPLUG_SLOT1
    slot_name = SCM_SLOT_1;
#endif

    if (PNULL != cfg_ptr)
    {
        if (gpio_state)
        {
            if (cfg_ptr->valid_level)
            {
                //Send message to SCM
                SCM_PlugIn (slot_name);
            }
            else
            {
                //Send message to SCM
                SCM_PlugOut (slot_name);
            }

            _GPIO_PROD_SetInterruptSense (GPIO_PROD_SDCARD_DETECT_ID, GPIO_INT_LEVEL_LOW);
        }
        else
        {
            if (!cfg_ptr->valid_level)
            {
                //Send message to SCM
                SCM_PlugIn (slot_name);
            }
            else
            {
                //Send message to SCM
                SCM_PlugOut (slot_name);
            }

            _GPIO_PROD_SetInterruptSense (GPIO_PROD_SDCARD_DETECT_ID, GPIO_INT_LEVEL_HIGH);
        }
    }

}


/*****************************************************************************/
//  Description:    Customer 1 detection handler function.
//  Author:         Liangwen.zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_Cus1DetectIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    GPIO_CFG_INFO_T cfg_info = {0};
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_CUS_1_DETECT_ID);

    SCI_ASSERT (PNULL != cfg_ptr);/*assert verified*/

    cfg_info.gpio_id    = GPIO_PROD_SDCARD_DETECT_ID;
    cfg_info.gpio_type  = GPIO_PROD_TYPE_EXT;
    cfg_info.gpio_num   = cfg_ptr->gpio_num;
    cfg_info.valid_level= GPIO_PROD_HIGH_LEVEL;

    GPIO_PROD_SetCfgInfo (&cfg_info);

    GPIO_SelectSPI2EXTCS (0);

    GPIO_SdcardDetectIntHandler (gpio_id, gpio_state);
}

/*****************************************************************************/
//  Description:    Customer 2 detection handler function.
//  Author:         Liangwen.zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_Cus2DetectIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    GPIO_CFG_INFO_T cfg_info = {0};
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_CUS_2_DETECT_ID);

    SCI_ASSERT (PNULL != cfg_ptr);/*assert verified*/

    cfg_info.gpio_id    = GPIO_PROD_SDCARD_DETECT_ID;
    cfg_info.gpio_type  = GPIO_PROD_TYPE_EXT;
    cfg_info.gpio_num   = cfg_ptr->gpio_num;
    cfg_info.valid_level= GPIO_PROD_LOW_LEVEL;

    GPIO_PROD_SetCfgInfo (&cfg_info);

    GPIO_SelectSPI2EXTCS (5);

    GPIO_SdcardDetectIntHandler (gpio_id, gpio_state);
}
#endif


/*****************************************************************************/
//  Description:    Camera cover detection handler function.
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_CameraCoverDetectHandler (uint32 gpio_id, uint32 gpio_state)
{
    TB_MSG  gpio_msg;
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_FLIP_ON_ID);

    if (PNULL != cfg_ptr)
    {
        if (gpio_state)
        {
            if (cfg_ptr->valid_level)
            {
                gpio_msg.message = TB_KPD_PRESSED;
            }
            else
            {
                gpio_msg.message = TB_KPD_RELEASED;
            }

            //gpio_msg.wparam  = SCI_VK_CAMERA_COVER;
            gpio_msg.lparam  = TB_NULL;
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_FLIP_ON_ID, GPIO_INT_LEVEL_LOW);

        }
        else
        {
            if (!cfg_ptr->valid_level)
            {
                gpio_msg.message = TB_KPD_PRESSED;
            }
            else
            {
                gpio_msg.message = TB_KPD_RELEASED;
            }

            // gpio_msg.wparam  = SCI_VK_CAMERA_COVER;
            gpio_msg.lparam  = TB_NULL;
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_FLIP_ON_ID, GPIO_INT_LEVEL_HIGH);

        }

        DRV_Callback (TB_GPIO_INT,&gpio_msg);
    }

}

/*****************************************************************************/
//  Description:    LCD Frame mark detection handler function.
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_LCDFrameMarkDetectHandler (uint32 gpio_id, uint32 gpio_state)
{


    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_LCD_FMARK_DETECT_ID);

    //SCI_TRACE_LOW:"GPIO_WiFiIrqHander()"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_935_112_2_18_0_33_2_1525,(uint8*)"");

    if (PNULL != cfg_ptr)
    {
        if (cfg_ptr->gpio_cb_fun && (cfg_ptr->valid_level == gpio_state))
        {
            cfg_ptr->gpio_cb_fun (0);
        }

    }
}

/*****************************************************************************/
//  Description:    Double key first action detection handler function.
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_DKeyFirstActionHandler (uint32 gpio_id, uint32 gpio_state)
{
    if (!gpio_state)
    {
        //    HAL_SetGPIOInterruptSense(gpio_id, GPIO_INT_LEVEL_HIGH);
        //    DKey_ChangeStarte(SPECIAL_KEY_DK_FIRST_PRESS);
    }
    else
    {
        //    HAL_SetGPIOInterruptSense(gpio_id, GPIO_INT_LEVEL_LOW);
        //    DKey_ChangeStarte(SPECIAL_KEY_DK_RELEASE);
    }
}

/*****************************************************************************/
//  Description:    Double key secord action detection handler function.
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_DKeySecordActionHandler (uint32 gpio_id, uint32 gpio_state)
{
    if (!gpio_state)
    {
        //    HAL_SetGPIOInterruptSense(gpio_id, GPIO_INT_LEVEL_HIGH);
        //    DKey_ChangeStarte(SPECIAL_KEY_DK_SECORD_PRESS);
    }
    else
    {
        //No need send message again.
        //    HAL_SetGPIOInterruptSense(gpio_id, GPIO_INT_LEVEL_LOW);
    }
}


/*****************************************************************************/
//  Description:    Bluetooth interrupte detection handler function.
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_BTIntHandler (
    uint32 gpio_id,
    uint32 gpio_state)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_BT_REQ_CLK_ID);
    BOOLEAN             is_open = SCI_FALSE;

    if (PNULL != cfg_ptr)
    {
        if (gpio_state)
        {
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_BT_REQ_CLK_ID,GPIO_INT_LEVEL_LOW);
        }
        else
        {
            _GPIO_PROD_SetInterruptSense (GPIO_PROD_BT_REQ_CLK_ID,GPIO_INT_LEVEL_HIGH);
        }

        if (gpio_state == cfg_ptr->valid_level)
        {
            is_open = SCI_TRUE;
        }

        DPSLP_XTLOpenInSleep (is_open);

        HAL_EnableGPIOIntCtl (gpio_id);

    }
}

/*****************************************************************************/
//  Description:    MicorUSB interrupte detection handler function.
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_MicroUSBIntHandler (
        uint32 gpio_id,
        uint32 gpio_state)
{
  	GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo( GPIO_PROD_MICRO_USB_INT_ID );

   	if(PNULL != cfg_ptr)
	{
	   	if (gpio_state)
	   	{
	      	_GPIO_PROD_SetInterruptSense(GPIO_PROD_MICRO_USB_INT_ID,GPIO_INT_LEVEL_LOW);
	   	}
	   	else
	   	{
	      	_GPIO_PROD_SetInterruptSense(GPIO_PROD_MICRO_USB_INT_ID,GPIO_INT_LEVEL_HIGH);
	   	}

		if(cfg_ptr->gpio_cb_fun && (cfg_ptr->valid_level == gpio_state))
		{
			cfg_ptr->gpio_cb_fun(0);
		}

	}
}

LOCAL void GPIO_HeadsetRecogPowerOn(void)
{
    SCI_TRACE_LOW("zgt GPIO_HeadsetRecogPowerOn   ");

    GPIO_TurnOnHeadsetMicbias(SCI_TRUE);
    SCI_Sleep(20);

    s_headset_ctl.status = HEADSET_CONNECT;
    s_headset_ctl.time   = SCI_GetTickCount();

}
/*****************************************************************************/
//  Description:    This function is used to Register product gpio
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
BOOLEAN GPIO_PROD_RegGpio (
    GPIO_PROD_ID_E          id,
    BOOLEAN                 direction,        // SCI_FALSE: Input dir; SCI_TRUE: output dir
    BOOLEAN                 default_value,    //
    BOOLEAN                 shaking_en,
    uint32                  shaking_interval,
    GPIO_PROD_CALLBACK      gpio_callback_fun
)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (id);
    PM_IS_E int_sense;

    if (cfg_ptr == PNULL)
    {
        return SCI_FALSE;
    }

    int_sense = PM_GetGPIOIntType (cfg_ptr->gpio_num);

    switch (cfg_ptr->gpio_type)
    {
        case GPIO_PROD_TYPE_BB0:

            switch (int_sense)
            {
                case PM_LEVEL:

                    if (HAL_GetGPIOVal (cfg_ptr->gpio_num))
                    {
                        HAL_SetGPIOInterruptSense (cfg_ptr->gpio_num, GPIO_INT_LEVEL_LOW);

                        if (cfg_ptr->valid_level)
                        {
                            // In case of headset in is detected when power on,
                            // call the related routine.
                            if (GPIO_PROD_HEADSET_DETECT_ID == id)
                            {
					            GPIO_HeadsetRecogPowerOn();
                            }
                        }
                    }
                    else
                    {
                        HAL_SetGPIOInterruptSense (cfg_ptr->gpio_num, GPIO_INT_LEVEL_HIGH);

                        if (!cfg_ptr->valid_level)
                        {
                            // In case of headset in is detected when power on,
                            // call the related routine.
                            if (GPIO_PROD_HEADSET_DETECT_ID == id)
                            {
					            GPIO_HeadsetRecogPowerOn();
                            }
                        }
                    }

                    break;

                case PM_RISING_EDGE:
                    HAL_SetGPIOInterruptSense (cfg_ptr->gpio_num, GPIO_INT_EDGES_RISING);
                    break;

                case PM_FALLING_EDGE:
                    HAL_SetGPIOInterruptSense (cfg_ptr->gpio_num, GPIO_INT_EDGES_FALLING);
                    break;

                case PM_BOTH_EDGE:
                    HAL_SetGPIOInterruptSense (cfg_ptr->gpio_num, GPIO_INT_EDGES_BOTH);
                    break;

                default:
                    //SCI_TRACE_LOW:"GPIO_PROD_RegGpio - INT type error!"
                    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1122_112_2_18_0_33_3_1526,(uint8*)"");
                    return SCI_FALSE;
            }

            HAL_AddGPIOToCallbackTable (cfg_ptr->gpio_num, shaking_en, shaking_interval,
                                        (GPIO_CALLBACK) gpio_callback_fun);
            break;

        case GPIO_PROD_TYPE_EXT:
            GPIO_EXT_RegGpio (cfg_ptr->gpio_num, direction, default_value,gpio_callback_fun);
            break;
#if defined(PLATFORM_SC6530) || defined(PLATFORM_SC8501C) || defined(PLATFORM_SC7702)
		case GPIO_PROD_TYPE_EIC_DBNC:

			if (EIC_HAL_GetValue (cfg_ptr->gpio_num))
                    {
  			EIC_DBNC_RegCallback(cfg_ptr->gpio_num,SCI_FALSE,shaking_en,shaking_interval,gpio_callback_fun);

                        if (cfg_ptr->valid_level)
                        {
                            // In case of headset in is detected when power on,
                            // call the related routine.
                            if (GPIO_PROD_HEADSET_DETECT_ID == id)
                            {
					            GPIO_HeadsetRecogPowerOn();
                            }
                        }
                    }
                    else
                    {
			EIC_DBNC_RegCallback(cfg_ptr->gpio_num,SCI_TRUE,shaking_en,shaking_interval,gpio_callback_fun);

                        if (!cfg_ptr->valid_level)
                        {
                            // In case of headset in is detected when power on,
                            // call the related routine.
                            if (GPIO_PROD_HEADSET_DETECT_ID == id)
                            {
					            GPIO_HeadsetRecogPowerOn();
                            }
                        }
                    }
			break;
#endif
#if defined(PLATFORM_UMS9117)
		case GPIO_PROD_TYPE_EICA_DBNC:

	             SCI_TRACE_LOW(" GPIO_PROD_RegGpio EICA_DBNC  num=%d EICA_HAL_GetValue=%d ",cfg_ptr->gpio_num,EICA_HAL_GetValue (cfg_ptr->gpio_num));
			if (EICA_HAL_GetValue (cfg_ptr->gpio_num))
                    {
  			    EICA_DBNC_RegCallback(cfg_ptr->gpio_num,GPIO_INT_LEVEL_LOW,shaking_en,shaking_interval,gpio_callback_fun);

                        if (cfg_ptr->valid_level)
                        {
                            // In case of headset in is detected when power on,
                            // call the related routine.
                            if (GPIO_PROD_HEADSET_DETECT_ID == id)
                            {
				    //GPIO_HeadsetRecogPowerOn();
				        s_headset_ctl.status = HEADSET_CONNECT;
                                    s_headset_ctl.time   = SCI_GetTickCount();
                            }
                        }
                    }
                    else
                    {

			    EICA_DBNC_RegCallback(cfg_ptr->gpio_num,GPIO_INT_LEVEL_HIGH,shaking_en,shaking_interval,gpio_callback_fun);

                        if (!cfg_ptr->valid_level)
                        {
                            // In case of headset in is detected when power on,
                            // call the related routine.
                            if (GPIO_PROD_HEADSET_DETECT_ID == id)
                            {
				    //GPIO_HeadsetRecogPowerOn();
			            s_headset_ctl.status = HEADSET_CONNECT;
                                s_headset_ctl.time   = SCI_GetTickCount();
                            }
                        }
                    }
                    if(GPIO_PROD_HEADSET_DETECT_ID == id)
                    {
                        __sprd_codec_headset_set_connection_status(s_headset_ctl.status);
                        if(HEADSET_CONNECT==s_headset_ctl.status)
                        {
                            __sprd_codec_headset_type_detect();
                        }
                    }
			break;
#endif
        default :
            // Do not use assert here, because the LCD is still not ready to
            // display assert information at the start of system init.
            //SCI_TRACE_LOW:"_GPIO_PROD_RegGpio : type %d - invalid"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1173_112_2_18_0_33_3_1527,(uint8*)"d", cfg_ptr->gpio_type);
    }

    return SCI_TRUE;

}

/*****************************************************************************/
//  Description:    This function is used to get gpio product information
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_PROD_InitCfgTable (void)
{
    uint32 i = 0;
    GPIO_PROD_ID_E      gpio_id        = GPIO_PROD_ID_MAX;
    GPIO_CFG_INFO_T_PTR cus_table_ptr  = GPIO_CFG_GetCusTable();
    GPIO_CFG_INFO_T_PTR full_table_ptr = s_gpio_prod_cfg_full_table;

    //SCI_TRACE_LOW:"_GPIO_PROD_InitCfgTable"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1190_112_2_18_0_33_3_1528,(uint8*)"");

    for (i = 0; i < GPIO_PROD_ID_MAX; i++)
    {
        full_table_ptr[i].gpio_id  = GPIO_PROD_ID_MAX;
        full_table_ptr[i].gpio_num = GPIO_PROD_NUM_INVALID;
    }

    for (i = 0; i < GPIO_PROD_ID_MAX; i++)
    {
        gpio_id = cus_table_ptr[i].gpio_id;

        if (GPIO_PROD_ID_MAX != gpio_id)
        {
            // unused, can fill the cfg information to full table, or else, assert
            if (GPIO_PROD_ID_MAX == full_table_ptr[gpio_id].gpio_id)
            {
                SCI_MEMCPY (&full_table_ptr[gpio_id], &cus_table_ptr[i], sizeof (GPIO_CFG_INFO_T));
            }
            else
            {
                SCI_PASSERT (0, ("GPIO full table %d line has been used !!", gpio_id));   /*assert verified*/
            }

            if (i == (GPIO_PROD_ID_MAX - 1))
            {
                SCI_PASSERT (0, ("GPIO cus cfg table has not end flag !!"));   /*assert verified*/
            }

        }
        else
        {
            break;
        }

    }

    // About Extend GPIO
    GPIO_EXT_InitCfgTab();
}

/*****************************************************************************/
//  Description:    This function is used to get gpio product information
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC GPIO_CFG_INFO_T_PTR GPIO_PROD_GetCfgInfo (GPIO_PROD_ID_E id)
{
    GPIO_CFG_INFO_T_PTR full_table_ptr = s_gpio_prod_cfg_full_table;

    SCI_ASSERT (id < GPIO_PROD_ID_MAX);/*assert verified*/

    if (full_table_ptr[id].gpio_id == id)
    {
        // Check if there is the valid hardware GPIO
        if ( (GPIO_PROD_NUM_INVALID != full_table_ptr[id].gpio_num)
                && (GPIO_PROD_TYPE_MAX  != full_table_ptr[id].gpio_type))
        {
            return (GPIO_CFG_INFO_T_PTR) &full_table_ptr[id];
        }
        // Check if there is callback function
        else if (PNULL != full_table_ptr[id].gpio_cb_fun)
        {
            return (GPIO_CFG_INFO_T_PTR) &full_table_ptr[id];
        }
        else
        {
            return PNULL;
        }
    }
    else
    {
        //SCI_PASSERT(id != 22, ("GET: Gpio prod id err %d", id));
        //SCI_TRACE_LOW("GPIO_PROD_GetCfgInfo: Gpio prod id err %d", id);
    }

    return PNULL;
}

/*****************************************************************************/
//  Description:    This function is used to set gpio product information
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_PROD_SetCfgInfo (GPIO_CFG_INFO_T_PTR cfg_info_ptr)
{
    GPIO_CFG_INFO_T_PTR full_table_ptr = s_gpio_prod_cfg_full_table;

    SCI_ASSERT (PNULL != cfg_info_ptr);/*assert verified*/
    SCI_ASSERT (cfg_info_ptr->gpio_id < GPIO_PROD_ID_MAX);/*assert verified*/

    if (full_table_ptr[cfg_info_ptr->gpio_id].gpio_id == cfg_info_ptr->gpio_id)
    {
        //SCI_TRACE_LOW:"GPIO_PROD_SetCfgInfo: Gpio prod id %d be Modified"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1279_112_2_18_0_33_3_1529,(uint8*)"d", cfg_info_ptr->gpio_id);
    }
    else
    {
        //SCI_TRACE_LOW:"GPIO_PROD_SetCfgInfo: Gpio prod id %d Create"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1283_112_2_18_0_33_3_1530,(uint8*)"d", cfg_info_ptr->gpio_id);
    }

    SCI_MEMCPY (&full_table_ptr[cfg_info_ptr->gpio_id], cfg_info_ptr, sizeof (GPIO_CFG_INFO_T));

    return SCI_TRUE;
}


/*****************************************************************************/
//  Description:    This function turn on/off the KeyPad backlight
//                  b_on = SCI_TRUE, turn on
//                  b_on = SCI_FALSE, turn off
//  Author:         Zhemin.Lin
//  Note:           KeyPad backlight is controlled by GPIO13 on SM5100B EVB1.0
/*****************************************************************************/
PUBLIC void GPIO_SetKeyPadBackLight (BOOLEAN b_on)
{
    //SCI_TRACE_LOW:"GPIO_SetKeyPadBackLight: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1299_112_2_18_0_33_3_1531,(uint8*)"d", b_on);

    _GPIO_PROD_SetVal (GPIO_PROD_KEYPAD_BL_ID, b_on);
#ifdef JTAG_SUPPORT   //only for IORA test!
	SCI_SLEEP(50);
	KeyPadBackLight_onoff = b_on;
#endif
}

/*****************************************************************************/
//  Description:    This function turn on/off the LCD1 backlight
//                  b_on = SCI_TRUE, turn on
//                  b_on = SCI_FALSE, turn off
//  Author:         Zhemin.Lin
//  Note:           LCD1 backlight is controlled by GPIO7 on SM5100B EVB1.0
/*****************************************************************************/
PUBLIC void GPIO_SetLcdBackLight (BOOLEAN b_on)
{
#ifndef MODEM_PLATFORM
    //SCI_TRACE_LOW:"GPIO_SetLcdBackLight: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1313_112_2_18_0_33_3_1532,(uint8*)"d", b_on);

    if (b_on)
    {
#ifdef BT_DIALER_BOARD
    	SCI_SLEEP(50);
#endif
        OS_TickDelay (20);
        LCD_SetBackLightBrightness (50);
        BOOT_SetBLStatus(1);  //set bk flag to be 1
    }
    else
    {
        LCD_SetBackLightBrightness (0);
        BOOT_SetBLStatus(0);  //clear bk flag
    }

    _GPIO_PROD_SetVal (GPIO_PROD_LCD_BL_EN_ID, b_on);

    SCI_LCDBacklightBrightless_EnableDeepSleep ( (uint32) !b_on);
#endif
}


/*****************************************************************************/
//  Description:    This function control lcd backlight
//                  is_high = SCI_TRUE, gpio in high level
//                  is_high = SCI_FALSE, gpio in low level
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetLcdBackLightness (BOOLEAN is_high)
{
    _GPIO_PROD_SetVal (GPIO_PROD_LCD_BL_BRIGHTNESS_ID, is_high);
}
/*****************************************************************************/
//  Description:    This function set BackLight.
//                  b_light = SCI_TRUE   turn on LED
//                  b_light = SCI_FALSE  turn off LED
//  Author:         Xueliang.Wang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetBackLight (BOOLEAN b_light)
{
    //SCI_TRACE_LOW:"GPIO_SetBackLight: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1350_112_2_18_0_33_3_1533,(uint8*)"d", b_light);
    GPIO_SetLcdBackLight (b_light);
    GPIO_SetKeyPadBackLight (b_light);
}

/*****************************************************************************/
//  Description:    This function set the state of vibrator.
//
//  Input      :
//      b_on   : If true, start vibrate. If false, stop it.
//  Return     : None
//  Author     : Lin.liu
//  Note       :
/*****************************************************************************/
PUBLIC void GPIO_SetVibrator (BOOLEAN b_on)
{
    //SCI_TRACE_LOW:"GPIO_SetVibrator: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1364_112_2_18_0_33_3_1534,(uint8*)"d", b_on);

    _GPIO_PROD_SetVal (GPIO_PROD_VIBRATIOR_EN_ID,  b_on);
}

LOCAL SCI_TIMER_PTR  S_SetVibirator_Timer = PNULL;
LOCAL void GPIO_Vibrator_Timer_Callback(uint32 para)
{
    if(S_SetVibirator_Timer)
    {
       SCI_DeactiveTimer(S_SetVibirator_Timer);
    }

    ANA_SetDevValule(ANA_DEV_ID_VIBRATOR, ANA_DEV_VAL_MIN);
}
/*****************************************************************************/
//  Description:    This function set the state of vibrator.
//
//  Input      :
//      b_on   : If true, start vibrate. If false, stop it.
//      on_period   : unit: ms, only for TURN ON, on_period == 0, it means VIB always ON without timer
//                    should be Turn off later
//      on_stress   : reserved for future use
//      reserved    : reserved for future use
//  Note       :
/*****************************************************************************/
PUBLIC void GPIO_SetVibrator_Ext(BOOLEAN b_on, uint32 on_period, uint32 on_stress, uint32 *reserved)
{
    //SCI_TRACE_LOW:"GPIO_SetVibrator_Ext: on = %d, period = %d, stress = %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1390_112_2_18_0_33_3_1535,(uint8*)"ddd", b_on, on_period, on_stress);
   // set Timer at b_on and on_period !=0, if on_period == 0, it means Vib=1 always without timer
   if(b_on && on_period)
   {
        if(PNULL == S_SetVibirator_Timer)
        {
            S_SetVibirator_Timer = SCI_CreateTimer("SetVibrator_Timer",
            GPIO_Vibrator_Timer_Callback, NULL, on_period, SCI_AUTO_ACTIVATE);
    	}
    	else
    	{
    		SCI_ChangeTimer(S_SetVibirator_Timer, GPIO_Vibrator_Timer_Callback, on_period);
    		SCI_ActiveTimer(S_SetVibirator_Timer);
    	}
    }
	else
	{
        if(S_SetVibirator_Timer)
        {
            SCI_DeactiveTimer(S_SetVibirator_Timer);
        }
	}

    // set VIBRATOR
    GPIO_SetVibrator(b_on);

    return;
}
/*****************************************************************************/
//  Description:    This function query the state of Flip
//                  return value = SCI_TRUE, flip is open
//                  return value = SCI_FALSE, flip is close
//  Author:         Zhemin.Lin
//  Note:
//                  Lin.liu.   don't use this file. (only for compatible with prev version)
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_GetFlipState (void)
{
    BOOLEAN  status = SCI_TRUE;

    _GPIO_PROD_GetVal (GPIO_PROD_FLIP_ON_ID, &status);

    //SCI_TRACE_LOW:"GPIO_GetFlipState: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1435_112_2_18_0_33_3_1536,(uint8*)"d", status);

    return status;
}
#if 0
PUBLIC EARJACK_TYPE GPIO_GetHeadsetTypeStatus(void)
{
    uint32 status_type = 0;

#ifdef GPIO_HEADSET_3_4_POLE_DETECT

    if(ADC_GetJIGConnectStatus()== ADC_JIG_UART)
    {
        return EAR_ADC_4_POLE;
    }
#ifdef HEADSET_ADC_CFG
    status_type = ADC_CheckHeadsetStatus();
#else
    status_type = GetHeadsetStatusVoltage();
#endif

    SCI_TRACE_LOW("GPIO_GetHeadsetTypeStatus %d\n", status_type);

    if((status_type >= 0) && (status_type <= KEY_3POLE_MAX_VOL))// 3_pole
    {
        SCI_TRACE_LOW("GPIO_Status 3_POLE\n");
        return EAR_ADC_3_POLE;
    }
    else if(status_type >=KEY_RELEASE_MIN_VOL && status_type <= KEY_RELEASE_MAX_VOL)// 4_pole
    {
        SCI_TRACE_LOW("GPIO_Status 4_POLE\n");
        return EAR_ADC_4_POLE;
    }
    else
       return EAR_ADC_ERROR;

#else
        return EAR_ADC_4_POLE;
#endif
}
#endif

PUBLIC EARJACK_TYPE GPIO_GetHeadsetTypeStatus(void)
{
    BOOLEAN gpio_val = SCI_FALSE;

    if (!_GPIO_PROD_GetVal (GPIO_PROD_HEADSET_DETECT_ID, &gpio_val))
    {
        return;
    }

    if ((HEADSET_CONNECT == s_headset_ctl.status) &&
    /*(SCI_GetTickCount() > (s_headset_ctl.time + HEADSET_POLETYPE_SHAKING_TIME)) &&  */
    gpio_val)
    {
        SCI_TRACE_LOW("GPIO_GetHeadsetTypeStatus: headset = %d, gpio_val =%d var_pole_type = %d",
                      s_headset_ctl.status,  gpio_val, var_pole_type);
        return var_pole_type;

    }else{
        var_pole_type = EAR_ADC_0_POLE;
    }

    SCI_TRACE_LOW("GPIO_GetHeadsetTypeStatus 1: headset = %d, gpio_val =%d var_pole_type = %d",
                      s_headset_ctl.status,  gpio_val, var_pole_type);

    return var_pole_type;

}
/*****************************************************************************/
//  Description:    This function check the headset detect pin status.
//
//  Input      : None
//  Return     :
//      True   : the headset is in.
//      False  : the headset is out.
//  Author     : Lin.liu
//  Note       : When call this function, make sure the gpio has been config correctly.
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_CheckHeadsetStatus (void)
{
    BOOLEAN  status = 0;
    uint32 adc_value;
    _GPIO_PROD_GetVal (GPIO_PROD_HEADSET_DETECT_ID, &status);
//if ((HEADSET_CONNECT == s_headset_ctl.status) &&
            //((tick > (s_headset_ctl.time + MAX_HEADSET_BUTTON_READY_TIME)) ||
            //((tick > (s_headset_ctl.time + MAX_HEADSET_BUTTON_READY_TIME_FOR_4POLE)) && (GPIO_GetHeadsetTypeStatus() == EAR_ADC_4_POLE))))

#if 0
    if( (adc_water_value > KEY_WATER_BOTTOM) && (ADC_GetJIGConnectStatus()!=ADC_JIG_UART) )
    {
        SCI_TRACE_LOW("headset water: %d %d\n", adc_water_value, status);
        status = SCI_FALSE;
    }
#endif
    //SCI_TRACE_LOW:"GPIO_CheckHeadsetStatus: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1455_112_2_18_0_33_3_1537,(uint8*)"d", status);

    return (status);
}


/*****************************************************************************/
//  Description:    This function check the headset button detect pin status.
//
//  Input      : None
//  Return     :
//      True   : the headset button is pressed.
//      False  : the headset isn't pressed.
//  Author     : Lin.liu
//	Note       : When call this function, make sure the gpio has been config correctly.
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_CheckHeadsetButtonStatus( void )
{
	BOOLEAN status=0;
    _GPIO_PROD_GetVal (GPIO_PROD_HEADSET_BUTTON_ID, &status);

    SCI_TRACE_LOW("GPIO_CheckHeadseButtontStatus: %d", status);

    return (status);
}



/*****************************************************************************/
//  Description:    This function check the SDCard detect pin status.
//
//  Input      : None
//  Return     :
//      True   : the SDCard is in.
//      False  : the SDCard is out.
//  Author     : yuehz
//  Note       : 20060410;when call this function, make sure the gpio has been config correctly.
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_CheckSDCardStatus (void)
{
    BOOLEAN  status = SCI_TRUE;

    _GPIO_PROD_GetVal (GPIO_PROD_SDCARD_DETECT_ID, &status);
    //SCI_TRACE_LOW:"GPIO_CheckSDCardStatus: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1473_112_2_18_0_33_3_1538,(uint8*)"d", status);

    return status;
}

/*****************************************************************************/
//  Description:    This function set enable/disable amplifier.
//                  b_enable = SCI_TRUE   Enable amplifier
//                  b_enable = SCI_FALSE  Disable amplifier
//  Author:         Xueliang.Wang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_EnableAmplifier (BOOLEAN b_enable)
{
    if (_GPIO_Check_Amplifier_Share())
    {
        _GPIO_Control_Amplifier_Shared (SCI_FALSE,b_enable);
    }
    else
    {
        _GPIO_PROD_SetVal (GPIO_PROD_SPEAKER_PA_EN_ID, b_enable);
    }
}

/*****************************************************************************/
//  Description:    This function set enable/disable amplifier.
//                  b_enable = SCI_TRUE   Enable amplifier
//                  b_enable = SCI_FALSE  Disable amplifier
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_EnableHeadsetAmplifier (BOOLEAN b_enable)
{
    if (_GPIO_Check_Amplifier_Share())
    {
        _GPIO_Control_Amplifier_Shared (SCI_TRUE,b_enable);
    }
    else
    {
        _GPIO_PROD_SetVal (GPIO_PROD_HEADSET_PA_EN_ID, b_enable);
    }
}

/*****************************************************************************/
//  Description:    This function turn on/off the Flash .
//                  b_on = SCI_TRUE, turn on
//                  b_on = SCI_FALSE, turn off
//  Author:         Zhemin.Lin
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetFlash (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal (GPIO_PROD_FLIP_ON_ID, b_on);
}

/*****************************************************************************/
//  Description:    This function set VCM enable.
//                  b_on = SCI_TRUE, High
//                  b_on = SCI_FALSE, Low
//  Author:        Hansen.sun
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetVCMEnable (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal (GPIO_PROD_AF_VCM_CTL_ID, b_on);

}

/*****************************************************************************/
//  Description:    This function turn on/off the Flash .
//                  b_on = SCI_TRUE, turn on
//                  b_on = SCI_FALSE, turn off
//  Author:         Tim.zhu
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetDCFlash (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal (GPIO_PROD_DC_FLASH_ID, b_on);
}

/*****************************************************************************/
//  Description:    This function set sensor reset signal .
//                  pulse_level = SCI_TRUE, High
//                  pulse_level = SCI_FALSE, Low
//  Author:         Benny.Zou
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_ResetSensor (
    BOOLEAN pulse_level,
    uint32  pulse_width
)
{
    _GPIO_PROD_SetVal (GPIO_PROD_SENSOR_RESET_ID, (BOOLEAN) !pulse_level);
    SCI_Sleep (10);
    _GPIO_PROD_SetVal (GPIO_PROD_SENSOR_RESET_ID, pulse_level);
    SCI_Sleep (pulse_width);
    _GPIO_PROD_SetVal (GPIO_PROD_SENSOR_RESET_ID, (BOOLEAN) !pulse_level);
    SCI_Sleep (10);

}
/*****************************************************************************/
//  Description:    This function is used to control the sensor power if the
//                  the sensor is enabled by a gpio.
//                  b_on = SCI_TRUE, open sensor power
//                  b_on = SCI_FALSE, close sensor power
//  Author:         junyi.lv
//  Note:
/*****************************************************************************/

PUBLIC void GPIO_SetSensorPower (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal (GPIO_PROD_SENSOR_POWER_ID, b_on);
}

/*****************************************************************************/
//  Description:    This function set sensor pwdn .
//                  b_on = SCI_TRUE, High
//                  b_on = SCI_FALSE, Low
//  Author:         Benny.Zou
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetSensorPwdn (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal (GPIO_PROD_SENSOR_PWDN_ID, b_on);
}

/*****************************************************************************/
//  Description:    This function set front sensor pwdn .
//                  b_on = SCI_TRUE, High
//                  b_on = SCI_FALSE, Low
//  Author:         Tim.zhu
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetFrontSensorPwdn (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal (GPIO_PROD_SENSOR_PWDN_FRONT_ID, b_on);

}

/*****************************************************************************/
//  Description:    This function do sensor reset.
//                  b_on = SCI_TRUE, High
//                  b_on = SCI_FALSE, Low
//  Author:         Benny.Zou
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetMainSensorReset (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal (GPIO_PROD_SENSOR_RESET_ID, b_on);
}

PUBLIC void GPIO_SetFrontSensorReset (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal (GPIO_PROD_SENSOR_RESET_FRONT_ID, b_on);
}

/*****************************************************************************/
//  Description:    This function select mic, .
//                  pulse_level = SCI_TRUE, High, Select Sub Mic
//                  pulse_level = SCI_FALSE, Low, Select Main Mic
//  Author:         Huitao.Yue
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_MicSwitch(BOOLEAN pulse_level)
{
    _GPIO_PROD_SetVal(GPIO_PROD_MIC_SEL_ID, pulse_level);
}
/*****************************************************************************/
//  Description:    This function set sensor reset signal .
//                  pulse_level = SCI_TRUE, High
//                  pulse_level = SCI_FALSE, Low
//  Author:         Tim.zhu
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetSensorResetLevel (BOOLEAN pulse_level)
{
    _GPIO_PROD_SetVal (GPIO_PROD_SENSOR_RESET_ID, pulse_level);
}
/*****************************************************************************/
//  Description:    This function set the state of flash light.
//
//  Note       :
/*****************************************************************************/
PUBLIC void GPIO_SetFlashLight(BOOLEAN b_on)
{
// GPIO_PROD_FLASH_EN_ID
    SCI_TRACE_LOW("GPIO_SetFlashLight: on = %d", b_on);
    _GPIO_PROD_SetVal (GPIO_PROD_FLASH_EN_ID, b_on);

    return;
}
/*****************************************************************************/
//  Description:    This function set analog tv reset signal .
//                  pulse_level = SCI_TRUE, High
//                  pulse_level = SCI_FALSE, Low
//  Author:         Tim.zhu
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetAnalogTVResetLevel (BOOLEAN pulse_level)
{
    _GPIO_PROD_SetVal (GPIO_PROD_ATV_RESET_ID, pulse_level);
}

/*****************************************************************************/
//  Description:    This function get charge interrupt gpio number
//  Author:         Benny.Zou
//  Note:
/*****************************************************************************/
PUBLIC uint32 GPIO_GetChargeIntGpio (void)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_CHARGE_PLUG_DETECT_ID);

    if (PNULL != cfg_ptr)
    {
        return cfg_ptr->gpio_num;
    }
    else
    {
        return 0xffffffff;
    }
}

/*****************************************************************************/
//  Description:    This function enable SPI CS select SD
//
//  Author:        yuehz
//  Note:           20060512 b_level :0 enable;1:disable
/*****************************************************************************/
PUBLIC void GPIO_SetSDCS (BOOLEAN b_enable)
{
    _GPIO_PROD_SetVal (GPIO_PROD_SDCARD_CS_ID,b_enable);
}

/*****************************************************************************/
//  Description:    This function selects headset mic or tv out in DVB
//                       gpio_state=Low:headset mic;High: TV Out
//  Author:        yuehz
//  Note:           20060628
/*****************************************************************************/
PUBLIC void GPIO_Sel_MIC_TVOut (BOOLEAN is_to_tv)
{
    _GPIO_PROD_SetVal (GPIO_PROD_TV_MIC_SW_ID, is_to_tv);
}

/*****************************************************************************/
//  Description:    This function Open PA or not
//
//  Author:        yuehz
//  Note:           20060627
/*****************************************************************************/
PUBLIC void GPIO_OpenFMPA (BOOLEAN is_open)
{
    ;
}

/*****************************************************************************/
//  Description:    This function Open FM short Ant
//
//  Author:
//  Note:           20191127
/*****************************************************************************/
PUBLIC void GPIO_SetFmLNA(BOOLEAN is_high)
{
#if defined (FM_SUPPORT) && defined (FM_S_ANT_SUPPORT)
#ifdef PLATFORM_UMS9117
	_GPIO_PROD_SetVal (GPIO_PROD_FMLNA_ID, is_high);//NemoL GPIO90
#else
	_GPIO_PROD_SetVal (GPIO_PROD_FMLNA_ID, is_high);//NemoL+Marlin2 GPIO113
#endif
#endif
}

/*****************************************************************************/
//  Description:    This function set GPIO which controls the motor
//
//  Author:        Jianping.wang
//  Note:
/*****************************************************************************/
void GPIO_MOTOR_SetPortValue (
    uint16 ic_num,      // define in gpio_dvb_ext.h
    uint16 gpio_mask,   // when the related bit is '1', use value of related bit in gpio_value to set gpio output value
    uint16 gpio_value   // represent 16 gpio value
)
{
    //GPIO_EXT_SetPortValue(ic_num, gpio_mask, gpio_value) ;
    ;
}


/*****************************************************************************/
//  Description:    This function control SD CARD Power ;
//                  is_open = SCI_TRUE,     Low
//                  is_open = SCI_FALSE,    High
//  Author:         Liangwen.zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_OpenSDPower (BOOLEAN is_open)
{
    _GPIO_PROD_SetVal (GPIO_PROD_SDCARD_PWR_ID, is_open);
}

/*****************************************************************************/
//  Description:    This function detect usb or adapter ,
//  then config the GPIO as GPIO input.
//  before call this function,you must config the gpio input in pinmap_**.c.
//  Author:         junyi.lv
//  Note:
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_DetectUsbOrAdapter (void)
{
    BOOLEAN  status = SCI_TRUE;

    _GPIO_PROD_GetVal (GPIO_PROD_USB_DETECT_ID, &status);

    return status;
}


/*****************************************************************************/
//  Description:    This function set TochLight pin status
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetTorchLight(BOOLEAN is_open)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_TORCH_EN_ID);
    if(cfg_ptr != PNULL)
    {
        _GPIO_PROD_SetVal(GPIO_PROD_TORCH_EN_ID, is_open);
    }

}

#if defined(TORCH_SUPPORT)
LOCAL BOOLEAN s_torch_is_on = FALSE;

PUBLIC void GPIO_SetTorch (BOOLEAN b_on)
{
    _GPIO_PROD_SetVal (GPIO_PROD_FLASH_EN_ID,  b_on);
    s_torch_is_on = b_on;
}

PUBLIC BOOLEAN GPIO_GetTorchStatus( void )
{           	
    return s_torch_is_on;
}

PUBLIC void GPIO_SetTorchStatus( BOOLEAN b_on )
{           	
    s_torch_is_on = b_on;
}
#endif
/*****************************************************************************/
//  Description:    This function select spi2 extend cs

//  Author:         Liangwen.zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SelectSPI2EXTCS (uint32 cs_num)
{
    //SCI_TRACE_LOW:"GPIO_SelectSPI2CS: cs_num = %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1718_112_2_18_0_33_4_1539,(uint8*)"d", cs_num);

    SCI_ASSERT (cs_num < 8);/*assert verified*/

    if (cs_num&0x1)
    {
        _GPIO_PROD_SetVal (GPIO_PROD_SPI2_EXT_CS_SEL0_ID,SCI_TRUE);
    }
    else
    {
        _GPIO_PROD_SetVal (GPIO_PROD_SPI2_EXT_CS_SEL0_ID,SCI_FALSE);
    }

    if (cs_num&0x2)
    {
        _GPIO_PROD_SetVal (GPIO_PROD_SPI2_EXT_CS_SEL1_ID,SCI_TRUE);
    }
    else
    {
        _GPIO_PROD_SetVal (GPIO_PROD_SPI2_EXT_CS_SEL1_ID,SCI_FALSE);
    }

    if (cs_num&0x4)
    {
        _GPIO_PROD_SetVal (GPIO_PROD_SPI2_EXT_CS_SEL2_ID,SCI_TRUE);
    }
    else
    {
        _GPIO_PROD_SetVal (GPIO_PROD_SPI2_EXT_CS_SEL2_ID,SCI_FALSE);
    }
}

/*****************************************************************************/
//  Description:    This function set RF transceiver reset GPIO output high,
//  then config the GPIO as GPIO input.
//  before call this function,you must config the gpio output low in pinmap_**.c.
//  Author:         Younger.yang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_ConfigRfReset (void)
{
    // OUTPUT
    _GPIO_PROD_SetDirection (GPIO_PROD_RF_RESET_ID, SCI_TRUE);

    if (_GPIO_PROD_SetVal (GPIO_PROD_RF_RESET_ID, SCI_FALSE))
    {
        OS_TickDelay (1);
    }
    else
    {
        // Because  the RF-reset GPIO is un-register, so don't need to continue
        return;
    }

    _GPIO_PROD_SetVal (GPIO_PROD_RF_RESET_ID, SCI_TRUE);

    // INPUT
    _GPIO_PROD_SetDirection (GPIO_PROD_RF_RESET_ID, SCI_FALSE);
}

/*****************************************************************************/
//  Description:    This function set BT set pin status
//  then config the GPIO as GPIO input.
//  before call this function,you must config the gpio output low in pinmap_**.c.
//  Author:         Liangwen.zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetBtReset (BOOLEAN is_high)
{
    _GPIO_PROD_SetVal (GPIO_PROD_BT_RESET_ID, is_high);
}

/******************************************************************************/
// Description:   PULL UP/DOWN RESET PIN TO FORCE WIFI TO ENTER POWERDOWN MODE
// Dependence:
// Author:        Haifeng.Yang
// Note:
/******************************************************************************/
PUBLIC void GPIO_SetWifiPowerDown (BOOLEAN is_powerdown)
{
    _GPIO_PROD_SetVal(GPIO_PROD_WIFI_PWD_ID, is_powerdown);
}

/*****************************************************************************/
//  Description:    wifi interrupt handler function.
//  Author:         Liangwen.Zhen
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_WiFiIntHander (uint32 gpio_id, uint32 gpio_state)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_WIFI_INT_ID);

    SCI_TRACE_LOW("GPIO_WiFiIntHander:gpio_id=%d, gpio_state=%d", gpio_id, gpio_state);

    if (PNULL != cfg_ptr)
    {
        if (gpio_state)
        {
            _GPIO_PROD_SetInterruptSense(GPIO_PROD_WIFI_INT_ID, GPIO_INT_LEVEL_LOW);
        }
        else
        {
            _GPIO_PROD_SetInterruptSense(GPIO_PROD_WIFI_INT_ID, GPIO_INT_LEVEL_HIGH);
        }

        if (cfg_ptr->gpio_cb_fun && (cfg_ptr->valid_level == gpio_state))
        {
            cfg_ptr->gpio_cb_fun(0);
        }

    }
}
/*****************************************************************************/
//  Description:    This function dot lcd reset
//  before call this function,you must config the gpio output low in pinmap_**.c.
//  Author:
//  Note:           pulse_level: pin level,
//                  delay_ms:  reset pulse time
/*****************************************************************************/
PUBLIC void GPIO_ResetLcd (BOOLEAN pulse_level, uint32  delay_ms)
{

    _GPIO_PROD_SetVal (GPIO_PROD_LCD_RST_ID, (BOOLEAN) !pulse_level);
    OS_TickDelay (delay_ms);
    _GPIO_PROD_SetVal (GPIO_PROD_LCD_RST_ID, pulse_level);
    OS_TickDelay (delay_ms);
    _GPIO_PROD_SetVal (GPIO_PROD_LCD_RST_ID, (BOOLEAN) !pulse_level);
    OS_TickDelay (delay_ms);

}
#ifdef KEYPAD_TYPE_QWERTY_KEYPAD
/*****************************************************************************/
//  Description:    This function is used to get the gpio data for expand key
//  Author:         hanjun.liu
//  Note:
/*****************************************************************************/
PUBLIC uint32  GPIO_GetExpandKeyHandler( void)
{
	GPIO_CFG_INFO_T_PTR cfg_ptr;
	uint32 i,gpio_state;

	for(i = GPIO_PROD_EXPAND_KEY0_ID;i <= GPIO_PROD_EXPAND_KEY4_ID;i++)
	{
		cfg_ptr = GPIO_PROD_GetCfgInfo(i);

		if(PNULL != cfg_ptr)
		{
			gpio_state = HAL_GetGPIOVal(cfg_ptr->gpio_num);
			SCI_TRACE_LOW("GPIO_GetExpandKeyHandler:gpio_num=%d,gpio_state=%d",cfg_ptr->gpio_num,gpio_state);
			if(cfg_ptr->valid_level == gpio_state)
			{
				return (i - GPIO_PROD_EXPAND_KEY0_ID);
			}
		}
	}
	return 0xFFFFFFFF;
}
#endif
#ifdef TOUCHPANEL_TYPE_MULTITP

/*****************************************************************************/
//  Name:           GPIO_ResetTP
//  Description:    This function set the interrupt pin. Because some tp chip
//                  supports to trigger manual.
/*****************************************************************************/
PUBLIC void GPIO_ResetTP(BOOLEAN is_on,uint32 delay_ms)
{
    _GPIO_PROD_SetVal (GPIO_PROD_TP_WAKE_ID,  is_on);
    OS_TickDelay (delay_ms);
    _GPIO_PROD_SetVal (GPIO_PROD_TP_WAKE_ID, (BOOLEAN) !is_on);
    OS_TickDelay (delay_ms);
    _GPIO_PROD_SetVal (GPIO_PROD_TP_WAKE_ID,  is_on);
    OS_TickDelay (delay_ms);
}
/*****************************************************************************/
//  Name:           GPIO_TPIRQCtrl
//  Description:    This function set the interrupt pin. Because some tp chip
//                  supports to trigger manual.
/*****************************************************************************/
PUBLIC void GPIO_TPIRQCtrl(BOOLEAN is_active)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo( GPIO_PROD_TP_INT_ID );
    if(PNULL != cfg_ptr )
    {
        if(is_active)
        {
            GPIO_EnableIntCtl(cfg_ptr->gpio_num);
        }
        else
        {
            GPIO_DisableIntCtl(cfg_ptr->gpio_num);
        }
    }

}
/*****************************************************************************/
//  Name:           GPIO_TPInterruptSense
//  Description:    This function set the interrupt pin. Because some tp chip
//                  supports to trigger manual.
/*****************************************************************************/
PUBLIC void GPIO_TPInterruptSense(uint32 status)
{
    if(status)
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_TP_INT_ID, GPIO_INT_LEVEL_HIGH);
    }
    else
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_TP_INT_ID, GPIO_INT_LEVEL_LOW);
    }
}

/*****************************************************************************/
//  Name:           GPIO_TPInterruptPin
//  Description:    This function set the interrupt pin. Because some tp chip
//                  supports to trigger manual.
/*****************************************************************************/
PUBLIC uint16 GPIO_TPInterruptPin(void)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo( GPIO_PROD_TP_INT_ID );
    if(PNULL != cfg_ptr )
    {
        return cfg_ptr->gpio_num;
    }
    return 0;
}

/*****************************************************************************/
//  Name:           GPIO_TPGetInterruptStatus
//  Description:    This function get valid interrupt level.
/*****************************************************************************/
PUBLIC uint16 GPIO_TPGetInterruptStatus(void)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo( GPIO_PROD_TP_INT_ID );
    if(PNULL != cfg_ptr )
    {
        return cfg_ptr->valid_level;
    }
    return 0;
}

#endif


PUBLIC void GPIO_SPIIRQCtrl(BOOLEAN is_active)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo( GPIO_PROD_WIFI_INT_ID );

    if(PNULL != cfg_ptr )
    {
         if(is_active)
        {
            GPIO_EnableIntCtl(cfg_ptr->gpio_num);
        }
        else
        {
            GPIO_DisableIntCtl(cfg_ptr->gpio_num);
        }
    }
}

/*****************************************************************************/
//  Description:    This function handle demod power
//  Author:
//  Note:           pulse_level
/*****************************************************************************/
PUBLIC void GPIO_DemodPower( BOOLEAN is_poweron )
{
#ifndef CMMB_WIFI_SHARE_SPI_SUPPORT
	_GPIO_PROD_SetVal(GPIO_PROD_DEMOD_POWER, is_poweron);
#endif
}
PUBLIC void CMMB_gpio_power_on(void)
{
#ifdef CMMB_WIFI_SHARE_SPI_SUPPORT
	_GPIO_PROD_SetVal(GPIO_PROD_DEMOD_POWER, 1);
#endif
}
PUBLIC void CMMB_gpio_power_off(void)
{
#ifdef CMMB_WIFI_SHARE_SPI_SUPPORT
	_GPIO_PROD_SetVal(GPIO_PROD_DEMOD_POWER, 0);
#endif
}

/*****************************************************************************/
//  Description:    This function handle demod reset
//  Author:
//  Note:           pulse_level
/*****************************************************************************/
PUBLIC void GPIO_DemodReset( BOOLEAN pulse_level )
{
	_GPIO_PROD_SetVal(GPIO_PROD_DEMOD_RESET, pulse_level);
}

/*****************************************************************************/
//  Description:    demod int handler
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_DemodIntHandler( uint32 gpio_id, uint32 gpio_state)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_DEMOD_INT);

    if (PNULL != cfg_ptr)
    {
#if defined(DEMOD_HW_SIANO) || defined(DEMOD_HW_INNOFIDEI)
           DemodSpiIsr(PNULL);
#endif
    }
}

#ifdef SIM_PLUG_IN_SUPPORT
/*****************************************************************************/
//  Description:    SIM card hot swap feature.
//  Input      :
//  Return     :    None
//  Author     :    wuding.yang
//	Note       :
/*****************************************************************************/
PUBLIC void GPIO_SIMIntHandler(uint32 gpio_id, uint32 gpio_state)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo(GPIO_PROD_SIM_PLUG_IN_ID);

    //SCI_TRACE_LOW:"GPIO_SIMIntHandler %d %d. %d %d %d %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_1997_112_2_18_0_33_5_1541,(uint8*)"dddddd",gpio_id,gpio_state,cfg_ptr->gpio_id,cfg_ptr->gpio_type,cfg_ptr->valid_level,cfg_ptr->gpio_num);

    if (gpio_state)
    {
     	   SCI_TraceLow("[debug]plug out\n");
          SIM2_HotswapGPIOCallback(FALSE);
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_SIM_PLUG_IN_ID, GPIO_INT_LEVEL_LOW);
    }
    else
    {
       	  SCI_TraceLow("[debug]plug in\n");
         SIM2_HotswapGPIOCallback(TRUE);
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_SIM_PLUG_IN_ID, GPIO_INT_LEVEL_HIGH);
    }

    if (PNULL != cfg_ptr)
    {
        //SCI_TRACE_LOW("GPIO_SIMIntHandler %d %d. %d %d %d %d",gpio_id,gpio_state,cfg_ptr->gpio_id,cfg_ptr->gpio_type,cfg_ptr->valid_level,cfg_ptr->gpio_num);
        if (NULL != cfg_ptr->gpio_cb_fun)
        {
            cfg_ptr->gpio_cb_fun((BOOLEAN)gpio_state);
        }
    }
}
#endif

/*****************************************************************************/
//  Description:    Get the GPIO Id which is connected with the hot swap sim slot.
//  Input      :
//  Return     :    None
//  Author     :    wuding.yang
//	Note       :
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_SIM_Hotswap_GetGPIOId(uint32 *gpio_id)
{
#ifdef SIM_PLUG_IN_SUPPORT
	GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo(GPIO_PROD_SIM_PLUG_IN_ID);
    if(cfg_ptr)
    {
        *gpio_id = cfg_ptr->gpio_num;
    	return TRUE;
    }
    else
    {
        return FALSE;
    }
#else
	return FALSE;//do not support sim hot swap
#endif
}

#ifdef GPIO_SIMULATE_SPI_SUPPORT
/*****************************************************************************/
//  Description:    Simulate a SPI by 3 GPIO, 2 SPI by 6 GPIO.
//  Author:        yun.wang
//  Note:           2010-11-23
/*****************************************************************************/
PUBLIC uint32 GPIO_SPI_id_get(GPIO_SPI_PIN_ID_E gpio_spi_pin)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = NULL ;
    switch(gpio_spi_pin)
    {
        case VIR_SPI0_CLK_PIN_ID:
        {
            cfg_ptr= GPIO_PROD_GetCfgInfo(GPIO_PROD_VIR_SPI0_CLK_ID);
            break;
        }
        case VIR_SPI0_DIN_PIN_ID:
        {
            cfg_ptr= GPIO_PROD_GetCfgInfo(GPIO_PROD_VIR_SPI0_DIN_ID);
            break;
        }
        case VIR_SPI0_DOUT_PIN_ID:
        {
            cfg_ptr= GPIO_PROD_GetCfgInfo(GPIO_PROD_VIR_SPI0_DOUT_ID);
            break;
        }
        case VIR_SPI0_CS_PIN_ID:
        {
            cfg_ptr= GPIO_PROD_GetCfgInfo(GPIO_PROD_VIR_SPI0_CS_ID);
            break;
        }
        case VIR_SPI1_CLK_PIN_ID:
        {
            cfg_ptr= GPIO_PROD_GetCfgInfo(GPIO_PROD_VIR_SPI1_CLK_ID);
            break;
        }
        case VIR_SPI1_DIN_PIN_ID:
        {
            cfg_ptr= GPIO_PROD_GetCfgInfo(GPIO_PROD_VIR_SPI1_DIN_ID);
            break;
        }
        case VIR_SPI1_DOUT_PIN_ID:
        {
            cfg_ptr= GPIO_PROD_GetCfgInfo(GPIO_PROD_VIR_SPI1_DOUT_ID);
            break;
        }
        case VIR_SPI1_CS_PIN_ID:
        {
            cfg_ptr= GPIO_PROD_GetCfgInfo(GPIO_PROD_VIR_SPI1_CS_ID);
            break;
        }
        default :
        //SCI_PASSERT(0, ("GPIO_SPI_id_get : gpio_spi_pin:%d", gpio_spi_pin));
        return GPIO_PROD_NUM_INVALID;
    }
    if(NULL==cfg_ptr)
    {
        //SCI_PASSERT(0, ("GPIO_SPI_id_get : gpio_spi_pin:%d", gpio_spi_pin));
        return GPIO_PROD_NUM_INVALID;
    }
    else
    {
        return cfg_ptr->gpio_num;
    }
}

#endif

#ifdef DPHONE_SUPPORT	// zhiguo.li_cr225830
//add by brezen
PUBLIC void GPIO_HookIntHandler(uint32 gpio_id, uint32 gpio_state)
{
    TB_MSG  gpio_msg;


    //SCI_TRACE_LOW:"GPIO_HookKeyIntHandler gpio_state(%d)"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_2113_112_2_18_0_33_5_1542,(uint8*)"d",gpio_state);
    if (gpio_state)
    {
        gpio_msg.message = TB_KPD_PRESSED;
        gpio_msg.wparam  = SCI_VK_HOOK;
        gpio_msg.lparam  = TB_NULL;
        HAL_SetGPIOInterruptSense(gpio_id, GPIO_INT_LEVEL_LOW);
    }
    else
    {
        gpio_msg.message = TB_KPD_RELEASED;
        gpio_msg.wparam  = SCI_VK_HOOK;
        gpio_msg.lparam  = TB_NULL;
        HAL_SetGPIOInterruptSense(gpio_id, GPIO_INT_LEVEL_HIGH);

    }
    DRV_Callback(TB_GPIO_INT,&gpio_msg);

}

PUBLIC BOOLEAN GPIO_GetHookState(void)
{
    return (!GPIO_GetGPIOState(37));
}

PUBLIC void GPIO_EnableHookInt(BOOLEAN b_enable)
{
	if (b_enable)
		GPIO_EnableIntCtl(37);
	else
    	GPIO_DisableIntCtl(37);

	//SCI_TRACE_LOW:"GPIO_EnableHookInt.........%d...."
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_2145_112_2_18_0_33_5_1543,(uint8*)"d", b_enable);
}

PUBLIC void GPIO_EnableHandFreeILed(BOOLEAN b_enable)
{
    if (b_enable)
       _GPIO_PROD_SetVal (GPIO_PROD_HANDFREE_ID, 0);
	else
       _GPIO_PROD_SetVal (GPIO_PROD_HANDFREE_ID, 1);

	//SCI_TRACE_LOW:"GPIO_EnableHandFreeILed.........%d...."
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_2155_112_2_18_0_33_5_1544,(uint8*)"d", b_enable);
}
#endif
PUBLIC void GPIO_USBResumeHandler (uint32 gpio_id, uint32 gpio_state)
{
	  GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo(GPIO_USB_RESUME_ID);


	  if (PNULL != cfg_ptr)
	  {
		    if (NULL != cfg_ptr->gpio_cb_fun)
		    {
			      cfg_ptr->gpio_cb_fun((BOOLEAN)gpio_state);
		    }
	  }
    if (gpio_state)
    {
        _GPIO_PROD_SetInterruptSense (GPIO_USB_RESUME_ID, GPIO_INT_LEVEL_LOW); //plug out det
    }
    else
    {
        _GPIO_PROD_SetInterruptSense (GPIO_USB_RESUME_ID, GPIO_INT_LEVEL_HIGH); //plug in det
    }
}
#ifdef MOTION_SENSOR_TYPE_OFN
/*****************************************************************************/
//  Description:    This function is used to handler ofn-sensor interrupt
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_OFNIntHandler (uint32 gpio_id, uint32 gpio_state)
{
    //Because Reading registers via IIC is very slow, so it CANNOT be done
    //in interrupt context.
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo (GPIO_PROD_OFN_INT_ID);

    if (gpio_state)
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_OFN_INT_ID, GPIO_INT_LEVEL_LOW);
    }
    else
    {
        _GPIO_PROD_SetInterruptSense (GPIO_PROD_OFN_INT_ID, GPIO_INT_LEVEL_HIGH);
    }

    if (cfg_ptr && (cfg_ptr->valid_level == gpio_state) && (cfg_ptr->gpio_cb_fun))
    {
        cfg_ptr->gpio_cb_fun (GPIO_PROD_OFN_INT_ID);
    }

}
#endif
#ifdef CMMB_WIFI_SHARE_SPI_SUPPORT
/*****************************************************************************/
//  Description:    Switch SPI Bus by Logic Id
//  Input      :
//  Return     :
//  Author     :    Chi.Chen
//	Note       :
/*****************************************************************************/
PUBLIC void GPIO_SPISwitch(uint32 * logic_id)
{
    uint32 wifi_spi_bus_logic_id, cmmb_spi_bus_logic_id;

    wifi_spi_bus_logic_id = WIFI_GetSPIBusLogicID();
    cmmb_spi_bus_logic_id = CMMB_GetSPIBusLogicID();
    SCI_ASSERT( (wifi_spi_bus_logic_id >= 0 && wifi_spi_bus_logic_id < 2) &&/*assert verified*/
    	        (cmmb_spi_bus_logic_id >= 0 && cmmb_spi_bus_logic_id < 2) );
    if (wifi_spi_bus_logic_id == *logic_id)
    {
        GPIO_SetSPISwitch(GPIO_SPI_SW_WIFI);
    }
    else if (cmmb_spi_bus_logic_id == *logic_id)
    {
        GPIO_SetSPISwitch(GPIO_SPI_SW_CMMB);
    }
    //SCI_TRACE_LOW("GPIO_SPISwitch: spi bus select cs%d", *logic_id);
}

/*****************************************************************************/
//  Description:    Set GPIO to Switch SPI Bus to WiFi or CMMB
//  Input      :
//  Return     :
//  Author     :    Chi.Chen
//	Note       :
/*****************************************************************************/
PUBLIC void GPIO_SetSPISwitch(GPIO_SPI_SW_E sw_type)
{
    BOOLEAN gpio_switch_default_value, gpio_switch_wifi_value, gpio_switch_cmmb_value;
    static GPIO_SPI_SW_E sw_state = GPIO_SPI_SW_MAX;

    if (sw_state == sw_type)
    {
        return;
    }
    switch (sw_type)
    {
    case GPIO_SPI_SW_WIFI:
        sw_state = GPIO_SPI_SW_WIFI;
        gpio_switch_wifi_value = GPIO_GetSwitchWiFiValue();
        _GPIO_PROD_SetVal (GPIO_PROD_SPISWICTH_ID, gpio_switch_wifi_value);
        //SCI_TRACE_LOW:"GPIO_SetSPISwitch: WIFI ON"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_2250_112_2_18_0_33_5_1545,(uint8*)"");
        break;

    case GPIO_SPI_SW_CMMB:
        sw_state = GPIO_SPI_SW_CMMB;
        gpio_switch_cmmb_value = GPIO_GetSwitchCMMBValue();
        _GPIO_PROD_SetVal (GPIO_PROD_SPISWICTH_ID, gpio_switch_cmmb_value);
        //SCI_TRACE_LOW:"GPIO_SetSPISwitch: CMMB ON"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_2257_112_2_18_0_33_5_1546,(uint8*)"");
        break;

    case GPIO_SPI_SW_MAX:
        sw_state = GPIO_SPI_SW_MAX;
        gpio_switch_default_value = GPIO_GetSwitchDefaultValue();
        _GPIO_PROD_SetVal (GPIO_PROD_SPISWICTH_ID, gpio_switch_default_value);
        //SCI_TRACE_LOW:"GPIO_SetSPISwitch: SWITCH DEFAULT"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_2264_112_2_18_0_33_5_1547,(uint8*)"");
        break;

    default:
        sw_state = GPIO_SPI_SW_MAX;
        gpio_switch_default_value = GPIO_GetSwitchDefaultValue();
        _GPIO_PROD_SetVal (GPIO_PROD_SPISWICTH_ID, gpio_switch_default_value);
        //SCI_TRACE_LOW:"GPIO_SetSPISwitch: PARAMETERS ERROR"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_PROD_2271_112_2_18_0_33_5_1548,(uint8*)"");
        break;
    }
}
#endif

#if defined(PRODUCT_DM)
/*****************************************************************************/
//  Description:    This function auto probe the crystal whether tcxo or not(dcxo)
//  Author:           zhen.liu
//  parameter:
//              gpio_tcxodcxo_index_id - gpio index id for tcxo-dcxo adapter
//              real_gpio_num - gpio num physic
//              pin_reg_addr - the pin reg address corresponded to the real_gpio_num
//              pin_begin_value - the pin value before set the new pin status
//              pin_finish_value - the pin value after set the new pin status
//  return:
//              TRUE-use DCXO, FALSE-use TCXO
//  Note:
//      1. use gpio to judge
//      2. for cr:NEWMS00211691
/*****************************************************************************/
PUBLIC  BOOLEAN GPIO_GetTCXO_DCXO_Status(
                            uint32 gpio_tcxodcxo_index_id,
                            uint16 real_gpio_num,
                            uint32 pin_reg_addr,
                            uint32 pin_begin_value,
                            uint32 pin_finish_value
                            )
{
        BOOLEAN bStatus = SCI_FALSE;
        GPIO_CFG_INFO_T cfg_info = {0};
        GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo(gpio_tcxodcxo_index_id);

        CHIP_REG_SET (pin_reg_addr, pin_begin_value);

        if  (PNULL != cfg_ptr)
        {
                    cfg_info.gpio_id    = gpio_tcxodcxo_index_id;
                    cfg_info.gpio_type  = cfg_ptr->gpio_type;
                    cfg_info.gpio_num   = real_gpio_num;
                    cfg_info.valid_level= cfg_ptr->valid_level;
        }
        else
        {
                    cfg_info.gpio_id    = gpio_tcxodcxo_index_id;
                    cfg_info.gpio_type  = GPIO_PROD_TYPE_BB0;
                    cfg_info.gpio_num   = real_gpio_num;
                    cfg_info.valid_level= GPIO_PROD_HIGH_LEVEL;
        }
        GPIO_PROD_SetCfgInfo (&cfg_info);

        // set MOSI to gpio pin, and output 1.
        GPIO_SetDataMask(real_gpio_num,SCI_TRUE);
        // Config it to be INPUT.(0)
        GPIO_SetDirection(real_gpio_num,SCI_FALSE);//SCI_TRUE : output  SCI_FALSE: input
        // MASK it.(0)
        GPIO_DisableIntCtl(real_gpio_num);        // disable int

        bStatus= GPIO_GetValue(real_gpio_num);

        CHIP_REG_SET (pin_reg_addr, pin_finish_value);

        SCI_TRACE_LOW ("GPIO_GetTCXO_DCXO_Status :: [%d]", bStatus);

        return bStatus;
}
#endif

/*****************************************************************************/
//  Description:    set USB Switch
//  Note:
/*****************************************************************************/

#ifdef USB_SWITCH_SUPPORT
uint32 s_mmi_usb_mode = 0; // 0= auto by adc;  1= force to usb; 2= force to uart;
PUBLIC void GPIO_MMISetUsbSwitchMode(uint32 mode)
{
    s_mmi_usb_mode = mode;
}

PUBLIC void GPIO_SetUsbSwitch(BOOLEAN is_usb_on)
{
    SCI_TRACE_LOW("[GPIO] GPIO_SetUsbSwitch = %d,  s_mmi_usb_mode = %d, ",
                  is_usb_on, s_mmi_usb_mode);

    if(1 == s_mmi_usb_mode)
    {
        _GPIO_PROD_SetVal (GPIO_PROD_USB_SWITCH_EN_ID, TRUE);
    }
	else if(2 == s_mmi_usb_mode)
	{
        _GPIO_PROD_SetVal (GPIO_PROD_USB_SWITCH_EN_ID, FALSE);
	}
    else
    {
        _GPIO_PROD_SetVal (GPIO_PROD_USB_SWITCH_EN_ID, is_usb_on);
    }
}


#endif


/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif

// End of tb_comm.c
