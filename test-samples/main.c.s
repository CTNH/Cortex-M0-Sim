main:
loop:
	adds r0, #2
	adds r1, #1
	cmp r0, #10
	bne loop

adds r2, #0  ;8
cmp r2, #1   ;a
beq l1		 ;c
cmp r2, #2	 ;e
beq l2		 ;10
cmp r2, #3	 ;12
beq l3		 ;14

b l5	; Non-existent label
b l4	; 16
l1:				;18
	adds r3, #1 ;18
l2:				;1a
	adds r3, #2 ;1a
l3:				;1c
	adds r3, #3 ;1c
l4:			;1e
	adds r4, #10	;1e

lsls r4, r4, #2
	nop				;20

movs r0, #200
asrs r0, r0, #3

