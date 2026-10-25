# Pokemon Violet (01008F6008C5E000) v3.0.1: the 60fps static set (Fl4sh9174, all 9 sites) PLUS the
# imgui-callback NOP (0x1526d64) from the Dynamic FPS ASM mod: the vanilla imgui compute function
# (registered at 0x1526d44-64, body at 0x1760ec8) faults the GPU (BIF0 page fault, read at VA 0x40)
# on its first draw (the opening cutscene / gift menu); with the registration suppressed the imgui
# compute goes through the alternative path, which ran fine on the Oct 24 evening build (0fe9589
# carried this exact NOP). Build-ID key = the NSO build id, first 32 hex.
CUTS = {
  '709bfd66115298640155fcc4979dba15': [
    (0x2b7a874, '81060011', '21008052', 'vsyncs-per-frame: add w1,w20,#1 -> mov w1,#1'),
    (0x00b92780, '000040f9', 'c10a94d2', 'fixed-dt: ldr x0,[x0] -> mov x1,#0xa056'),
    (0x00b92784, '010540f9', '813fa0f2', 'fixed-dt: ldr x1,[x8,#8] -> movk x1,#0x1fc,lsl #16'),
    (0x02b7c540, '0031891a', '00008052', 'csel w0,w8,w9,lo -> mov w0,#0'),
    (0x0170c408, '0af0a752', '0a4ea852', 'time scalar 1.0f -> 60.0f'),
    (0x020a8d3c, '68e8a872', '68f8a872', 'movk time scalar 50000 -> 100000'),
    (0x03c2fbb0, '56a0fc01', '2b50fe00', 'EMU frame-time site'),
    (0x00d9643c, '087974f8', 'c80a94d2', 'fixed-dt 2: ldr x8,[x8,x20,lsl#3] -> mov x8,#0xa056'),
    (0x00d96440, '080500d1', '883fa0f2', 'fixed-dt 2: sub x8,x8,#1 -> movk x8,#0x1fc,lsl #16'),
    (0x1526d64, 'e92f05a9', '1f2003d5', 'imgui callback registration NOP (the compute goes through the alternative path; the vanilla one faults the GPU)'),
  ],
}
