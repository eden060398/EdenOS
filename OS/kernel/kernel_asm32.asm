[section .text]

[global start]
[extern kernel_main]
start:
	call kernel_main
.hang: hlt
	jmp .hang


%include "kernel/isr32.asm"
	
[global load_idtr]
[extern idtp]
load_idtr:
	lidt[idtp]
	ret