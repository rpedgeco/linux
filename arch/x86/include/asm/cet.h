/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_CET_H
#define _ASM_X86_CET_H

#ifndef __ASSEMBLY__
#include <linux/types.h>

struct task_struct;

#ifdef CONFIG_X86_USER_SHADOW_STACK
struct thread_shstk {
	u64	base;
	u64	size;
};

long cet_prctl(struct task_struct *task, int option, unsigned long features);
void reset_thread_features(void);
int shstk_alloc_thread_stack(struct task_struct *p, unsigned long clone_flags,
			     unsigned long stack_size,
			     unsigned long *shstk_addr);
void shstk_free(struct task_struct *p);
#else
static inline long cet_prctl(struct task_struct *task, int option,
			     unsigned long features) { return -EINVAL; }
static inline void reset_thread_features(void) {}
static inline int shstk_alloc_thread_stack(struct task_struct *p,
					   unsigned long clone_flags,
					   unsigned long stack_size,
					   unsigned long *shstk_addr) { return 0; }
static inline void shstk_free(struct task_struct *p) {}
#endif /* CONFIG_X86_USER_SHADOW_STACK */

#endif /* __ASSEMBLY__ */

#endif /* _ASM_X86_CET_H */
