	.file	"shm_linux_producer.c"
	.text
	.section	.rodata
.LC0:
	.string	"OS"
.LC1:
	.string	"Studying "
.LC2:
	.string	"Operating Systems "
.LC3:
	.string	"Is Fun!"
.LC4:
	.string	"Map failed"
.LC5:
	.string	"%s"
	.text
	.globl	main
	.type	main, @function
main:
.LFB6:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movl	$4096, -48(%rbp)
	leaq	.LC0(%rip), %rax
	movq	%rax, -40(%rbp)
	leaq	.LC1(%rip), %rax
	movq	%rax, -32(%rbp)
	leaq	.LC2(%rip), %rax
	movq	%rax, -24(%rbp)
	leaq	.LC3(%rip), %rax
	movq	%rax, -16(%rbp)
	movq	-40(%rbp), %rax
	movl	$438, %edx
	movl	$66, %esi
	movq	%rax, %rdi
	call	shm_open@PLT
	movl	%eax, -44(%rbp)
	movl	-48(%rbp), %eax
	movslq	%eax, %rdx
	movl	-44(%rbp), %eax
	movq	%rdx, %rsi
	movl	%eax, %edi
	call	ftruncate@PLT
	movl	-48(%rbp), %eax
	cltq
	movl	-44(%rbp), %edx
	movl	$0, %r9d
	movl	%edx, %r8d
	movl	$1, %ecx
	movl	$3, %edx
	movq	%rax, %rsi
	movl	$0, %edi
	call	mmap@PLT
	movq	%rax, -8(%rbp)
	cmpq	$-1, -8(%rbp)
	jne	.L2
	leaq	.LC4(%rip), %rdi
	call	puts@PLT
	movl	$-1, %eax
	jmp	.L3
.L2:
	movq	-32(%rbp), %rdx
	movq	-8(%rbp), %rax
	leaq	.LC5(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	sprintf@PLT
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	%rax, -8(%rbp)
	movq	-24(%rbp), %rdx
	movq	-8(%rbp), %rax
	leaq	.LC5(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	sprintf@PLT
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	%rax, -8(%rbp)
	movq	-16(%rbp), %rdx
	movq	-8(%rbp), %rax
	leaq	.LC5(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	sprintf@PLT
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	%rax, -8(%rbp)
	movl	$0, %eax
.L3:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
