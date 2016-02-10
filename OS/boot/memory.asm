a20_wait_output:
	in al, 0x64
	test al, 1
	jnz a20_wait_output
	ret

a20_wait_input:
	in al, 0x64
	test al, 2
	jnz a20_wait_input
	ret

enable_a20:
	cli
	call a20_wait_input
	mov al, 0xAD
	; Disable keyboard
	out 0x64, al
	
	call a20_wait_input
	mov al, 0xD0
	; Read output port
	out 0x64, al
	
	call a20_wait_output
	in al, 0x60
	push ax
	
	call a20_wait_input
	mov al, 0xD1
	; Write output port
	out 0x64, al
	
	call a20_wait_input
	pop ax
	or al, 2
	out 0x60, al
	
	call a20_wait_input
	mov al, 0xAE
	; Enable keboard
	out 0x64, al
	
	call a20_wait_input
	sti
	ret

detect_memory:
	xor ebx, ebx
	; Write structure starting at 0x15000 (segment = 0x1500)
	mov ax, 0x1500
	mov es, ax
	mov di, 2
.detect_memory_next:
	mov ecx, 24
	mov edx, 0x534D4150
	mov eax, 0xE820
	; Write entry of BIOS Memory-Map Table into [ES:DI]
	int 0x15
	jc .detect_memory_end
	; Check that length of entry is not zero
	jcxz .detect_memory_next
	; Compare Magic-Number
	cmp eax, 0x534D4150
	jne .detect_memory_end
	mov word [es:di - 2], cx
	mov ecx, [es:di + 8]
	or ecx, [es:di + 12]
	; Check that entry is not all zeros
	jz .detect_memory_next
	add di, 26
	cmp ebx, 0
	je .detect_memory_end
	; Check whether DI has reached its maximum value
	cmp di, 0x1000
	jl .detect_memory_next
.detect_memory_end:
	mov ebx, 0x16000
	; Write size of structure into memory address 0x16000
	mov word [ebx], di
	ret