[section .text]

[bits 64]

; void pic_disable();
; void lapic_enable();
; void lapic_enable_spurious();
; uint32_t lapic_read(uint32_t reg);
; void lapic_write(uint32_t reg, uint32_t val);