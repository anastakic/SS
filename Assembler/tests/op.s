.global add_func, sub_func, div_func, mul_func
.section text

add_func:
	add %r4, %r2
	ret

sub_func:
	sub %r4, %r2
	ret

div_func:
	cmp $0, %r4
	jeq end
	div %r4, %r2
	ret
end: int 1

mul_func:
	mul %r4, %r2
	ret

