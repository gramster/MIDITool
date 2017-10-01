; IO401.ASM  c calls for mpu-401 input/output.

DATAPORT  EQU	330H	;MPU401 DATA PORT
STATPORT  EQU	331H	;MPU401 STATUS PORT
DRR	EQU	040H
MPUACK	EQU	0FEH	;MPU401 CODES: ACKNOWLEGE

TRIES	EQU	0FFH	;MAXIMUM TRIES ON GETTING RESPONSE FROM 401
;----------------------------------------------------------------
PUBLIC	_putcmd
;----------------------------------------------------------------

SAVSTK	MACRO		;MACRO TO SAVE REGS FOR C FUNCTION
	PUSH	BP
	MOV	BP,SP
	PUSH	DI
	PUSH	SI
	ENDM
	
RCLSTK	MACRO		;MACRO TO RECALL SAVED REGS FOR C
	POP	SI
	POP	DI
	MOV	SP,BP
	POP	BP
	ENDM

;----------------------------------------------------------------

STACK  	SEGMENT para stack 'STACK'
STACK   ENDS

_DATA  	SEGMENT word public 'DATA'
_DATA	ENDS

_TEXT	SEGMENT byte public 'CODE'
	ASSUME CS:_TEXT, DS:_DATA, SS:STACK, ES:NOTHING

;----------------------------------------------------------------
;	putcmd(n)
;	output a byte of data to MPU401, check for acknowledge

_putcmd	PROC near
	SAVSTK
	MOV	DX,STATPORT
	MOV	CX,TRIES	;RETRY COUNTER IN CL
LBL1:	IN	AL,DX		;READ STATUS
	TEST	AL,DRR		;FIND IF BIT 6 = 1
	JZ	LBL2		;OK SO CONTINUE
	DEC	CX
	CMP 	CX,1		;USED UP ALL TRIES?
	JGE	LBL1		;RETRY
	MOV	AX,-1
	JMP	LBL5  		;QUIT RETURNING -1

LBL2:	CLI
	MOV	AX,[BP+4]	;PUT CHAR (AS INT) IN AX (AL)
	OUT 	DX,AL		;OUTPUT CHAR

	MOV	CX,TRIES
LBL3:	IN	AL,DX		;READ STATUS
	ROL	AL,1		;PUT BIT 7 TO CARRY
	JNB	LBL4 		;IF CARRY <> 1, NOT READY
	DEC	CX
	CMP 	CX,1		;USED UP ALL TRIES?
	JGE   	LBL3		;RETRY
	MOV	AX,-1
	JMP	LBL5 		;QUIT RETURNING -1

LBL4:	MOV	DX,DATAPORT	;ELSE READ DATA
	IN 	AL,DX
	CMP	AL,MPUACK
	JZ	LBL5		;GOT ACK SO RETURN
	MOV	AX,-1		;IF NOT, RETURN -1
LBL5:	STI			;ENABLE INTERUPTS
	RCLSTK
	RET
_putcmd	ENDP

;----------------------------------------------------------------
;	getdata()
;	get a byte of data from MPU401

;_getdata PROC near
;	SAVSTK
;	MOV	DX,STATPORT
;	MOV	CX,TRIES	;RETRY COUNTER IN CL
;LBL6:	IN	AL,DX		;READ STATUS
;	ROL	AL,1		;PUT BIT 7 TO CARRY
;	JNB	LBL7		;IF CARRY <> 1, NOT READY
;	DEC	CX
;	CMP 	CX,1		;USED UP ALL TRIES?
;	JGE   	LBL6		;RETRY
;	MOV	AX,-1
;	JMP	GEND  		;QUIT RETURNING -1
;
;LBL7:	MOV	DX,DATAPORT	;ELSE READ DATA
;	MOV	AH,0		;CLEAR AH FOR C RETURN
;	IN 	AL,DX
;GEND:	RCLSTK
;	RET
;_getdata ENDP

;----------------------------------------------------------------
;	putdata()
;	send a byte of data to MPU401

;_putdata PROC near
;	SAVSTK
;	MOV	DX,STATPORT
;	MOV	CX,TRIES	;RETRY COUNTER IN CL
;LBL8:	IN	AL,DX		;READ STATUS
;	TEST	AL,DRR		;TEST BIT 6
;	JZ	LBL9		;OK SO CONTINUE
;	DEC	CX
;	CMP 	CX,1		;USED UP ALL TRIES?
;	JGE   	LBL8		;RETRY
;	MOV	AX,-1
;	JMP	LBL10  		;QUIT RETURNING -1
;
;LBL9:	MOV	DX,DATAPORT
;	MOV	AX,[BP+4]	;PUT DATA IN AX (AL)
;	OUT	DX,AL
;LBL10:	RCLSTK
;	RET
;_putdata ENDP


;----------------------------------------------------------------
_TEXT	ENDS
	END

