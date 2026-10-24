.text
.p2align 2
.global lab_anim_value
.hidden lab_anim_value
.type lab_anim_value, %function
// Original Rust Option<f32> returns presence in w0 and value in s0.
// C's mixed-field aggregate return ABI differs, so bridge explicitly.
// x0 = original AnimFloat*, x1 = validated original now_opt function.
lab_anim_value:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    blr x1
    tbnz w0, #0, 1f
    fmov s0, wzr
1:
    ldp x29, x30, [sp], #16
    ret
.size lab_anim_value, .-lab_anim_value
.section .note.GNU-stack,"",%progbits
