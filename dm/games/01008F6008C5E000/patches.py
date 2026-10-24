# Pokemon Violet (01008F6008C5E000) v3.0.1: 60fps enablement (Fl4sh9174's static 60FPS v4.0.0
# pchtxt, all 9 sites verbatim). Build-ID key is the NSO build id, first 32 hex.
# These bytes are what the shipped image carries and what BOTH 帧率上限 modes run: the game loop is
# paced at 60 (interval clamp 1) in both, and the 30 档 holds every frame for two refreshes on the
# display link (nvn_mtl.m) — 60 fps simulation, 30 fps on screen. Restoring the shipped words at 30
# (two user-tested rounds, down to all eight timing sites) still walked at half speed: the engine's
# per-frame step count (main+0xcd27bc, [slot+0x180]) cannot be fixed from the timing sites. The site
# table lives in hle_nvn.c k_fps_sites as a load-time check of these bytes.
CUTS = {
  '709bfd66115298640155fcc4979dba15': [
    (0x2b7a874, '81060011', '21008052', 'vsyncs-per-frame: add w1,w20,#1 -> mov w1,#1'),
    (0xb92780, '000040f9', 'c10a94d2', 'fixed-dt constant: ldr x0,[x0] -> movz x1,#0xa056,lsl#16'),
    (0xb92784, '010540f9', '813fa0f2', 'fixed-dt constant: ldr x1,[x8,#8] -> movk x1,#0x81fc,lsl#32'),
    (0x2b7c540, '0031891a', '00008052', 'csel w0,w8,w9,lo -> mov w0,#0'),
    (0x170c408, '0af0a752', '0a4ea852', 'time scalar 1.0f -> 0.5f-class constant'),
    (0x20a8d3c, '68e8a872', '68f8a872', 'movk time scalar'),
    (0x3c2fbb0, '56a0fc01', '2b50fe00', 'EMU frame-time site'),
    (0xd9643c, '087974f8', 'c80a94d2', 'ldr x8,[x8,x20,lsl#3] -> movz x1,#0xa056,lsl#16'),
    (0xd96440, '080500d1', '883fa0f2', 'sub x8,x8,#1 -> movk x1,#0x81fc,lsl#32'),
  ],
}
