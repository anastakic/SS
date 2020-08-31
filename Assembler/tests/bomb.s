.extern data_in, data_out, timer_cfg, to_digits
.global _start

.section text

_start:
	mov $0x00, %r0			#operand
	mov $0x2000, %psw		#maskiran timer
	mov $term, 0x0006 	#interrupt za terminal
	mov $timer, 0x0004 	#interrupt za terminal
	mov $0x2000, %sp
	
loop: 
	cmp $1, %r4
	jne loop
	
	mov $0x0001, timer_cfg
	mov $0x4000, %psw		#maskiran term
	
	mov %r0, %r1
	div $60,  %r0
	mov $60, %r2
	mul %r0, %r2
	sub %r2, %r1
	
wait: jmp wait
	halt
		
.section terminal
term:
	# =
	cmp $0x3d, data_in
	jeq count

	# ako nisu cifre greska
		cmp $0x2f, data_in
		jgt ok
	return:	int 1
		
	ok:	cmp $0x39, data_in
		jgt return
		###
		
		sub $0x30, data_in
		mul $10, %r0
		add data_in, %r0
		iret		

count:
	mov $1, %r4
	iret
	
timer:	
	push %r0
	push %r1
	mov %r0, %r2
	call to_digits
	pop %r1
	pop %r0
	
	mov $0x3a, data_out(%pc)
	
	push %r0
	push %r1
	mov %r1, %r2
	call to_digits
	pop %r1
	pop %r0
	
	cmp $0, %r1
	jeq mins
	sub $1, %r1
	mov $0x0a, data_out(%pc)
	iret
	
mins:
	cmp $0, %r0
	jeq end
	mov $59, %r1
	sub $1, %r0
	
	
	mov $0x0a, data_out(%pc)
	iret

end:	
	mov $0x0a, data_out(%pc)
	mov $0x42, data_out(%pc)
	mov $0x4f, data_out(%pc)
	mov $0x4f, data_out(%pc)
	mov $0x4d, data_out(%pc)
	mov $0x0a, data_out(%pc)
	halt





