.extern _start
.global ivt_0, ivt_1, ivt_2, ivt_3, ivt_4, ivt_5, ivt_6, ivt_7

.global timer_cfg, data_out, data_in
.equ data_out, 0xff00
.equ data_in, 0xff02
.equ timer_cfg, 0xff10


.section ivt

ivt_0:	push $_start
		iret

ivt_1:	mov $0x45, 0xff00
		mov $0x52, 0xff00
		mov $0x52, 0xff00  		
		mov $0x4f, 0xff00
		mov $0x52, 0xff00
		mov $0x5f, 0xff00
		mov $0x69, 0xff00
		mov $0x76, 0xff00 
		mov $0x74, 0xff00
		mov $0x31, 0xff00
		mov $0x0a, 0xff00
		halt

ivt_2:	iret
ivt_3:	iret
ivt_4:	iret
ivt_5:	iret
ivt_6:	iret
ivt_7:	iret

