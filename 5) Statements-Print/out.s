	.text
	.globl main
main:
	movq	$1, %r8
	movq	$0, %r9
	cmpq	%r9, %r8
	setl	%r9b
	movzbq	%r9b, %r9
	cmpq	$0, %r9
	je	L1
	movq	$10, %r8
	movq	%r8, %rdi
	call	printint
	jmp	L2
L1:
	movq	$1, %r8
	movq	$1, %r9
	cmpq	%r9, %r8
	sete	%r9b
	movzbq	%r9b, %r9
	cmpq	$0, %r9
	je	L3
	movq	$100, %r8
	movq	%r8, %rdi
	call	printint
	jmp	L4
L3:
	movq	$10000, %r8
	movq	%r8, %rdi
	call	printint
L4:
L2:
	movq	$0, %rdi
	call	exit
