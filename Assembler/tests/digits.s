.extern data_out
.global to_digits

.section nums
to_digits: 

	mov $0, %r1
	mov $0, %r3
	mov $0, %r4
	mov $0, %r5
	
	cmp $10000, %r2
	jgt d
	cmp $1000, %r2
	jgt f
	cmp $100, %r2
	jgt h
	cmp $10, %r2
	jgt k
	jeq k
	jmp p

d:	mov %r2, %r1
	div $10000, %r1
	mov %r1, %r3
	
	add $0x30, %r1
	mov %r1, data_out
	
f:	mul $10000, %r3
	sub %r3, %r2
	mov %r2, %r3
	div $1000, %r3
	mov %r3, %r4
	
	add $0x30, %r3
	mov %r3, data_out
	
h:	mul $1000, %r4
	sub %r4, %r2
	mov %r2, %r4
	div $100, %r4
	mov %r4, %r5
	
	add $0x30, %r4
	mov %r4, data_out
	
k:	mul $100, %r5
	sub %r5, %r2
	mov %r2, %r5
	div $10, %r5
	mov %r5, %r1
	
	add $0x30, %r5
	mov %r5, data_out
	
p:	mul $10, %r1
	sub %r1, %r2
	
	add $0x30, %r2
	mov %r2, data_out

	ret
