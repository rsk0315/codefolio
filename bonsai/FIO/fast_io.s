	.text
	.align 1,0x90
	.align 4,0x90
	.globl __ZN4fast7scanner4scanImLPm0EEEvRT_
	.weak_definition __ZN4fast7scanner4scanImLPm0EEEvRT_
__ZN4fast7scanner4scanImLPm0EEEvRT_:
LFB2850:
	pushq	%r14
LCFI0:
	pushq	%r13
LCFI1:
	pushq	%r12
LCFI2:
	pushq	%rbp
LCFI3:
	movq	%rsi, %rbp
	pushq	%rbx
LCFI4:
	movq	(%rdi), %rsi
	movq	%rdi, %rbx
	movq	8(%rdi), %rax
	leaq	20(%rsi), %rdx
	cmpq	%rdx, %rax
	jbe	L11
L2:
	movq	$0, 0(%rbp)
# 50 "fast_io.cpp" 1
	nya:
# 0 "" 2
	movq	(%rbx), %rdx
	movsbl	(%rdx), %ecx
	leaq	1(%rdx), %rax
	addq	$2, %rdx
	subl	$48, %ecx
	movslq	%ecx, %rcx
	movq	%rcx, 0(%rbp)
	movq	%rax, (%rbx)
	movsbq	-1(%rdx), %rax
	cmpb	$47, %al
	jle	L5
L4:
	leaq	(%rcx,%rcx,4), %rcx
	leaq	-48(%rax,%rcx,2), %rcx
	movq	%rcx, 0(%rbp)
	movq	%rdx, (%rbx)
	movsbq	(%rdx), %rax
	addq	$1, %rdx
	cmpb	$47, %al
	jg	L4
L5:
# 65 "fast_io.cpp" 1
	nyan:
# 0 "" 2
	addq	$1, (%rbx)
	popq	%rbx
LCFI5:
	popq	%rbp
LCFI6:
	popq	%r12
LCFI7:
	popq	%r13
LCFI8:
	popq	%r14
LCFI9:
	ret
L11:
LCFI10:
	leaq	__ZN4fastL3bufE(%rip), %r14
	subq	%rsi, %rax
	leaq	(%rax,%r14), %r13
	movq	%rax, %r12
	cmpq	%r13, %rsi
	jb	L2
	movq	%rax, %rdx
	movq	%r14, %rdi
	call	_memcpy
	movq	___stdinp@GOTPCREL(%rip), %rax
	movl	$131072, %edx
	movq	%r13, %rdi
	subq	%r12, %rdx
	movl	$1, %esi
	movq	(%rax), %rcx
	call	_fread
	addq	%rax, %r13
	movq	%r13, 8(%rbx)
	movb	$0, 0(%r13)
	movq	%r14, (%rbx)
	jmp	L2
LFE2850:
	.cstring
	.align 3
lC0:
	.ascii "cannot create std::vector larger than max_size()\0"
lC1:
	.ascii "%d\12\0"
lC2:
	.ascii "%u\12\0"
	.section __TEXT,__text_cold,regular,pure_instructions
LCOLDB3:
	.section __TEXT,__text_startup,regular,pure_instructions
LHOTB3:
	.align 4
	.globl _main
_main:
LFB2619:
	pushq	%r15
LCFI11:
	leaq	_cin(%rip), %rdi
	pushq	%r14
LCFI12:
	pushq	%r13
LCFI13:
	pushq	%r12
LCFI14:
	pushq	%rbp
LCFI15:
	pushq	%rbx
LCFI16:
	subq	$40, %rsp
LCFI17:
	leaq	16(%rsp), %rsi
