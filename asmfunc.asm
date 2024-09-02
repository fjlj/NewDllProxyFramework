.code	
	getPeb proc
		mov rax, gs:[60h]
		ret
	getPeb endp

	getLdrData proc
		call GetPeb
		mov rax, qword ptr [rax+18h]
		ret
	getLdrData endp

	getRip proc
		pop rax
		push rax
		ret
	getRip endp
	
	sicmp proc
		xor rax,rax
		push rbx
		beg:
		push r8
		movzx r8, byte ptr[rcx]
		cmp r8b,0h
		je fin
		sub r8b, byte ptr[rdx]
		mov rbx,r8
		neg r8b
		cmovl r8,rbx
		xor rbx,rbx
		cmp r8b,20h
		cmove r8,rbx
		test r8,r8
		setne al
		pop r8
		jne fin_e
		lea rcx, qword ptr[rcx+r8+1]
		lea rdx, qword ptr[rdx+r8+1]
		jmp beg
		fin:
		xor rbx,rbx
		dec rbx
		sub r8b,byte ptr[rdx]
		cmovl rax,rbx
		pop r8
		fin_e:
		pop rbx
		ret
	sicmp endp

	getExeName proc
		call getLdrData
		mov rax,[rax+20h]
		mov rax,[rax+50h]
		ret
	getExeName endp

;	makeOffset proc
;		pop rax
;		push rax
;		add rax,rcx
;		ret
;	makeOffset endp

;	runPatch proc
;		cmp rcx,1
;		jne run
;		call retadr
;		retadr:
;		pop rax
;		ret
;		run:
;		array1  byte  1431 DUP(90h)
;		ret
;		array2  byte  523 DUP(90h)
;		ret
;	runPatch endp

end