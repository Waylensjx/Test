/*
 * <Keymap_cfg.c> - <Keypad map driver>
 *
 * Copyright (c) 2019 Unisoc Communications Inc.
 * History
 *      <2019/12/12> <Hery.luo>
 *      Keypad map initial draft
 */
#include "tb_dal.h"

#ifdef TP_VK_SUPPORT
#include "lcd_cfg.h"

LOCAL const VIR_KEY_T virkey[] = {
	// NOTE:
	// *) data is tested by real TP value (after TP Calibration)
	// *) please enlarge RIGHT, BOTTOM as necessary
	// *) after NEWMS00176261, TP point value are "real" value, not LCD related

	// new TP: invalid TP : 320X480, x and y are in sepcial zone, NOT increase step by step
	// new TP BOOTOM do not > 530: if quick touch valid TP zone, TP will report (0, 556),(0, 550)... which is a "wrong" VK
	{{       -10,         482,          10,             530},   SCI_VK_MENU_SELECT},  // default valid: (  0, 500)
	{{       300,         482,          360,            530},   SCI_VK_MENU_CANCEL},  // default valid: (330, 500)

	// comply for OLD TP, should be remove if necessary
	// old TP: invalid TP : 320X480, x and y increase step by step
	// old letf 0->1 : diff with valid TP Zone(0 ~~ width)
	//        left,         top,        right,          bottom,   vir_key_code
	{{        10,         510,          100,            660},   SCI_VK_MENU_SELECT},  // default valid: ( 80, 580)
	{{       150,         510,          360,            660},   SCI_VK_MENU_CANCEL},  // default valid: (250, 580)
};

PUBLIC const VIR_KEY_T *KPD_GetVirKeyMap (uint32 *virkey_size)
{
	*virkey_size = sizeof (virkey) / sizeof (virkey[0]);
	return virkey;
}
#endif

#if defined(REF_PCB_VER_B3)
LOCAL const uint16 keymap[] = {
	/*			KEYOUT0					KEYOUT1					KEYOUT2					KEYOUT3					KEYOUT4					KEYOUT5					KEYOUT6					KEYOUT7					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN0--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_CAMERA,			SCI_VK_VOL_UP,			SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN1--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_WEB,				SCI_VK_VOL_DOWN,            SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN2--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN3--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN4--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------    */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN5--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN6--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN7--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,          SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
};
#elif defined(FPGA_VERIFICATION)
//FPGA KEYMAP Version
LOCAL const uint16 keymap[] = {
	/*			KEYOUT0					KEYOUT1					KEYOUT2					KEYOUT3					KEYOUT4					KEYOUT5					KEYOUT6					KEYOUT7					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN0--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,		SCI_VK_CALL,				SCI_VK_INVALID_KEY,        SCI_VK_WEB,				SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN1--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_1,				SCI_VK_2,				SCI_VK_3,				SCI_VK_STAR,				SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN2--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_4,				SCI_VK_5,				SCI_VK_6,				SCI_VK_POUND,			SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN3--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_7,				SCI_VK_8,				SCI_VK_9,				SCI_VK_0,				SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN4--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------    */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_LEFT,				SCI_VK_RIGHT,			SCI_VK_UP,				SCI_VK_DOWN,			SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN5--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_POWER,			SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN6--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,         SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN7--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_MENU_SELECT,	      SCI_VK_MENU_CANCEL,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY,	      SCI_VK_INVALID_KEY
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
};
#else
//OPen phone &bar phone
LOCAL const uint16 keymap[] = {
	/*			KEYOUT0					KEYOUT1					KEYOUT2					KEYOUT3					KEYOUT4					KEYOUT5					KEYOUT6					KEYOUT7					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN0--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_STAR,				SCI_VK_7,				SCI_VK_1,				SCI_VK_CALL,				SCI_VK_4,				SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN1--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_0,				SCI_VK_8,				SCI_VK_2,				SCI_VK_DOWN,			SCI_VK_5,				SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN2--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_MENU_SELECT,		SCI_VK_LEFT,				SCI_VK_INVALID_KEY,		SCI_VK_WEB,				SCI_VK_UP,				SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN3--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_POUND,			SCI_VK_9,				SCI_VK_6,				SCI_VK_RIGHT,			SCI_VK_MENU_CANCEL,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN4--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------    */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN5--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN6--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
	/*KEYIN7--------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o-----------------------o------------------   */
	/*				|						|						|						|						|						|						|						|					*/
					SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,		SCI_VK_INVALID_KEY,
	/*				|						|						|						|						|						|						|						|					*/
	/*				|						|						|						|						|						|						|						|					*/
};
#endif
PUBLIC uint16 *KPD_GetKeyMap (uint32 *keymap_size)
{
	*keymap_size = sizeof (keymap) / sizeof (keymap[0]);
	return (uint16 *) keymap;
}


