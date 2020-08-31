.extern data_out, to_digits
.global _start

.equ n, array_end-array_start

.section data
array_start: 
	.word 0x0001, 0x0002, 0x0003, 0x0004, 0x0005
	.word 0x0006, 0x0007, 0x0008, 0x0009, 0x000a
	.word 0x000b, 0x000c, 0x000d, 0x000e, 0x000f
	.word 0x0010, 0x0011, 0x0012, 0x0013, 0x0014
	.word 0x0015, 0x0016, 0x0017, 0x0018, 0x0019
	.word 0x001a, 0x001b, 0x001c, 0x001d, 0x001e
	.word 0x001f, 0x0020, 0x0021, 0x0022, 0x0023
	.word 0x0024, 0x0025, 0x0026, 0x0027, 0x0028
array_end: .skip 1	

.section text
_start:	
	mov $n, %r1
	cmp $0, %r1
	jeq end

loop:
	mov %r1, %r3
	add $array_start, %r3
	sub $2, %r3
	add (%r3), %r2
	
	push %r1
	push %r2
	mov (%r3), %r2
	call to_digits
	
	pop %r2
	pop %r1
	
	sub $2, %r1
	cmp $0, %r1
	jeq jump_plus
	mov $0x2b, data_out
jump_plus:
	cmp $0, %r1
	jne loop 	
	
end:
	mov $0xa, data_out
	mov $0x3d, data_out
	mov $0x20, data_out
	call to_digits
	halt