LEHB0:
	call	__ZN4fast7scanner4scanImLPm0EEEvRT_
	leaq	24(%rsp), %rsi
	leaq	_cin(%rip), %rdi
	call	__ZN4fast7scanner4scanImLPm0EEEvRT_
	movq	16(%rsp), %r12
	movabsq	$2305843009213693951, %rax
	cmpq	%rax, %r12
	ja	L84
	testq	%r12, %r12
	je	L14
	leaq	_cin(%rip), %rbx
	salq	$2, %r12
	leaq	__ZN4fastL3bufE(%rip), %r14
	movq	%r12, %rdi
	call	__Znwm
LEHE0:
	movq	%r12, %rdx
	xorl	%esi, %esi
	movq	%rax, %rdi
	movq	%rax, %r15
	call	_memset
	movq	_cin(%rip), %rsi
	addq	%r15, %r12
	movq	%r15, %rbp
L22:
	movq	8+_cin(%rip), %rax
	leaq	20(%rsi), %rdx
	cmpq	%rdx, %rax
	jbe	L85
L15:
	movl	$0, 0(%rbp)
# 50 "fast_io.cpp" 1
	nya:
# 0 "" 2
	movq	(%rbx), %rdx
	movsbl	(%rdx), %eax
	cmpb	$45, %al
	je	L86
	leaq	1(%rdx), %rcx
	subl	$48, %eax
	xorl	%esi, %esi
	movq	%rcx, (%rbx)
	movsbl	1(%rdx), %edx
	cmpb	$47, %dl
	jle	L19
L18:
	leal	(%rax,%rax,4), %eax
	addq	$1, %rcx
	leal	-48(%rdx,%rax,2), %eax
	movq	%rcx, (%rbx)
	movsbl	(%rcx), %edx
	cmpb	$47, %dl
	jg	L18
	testb	%sil, %sil
	je	L19
L17:
	negl	%eax
L19:
	movl	%eax, 0(%rbp)
# 65 "fast_io.cpp" 1
	nyan:
# 0 "" 2
	movq	(%rbx), %rax
	addq	$4, %rbp
	cmpq	%rbp, %r12
	leaq	1(%rax), %rsi
	movq	%rsi, (%rbx)
	jne	L22
	movabsq	$2305843009213693951, %rax
	movq	24(%rsp), %r13
	cmpq	%rax, %r13
	ja	L87
	testq	%r13, %r13
	je	L25
L46:
	salq	$2, %r13
	movq	%r13, %rdi
LEHB1:
	call	__Znwm
LEHE1:
	movq	%r13, %rdx
	xorl	%esi, %esi
	movq	%rax, %rdi
	movq	%rax, (%rsp)
	leaq	_cin(%rip), %rbp
	movq	%rax, %rbx
	call	_memset
	movq	_cin(%rip), %rsi
	addq	%rbx, %r13
L28:
	movq	8+_cin(%rip), %rax
	leaq	20(%rsi), %rdx
	cmpq	%rdx, %rax
	jbe	L88
L26:
	movl	$0, (%rbx)
# 50 "fast_io.cpp" 1
	nya:
# 0 "" 2
	movq	0(%rbp), %rax
	movsbl	(%rax), %edx
	leaq	1(%rax), %rcx
	addq	$2, %rax
	movq	%rcx, 0(%rbp)
	subl	$48, %edx
	movl	%edx, (%rbx)
	movsbl	-1(%rax), %ecx
	cmpb	$47, %cl
	jle	L31
L30:
	leal	(%rdx,%rdx,4), %edx
	movq	%rax, 0(%rbp)
	addq	$1, %rax
	leal	-48(%rcx,%rdx,2), %edx
	movl	%edx, (%rbx)
	movsbl	-1(%rax), %ecx
	cmpb	$47, %cl
	jg	L30
L31:
# 65 "fast_io.cpp" 1
	nyan:
# 0 "" 2
	movq	0(%rbp), %rax
	addq	$4, %rbx
	cmpq	%rbx, %r13
	leaq	1(%rax), %rsi
	movq	%rsi, 0(%rbp)
	jne	L28
	cmpq	%r12, %r15
	je	L44
