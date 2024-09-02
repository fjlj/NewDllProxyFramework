MEMSEG segment READ WRITE EXECUTE 'STACK'
	msize dq 2000h
	mcapacity dq 0h
	array1  byte  8192 DUP(90h)
MEMSEG ends

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

	allocMem proc
		mov rax,rcx
		add rax,mcapacity
		cmp rax,msize
		jg oom
		mov rcx,OFFSET array1
		add rcx,mcapacity
		mov mcapacity,rax
		mov rax,rcx
		jmp fin
		oom:
		xor rax,rax
		fin:
		ret
	allocMem endp


end