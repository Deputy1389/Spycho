# Spycho

A small first-person listening duel. Opaque, paper-thin partitions hide an opponent but let lethal handgun shots through. Find them by their footsteps before they find you.

## Play

Open `Spycho.uproject` in Unreal Engine **5.8.2**, build SpychoEditor / Win64 / Development, then Play `/Game/Maps/House`. Or launch `Windows/Spycho.exe` from the packaged build; keep its adjacent folders intact.

The 14 x 9 m house follows the supplied overhead reference: lounge at one end of a short central hall, study and den on one side, dining and bedroom on the other. You start in the lounge; the armed bot starts in the opposite den. Doors connect the hall and adjacent rooms. Listen, move quietly, and shoot through 2.5 cm partitions. The exterior masonry stops shots. Bullet marks remain until reset.

| Input | Action |
|---|---|
| WASD / mouse | Move / look |
| Hold Shift | Sprint (4.2 m/s), loud footsteps |
| Hold Alt | Slow walk (0.95 m/s), quieter footsteps and doors |
| Hold Ctrl | Crouch (0.85 m/s) |
| Left mouse | One shot per click |
| Hold right mouse | Smooth iron-sight aiming |
| R | Audible 2.1-second reload |
| E | Open/close a door within 1.9 m |
| F3 | Developer traces and noise overlay |
| F5 | Reset solo/listen-host round |
| Tilde | Console |

Six rounds loaded, twelve spare. Mouse-up looks up. Valid shots give immediate sound/recoil; the pistol accepts one shot every **0.16 seconds**. A click during that cooldown queues one follow-up shot, including while aiming. Holding the trigger does not repeat. A lethal hit ends the round; both combatants respawn after five seconds. No crosshair, hit markers or enemy location UI.

The bot patrols connected rooms, pauses, opens doors, and uses the same footsteps, ammunition, damage and penetration as a player. It reacts to visible opponents or a noisy estimated location and can shoot through a wall. Quiet movement reduces its hearing range; it does not track your live position through walls.

## Two-player direct IP

Enter `Host` in the console; in another instance/computer enter `Join HOST_IP:7777`, or `Join 127.0.0.1:7777` locally. UDP 7777 must be reachable. A second human replaces the bot and starts a fresh round. There is no lobby, matchmaking or relay; third connections are rejected.

The server owns cadence, ammo, reload, damage and reset. The owner predicts shot presentation; replicated acceptance does not repeat the owner's sound or recoil. Movement, doors, spatial sounds, wall evidence and death replicate.

## Build and verify

Python 3 helpers discover the installed engine or use `SPYCHO_UE_ROOT`. Windows requires Visual Studio C++ tools and a Windows SDK. This build uses MSVC 14.51 and SDK 10.0.26100; Unreal warns the compiler is newer than preferred.

- `python Tools/run.py build`: editor build.
- `python Tools/run.py test`: penetration/ammunition rule tests.
- `python Tools/run.py smoke`: actual saved-map input, rapid ADS fire, reload, penetration, doors and reset.
- `python Tools/run.py botsmoke`: bot roaming, quiet/noisy hearing, wall shots and visible combat.
- `python Tools/network_smoke.py`: two separate game processes; accepts a packaged inner executable path.
- `python Tools/run.py play`: standalone play.
- `python Tools/package.py OUTPUT_DIRECTORY`: Windows Development package.
- `python Tools/run.py assets`: reimport checked-in source art, rebuild House, and apply final materials/signs. **Replaces hand edits to House.**

All map/material/audio/art assets are checked in. Editor Python is only for asset generation. Helpers set COMSPEC locally to cmd.exe. See [asset sources and licenses](docs/ASSETS.md), [verification and limitations](docs/CURRENT_STATUS.md). Original procedural audio tooling remains `Tools/generate_audio.py`.
