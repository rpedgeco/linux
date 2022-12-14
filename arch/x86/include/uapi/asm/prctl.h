/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _ASM_X86_PRCTL_H
#define _ASM_X86_PRCTL_H

#define ARCH_SET_GS			0x1001
#define ARCH_SET_FS			0x1002
#define ARCH_GET_FS			0x1003
#define ARCH_GET_GS			0x1004

#define ARCH_GET_CPUID			0x1011
#define ARCH_SET_CPUID			0x1012

#define ARCH_GET_XCOMP_SUPP		0x1021
#define ARCH_GET_XCOMP_PERM		0x1022
#define ARCH_REQ_XCOMP_PERM		0x1023
#define ARCH_GET_XCOMP_GUEST_PERM	0x1024
#define ARCH_REQ_XCOMP_GUEST_PERM	0x1025

#define ARCH_MAP_VDSO_X32		0x2001
#define ARCH_MAP_VDSO_32		0x2002
#define ARCH_MAP_VDSO_64		0x2003

/* Don't use 0x3001-0x3004 because of old glibcs */

#define ARCH_CET_ENABLE			0x5001
#define ARCH_CET_DISABLE		0x5002
#define ARCH_CET_LOCK			0x5003
#define ARCH_CET_UNLOCK			0x5004

/* ARCH_CET_ features bits */
#define CET_SHSTK			(1ULL <<  0)
#define CET_WRSS			(1ULL <<  1)

#endif /* _ASM_X86_PRCTL_H */
