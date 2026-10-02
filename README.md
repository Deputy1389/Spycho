# Spycho

A small first-person duel: listen, infer, manipulate, commit. Opaque interior walls block sight but often let a lethal handgun shot through. Silence and scarce ammunition make each shot matter.

## Requirements and run

Unreal Engine **5.8.2**, Windows x64, Visual Studio C++ build tools and a Windows SDK. This machine has MSVC 14.51 and Windows SDK 10.0.26100. UE warns that MSVC 14.51 is newer than its preferred compiler.

Open `Spycho.uproject`, build the **SpychoEditor / Development Editor / Win64** target, and press Play in the saved `/Game/Maps/House` map. All map, material, physical material and sound assets are checked in. Python plugins are editor-only and used for original asset generation; the game systems are C++.

The single-player game creates a damageable patrol in the northeast study. Enter through the front door. Listen from the north end of the central hall, aim into the east partition away from its doorway, and commit to a shot. F5 resets practice. The patrol pauses between steps and never shoots; it is a test opponent.

| Input | Action |
|---|---|
| WASD / mouse | Grounded movement / look |
| Hold Ctrl | Crouch |
| Hold Shift | Careful walk; quieter door use |
| Left mouse | Fire one shot |
| Hold right mouse | Align primitive iron sights |
| R | Audible 2.1-second reload |
| E | Open/close the door you face within 1.9 m |
| F3 | Local developer overlay, bullet traces and noise sources |
| F5 | Reset round (practice/listen host only) |
| Tilde | Unreal console |

Six rounds loaded, twelve in reserve, no pickups or crosshair. Direct hits and one ordinary drywall penetration are lethal. Wood spends more energy; masonry stops bullets. Wall marks persist until reset. Movement is 2.1 m/s, careful walk 1.05 m/s, crouch 0.85 m/s.

## Two-player LAN / direct IP

Run two standalone game instances, or Play with **2 players / Play As Listen Server** and **Use Single Process disabled**. In a standalone instance, enter `Host` in the console to open the house as a listen server. On the other machine/instance enter `Join 192.168.1.10:7777` (replace the address; `Join 127.0.0.1:7777` locally). UDP 7777 must be reachable. The second human replaces the practice patrol and starts a fresh round. There is no matchmaking or online service.

The server owns firing cadence, ammo, reload, damage and round reset. Pawn movement uses Unreal character replication. Doors, positional sounds, impacts and death replicate. Five seconds after a kill, both players respawn with fresh ammunition and all marks are cleared. Third connections are rejected.

## Build and verify

Run `Tools/run.py build` with local Python 3; it discovers the installed engine from Epic's installation manifest or `SPYCHO_UE_ROOT`. `Tools/run.py test` runs the Unreal rule tests. `Tools/run.py smoke` launches the real saved map headlessly and checks lethal drywall penetration, solid masonry and round cleanup. `Tools/run.py play` opens a standalone practice instance.

Physical materials in `Content/Surfaces` expose resistance per centimeter, entry cost, maximum thickness and floor noise gain. Ballistic defaults are on the character's penetration component. Original WAV tooling is `Tools/generate_audio.py`; optional map regeneration is `Tools/run.py assets` (replaces the graybox map). Do not regenerate casually after editing the map by hand.

`Tools/network_smoke.py` runs two separate editor game processes on local port 17777, verifies a remote shot and replicated reset, then exits. Pass a packaged `Spycho/Binaries/Win64/Spycho.exe` path to test the cooked build instead. `Tools/package.py OUTPUT_DIRECTORY` builds/cooks a Development Windows package. The helpers set `COMSPEC` locally to `cmd.exe` because this machine's inherited shell setting points to a missing PowerShell executable.

For the packaged prototype, launch the top-level `Windows/Spycho.exe`. No editor is required. Host and Join console commands and all controls above work there too.

See `docs/CURRENT_STATUS.md` for actual verification and limitations.
