MEMSEG segment READ WRITE 'STACK'

IFNDEF _WIN64
	msize dd 3FF0h
	mcapacity dd 0h
	array1  byte  3FF0h DUP(00h)
ELSE
	msize dq 3FF8h
	mcapacity dq 0h
	array1  byte  3FF8h DUP(00h)
ENDIF

MEMSEG ends

IFNDEF _WIN64
.686p
.XMM
.model flat, C
ENDIF

PUBLIC allocMem
PUBLIC getPeb
PUBLIC getLdrData
PUBLIC sicmp
PUBLIC getExeName

.code
IFDEF _WIN64
	getPeb proc
		mov rax, gs:[60h]
		ret
	getPeb endp

	getLdrData proc
		call GetPeb
		mov rax, qword ptr [rax+18h]
		ret
	getLdrData endp

;	getRip proc
;		pop rax
;		push rax
;		ret
;	getRip endp

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

ELSE
	ASSUME FS:NOTHING
	getPeb proc
		mov eax, fs:[30h]
		ret
	getPeb endp

	getLdrData proc
		call GetPeb
		mov eax, dword ptr [eax+0Ch]
		ret
	getLdrData endp

	getExeName proc
		call getLdrData
		mov eax,[eax+14h]
		mov eax,[eax+28h]
		ret
	getExeName endp

	sicmp proc
		xor eax,eax
		push ebx
		push ecx
		push edx
		push esi
		mov esi, dword ptr[esp+14h]
		mov edx, dword ptr[esp+18h]
		mov ecx, dword ptr[esp+1Ch]
		beg:
		push ecx
		movzx ecx, byte ptr[esi]
		cmp cl,0h
		je fin
		sub cl, byte ptr[edx]
		mov ebx,ecx
		neg cl
		cmovl ecx,ebx
		xor ebx,ebx
		cmp cl,20h
		cmove ecx,ebx
		test ecx,ecx
		setne al
		pop ecx
		jne fin_e
		lea esi, dword ptr[esi+ecx+1]
		lea edx, dword ptr[edx+ecx+1]
		jmp beg
		fin:
		xor ebx,ebx
		dec ebx
		sub cl,byte ptr[edx]
		cmovl eax,ebx
		pop ecx
		fin_e:
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret
	sicmp endp


	allocMem proc
		mov eax,dword ptr[esp+4h]
		push ecx
		add eax,mcapacity
		cmp eax,msize
		jg oom
		mov ecx,OFFSET array1
		add ecx,mcapacity
		mov mcapacity,eax
		mov eax,ecx
		jmp fin
		oom:
		xor eax,eax
		fin:
		pop ecx
		ret
	allocMem endp

ENDIF


end