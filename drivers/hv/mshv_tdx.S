/* SPDX-License-Identifier: GPL-2.0 */
#include <asm/asm-offsets.h>
#include <asm/asm.h>
#include <asm/tdx.h>

#include <linux/linkage.h>
#include <linux/errno.h>

.section .noinstr.text, "ax"

#define tdcall          .byte 0x66,0x0f,0x01,0xcc

/*
 * __tdg_vp_enter()  - Make a VP.ENTER TDCALL
 *
 * @args (RDI)	- struct tdx_module_args for input and output
 *
 * Return status of TDCALL via RAX.
 */
SYM_FUNC_START(__tdg_vp_enter)

        push %_ASM_BP
        _ASM_MOV %_ASM_SP, %_ASM_BP

        movq 	$TDG_VP_ENTER, %rax
        movq    TDX_MODULE_ARGS_rcx(%rdi), %rcx
        movq    TDX_MODULE_ARGS_rdx(%rdi), %rdx

        pushq   %rbx
        pushq   %r12
        pushq   %r13
        pushq   %r14
        pushq   %r15

	pushq	%rdi

	tdcall

	pushq	%rax
	movq	8(%rsp), %rax
	movq	%rdi, TDX_MODULE_ARGS_rdi(%rax)
	popq	%rax
	popq	%rdi

	movq	%rcx, TDX_MODULE_ARGS_rcx(%rdi)
	movq	%rdx, TDX_MODULE_ARGS_rdx(%rdi)
	movq	%rsi, TDX_MODULE_ARGS_rsi(%rdi)
	movq	%r8, TDX_MODULE_ARGS_r8(%rdi)
	movq	%r9, TDX_MODULE_ARGS_r9(%rdi)
	movq	%r10, TDX_MODULE_ARGS_r10(%rdi)
	movq	%r11, TDX_MODULE_ARGS_r11(%rdi)
	movq	%r12, TDX_MODULE_ARGS_r12(%rdi)
	movq	%r13, TDX_MODULE_ARGS_r13(%rdi)
	movq	%r14, TDX_MODULE_ARGS_r14(%rdi)
	movq	%r15, TDX_MODULE_ARGS_r15(%rdi)

        xorl %ecx,  %ecx
        xorl %edx,  %edx
        xorl %r8d,  %r8d
        xorl %r9d,  %r9d
        xorl %r10d, %r10d
        xorl %r11d, %r11d
        xorl %r12d, %r12d
        xorl %r13d, %r13d
        xorl %r14d, %r14d
        xorl %r15d, %r15d
        xorl %ebx,  %ebx
        xorl %edi,  %edi

        popq    %r15
        popq    %r14
        popq    %r13
        popq    %r12
        popq    %rbx

        pop %_ASM_BP

SYM_FUNC_END(__tdg_vp_enter)

