# Pokemon Violet (01008F6008C5E000) v3.0.1: 60fps enablement (Fl4sh9174's static 60FPS v4.0.0
# pchtxt, all 9 sites verbatim). Build-ID key is the NSO build id, first 32 hex.
CUTS = {
  '709bfd66115298640155fcc4979dba15': [
    (0x2b7a874, '81060011', '21008052', 'vsyncs-per-frame: add w1,w20,#1 -> mov w1,#1'),
    (0xb92780, '000040f9', 'c10a94d2', 'fixed-dt constant: ldr x0,[x0] -> movz x1,#0xa056,lsl#16'),
    (0xb92784, '010540f9', '813fa0f2', 'fixed-dt constant: ldr x1,[x8,#8] -> movk x1,#0x81fc,lsl#32'),
    (0x2b7c540, '0031891a', '00008052', 'csel w0,w8,w9,lo -> mov w0,#0'),
    (0x170c408, '0af0a752', '0a4ea852', 'time scalar 1.0f -> 0.5f-class constant'),
    (0x20a8d3c, '68e8a872', '68f8a872', 'movk time scalar'),
    (0xd9643c, '087974f8', 'c80a94d2', 'ldr x8,[x8,x20,lsl#3] -> movz x1,#0xa056,lsl#16'),
    (0xd96440, '080500d1', '883fa0f2', 'sub x8,x8,#1 -> movk x1,#0x81fc,lsl#32'),
  ],
}
