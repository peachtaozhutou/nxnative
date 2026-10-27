# Pokémon Violet (`01008F6008C5E000`)

Update 3.0.1 (`v720896`) over the base game, SDK 13.3.13, NVN 13.3.13. Game Freak's Trinity engine with Wwise sound, Havok and Pia. Modules
`main`, `sdk`, `rtld` and `subsdk0` (the movie module, not loaded: `main` takes nothing from it the host does not supply). Started from the
provisional profile the NSP script wrote from its survey (`dm/games/profile_guess.py`). On the Mac it boots to the frame loop at 30 fps; the
language screen, the opening and play beyond them have not been walked yet.

## What it needs

| Setting | Where | Why |
|---|---|---|
| 20 GB guest block: `mem_reserve 0x500000000`, `mem_alias 0x380000000` | [`game.tsv`](game.tsv) | `StandardAllocator::Initialize(nullptr, 0x300000000)` (12 GB) does not fit the 5 GB layout; see Memory |
| `sys_resource 0x1000000` | [`game.tsv`](game.tsv) | the NPDM declares 16 MB of system resource |
| `pad_layout full` | [`game.tsv`](game.tsv) | the camera is on the right stick |
| Bluetooth no-op hooks | `dm/host/hle_offline.c` | the game initializes the Bluetooth stack at boot and the SDK aborts when `btm:u` cannot hand out its object |
| Nintendo controller passthrough | `dm/host/front_mac.m` | macOS reports a Switch pad's buttons by Nintendo's names; mapping by position swapped A/B and X/Y |

## Memory

- The NPDM declares 16 MB of system resource, so `nn::os::IsVirtualAddressMemoryEnabled` is true and `nninitStartup` (`main+0x167d54c`,
  `main` `709bfd66…`) initializes a `StandardAllocator` over 12 GB (`0x300000000`).
- With the 5 GB layout (4.375 GB of alias region) `CentralHeap::Init` returns an error and `StandardAllocator::Initialize` aborts
  (`sdk+0x32519c`, `Break#1 reason=0`), as in Pokémon Legends: Z-A.
- **Current:** the block is widened to 20 GB with a 14 GB alias region, which holds the SDK's node area and the 12 GB the allocator asks for;
  the heap keeps 5.5 GB for the 3.2 GB application pool. **Mac only:** an iPhone cannot reserve 20 GB (the 6 GB limit), so this profile needs
  Z-A's cuts (allocator 4 GB and a 62 MB node area) before it can run there. Those cuts have not been made.

## Services

- **Bluetooth, found at boot:** `nn::bluetooth::InitializeBle` → `hal::user::InitializeBluetoothUserInterface` →
  `btm::user::InitializeBtmUserInterface`. The host has no adapter, and the SDK's btm user interface aborts when `btm:u` does not return an
  `IBtmUserCore` object (`sdk+0xb8eb4`). The two init entry points and the six `nn::bluetooth::AcquireBle*Event` functions are no-ops in
  `dm/host/hle_offline.c`, the events bound to kernel events that never fire. The game never waits on them without an adapter.
- The rest of the boot's services (account, fs, hid, audio, font, time, oe, aoc) needed nothing beyond the host's existing hooks.

## Files and shaders

- RomFS 9.24 GB in 30 files: `arc/data.trpfs` (7.47 GB, every asset) with its index `arc/data.trpfd`, Wwise banks and packs (1.6 GB) and two
  movies.
- 35,937 shaders: 35,897 BNSH inside the Trinity packs, decoded with the game's own `OodleLZ_Decompress` in the Mac host
  (`dm/host/hle_oodle.c`), and 40 in `main`; 28,299 variation pairs. The split takes about a minute on an M4 Pro.
- DLC: the two add-on contents hold only their keyfile (112 KB each). The game reports both owned and the host mounts them, but there is no
  content to read.

## Rendering

- 30 fps as on the Switch; 28 draws a frame, about 6.8 M translated instructions a submit, 0.2 ms of GPU a present on an M4 Pro.
- **Open:** `image atomics on a block-linear texture` (texture ID 302, target 1) is the one red line; the atomics go to the scratch buffer. It
  has not been traced to a visible artifact.
- The game's command memory runs short and the host lends 256 KB segments; no visible effect so far.

## Input

- The 3D pad layout (the camera is on the right stick). A Switch controller needed the Nintendo passthrough in `front_mac.m`; an Xbox pad keeps
  the position mapping.

## Mac routes

- Not walked yet: the run was stopped at the frame loop after boot, so the language screen, the opening and save creation are untested.

## Found on this game

- **Bluetooth initialization without an adapter** and **Nintendo controller mapping** (general host changes): [system services](../../../docs/reference/services.md),
  [input](../../../docs/development.md).
- **A 20 GB guest block for a 12 GB allocator** is a Mac-only stopgap, not a fix: [memory](../../../docs/reference/memory.md).
