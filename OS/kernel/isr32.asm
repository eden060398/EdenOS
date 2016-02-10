[extern exception_handler]
%macro exception_isr 1
[global exception%1]
exception%1:
	pushfd
	pushad
	push dword %1
	call exception_handler
	add esp, 4
	popad
	popfd
	iret
%endmacro

[extern reserved_handler]
%macro reserved_isr 1
[global reserved%1]
reserved%1:
	pushfd
	pushad
	push dword %1
	call reserved_handler
	add esp, 4
	popad
	popfd
	iret
%endmacro

[extern irq_handler]
%macro irq_isr 1
[global irq%1]
irq%1:
	pushfd
	pushad
	push dword %1
	call irq_handler
	add esp, 4
	popad
	popfd
	iret
%endmacro

exception_isr 0
exception_isr 1
exception_isr 2
exception_isr 3
exception_isr 4
exception_isr 5
exception_isr 6
exception_isr 7
exception_isr 8
exception_isr 9
exception_isr 10
exception_isr 11
exception_isr 12
exception_isr 13
exception_isr 14
exception_isr 15
exception_isr 16
exception_isr 17
exception_isr 18

reserved_isr 19
reserved_isr 20
reserved_isr 21
reserved_isr 22
reserved_isr 23
reserved_isr 24
reserved_isr 25
reserved_isr 26
reserved_isr 27
reserved_isr 28
reserved_isr 29
reserved_isr 30
reserved_isr 31

irq_isr 0
irq_isr 1
irq_isr 2
irq_isr 3
irq_isr 4
irq_isr 5
irq_isr 6
irq_isr 7
irq_isr 8
irq_isr 9
irq_isr 10
irq_isr 11
irq_isr 12
irq_isr 13
irq_isr 14
irq_isr 15