L43:
	leaq	lC1(%rip), %rbp
	movq	%r15, %rbx
L32:
	movl	(%rbx), %esi
	movq	%rbp, %rdi
	xorl	%eax, %eax
LEHB2:
	call	_printf
	addq	$4, %rbx
	cmpq	%rbx, %r12
	jne	L32
	cmpq	%r13, (%rsp)
	je	L33
L44:
	movq	(%rsp), %rbx
	leaq	lC2(%rip), %rbp
L34:
	movl	(%rbx), %esi
	movq	%rbp, %rdi
	xorl	%eax, %eax
	call	_printf
LEHE2:
	addq	$4, %rbx
	cmpq	%rbx, %r13
	jne	L34
L33:
	movq	(%rsp), %rax
	testq	%rax, %rax
	je	L35
	movq	%rax, %rdi
	call	__ZdlPv
L35:
	testq	%r15, %r15
	je	L61
L45:
	movq	%r15, %rdi
	call	__ZdlPv
L61:
	addq	$40, %rsp
LCFI18:
	xorl	%eax, %eax
	popq	%rbx
LCFI19:
	popq	%rbp
LCFI20:
	popq	%r12
LCFI21:
	popq	%r13
LCFI22:
	popq	%r14
LCFI23:
	popq	%r15
LCFI24:
	ret
L86:
LCFI25:
	leaq	1(%rdx), %rax
	leaq	2(%rdx), %rcx
	movq	%rax, (%rbx)
	movsbl	1(%rdx), %eax
	movq	%rcx, (%rbx)
	movsbl	2(%rdx), %edx
	subl	$48, %eax
	cmpb	$47, %dl
	jle	L17
	movl	$1, %esi
	jmp	L18
L85:
	subq	%rsi, %rax
	leaq	(%r14,%rax), %r13
	movq	%rax, %rdx
	cmpq	%r13, %rsi
	jb	L15
	movq	%r14, %rdi
	movq	%rax, (%rsp)
	call	_memcpy
	movq	(%rsp), %rdx
	movl	$131072, %eax
	movq	%r13, %rdi
	movl	$1, %esi
	subq	%rdx, %rax
	movq	%rax, %rdx
	movq	___stdinp@GOTPCREL(%rip), %rax
	movq	(%rax), %rcx
LEHB3:
	call	_fread
LEHE3:
	addq	%rax, %r13
	movq	%r14, (%rbx)
	movq	%r13, 8+_cin(%rip)
	movb	$0, 0(%r13)
	jmp	L15
L88:
	subq	%rsi, %rax
	movq	%rax, %rdx
	leaq	__ZN4fastL3bufE(%rip), %rax
	leaq	(%rax,%rdx), %r14
	cmpq	%r14, %rsi
	jb	L26
	movq	%rax, %rdi
	movq	%rdx, 8(%rsp)
	call	_memcpy
	movq	8(%rsp), %rdx
	movl	$131072, %edi
	movl	$1, %esi
	movq	___stdinp@GOTPCREL(%rip), %rax
	subq	%rdx, %rdi
	movq	(%rax), %rcx
	movq	%rdi, %rdx
	movq	%r14, %rdi
LEHB4:
	call	_fread
LEHE4:
	addq	%rax, %r14
	leaq	__ZN4fastL3bufE(%rip), %rax
	movq	%r14, 8+_cin(%rip)
	movb	$0, (%r14)
	movq	%rax, 0(%rbp)
	jmp	L26
L14:
	movq	24(%rsp), %r13
	cmpq	%rax, %r13
	ja	L84
	xorl	%r12d, %r12d
	xorl	%r15d, %r15d
	testq	%r13, %r13
	jne	L46
	jmp	L61
L25:
	xorl	%r13d, %r13d
	cmpq	%r12, %r15
	movq	$0, (%rsp)
	jne	L43
	jmp	L45
L84:
	leaq	lC0(%rip), %rdi
