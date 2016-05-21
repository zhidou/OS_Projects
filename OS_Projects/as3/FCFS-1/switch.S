.global switch_to

switch_to:

    testl %esi,%esi
    jz 1f

    # push all flags register and general register
    pushfl
    pushal

    # push segment register
    pushw %ds
    pushw %es
    pushw %fs
    pushw %gs

    # This part is beautiful! is brilliant!
    # (%esi) means mov thing in esp to the place esi point to!
    # 64 means 64 bytes after the entry of esi, indeed the place we 
    # store stack pointer!!!!! So we save it there!!
    movl %esp, (%esi)

1:
    # This part is more brilliant!!
    # It seems it just load the stack pointer of the new thread, yes it does
    # But another important things here is, when we run the thread first time
    # it will automatically set the stack pointer register for us!!!
    # we never need to set it in the C code! This brilliant! 
    #call success
    movl (%edi) , %esp

    popw %gs
    #pushw %gs
    #call success
    popw %fs

    #pushw %fs
    #call success
    popw %es
    popw %ds

    popal
    popfl

    ret
