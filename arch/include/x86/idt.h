/**
 * Copyright (C) 2009 Renê de Souza Pinto
 * Tempos - Tempos is an Educational and multi purposing Operating System
 *
 * File: idt.h
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef ARCH_IDT_H

	#define ARCH_IDT_H

	#include <x86/x86.h>

	#define IDT_TABLE_SIZE		31
	#define FIRST_NONUSED_INT	 0//32

	#define IDT_SET_OFFSET(a, offset)	a->offset_low       = (offset & 0xFFFF);          \
										a->high.offset_high = ((offset >> 16) & 0xFFFF);


	#define IDT_INT_GATE		0x6
	#define IDT_TRAP_GATE		0x7
	#define IDT_INTGATE_S16		0
	#define IDT_INTGATE_S32		1

	/**
	 * idt_entry
	 *
	 * Each entry of IDT stores a Gate Descriptor. A gate descriptor in IDT
	 * should describe any of follow gates below:
	 *
	 * 		TSS_DESC  - Task-gate descriptor
	 * 		INT_DESC  - Interrupt-gate descriptor
	 * 		TRAP_DESC - Trap-gate descriptor
	 * 		  ^              ^
	 *        |              |
	 * 		  |              |
	 * 		  |              [---- Intel terminology
	 * 		  |
	 * 		  [---- TempOS terminology
	 *
	 */
	struct _idt_entry {
		uint32_t lower;
		uint32_t high;
	} __attribute__ ((packed));


	/**
	 * idt_tpint_desc
	 *
	 * The IDT table entry format for Interrupt-gate and Trap-gate descriptors. Both are
	 * very similar, so we can use just one structure to keep then.
	 * This is not the better way to express a IDT entry, but it's clear and more easy 
	 * to understand.
	 *
	 * For more information, see Intel Manual vol.3, chapter 5.
	 */
	struct _idt_tpint_desc {
		uint16_t  offset_low;
		uint16_t  seg_selector;
		struct _idt_high {
			uint16_t notused     : 5;
			uint16_t reserved3   : 3;
			uint16_t type        : 3;
			uint16_t gate_size   : 1;
			uint16_t reserved1   : 1;
			uint16_t DPL         : 2;
			uint16_t present     : 1;
			uint16_t offset_high;
		} __attribute__ ((packed)) high;
	} __attribute__ ((packed));

	/**
	 * idtr_t
	 *
	 * IDTR store IDT table size (limit) and the pointer to the table
	 */
	struct _idt_idtr {
		uint16_t table_limit;
		uint32_t idt_ptr;
	} __attribute__ ((packed)) IDTR;

	typedef struct _idt_entry      idt_t;
	typedef struct _idt_tpint_desc idt_tpintdesc_t;

	void setup_IDT(void);
	inline void load_idt(void);

#endif /* ARCH_IDT_H */

