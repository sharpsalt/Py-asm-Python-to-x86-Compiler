	.text
	.globl main
main:
	movq	$2, %r8
	movq	$3, %r9
	movq	$5, %r10
	imulq	%r9, %r10
	addq	%r8, %r9
	movq	$8, %r8
	movq	$3, %r10
	movq	%r8,%rax
	cqo
	idivq	%r10
	movq	%rax,%r8
	subq	%r8, %r9
	movq	%r9, %rdi
	call	printint
	movq	$0, %rdi
	call	exit
