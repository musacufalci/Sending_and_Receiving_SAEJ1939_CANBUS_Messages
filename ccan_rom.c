



/*
	****************************************************************************************
	Filename   :    cr_startup_lpc11xx.c
	Description:    CAN J1939
	Generated by Musa ÇUFALCI 2019
	Copyright (c) Musa ÇUFALCI All rights reserved.
	*****************************************************************************************
*/

/*-----------------------------------------------------------------------------
Filename: cr_startup_lpc11xx.c
Description: This file sets up and arranges the startup.
Generated by Musa ÇUFALCI. Friday February 08 08.02.2019
----------------------------------------------------------------------------- */
/*
/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
Musa  ÇUFALCI
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
2019 MK Added First creating after project editing
-----------------------------------------------------------------------------
*/



#include "board.h"
#include "timer_11xx.h"
#include <cr_section_macros.h>
const uint32_t OscRateIn = 12000000;

#define TEST_CCAN_BAUD_RATE 500000

#define operation_vehicle_speed_delay		1529    //((255*2)*3-1)-----3 times loop
#define operation_fuel_level_delay			767		//((255)*3-1)-----3 times loop
#define operation_engine_speed_delay 		1529	//((255*2)*3-1)-----3 times loop


#define single_delay		42400
//42400 100ms


#define vehicle_speed_delay		single_delay
#define fuel_level_delay		single_delay
#define engine_speed_delay 		single_delay



static 	uint32_t vehicle_speed_delay_count=0;
static	uint32_t fuel_level_delay_count=0;
static	uint32_t engine_speed_delay_count=0;


static	uint8_t vehicle_speed_count1=0;
static	uint8_t vehicle_speed_count2=0;


static	uint8_t fuel_level_count1=0;


static	uint8_t engine_speed_count1=0;
static	uint8_t engine_speed_count2=0;


void vehicle_speed_init(void);
void fuel_level_init(void);
void engine_speed_init(void);

void vehicle_speed_function(void);
void fuel_level_function(void);
void engine_speed_function(void);

void delay32Ms(uint8_t timer_num, uint32_t delayInMs)
{
 if (timer_num == 0)
 {
Chip_TIMER_Init(LPC_TIMER32_0);
   /* setup timer #0 for delay */
   LPC_TIMER32_0->TCR = 0x02;/* reset timer */
   LPC_TIMER32_0->PR  = 0x00;/* set prescaler to zero */
   LPC_TIMER32_0->MR[0]  = delayInMs * ((SystemCoreClock/(LPC_TIMER32_0->PR+1)) / 1000);
   LPC_TIMER32_0->IR  = 0xff;/* reset all interrrupts */
   LPC_TIMER32_0->MCR = 0x04;/* stop timer on match */
   LPC_TIMER32_0->TCR = 0x01;/* start timer */

   /* wait until delay time has elapsed */
   while (LPC_TIMER32_0->TCR & 0x01);
   Chip_TIMER_DeInit(LPC_TIMER32_0);
 }
 else if (timer_num == 1)
 {
 Chip_TIMER_Init(LPC_TIMER32_1);
   /* setup timer #1 for delay */
 LPC_TIMER32_1->TCR = 0x02;/* reset timer */
 LPC_TIMER32_1->PR  = 0x00;/* set prescaler to zero */
 LPC_TIMER32_1->MR[0] = delayInMs * ((SystemCoreClock/(LPC_TIMER32_1->PR+1)) / 1000);
 LPC_TIMER32_1->IR  = 0xff;/* reset all interrrupts */
 LPC_TIMER32_1->MCR = 0x04;/* stop timer on match */
 LPC_TIMER32_1->TCR = 0x01;/* start timer */

   /* wait until delay time has elapsed */
   while (LPC_TIMER32_1->TCR & 0x01);
   Chip_TIMER_DeInit(LPC_TIMER32_1);
 }
 return;
}


CCAN_MSG_OBJ_T msg_obj;