LEHB5:
	call	__ZSt20__throw_length_errorPKc
LEHE5:
L87:
	leaq	lC0(%rip), %rdi
LEHB6:
	call	__ZSt20__throw_length_errorPKc
LEHE6:
L54:
	movq	%rax, %rbp
	jmp	L39
L52:
	movq	%rax, %rbp
	jmp	L40
L55:
	movq	%rax, %rbp
	jmp	L42
L53:
	movq	%rax, %rbp
	jmp	L37
	.section __DATA,__gcc_except_tab
GCC_except_table0:
LLSDA2619:
	.byte	0xff
	.byte	0xff
	.byte	0x3
	.byte	0x5b
	.set L$set$0,LEHB0-LFB2619
	.long L$set$0
	.set L$set$1,LEHE0-LEHB0
	.long L$set$1
	.long	0
	.byte	0
	.set L$set$2,LEHB1-LFB2619
	.long L$set$2
	.set L$set$3,LEHE1-LEHB1
	.long L$set$3
	.set L$set$4,L52-LFB2619
	.long L$set$4
	.byte	0
	.set L$set$5,LEHB2-LFB2619
	.long L$set$5
	.set L$set$6,LEHE2-LEHB2
	.long L$set$6
	.set L$set$7,L53-LFB2619
	.long L$set$7
	.byte	0
	.set L$set$8,LEHB3-LFB2619
	.long L$set$8
	.set L$set$9,LEHE3-LEHB3
	.long L$set$9
	.set L$set$10,L55-LFB2619
	.long L$set$10
	.byte	0
	.set L$set$11,LEHB4-LFB2619
	.long L$set$11
	.set L$set$12,LEHE4-LEHB4
	.long L$set$12
	.set L$set$13,L54-LFB2619
	.long L$set$13
	.byte	0
	.set L$set$14,LEHB5-LFB2619
	.long L$set$14
	.set L$set$15,LEHE5-LEHB5
	.long L$set$15
	.long	0
	.byte	0
	.set L$set$16,LEHB6-LFB2619
	.long L$set$16
	.set L$set$17,LEHE6-LEHB6
	.long L$set$17
	.set L$set$18,L55-LFB2619
	.long L$set$18
	.byte	0
	.section __TEXT,__text_startup,regular,pure_instructions
	.section __TEXT,__text_cold,regular,pure_instructions
_main.cold:
LFSB2619:
L37:
LCFI26:
	cmpq	$0, (%rsp)
	je	L40
L39:
	movq	(%rsp), %rdi
	call	__ZdlPv
L40:
	testq	%r15, %r15
	je	L41
L42:
	movq	%r15, %rdi
	call	__ZdlPv
L41:
	movq	%rbp, %rdi
LEHB7:
	call	__Unwind_Resume
LEHE7:
LFE2619:
	.section __DATA,__gcc_except_tab
GCC_except_table1:
LLSDAC2619:
	.byte	0xff
	.byte	0xff
	.byte	0x3
	.byte	0xd
	.set L$set$19,LEHB7-LCOLDB3
	.long L$set$19
	.set L$set$20,LEHE7-LEHB7
	.long L$set$20
	.long	0
	.byte	0
	.section __TEXT,__text_cold,regular,pure_instructions
LCOLDE3:
	.section __TEXT,__text_startup,regular,pure_instructions
LHOTE3:
	.section __TEXT,__text_cold,regular,pure_instructions
	.section __TEXT,__text_startup,regular,pure_instructions
	.align 4
__GLOBAL__sub_I_fast_io.cpp:
LFB3221:
	pushq	%rbx
LCFI27:
	leaq	__ZN4fastL3bufE(%rip), %rbx
	movl	$131072, %edx
	movl	$1, %esi
	leaq	131072(%rbx), %rax
	movq	%rbx, %xmm0
	movq	%rbx, %rdi
	movq	%rax, %xmm1
	movq	___stdinp@GOTPCREL(%rip), %rax
	punpcklqdq	%xmm1, %xmm0
	movaps	%xmm0, _cin(%rip)
	movq	(%rax), %rcx
	call	_fread
	addq	%rax, %rbx
	movq	%rbx, 8+_cin(%rip)
	popq	%rbx
