.global _start

.section text
.word 0xaaaa
pom: .word 0xccdd
.word 0xbbbb

.section data
_start:
mov pom(%pc), %r1
push pom(%pc)
pop %r2
halt

