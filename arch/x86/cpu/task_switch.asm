section .text
global task_switch
extern current_task

task_switch:
    pusha
    pushf

    mov eax, [current_task]
    mov [eax], esp

    mov ebx, [esp+40]        
                              

    mov esp, [ebx]            

    mov eax, [current_task]
    mov ecx, [eax+4]
    mov edx, [ebx+4]
    cmp ecx, edx
    je .skip_pd
    mov cr3, edx
.skip_pd:

    mov [current_task], ebx

    popf
    popa
    ret