LCFI28:
	ret
LFE3221:
	.globl _cin
	.zerofill __DATA,__pu_bss4,_cin,16,4
	.zerofill __DATA,__bss5,__ZN4fastL3bufE,131073,5
	.section __TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame1:
	.set L$set$21,LECIE1-LSCIE1
	.long L$set$21
LSCIE1:
	.long	0
	.byte	0x1
	.ascii "zPLR\0"
	.byte	0x1
	.byte	0x78
	.byte	0x10
	.byte	0x7
	.byte	0x9b
	.long	___gxx_personality_v0+4@GOTPCREL
	.byte	0x10
	.byte	0x10
	.byte	0xc
	.byte	0x7
	.byte	0x8
	.byte	0x90
	.byte	0x1
	.align 3
LECIE1:
LSFDE1:
	.set L$set$22,LEFDE1-LASFDE1
	.long L$set$22
LASFDE1:
	.long	LASFDE1-EH_frame1
	.quad	LFB2850-.
	.set L$set$23,LFE2850-LFB2850
	.quad L$set$23
	.byte	0x8
	.quad	0
	.byte	0x4
	.set L$set$24,LCFI0-LFB2850
	.long L$set$24
	.byte	0xe
	.byte	0x10
	.byte	0x8e
	.byte	0x2
	.byte	0x4
	.set L$set$25,LCFI1-LCFI0
	.long L$set$25
	.byte	0xe
	.byte	0x18
	.byte	0x8d
	.byte	0x3
	.byte	0x4
	.set L$set$26,LCFI2-LCFI1
	.long L$set$26
	.byte	0xe
	.byte	0x20
	.byte	0x8c
	.byte	0x4
	.byte	0x4
	.set L$set$27,LCFI3-LCFI2
	.long L$set$27
	.byte	0xe
	.byte	0x28
	.byte	0x86
	.byte	0x5
	.byte	0x4
	.set L$set$28,LCFI4-LCFI3
	.long L$set$28
	.byte	0xe
	.byte	0x30
	.byte	0x83
	.byte	0x6
	.byte	0x4
	.set L$set$29,LCFI5-LCFI4
	.long L$set$29
	.byte	0xa
	.byte	0xe
	.byte	0x28
	.byte	0x4
	.set L$set$30,LCFI6-LCFI5
	.long L$set$30
	.byte	0xe
	.byte	0x20
	.byte	0x4
	.set L$set$31,LCFI7-LCFI6
	.long L$set$31
	.byte	0xe
	.byte	0x18
	.byte	0x4
	.set L$set$32,LCFI8-LCFI7
	.long L$set$32
	.byte	0xe
	.byte	0x10
	.byte	0x4
	.set L$set$33,LCFI9-LCFI8
	.long L$set$33
	.byte	0xe
	.byte	0x8
	.byte	0x4
	.set L$set$34,LCFI10-LCFI9
	.long L$set$34
	.byte	0xb
	.align 3
LEFDE1:
LSFDE3:
	.set L$set$35,LEFDE3-LASFDE3
	.long L$set$35
