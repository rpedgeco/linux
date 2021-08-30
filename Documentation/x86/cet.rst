.. SPDX-License-Identifier: GPL-2.0

=========================================
Control-flow Enforcement Technology (CET)
=========================================

Overview
========

Control-flow Enforcement Technology (CET) is term referring to several
related x86 processor features that provides protection against control
flow hijacking attacks. The HW feature itself can be set up to protect
both applications and the kernel.

CET introduces Shadow Stack and Indirect Branch Tracking (IBT). Shadow stack
is a secondary stack allocated from memory and cannot be directly modified by
applications. When executing a CALL instruction, the processor pushes the
return address to both the normal stack and the shadow stack. Upon
function return, the processor pops the shadow stack copy and compares it
to the normal stack copy. If the two differ, the processor raises a
control-protection fault. IBT verifies indirect CALL/JMP targets are intended
as marked by the compiler with 'ENDBR' opcodes. Not all CPU's have both Shadow
Stack and Indirect Branch Tracking. Today in the 64-bit kernel, only userspace
Shadow Stack and kernel IBT is supported in the kernel.

The Kconfig option is X86_USER_SHADOW_STACK, and it can be disabled with
the kernel parameter clearcpuid, like this: "clearcpuid=user_shstk".

To build a user shadow stack enabled kernel, Binutils v2.29 or LLVM v6 or later
are required.

At run time, /proc/cpuinfo shows CET features if the processor supports
CET. "shstk" and "ibt" relate to the individual HW features. "user_shstk"
relates to whether the userspace shadow stack specifically is supported.

Application Enabling
====================

An application's CET capability is marked in its ELF note and can be verified
from readelf/llvm-readelf output:

    readelf -n <application> | grep -a SHSTK
        properties: x86 feature: SHSTK

The kernel does not process these applications markers directly. Applications
or loaders must enable CET features using the interface described in section 4.
Typically this would be done in dynamic loader or static runtime objects, as is
the case in GLIBC.

CET arch_prctl()'s
==================

Elf features should be enabled by the loader using the below arch_prctl's.

arch_prctl(ARCH_CET_ENABLE, unsigned int feature)
    Enable a single feature specified in 'feature'. Can only operate on
    one feature at a time.

arch_prctl(ARCH_CET_DISABLE, unsigned int feature)
    Disable a single feature specified in 'feature'. Can only operate on
    one feature at a time.

arch_prctl(ARCH_CET_LOCK, unsigned int features)
    Lock in features at their current enabled or disabled status. 'features'
    is a mask of all features to lock. All bits set are processed, unset bits
    are ignored. The mask is ORed with the existing value. So any feature bits
    set here cannot be enabled or disabled afterwards.

The return values are as following:
    On success, return 0. On error, errno can be::

        -EPERM if any of the passed feature are locked.
        -EOPNOTSUPP if the feature is not supported by the hardware or
         disabled by kernel parameter.
        -EINVAL arguments (non existing feature, etc)

Currently shadow stack and WRSS are supported via this interface. WRSS
can only be enabled with shadow stack, and is automatically disabled
if shadow stack is disabled.

Proc status
===========
To check if an application is actually running with shadow stack, the
user can read the /proc/$PID/status. It will report "wrss" or "shstk"
depending on what is enabled. The lines look like this::

    x86_Thread_features: shstk wrss
    x86_Thread_features_locked: shstk wrss

The implementation of the Shadow Stack
======================================

Shadow Stack size
-----------------

A task's shadow stack is allocated from memory to a fixed size of
MIN(RLIMIT_STACK, 4 GB). In other words, the shadow stack is allocated to
the maximum size of the normal stack, but capped to 4 GB. However,
a compat-mode application's address space is smaller, each of its thread's
shadow stack size is MIN(1/4 RLIMIT_STACK, 4 GB).

Signal
------

By default, the main program and its signal handlers use the same shadow
stack. Because the shadow stack stores only return addresses, a large
shadow stack covers the condition that both the program stack and the
signal alternate stack run out.

When a signal happens, the old pre-signal state is pushed on the stack. When
shadow stack is enabled, the shadow stack specific state is pushed onto the
shadow stack. Today this is only the old SSP (shadow stack pointer), pushed
in a special format with bit 63 set. On sigreturn this old SSP token is
verified and restored by the kernel. The kernel will also push the normal
restorer address to the shadow stack to help userspace avoid a shadow stack
violation on the sigreturn path that goes through the restorer.

So the shadow stack signal frame format is as follows::

    |1...old SSP| - Pointer to old pre-signal ssp in sigframe token format
                    (bit 63 set to 1)
    |        ...| - Other state may be added in the future



Fork
----

The shadow stack's vma has VM_SHADOW_STACK flag set; its PTEs are required
to be read-only and dirty. When a shadow stack PTE is not RO and dirty, a
shadow access triggers a page fault with the shadow stack access bit set
in the page fault error code.

When a task forks a child, its shadow stack PTEs are copied and both the
parent's and the child's shadow stack PTEs are cleared of the dirty bit.
Upon the next shadow stack access, the resulting shadow stack page fault
is handled by page copy/re-use.

When a pthread child is created, the kernel allocates a new shadow stack
for the new thread. New shadow stack's behave like mmap() with respect to
ASLR behavior.

Exec
----

On exec, shadow stack features are disabled by the kernel. At which point,
userspace can choose to re-enable, or lock them.