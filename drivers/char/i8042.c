/**
 * Copyright (C) 2009 Renê de Souza Pinto
 * Tempos - Tempos is an Educational and multi purposing Operating System
 *
 * File: i8042.c
 * Desc: Driver for keyboard controller
 *
 * This file is part of TempOS.
 *
 * TempOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * TempOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <tempos/kernel.h>
#include <drv/i8042.h>
#include <x86/irq.h>
#include <x86/io.h>

static void keyboard_handler(int id, pt_regs *regs);


/**
 * Scan codes table
 */
static uint32_t scan2ascii_table[][8] =
{
/*      ASCII - Shift - Ctrl -  Alt -   Num -   Caps -  Shift Caps -    Shift Num */
{       0,      0,      0,      0,      0,      0,      0,              0},
{       0x1B,   0x1B,   0x1B,   0,      0x1B,   0x1B,   0x1B,           0x1B},
/* 1 -> 9 */
{       0x31,   0x21,   0,      0x7800, 0x31,   0x31,   0x21,           0x21},
{       0x32,   0x40,   0x0300, 0x7900, 0x32,   0x32,   0x40,           0x40},
{       0x33,   0x23,   0,      0x7A00, 0x33,   0x33,   0x23,           0x23},
{       0x34,   0x24,   0,      0x7B00, 0x34,   0x34,   0x24,           0x24},
{       0x35,   0x25,   0,      0x7C00, 0x35,   0x35,   0x25,           0x25},
{       0x36,   0x5E,   0x1E,   0x7D00, 0x36,   0x36,   0x5E,           0x5E},
{       0x37,   0x26,   0,      0x7E00, 0x37,   0x37,   0x26,           0x26},
{       0x38,   0x2A,   0,      0x7F00, 0x38,   0x38,   0x2A,           0x2A},
{       0x39,   0x28,   0,      0x8000, 0x39,   0x39,   0x28,           0x28},
{       0x30,   0x29,   0,      0x8100, 0x30,   0x30,   0x29,           0x29},
/* -, =, Bksp, Tab */
{       0x2D,   0x5F,   0x1F,   0x8200, 0x2D,   0x2D,   0x5F,           0x5F},
{       0x3D,   0x2B,   0,      0x8300, 0x3D,   0x3D,   0x2B,           0x2B},
{       0x08,   0x08,   0x7F,   0,      0x08,   0x08,   0x08,           0x08},
{       0x09,   0x0F00, 0,      0,      0x09,   0x09,   0x0F00,         0x0F00},
/*      QWERTYUIOP[] */
{       0x71,   0x51,   0x11,   0x1000, 0x71,   0x51,   0x71,           0x51},
{       0x77,   0x57,   0x17,   0x1100, 0x77,   0x57,   0x77,           0x57},
{       0x65,   0x45,   0x05,   0x1200, 0x65,   0x45,   0x65,           0x45},
{       0x72,   0x52,   0x12,   0x1300, 0x72,   0x52,   0x72,           0x52},
{       0x74,   0x54,   0x14,   0x1400, 0x74,   0x54,   0x74,           0x54},
{       0x79,   0x59,   0x19,   0x1500, 0x79,   0x59,   0x79,           0x59},
{       0x75,   0x55,   0x15,   0x1600, 0x75,   0x55,   0x75,           0x55},
{       0x69,   0x49,   0x09,   0x1700, 0x69,   0x49,   0x69,           0x49},
{       0x6F,   0x4F,   0x0F,   0x1800, 0x6F,   0x4F,   0x6F,           0x4F},
{       0x70,   0x50,   0x10,   0x1900, 0x70,   0x50,   0x70,           0x50},
{       0x5B,   0x7B,   0x1B,   0x0,    0x5B,   0x5B,   0x7B,           0x7B},
{       0x5D,   0x7D,   0x1D,   0,      0x5D,   0x5D,   0x7D,           0x7D},
/* ENTER, CTRL */
{       0x0A,   0x0A,   0x0D,   0,      0x0A,   0x0A,   0x0D,           0x0D},
{       0,      0,      0,      0,      0,      0,      0,              0},
/* ASDFGHJKL;'~ */
{       0x61,   0x41,   0x01,   0x1E00, 0x61,   0x41,   0x61,           0x41},
{       0x73,   0x53,   0x13,   0x1F00, 0x73,   0x53,   0x73,           0x53},
{       0x64,   0x44,   0x04,   0x2000, 0x64,   0x44,   0x64,           0x44},
{       0x66,   0x46,   0x06,   0x2100, 0x66,   0x46,   0x66,           0x46},
{       0x67,   0x47,   0x07,   0x2200, 0x67,   0x47,   0x67,           0x47},
{       0x68,   0x48,   0x08,   0x2300, 0x68,   0x48,   0x68,           0x48},
{       0x6A,   0x4A,   0x0A,   0x2400, 0x6A,   0x4A,   0x6A,           0x4A},
{       0x6B,   0x4B,   0x0B,   0x3500, 0x6B,   0x4B,   0x6B,           0x4B},
{       0x6C,   0x4C,   0x0C,   0x2600, 0x6C,   0x4C,   0x6C,           0x4C},
{       0x3B,   0x3A,   0,      0,      0x3B,   0x3B,   0x3A,           0x3A},
{       0x27,   0x22,   0,      0,      0x27,   0x27,   0x22,           0x22},
{       0x60,   0x7E,   0,      0,      0x60,   0x60,   0x7E,           0x7E},
/* Left Shift*/
{       0x2A,   0,      0,      0,      0,      0,      0,              0},
/* \ZXCVBNM,./ */
{       0x5C,   0x7C,   0x1C,   0,      0x5C,   0x5C,   0x7C,           0x7C},
{       0x7A,   0x5A,   0x1A,   0x2C00, 0x7A,   0x5A,   0x7A,           0x5A},
{       0x78,   0x58,   0x18,   0x2D00, 0x78,   0x58,   0x78,           0x58},
{       0x63,   0x43,   0x03,   0x2E00, 0x63,   0x43,   0x63,           0x43},
{       0x76,   0x56,   0x16,   0x2F00, 0x76,   0x56,   0x76,           0x56},
{       0x62,   0x42,   0x02,   0x3000, 0x62,   0x42,   0x62,           0x42},
{       0x6E,   0x4E,   0x0E,   0x3100, 0x6E,   0x4E,   0x6E,           0x4E},
{       0x6D,   0x4D,   0x0D,   0x3200, 0x6D,   0x4D,   0x6D,           0x4D},
{       0x2C,   0x3C,   0,      0,      0x2C,   0x2C,   0x3C,           0x3C},
{       0x2E,   0x3E,   0,      0,      0x2E,   0x2E,   0x3E,           0x3E},
{       0x2F,   0x3F,   0,      0,      0x2F,   0x2F,   0x3F,           0x3F},
/* Right Shift */
{       0,      0,      0,      0,      0,      0,      0,              0},
/* Print Screen */
{       0,      0,      0,      0,      0,      0,      0,              0},
/* Alt  */
{       0,      0,      0,      0,      0,      0,      0,              0},
/* Space */
{       0x20,   0x20,   0x20,   0,      0x20,   0x20,   0x20,           0x20},
/* Caps */
{       0,      0,      0,      0,      0,      0,      0,              0},
/* F1-F10 */
{       0x3B00, 0x5400, 0x5E00, 0x6800, 0x3B00, 0x3B00, 0x5400,         0x5400},
{       0x3C00, 0x5500, 0x5F00, 0x6900, 0x3C00, 0x3C00, 0x5500,         0x5500},
{       0x3D00, 0x5600, 0x6000, 0x6A00, 0x3D00, 0x3D00, 0x5600,         0x5600},
{       0x3E00, 0x5700, 0x6100, 0x6B00, 0x3E00, 0x3E00, 0x5700,         0x5700},
{       0x3F00, 0x5800, 0x6200, 0x6C00, 0x3F00, 0x3F00, 0x5800,         0x5800},
{       0x4000, 0x5900, 0x6300, 0x6D00, 0x4000, 0x4000, 0x5900,         0x5900},
{       0x4100, 0x5A00, 0x6400, 0x6E00, 0x4100, 0x4100, 0x5A00,         0x5A00},
{       0x4200, 0x5B00, 0x6500, 0x6F00, 0x4200, 0x4200, 0x5B00,         0x5B00},
{       0x4300, 0x5C00, 0x6600, 0x7000, 0x4300, 0x4300, 0x5C00,         0x5C00},
{       0x4400, 0x5D00, 0x6700, 0x7100, 0x4400, 0x4400, 0x5D00,         0x5D00},
/* Num Lock, Scrl Lock */
{       0,      0,      0,      0,      0,      0,      0,              0},
{       0,      0,      0,      0,      0,      0,      0,              0},
/* HOME, Up, Pgup, -kpad, left, center, right, +keypad, end, down, pgdn, ins, del */
{       0x4700, 0x37,   0x7700, 0,      0x37,   0x4700, 0x37,           0x4700},
{       0x4800, 0x38,   0,      0,      0x38,   0x4800, 0x38,           0x4800},
{       0x4900, 0x39,   0x8400, 0,      0x39,   0x4900, 0x39,           0x4900},
{       0x2D,   0x2D,   0,      0,      0x2D,   0x2D,   0x2D,           0x2D},
{       0x4B00, 0x34,   0x7300, 0,      0x34,   0x4B00, 0x34,           0x4B00},
{       0x4C00, 0x35,   0,      0,      0x35,   0x4C00, 0x35,           0x4C00},
{       0x4D00, 0x36,   0x7400, 0,      0x36,   0x4D00, 0x36,           0x4D00},
{       0x2B,   0x2B,   0,      0,      0x2B,   0x2B,   0x2B,           0x2B},
{       0x4F00, 0x31,   0x7500, 0,      0x31,   0x4F00, 0x31,           0x4F00},
{       0x5000, 0x32,   0,      0,      0x32,   0x5000, 0x32,           0x5000},
{       0x5100, 0x33,   0x7600, 0,      0x33,   0x5100, 0x33,           0x5100},
{       0x5200, 0x30,   0,      0,      0x30,   0x5200, 0x30,           0x5200},
{       0x5300, 0x2E,   0,      0,      0x2E,   0x5300, 0x2E,           0x5300}
};



