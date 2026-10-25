// 金手指 cheats (the player's cheat-file folder, DM_CHEATS_DIR): AMS-style sections parsed from
// cheats/<first 16 hex of main's build id>.txt. Each [name] section is one toggleable cheat; its
// code lines are type-0 "store static value" words written into main's text (0TMR00AA AAAAAAAA
// VVVVVVVV: width nibble / address = main + offset / value). Disabling writes the word from the
// file's [还原代码] restore section back (the original bytes for exactly the sites the pack's
// cheats touch). Offsets owned by the frame-cap machinery (k_fps_sites in hle_nvn.c) are skipped
// in both directions so the two never fight. Non-store code lines (the pack's conditional guards)
// are skipped per cheat with one log.
// Menu: the front end's 金手指 submenu (dm_cheat_count / dm_cheat_name / dm_cheat_enabled /
// dm_cheat_toggle). Writes reuse the frame-cap pattern: mprotect RW, store, i-cache invalidate,
// back to R+X (hle_nvn.c k_fps_sites).
#include "hle.h"
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <libkern/OSCacheControl.h>

#define CHEAT_MAX 128
#define CHEAT_CODE_MAX 96
#define CHEAT_NAME_MAX 160
typedef struct { uint32_t off, val; int size; } cheat_code;   // size in bytes (1/2/4/8; 8 = two words)
typedef struct { char name[CHEAT_NAME_MAX]; cheat_code codes[CHEAT_CODE_MAX]; int ncodes, on; } cheat_t;
typedef struct { volatile void *at; uint64_t val; int size; } cheat_job;
static cheat_t g_cheats[CHEAT_MAX];
static int g_ncheat;
static struct { uint32_t off; uint64_t val; int size; } g_restore[256];
static int g_nrestore;
static int g_parsed;   // -1 = parse attempted and failed / nothing usable
static pthread_mutex_t g_cheat_mu = PTHREAD_MUTEX_INITIALIZER;

// The frame-cap timing sites (hle_nvn.c k_fps_sites): owned by the cap machinery, never touched here
static const uint32_t k_owned_offs[] = { 0x2b7a874, 0x0b92780, 0x0b92784, 0x02b7c540, 0x170c408, 0x20a8d3c, 0x0d9643c, 0x0d96440, 0x3c2fbb0 };
static int off_owned(uint32_t off) {
    for (unsigned k = 0; k < sizeof k_owned_offs / sizeof *k_owned_offs; k++) if (k_owned_offs[k] == off) return 1;
    return 0;
}

static int hexv(char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'A' && c <= 'F' ? c - 'A' + 10 : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1; }
static uint32_t hex32(const char *s) { uint32_t v = 0; for (int i = 0; i < 8; i++) { int h = hexv(s[i]); if (h < 0) return 0; v = v << 4 | h; } return v; }

// One code line: 0TMR00AA AAAAAAAA VV... (width T = 1/2/4/8 bytes; an 8-byte value's high word
// continues in a second value word when present). Adds as size-sized write at main+AA.
static int cheat_parse_code(const char *p, cheat_code *out) {
    if (hexv(p[0]) != 0) return 0;
    int width = hexv(p[1]);
    if (width != 1 && width != 2 && width != 4 && width != 8) return -1;
    if (hexv(p[2]) != 0) return 0;   // M: main only
    uint32_t off = hex32(p + 8);
    if (!off) return 0;
    uint64_t val = hex32(p + 16);
    int size = width > 4 ? 8 : width;
    if (width == 8 && strlen(p) >= 32) val = (uint64_t)hex32(p + 24) << 32 | val;
    if (size < 8) val &= size == 1 ? 0xffu : size == 2 ? 0xffffu : 0xffffffffu;
    *out = (cheat_code){ off, (uint32_t)val, size };
    return 1;
}

