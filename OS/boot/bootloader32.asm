[BITS 16]
[ORG 0x7C00]

%macro load_kernel 2 ; drive number, number of sectors to read
	jmp %%reset
	%%dap:
	db 0x10
	db 0x00
	dw %2
	dd 0x08000000
	dq 1
	%%reset:
		xor ax, ax
		xor dh, dh
		mov dl, %1 ; Drive number
		; Reset the drive
		int 0x13
		jc %%reset
	%%load:
		mov ah, 0x42
		mov dl, %1
		mov si, %%dap
		; Load the kernel into memory
		int 0x13
		jc %%load
%endmacro
	
; -------------------- CODE --------------------
start:	
	xor ax, ax
	mov ds, ax
	
	; Set to text-mode
	mov ax, 0x0002
	int 0x10
	
	; Set curser shape
	mov ah, 0x01
	mov cx, 0x0607
	int 0x10
	
	call enable_a20
	
	call detect_memory
	
	load_kernel 0x80, 100
	
	; Set GDTR
	lgdt[GDTR]
	
	; Enter Protected Mode
	cli
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	
	jmp 0x08:pmode

%include "boot/memory.asm"

; -------------------- DATA --------------------

GDT:
; Null Selector
dq 0x0000000000000000
; Code Selector
dq 0x00CF9A000000FFFF
; Data Selector
dq 0x00CF92000000FFFF

GDTR:
GDTsize dw 0x0018
GDTbase dd GDT

; -------------------- PMODE -------------------
[BITS 32]

pmode:
	mov ax, 0x0010
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov esp, 0x20000
	jmp 0x08:0x8000


; -------------------- END ---------------------
times 510-($-$$) db 0
dw 0xAA55