/**
 * init_8042
 *
 * Initialize the two PICs (Master and Slave):
 */
void init_8042(void)
{
	kprintf(KERN_INFO "Initializing i8042 keyboard controller...\n");

	kbc_sendcomm(KB_OK);
	if(kbc_read() != 0x55)
			kprintf(KERN_ERROR "Error on initialize i8042\n");

	if( request_irq(KBD_IRQ, keyboard_handler, 0, "i8042") < 0 ) {
		kprintf(KERN_ERROR "Error on initialize i8042\n");
	}
}


/**
 * Interrupt handler
 */
static void keyboard_handler(int id, pt_regs *regs)
{
	uchar8_t key = read_key();
		kprintf( "%c", key );
}


/**
 * Send a command to keyboard controller
 */
void kbc_sendcomm(uchar8_t command)
{
	wait_write_8042();
	outb(command, KBD_CMD_BUF);
	wait_read_8042();
}


/**
 * Read output buffer
 */
uchar8_t kbc_read(void)
{
	return(inb(KBD_OUT_BUF));
}


/**
 * Wait for data
 */
void wait_read_8042(void)
{
	while( !(inb(STATUS_PORT) & OUT_BUF_FULL) );
	return;
}


/**
 * Wait while 8042 is busy
 */
void wait_write_8042(void)
{
	while( (inb(STATUS_PORT) & INPT_BUF_FULL) );
	return;
}


/**
 * Read a key
 */
uchar8_t read_key(void)
{
	unsigned char key = kbc_read();
	return(scan2ascii_table[key][0]);
}