void dm_cheats_init(void) {
    static int once; if (once++) return;
    const char *bid = dm_main_build();
    if (!bid[0]) return;
    char upper17[17];
    for (int i = 0; i < 16 && bid[i]; i++) upper17[i] = bid[i] >= 'a' && bid[i] <= 'f' ? bid[i] - 32 : bid[i];
    upper17[16] = 0;
    const char *dir = getenv("DM_CHEATS_DIR");
    if (!dir) { LOG("cheats: DM_CHEATS_DIR unset, no 金手指"); g_parsed = -1; return; }
    char path[1024];
    snprintf(path, sizeof path, "%s/%s.txt", dir, upper17);
    FILE *f = fopen(path, "r");
    if (!f) { LOG("cheats: no cheat file %s for this build", path); g_parsed = -1; return; }
    LOG("cheats: parsing %s", path);
    char line[512];
    int in_restore = 0, cur = -1, skip_types = 0, skip_owned = 0;
    while (fgets(line, sizeof line, f)) {
        for (char *e = line + strlen(line); e > line && (e[-1] == '\n' || e[-1] == '\r' || e[-1] == ' ' || e[-1] == '\t'); e--) *--e = 0;
        char *p = line; while (*p == ' ' || *p == '\t') p++;
        if (!*p) continue;
        {   // the code fields are space-separated on disk: compact to one hex run for parsing
            char *w = p;
            for (char *r = p; *r; r++) if (*r != ' ' && *r != '\t') *w++ = *r;
            *w = 0;
        }
        if (!*p) continue;
        if (*p == '[') {   // section header
            int was_restore = in_restore;
            in_restore = strstr(p, "还原") != NULL;
            if (strstr(p, "--SectionStart:") || strstr(p, "--SectionEnd")) { cur = -1; (void)was_restore; continue; }
            if (in_restore || g_ncheat >= CHEAT_MAX) { cur = -1; continue; }
            cheat_t *c = &g_cheats[g_ncheat];
            snprintf(c->name, CHEAT_NAME_MAX, "%.*s", (int)strcspn(p + 1, "]"), p + 1);
            c->ncodes = 0; c->on = 0;
            cur = g_ncheat++;
            continue;
        }
        if (strlen(p) < 24 || hexv(p[0]) < 0) continue;
        cheat_code code;
        int pr = cheat_parse_code(p, &code);
        if (pr <= 0) { if (pr < 0) skip_types++; continue; }
        if (off_owned(code.off)) { skip_owned++; continue; }
        if (in_restore && g_nrestore < (int)(sizeof g_restore / sizeof *g_restore))
            g_restore[g_nrestore++] = (typeof(g_restore[0])){ code.off, code.val, code.size };
        else if (!in_restore && cur >= 0 && g_cheats[cur].ncodes < CHEAT_CODE_MAX)
            g_cheats[cur].codes[g_cheats[cur].ncodes++] = code;
        else skip_types++;
    }
    fclose(f);
    int real = 0;
    for (int i = 0; i < g_ncheat; i++) if (g_cheats[i].ncodes) { if (real != i) g_cheats[real] = g_cheats[i]; real++; }
    g_ncheat = real;
    g_parsed = g_ncheat > 0 ? 1 : -1;
    LOG("cheats: %d 金手指 parsed, %d restore words%s%s", g_ncheat, g_nrestore,
        skip_types ? " (non-store lines skipped)" : "", skip_owned ? " (frame-cap-owned offsets skipped)" : "");
}
int dm_cheat_count(void) { dm_cheats_init(); return g_parsed > 0 ? g_ncheat : 0; }
const char *dm_cheat_name(int i) { return i >= 0 && i < g_ncheat ? g_cheats[i].name : ""; }
int dm_cheat_enabled(int i) { return i >= 0 && i < g_ncheat ? g_cheats[i].on : 0; }

static int32_t restore_word(uint32_t off, int size) {
    for (int i = 0; i < g_nrestore; i++) if (g_restore[i].off == off && g_restore[i].size == size) return (int32_t)g_restore[i].val;
    return -1;
}

void dm_cheat_toggle(int i) {
    dm_cheats_init();
    if (i < 0 || i >= g_ncheat) return;
    pthread_mutex_lock(&g_cheat_mu);
    cheat_t *c = &g_cheats[i];
    int turning_on = !c->on;
    module_t *mm = NULL;
    for (int k = 0; k < g_nmod; k++) if (!strcmp(g_mod[k].name, "main")) mm = &g_mod[k];
    if (!mm) { pthread_mutex_unlock(&g_cheat_mu); return; }
    cheat_job jobs[CHEAT_CODE_MAX * 2];
    int nj = 0, unres = 0;
    for (int k = 0; k < c->ncodes; k++) {
        cheat_code *code = &c->codes[k];
        if (off_owned(code->off)) continue;
        uint64_t want; int have = 1;
        if (turning_on) want = code->val;
        else { int32_t r = restore_word(code->off, code->size); if (r < 0) { unres++; continue; } want = code->size == 8 ? (uint64_t)(uint32_t)r : (uint64_t)(uint32_t)r; }
        for (int b = 0; b < code->size; b += 4) {   // 8-byte values as two adjacent words (page runs stay contiguous)
            uint64_t at = (uint64_t)(mm->base + code->off + b);
            uint32_t cur = *(volatile uint32_t *)at, w = (uint32_t)(want >> (32 * b));
            if (cur == w) continue;
            if (code->off + b >= mm->text_end) { unres++; continue; }
            jobs[nj].at = (volatile void *)at; jobs[nj].val = w; jobs[nj].size = 4; nj++;
        }
    }
    for (int i = 1; i < nj; i++) { cheat_job t = jobs[i]; int j = i - 1; while (j >= 0 && jobs[j].at > t.at) { jobs[j + 1] = jobs[j]; j--; } jobs[j + 1] = t; }
    for (int i = 0; i < nj; ) {
        uint64_t pg = (uint64_t)jobs[i].at & ~0x3fffull;   // 16K pages (arm64 macOS)
        int j = i; while (j < nj && ((uint64_t)jobs[j].at & ~0x3fffull) == pg) j++;
        if (mprotect((void *)pg, 0x4000, PROT_READ | PROT_WRITE)) {
            LOG("[RED] 金手指 %s: mprotect RW failed (%s)", c->name, strerror(errno));
        } else {
            for (int k = i; k < j; k++) *(volatile uint32_t *)jobs[k].at = jobs[k].val;
            sys_icache_invalidate((void *)jobs[i].at, (size_t)((uintptr_t)jobs[j - 1].at - (uintptr_t)jobs[i].at) + 4);
            if (mprotect((void *)pg, 0x4000, PROT_READ | PROT_EXEC))
                LOG("[RED] 金手指: back to R+X failed (%s) — page left writable", strerror(errno));
        }
        i = j;
    }
    c->on = turning_on;
    LOG("cheats: %s「%s」(%d words written%s)", turning_on ? "开启" : "关闭", c->name, nj, unres ? "; 无还原字的跳过" : "");
    pthread_mutex_unlock(&g_cheat_mu);
}
