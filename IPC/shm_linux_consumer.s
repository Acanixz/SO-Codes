	.file	"shm_linux_consumer.c"
	.text
	.section	.rodata
.LC0:
	.string	"OS"
.LC1:
	.string	"shared memory failed"
.LC2:
	.string	"Map failed"
.LC3:
	.string	"%s"
.LC4:
	.string	"Error removing %s\n"
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
	subq	$32, %rsp
	leaq	.LC0(%rip), %rax
	movq	%rax, -16(%rbp)
	movl	$4096, -24(%rbp)
	movq	-16(%rbp), %rax
	movl	$438, %edx
	movl	$0, %esi
	movq	%rax, %rdi
	call	shm_open@PLT
	movl	%eax, -20(%rbp)
	cmpl	$-1, -20(%rbp)
	jne	.L2
	leaq	.LC1(%rip), %rdi
	call	puts@PLT
	movl	$-1, %edi
	call	exit@PLT
.L2:
	movl	-24(%rbp), %eax
	cltq
	movl	-20(%rbp), %edx
	movl	$0, %r9d
	movl	%edx, %r8d
	movl	$1, %ecx
	movl	$1, %edx
	movq	%rax, %rsi
	movl	$0, %edi
	call	mmap@PLT
	movq	%rax, -8(%rbp)
	cmpq	$-1, -8(%rbp)
	jne	.L3
	leaq	.LC2(%rip), %rdi
	call	puts@PLT
	movl	$-1, %edi
	call	exit@PLT
.L3:
	movq	-8(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC3(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	shm_unlink@PLT
	cmpl	$-1, %eax
	jne	.L4
	movq	-16(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC4(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	$-1, %edi
	call	exit@PLT
.L4:
	movl	$0, %eax
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
