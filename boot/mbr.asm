;
; Note: this example is written in Intel Assembly syntax
;
[BITS 16]

boot:
; Load rest of bootloader from disk
mov ah, 0x2
mov al, 1 ; Num sectors to read
mov ch, 0 ; Cylinder
mov cl, 2 ; Sector
mov dh, 0 ; Head
xor bx, bx
mov es, bx ; Buffer offset
mov bx, 0x7e00
int 0x13
a:
jmp a


times 510-($-$$) db 0

db 0x55
db 0xaa
