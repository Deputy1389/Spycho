# Spycho

A small first-person listening duel. Opaque, paper-thin partitions hide an opponent but let lethal handgun shots through. Find them by their footsteps before they find you.

## Play

Open `Spycho.uproject` in Unreal Engine **5.8.2**, build SpychoEditor / Win64 / Development, then Play `/Game/Maps/House`. Or launch `Windows/Spycho.exe` from the packaged build; keep its adjacent folders intact.

The house is now a 30 x 24 m ground-floor Old Hollywood mansion, almost six times the original floor area. Fifteen rooms and gallery areas form two flanking loops around the private hall: a chequered-stone grand foyer, walnut library, ballroom, piano room, screening salon, dressing room and quieter residential rooms. Worn plaster, tarnished brass, faded velvet, cold fireplaces and pools of lamplight give it an eerie, faded-luxury atmosphere. Twenty-three usable doors and four open arches connect the wings. Six paired starts rotate through the mansion, swapping sides each round. Listen, move quietly, and shoot through opaque 2.5 cm partitions. Exterior masonry stops shots. Bullet marks remain until reset.

| Input | Action |
|---|---|
| WASD / mouse | Move / look |
| Hold Shift | Sprint (4.2 m/s), loud footsteps |
| Hold Ctrl (Alt also works) | Slow walk (0.95 m/s), quieter footsteps and doors |
| Left mouse | One shot per click |
| Hold right mouse | Smooth iron-sight aiming |
| R | Audible 2.1-second reload |
| E | Open/close a door within 2.4 m |
| F3 | Developer traces and noise overlay |
| Q | Toss a coin (two per round) |
| Enter | Rematch after a match ends (solo/listen host) |
| F5 | Restart solo/listen-host match |
| Escape | Start/pause menu and settings |
| Tilde | Console |

Escape opens the start/pause menu: solo play, direct-IP host/join, Cautious/Standard/Sharp bots, sensitivity, separate action/ambience volumes, brightness and a left/right headphone test. Solo pauses; online play continues. Settings persist locally.

Six rounds loaded, twelve spare. Mouse-up looks up. Valid shots give immediate sound/recoil; the pistol accepts one shot every **0.16 seconds**. A click during that cooldown queues one follow-up shot, including while aiming. Holding the trigger does not repeat. A lethal hit ends the round; the winner earns one point and both combatants respawn after five seconds. First to three wins. Each round has a three-second ready gate and 120-second clock. Timeouts award no point and swap positions; three consecutive stalemates end the match as a draw. Enter starts a rematch. The result panel explains the lethal shot after death. No crosshair, hit markers or enemy location UI.

The bot slow-walks, listens, cautiously investigates sound areas, pauses at doorways and relocates after inferred gunfire. It uses the same ammunition, footsteps, damage and penetration as a player. It only updates exact target position when both chest and head are visible. Hidden targets supply no live location updates. Gunfire can prompt one delayed, uncertain wall shot; other clues prompt investigation. A coin can distract it toward the landing rather than the thrower. Difficulty changes reaction delay, hearing uncertainty and aimed-shot spread. On fresh visual contact, its first shot intentionally misses; it backs off before slower lethal follow-ups. This warning does not apply to inferred wall shots.

Movement starts/stops more promptly without increasing top speeds. Pistol fire/reload animations, restrained sway/recoil, shell ejection and automatic lowering near walls improve handling. Doors have visible handles/insets, swing away from the operator, pause for occupied space, and show a small E prompt. Open door routes preserve more sound detail; closed doors and successive walls reduce volume/high frequencies. Short room-dependent reverb is restrained. The house uses the supplied dim hallway reference: desaturated plaster, dark wood, wall sconces, layered trim and framed art.

## Two-player direct IP

Use Host/Join in the menu, or enter `Host` in the console; in another instance/computer enter `Join HOST_IP:7777`, or `Join 127.0.0.1:7777` locally. UDP 7777 must be reachable. A second human replaces the bot and starts a fresh match. There is no lobby, matchmaking or relay; third connections are rejected.