void baudrateCalculate(uint32_t baud_rate, uint32_t *can_api_timing_cfg)
{
	uint32_t pClk, div, quanta, segs, seg1, seg2, clk_per_bit, can_sjw;
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CAN);
	pClk = Chip_Clock_GetMainClockRate();

	clk_per_bit = pClk / baud_rate;

	for (div = 0; div <= 15; div++) {
		for (quanta = 1; quanta <= 32; quanta++) {
			for (segs = 3; segs <= 17; segs++) {
				if (clk_per_bit == (segs * quanta * (div + 1))) {
					segs -= 3;
					seg1 = segs / 2;
					seg2 = segs - seg1;
					can_sjw = seg1 > 3 ? 3 : seg1;
					can_api_timing_cfg[0] = div;
					can_api_timing_cfg[1] =
						((quanta - 1) & 0x3F) | (can_sjw & 0x03) << 6 | (seg1 & 0x0F) << 8 | (seg2 & 0x07) << 12;
					return;
				}
			}
		}
	}
}


/*	CAN receive callback */
/*	Function is executed by the Callback handler after
    a CAN message has been received */
void CAN_rx(uint8_t msg_obj_num) {
	/* Determine which CAN message has been received */
	msg_obj.msgobj = msg_obj_num;
	/* Now load up the msg_obj structure with the CAN message */
	LPC_CCAN_API->can_receive(&msg_obj);
	if (msg_obj_num == 1) {
		/* Simply transmit CAN frame (echo) with with ID +0x100 via buffer 2 */
		msg_obj.msgobj = 2;
		msg_obj.mode_id += 0x100;
		LPC_CCAN_API->can_transmit(&msg_obj);
	}
}

/*	CAN transmit callback */
/*	Function is executed by the Callback handler after
    a CAN message has been transmitted */
void CAN_tx(uint8_t msg_obj_num) {}

/*	CAN error callback */
/*	Function is executed by the Callback handler after
    an error has occured on the CAN bus */
void CAN_error(uint32_t error_info) {}


void CAN_IRQHandler(void) {
	LPC_CCAN_API->isr();
}

int main(void)
{
	uint32_t CanApiClkInitTable[2];
	/* Publish CAN Callback Functions */
	CCAN_CALLBACKS_T callbacks = {
		CAN_rx,
		CAN_tx,
		CAN_error,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	};

	SystemCoreClockUpdate();
	Board_Init();
	baudrateCalculate(TEST_CCAN_BAUD_RATE, CanApiClkInitTable);

	LPC_CCAN_API->init_can(&CanApiClkInitTable[0], TRUE);
	/* Configure the CAN callback functions */
	LPC_CCAN_API->config_calb(&callbacks);
	/* Enable the CAN Interrupt */
	NVIC_EnableIRQ(CAN_IRQn);
//*****************************************************************************
//	delay32Ms(0,500);

	vehicle_speed_init();
	fuel_level_init();
	engine_speed_init();
//*****************************************************************************
	while (1) {

		vehicle_speed_function();
		fuel_level_function();
		engine_speed_function();


//		delay32Ms(0,100);

		__WFI();	/* Go to Sleep */
	}
}

