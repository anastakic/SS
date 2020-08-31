.extern add_func, sub_func, div_func, mul_func, data_in, data_out, to_digits
.global _start

.section main_sec
_start:
	mov $0x00, %r3
	mov $0x00, %r4			#operand
	mov $0x2000, %psw		#maskiran timer
	mov $terminal, 0x0006 	#interrupt za terminal
	mov $0x2000, %sp
	
	loop: jmp loop
	halt
	
.section term_sec
terminal:
	#mov data_in, data_out
	
	# +
	cmp $0x2b, data_in
	jeq instr
	
	# -
	cmp $0x2d, data_in
	jeq instr
	
	# *
	cmp $0x2a, data_in
	jeq instr
	
	# /
	cmp $0x2f, data_in
	jeq instr
	
	# =
	cmp $0x3d, data_in
	jeq equal
	
	# ako nisu cifre ignorisi
	cmp $0x2f, data_in
	jgt ok
return:	iret
	
ok:	cmp $0x39, data_in
	jgt return
	###
	
	sub $0x30, data_in
	mul $10, %r4
	add data_in, %r4
	iret
	
equal: 	
	cmp $0x2b, %r3
	jne mm
	call add_func
	
mm:	cmp $0x2d, %r3
	jne nn
	call sub_func
	
nn:	cmp $0x2a, %r3
	jne tt
	call mul_func
	
tt:	cmp $0x2f, %r3
	jne pp
	call div_func
	
pp:	mov $0x20, data_out	#
	cmp $0, %r2
	jeq pozitivan
	jgt pozitivan
	
negativan:
	mov $0x2d, data_out	#
	mov $0xffff, %r3 
	xor %r3, %r2
	sub %r3, %r2
	
pozitivan:
	call to_digits 
	halt

instr:	
	cmp $0x2b, %r3
	jeq add
	
	cmp $0x2d, %r3
	jeq sub
	
	cmp $0x2a, %r3
	jeq mul
	
	cmp $0x2f, %r3
	jeq div
	
	mov data_in, %r3
	mov %r4, %r2
	mov $0, %r4
	iret
	
add: call add_func
	 mov $0, %r4
	 mov data_in, %r3
	 iret

sub: call sub_func
	 mov $0, %r4
	 mov data_in, %r3
	 iret

mul: call mul_func
	 mov $0, %r4
	 mov data_in, %r3
	 iret

div: call div_func
	 mov $0, %r4
	 mov data_in, %r3
	 iret