The server owns cadence, ammo, reload, damage and reset. The owner predicts shot presentation; replicated acceptance does not repeat the owner's sound or recoil. Movement, doors, spatial sounds, wall evidence and death replicate.

## Build and verify

Python 3 helpers discover the installed engine or use `SPYCHO_UE_ROOT`. Windows requires Visual Studio C++ tools and a Windows SDK. This build uses MSVC 14.51 and SDK 10.0.26100; Unreal warns the compiler is newer than preferred.

- `python Tools/run.py build`: editor build.
- `python Tools/run.py test`: penetration/ammunition/acoustic rule tests.
- `python Tools/run.py smoke`: actual saved-map input, rapid ADS fire, reload, penetration, doors and reset.
- `python Tools/run.py botsmoke`: bot roaming, quiet/noisy hearing, wall shots and visible combat.
- `python Tools/run.py mansionsmoke`: all 78 route links, 15 room centres, 12 production spawn positions, 23 door mappings, open-arch acoustics, cross-room wall penetration and real bot traversal without silent-player knowledge.
- `python Tools/run.py experiencesmoke`: production ready gate, round clock, difficulty, real pause/input restoration, settings save/load, animated slide, live sound mix and draw/rematch flow.
- `python Tools/run.py polishsmoke`: real coin bounce/deception, room-route clearance, door acoustics, weapon lowering and best-of-five/rematch checks.
- `python Tools/network_smoke.py`: two separate game processes; accepts a packaged inner executable path.
- `python Tools/run.py play`: standalone play.
- `python Tools/package.py OUTPUT_DIRECTORY`: Windows Development package.
- `python Tools/run.py assets`: reimport checked-in source art, rebuild the mansion and apply its faded atmosphere. **Replaces hand edits to House.**

All map/material/audio/art assets are checked in. Editor Python is only for asset generation. Helpers set COMSPEC locally to cmd.exe. See [asset sources and licenses](docs/ASSETS.md), [verification and limitations](docs/CURRENT_STATUS.md). Recorded CC0 audio replaces the original placeholders, with four footstep variants per surface. Optional audio regeneration: install numpy and soundfile, then run `Tools/prepare_recorded_audio.py` and `Tools/run.py assets`. Door panels overlap their frames; E uses a nearby visible door within a forgiving facing cone and also works during the round countdown. Crouch has been removed.

### Mansion layout and art

`Tools/mansion_layout.py` is the source of truth for rooms, partitions, 66 navigation points, 78 route links, portals and starts. `Tools/generate_mansion_layout.py` writes the checked-in native layout header; the build helper runs it before compiling. `Tools/build_mansion.py` rebuilds the saved House map from the same layout. Geometry remains single-storey; there is no upstairs or exterior exploration in this pass. Furniture blocks movement; decorative trim and landmarks use no collision. Route/spawn capsule sweeps check the actual map. Lumen GI/reflections and TSR remain enabled. Actual captures include foyer, library, ballroom, music room, west gallery, screening salon and the roofless layout (`python Tools/run.py capture --view foyer`, etc.).

## Held pistol and sound pass

The view and opponent weapons use the skinned pistol with an animated slide, trigger and magazine. A native animation pose blends gripping idle, correct mesh-space additive fire and reload; the firing fingers keep their grip while the support hand handles the removed magazine. Hip/ADS/reload transitions, anchored hands, close-wall lowering and immediate shot feedback remain. The empty slide locks back. These are template animations with procedural part timing, not a bespoke reload rig.

Sounds retain their original source for obstruction checks and update their low-pass filter and volume while playing. An open-door route pans from its final doorway with the whole route's attenuation distance. Live volume controls distinguish house ambience from action sounds. Spatial propagation uses a 15-room portal approximation; headphones and human playtesting are needed to judge perceptual balance.

The legacy smoke/bot/polish/network checks bypass the opening countdown to test established behavior quickly. Experience smoke uses the production ready gate. Automated captures suppress bot decisions and use default settings; overhead layout alone adds a presentation fill light.