//*****************************************************************************
//*****************************************************************************
void vehicle_speed_init(void)
{
//---------------------------------------------------------------------------
//		PGN 65265 Cruise Control/Vehicle Speed 			CCVS
//		2-3 2 bytes Wheel-Based Vehicle Speed
//		vehicle_speed_init
//---------------------------------------------------------------------------
	/* Send a simple one time CAN message */
	msg_obj.msgobj  = 1;
	msg_obj.mode_id = 0x18FEF1FE | CAN_MSGOBJ_EXT;
	msg_obj.mask    = 0x0;
	msg_obj.dlc     = 8;
	msg_obj.data[0] = 255;
	msg_obj.data[1] = 255;
	msg_obj.data[2] =  0 ;	//lsb
	msg_obj.data[3] =  0 ;	//msb
	msg_obj.data[4] = 255;
	msg_obj.data[5] = 255;
	msg_obj.data[6] = 255;
	msg_obj.data[7] = 255;
	LPC_CCAN_API->can_transmit(&msg_obj);
//	/* Configure message object 1 to receive all 11-bit messages 0x400-0x4FF */
//	msg_obj.msgobj = 1;
//	msg_obj.mode_id = 0x400;
//	msg_obj.mask = 0x700;
//	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}
//*****************************************************************************
//*****************************************************************************
void fuel_level_init(void)
{
//---------------------------------------------------------------------------
//		PGN 65276 Dash Display 						DD
//		2 1 byte Fuel Level 1
//		fuel_level_init
//---------------------------------------------------------------------------
	/* Send a simple one time CAN message */
	msg_obj.msgobj  = 1;
	msg_obj.mode_id = 0x18FEFCFE | CAN_MSGOBJ_EXT;
	msg_obj.mask    = 0x0;
	msg_obj.dlc     = 8;
	msg_obj.data[0] = 255;
	msg_obj.data[1] = 0  ;
	msg_obj.data[2] = 255;
	msg_obj.data[3] = 255;
	msg_obj.data[4] = 255;
	msg_obj.data[5] = 255;
	msg_obj.data[6] = 255;
	msg_obj.data[7] = 255;
	LPC_CCAN_API->can_transmit(&msg_obj);
//	/* Configure message object 1 to receive all 11-bit messages 0x400-0x4FF */
//	msg_obj.msgobj = 1;
//	msg_obj.mode_id = 0x400;
//	msg_obj.mask = 0x700;
//	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}
//*****************************************************************************
//*****************************************************************************
void engine_speed_init(void)
{
//---------------------------------------------------------------------------
//		PGN 65276 Dash Display 						DD
//		2 1 byte Fuel Level 1
//		fuel_level_init
//---------------------------------------------------------------------------
	/* Send a simple one time CAN message */
		msg_obj.msgobj  = 1;
		msg_obj.mode_id = 0xCF004FE | CAN_MSGOBJ_EXT;
		msg_obj.mask    = 0x0;
		msg_obj.dlc     = 8;
		msg_obj.data[0] = 255;
		msg_obj.data[1] = 255;
		msg_obj.data[2] = 255;
		msg_obj.data[3] = 255;
		msg_obj.data[4] = 0  ;	//msb
		msg_obj.data[5] = 0  ;	//lsb
		msg_obj.data[6] = 255;
		msg_obj.data[7] = 255;
		LPC_CCAN_API->can_transmit(&msg_obj);
//	/* Configure message object 1 to receive all 11-bit messages 0x400-0x4FF */
//	msg_obj.msgobj = 1;
//	msg_obj.mode_id = 0x400;
//	msg_obj.mask = 0x700;
//	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}
//*****************************************************************************
//*****************************************************************************
void vehicle_speed_function(void)
{
//---------------------------------------------------------------------------
//		PGN 65265 Cruise Control/Vehicle Speed 			CCVS
//		2 bytes Wheel-Based Vehicle Speed
//		vehicle_speed_function
//---------------------------------------------------------------------------
/* Send a simple one time CAN message */
		msg_obj.msgobj  = 1;
		msg_obj.mode_id = 0x18FEF1FE | CAN_MSGOBJ_EXT;
//		msg_obj.mode_id = 0x18FEF1FE | CAN_MSGOBJ_EXT;
		msg_obj.mask    = 0x0;
		msg_obj.dlc     = 8;
		msg_obj.data[0] = 255;
		msg_obj.data[1] = 255;
//		msg_obj.data[2] =  0 ;
//		msg_obj.data[3] =  0 ;
//---------------------------------------------------------------------------
	if (vehicle_speed_count1 < 255)
		{
		vehicle_speed_count1++;
		msg_obj.data[3] = vehicle_speed_count1;
		for (vehicle_speed_delay_count = 0; vehicle_speed_delay_count < vehicle_speed_delay; vehicle_speed_delay_count++);
//		delay32Ms(0,100);

		}
	else
	{
		vehicle_speed_count2++;
		msg_obj.data[2] = vehicle_speed_count2;
		for (vehicle_speed_delay_count = 0; vehicle_speed_delay_count < vehicle_speed_delay; vehicle_speed_delay_count++);
//		delay32Ms(0,100);
		if (vehicle_speed_count2 == 255)
			{
				vehicle_speed_count1=0;
				vehicle_speed_count2=0;
				msg_obj.data[2]=0;
				msg_obj.data[3]=0;
			}
	}
//---------------------------------------------------------------------------
		msg_obj.data[4] = 255;
		msg_obj.data[5] = 255;
		msg_obj.data[6] = 255;
		msg_obj.data[7] = 255;
		LPC_CCAN_API->can_transmit(&msg_obj);
//		/* Configure message object 1 to receive all 11-bit messages 0x400-0x4FF */
//		msg_obj.msgobj = 1;
//		msg_obj.mode_id = 0x400;
//		msg_obj.mask = 0x700;
//		LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}
//*****************************************************************************
//*****************************************************************************
void fuel_level_function(void)
{
//---------------------------------------------------------------------------
//		PGN 65276 Dash Display 						DD
//		2 1 byte Fuel Level 1
//		fuel_level_function
//---------------------------------------------------------------------------
/* Send a simple one time CAN message */
	msg_obj.msgobj  = 1;
	msg_obj.mode_id = 0x18FEFCFE | CAN_MSGOBJ_EXT;
//	msg_obj.mode_id = 0x18FEF2FE | CAN_MSGOBJ_EXT;
	msg_obj.mask    = 0x0;
	msg_obj.dlc     = 8;
	msg_obj.data[0] = 255;
//	msg_obj.data[1] = 0  ;
//---------------------------------------------------------------------------
	if (fuel_level_count1 < 255)
		{
		fuel_level_count1++;
		msg_obj.data[1] = fuel_level_count1;
		for (fuel_level_delay_count = 0; fuel_level_delay_count< fuel_level_delay; fuel_level_delay_count++);
//		delay32Ms(0,100);
		}
	else
	{
		if (fuel_level_count1 == 255)
		{
			fuel_level_count1=0;
			msg_obj.data[1]=0;
		}
	}
//---------------------------------------------------------------------------
	msg_obj.data[2] = 255;
	msg_obj.data[3] = 255;
	msg_obj.data[4] = 255;
	msg_obj.data[5] = 255;
	msg_obj.data[6] = 255;
	msg_obj.data[7] = 255;
	LPC_CCAN_API->can_transmit(&msg_obj);
//	/* Configure message object 1 to receive all 11-bit messages 0x400-0x4FF */
//	msg_obj.msgobj = 1;
//	msg_obj.mode_id = 0x400;
//	msg_obj.mask = 0x700;
//	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}
//*****************************************************************************
//*****************************************************************************
void engine_speed_function(void)
{
//---------------------------------------------------------------------------
//		PGN 61444 Electronic Engine Controller 1 			EEC1
//		4-5 2 bytes Engine Speed 190
//		engine_speed_function
//---------------------------------------------------------------------------
/* Send a simple one time CAN message */
	msg_obj.msgobj  = 1;
	msg_obj.mode_id = 0xCF004FE | CAN_MSGOBJ_EXT;
//	msg_obj.mode_id = 0x18F1F3FE | CAN_MSGOBJ_EXT;
	msg_obj.mask    = 0x0;
	msg_obj.dlc     = 8;
	msg_obj.data[0] = 255;
	msg_obj.data[1] = 255;
	msg_obj.data[2] = 255;
	msg_obj.data[3] = 255;
//	msg_obj.data[4] = 0  ;	//msb
//	msg_obj.data[5] = 0  ;	//lsb
//---------------------------------------------------------------------------
	if (engine_speed_count1 < 255)
		{
		engine_speed_count1++;
		msg_obj.data[5] = engine_speed_count1;
		for (engine_speed_delay_count = 0; engine_speed_delay_count < engine_speed_delay; engine_speed_delay_count++);
//		delay32Ms(0,100);
		}
	else
	{
		engine_speed_count2++;
		msg_obj.data[4] = engine_speed_count2;
		for (engine_speed_delay_count = 0; engine_speed_delay_count < engine_speed_delay; engine_speed_delay_count++);
//		delay32Ms(0,100);
		if (engine_speed_count2 == 255)
			{
				engine_speed_count1=0;
				engine_speed_count2=0;
				msg_obj.data[4]=0;
				msg_obj.data[5]=0;
			}
	}
//---------------------------------------------------------------------------
	msg_obj.data[6] = 255;
	msg_obj.data[7] = 255;
	LPC_CCAN_API->can_transmit(&msg_obj);
//	/* Configure message object 1 to receive all 11-bit messages 0x400-0x4FF */
//	msg_obj.msgobj = 1;
//	msg_obj.mode_id = 0x400;
//	msg_obj.mask = 0x700;
//	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}
//*****************************************************************************
//*****************************************************************************
