	.text
	.globl main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$0, %rsp
	movq	$5, %r8
	movq	%r8, 0(%rbp)
	movq	0(%rbp), %r9
	movq	$10, %r10
	addq	%r9, %r10
	movq	%r10, -8(%rbp)
	movq	-8(%rbp), %r9
	movq	%r9, %rdi
	call	printint
	movq	-8(%rbp), %r9
	movq	$2, %r11
	imulq	%r9, %r11
	movq	%r11, -16(%rbp)
	movq	-16(%rbp), %r9