LASFDE3:
	.long	LASFDE3-EH_frame1
	.quad	LFB2619-.
	.set L$set$36,LHOTE3-LFB2619
	.quad L$set$36
	.byte	0x8
	.quad	LLSDA2619-.
	.byte	0x4
	.set L$set$37,LCFI11-LFB2619
	.long L$set$37
	.byte	0xe
	.byte	0x10
	.byte	0x8f
	.byte	0x2
	.byte	0x4
	.set L$set$38,LCFI12-LCFI11
	.long L$set$38
	.byte	0xe
	.byte	0x18
	.byte	0x8e
	.byte	0x3
	.byte	0x4
	.set L$set$39,LCFI13-LCFI12
	.long L$set$39
	.byte	0xe
	.byte	0x20
	.byte	0x8d
	.byte	0x4
	.byte	0x4
	.set L$set$40,LCFI14-LCFI13
	.long L$set$40
	.byte	0xe
	.byte	0x28
	.byte	0x8c
	.byte	0x5
	.byte	0x4
	.set L$set$41,LCFI15-LCFI14
	.long L$set$41
	.byte	0xe
	.byte	0x30
	.byte	0x86
	.byte	0x6
	.byte	0x4
	.set L$set$42,LCFI16-LCFI15
	.long L$set$42
	.byte	0xe
	.byte	0x38
	.byte	0x83
	.byte	0x7
	.byte	0x4
	.set L$set$43,LCFI17-LCFI16
	.long L$set$43
	.byte	0xe
	.byte	0x60
	.byte	0x4
	.set L$set$44,LCFI18-LCFI17
	.long L$set$44
	.byte	0xa
	.byte	0xe
	.byte	0x38
	.byte	0x4
	.set L$set$45,LCFI19-LCFI18
	.long L$set$45
	.byte	0xe
	.byte	0x30
	.byte	0x4
	.set L$set$46,LCFI20-LCFI19
	.long L$set$46
	.byte	0xe
	.byte	0x28
	.byte	0x4
	.set L$set$47,LCFI21-LCFI20
	.long L$set$47
	.byte	0xe
	.byte	0x20
	.byte	0x4
	.set L$set$48,LCFI22-LCFI21
	.long L$set$48
	.byte	0xe
	.byte	0x18
	.byte	0x4
	.set L$set$49,LCFI23-LCFI22
	.long L$set$49
	.byte	0xe
	.byte	0x10
	.byte	0x4
	.set L$set$50,LCFI24-LCFI23
	.long L$set$50
	.byte	0xe
	.byte	0x8
	.byte	0x4
	.set L$set$51,LCFI25-LCFI24
	.long L$set$51
	.byte	0xb
	.align 3
LEFDE3:
LSFDE5:
	.set L$set$52,LEFDE5-LASFDE5
	.long L$set$52
LASFDE5:
	.long	LASFDE5-EH_frame1
	.quad	LFSB2619-.
	.set L$set$53,LCOLDE3-LFSB2619
	.quad L$set$53
	.byte	0x8
	.quad	LLSDAC2619-.
	.byte	0x4
	.set L$set$54,LCFI26-LFSB2619
	.long L$set$54
	.byte	0xe
	.byte	0x60
	.byte	0x83
	.byte	0x7
	.byte	0x86
	.byte	0x6
	.byte	0x8c
	.byte	0x5
	.byte	0x8d
	.byte	0x4
	.byte	0x8e
	.byte	0x3
	.byte	0x8f
	.byte	0x2
	.align 3
LEFDE5:
LSFDE7:
	.set L$set$55,LEFDE7-LASFDE7
	.long L$set$55
LASFDE7:
	.long	LASFDE7-EH_frame1
	.quad	LFB3221-.
	.set L$set$56,LFE3221-LFB3221
	.quad L$set$56
	.byte	0x8
	.quad	0
	.byte	0x4
	.set L$set$57,LCFI27-LFB3221
	.long L$set$57
	.byte	0xe
	.byte	0x10
	.byte	0x83
	.byte	0x2
	.byte	0x4
	.set L$set$58,LCFI28-LCFI27
	.long L$set$58
	.byte	0xe
	.byte	0x8
	.align 3
LEFDE7:
	.ident	"GCC: (Homebrew GCC 9.1.0) 9.1.0"
	.mod_init_func
	.align 3
	.quad	__GLOBAL__sub_I_fast_io.cpp
	.constructor
	.destructor
	.align 1
	.subsections_via_symbols
