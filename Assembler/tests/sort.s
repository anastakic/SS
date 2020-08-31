# main.s - Bubblesort algorithm demonstration.
.extern print_num, data_in, data_out
.global _start

# niz od adrese 0x0050
.equ array, 0x0500


.section text
_start:
			
			mov $0x2000, %psw		#maskiran timer
			mov $terminal, 0x0006 	#interrupt za terminal
			mov $0x2000, %sp
			
			mov $0, %r0				#r0 za size
			loop: jmp loop
			halt
	
	
.section term_sec
terminal:	
			#mov data_in, data_out
			
			cmp $0x20, data_in
			jeq save_num
			
			cmp $0x3d, data_in
			jeq save_num
			
			# ako nisu cifre ignorisi
				cmp $0x2f, data_in
				jgt ok
			return:	int 1
				
			ok:	cmp $0x39, data_in
				jgt return
				###
			
			
			sub $0x30, data_in
			mul $10, %r2
			add data_in, %r2
			iret
	
	
save_num:
		
			cmp $0, %r2
			jeq ignore

			mov %r2, array(%r0)
			mov $0, %r2
			add $2, %r0
			
			cmp $0x3d, data_in
			jeq sort
			
ignore:
			cmp $0x3d, data_in
			jeq sort
			
			iret
	
sort: 
			mov %r0, %r1
			mov %r0, %r2
			add $array, %r1			#u r1 pocetak sortiranog niza
			mov %r1, %r5
			mov %r0, %r4
			sub $2, %r0			#r0 na poslednji elem niza
			
			
for:
			call find_min
			mov %r3, (%r1)
			
			cmp $0, %r0
			jeq print
			
			sub $2, %r0
			add $2, %r1
			
			jmp for
			
print:	
			#mov $0x20, data_out
			mov $0x53, data_out
			mov $0x4f, data_out
			mov $0x52, data_out
			mov $0x54, data_out
			mov $0x20, data_out
			#od r5 do r2
			add %r5, %r2
			
printloop:	push (%r5)
			
			mov (%r5), %r1
			push %r1
			
			call print_num
			pop %r1
			
			mov $0x20, data_out
			
			add $2, %r5
			cmp %r5, %r2
			jeq end_func
			
			
			jmp printloop
			
end_func:	halt


find_min:	
			push %r1
			push %r4
			
			mov $0x7ffe, %r3	#u r3 min
			sub $2, %r4
			
doloop:	
			cmp array(%r4), %r3
			jgt new_min
			jmp cond
			
new_min:	mov array(%r4), %r3	#u r3 min
			mov %r4, %r1		#u r1 index min
			
cond:		
			cmp $0, %r4
			jeq end
			
			sub $2, %r4
			jmp doloop
			
end:		mov $0x7fff, array(%r1)
			
			pop %r4
			pop %r1
			ret

.end	
