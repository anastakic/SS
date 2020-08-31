.global _start
.extern data_out, data_in, timer_cfg, print_num

.section timer
_start:
.equ nula, 0
			mov $0x2155, %sp
			mov $ivt_terminal, 0x0006
			mov $ivt_timer, 0x0004
			mov $0x0001, timer_cfg
			mov $0, %r0
			
			for: jmp for
	
ivt_timer:
			add $1, %r0
			mov $0xa, data_out
			push %r0
			call print_num
			pop %r0
			iret
			
ivt_terminal: 		
			halt
			push $5
			call print_num
			pop %r5
			iret
	
.end
