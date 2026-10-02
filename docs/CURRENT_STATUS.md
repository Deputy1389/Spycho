# Current status — playable v0.2

Unreal Engine **5.8.2**, Windows Development. Branch `codex/spycho-playable-v0.1`; review in draft PR #1, do not merge automatically.

## Implemented

- Reference-inspired 14 x 9 m house: lounge, short hall, study/den and dining/bedroom. Seven hinged doors, room labels, furnished rooms, parquet/plaster textures, soft ceiling lights and frosted window panels.
- CC0 Kenney furnishings and ambientCG textures, plus Epic template pistol, first-person hands, animated Manny opponent and death ragdoll. See ASSETS.md for licenses.
- Correct vertical mouse look, smooth aiming, immediate predicted shot feedback, 0.16-second semiautomatic cadence and one queued click throughout cooldown. Rapid clicks remain accepted during ADS. Six loaded/twelve reserve; audible 2.1-second reload.
- Replicated sprint (Shift, 4.2 m/s), slow walk (Alt, 0.95 m/s), normal walk (2.1 m/s) and crouch (Ctrl, 0.85 m/s). Footsteps follow distance traveled; sprint is loud, slow/crouch quiet.
- Armed bot patrols the connected rooms, opens doors and reloads. It reacts after a delay to visible opponents or a noisy estimated position; the estimate is frozen while the target is hidden. It can fire through walls. A second human removes the bot.
- Opaque 2.5 cm interior walls block movement/sight and pass lethal handgun shots. Measured convex penetration consumes energy by actual thickness/material; masonry and unknown surfaces stop bullets. Persistent entry/exit evidence clears on round reset.
- Positional footsteps, gunshots, reloads, doors, impacts and sparse creaks; simple wall obstruction filtering. No music, enemy UI or hit markers.
- Authoritative two-player direct-IP/listen-server duel, replicated ammo/death/evidence/doors, automatic five-second reset. F5 host/solo reset; F3 optional developer overlay.

## Verification

Editor build and two Unreal rule tests pass. Saved-map checks cover actual movement/mouse/stance inputs, immediate shot feedback, six rapid shots while ADS, timed reload, wall kill, masonry protection, doors, sounds and reset. Bot checks cover roaming with actual footsteps, quiet-noise rejection, loud-noise investigation, an inferred wall shot and lethal visible combat. Two-process network checks cover one remote-client feedback event, wall kill, replicated ammo/evidence/door/death and automatic reset. Windows cooking/packaging and equivalent packaged checks are recorded in the deliverable VALIDATION.md. Rendered room, bot, ADS and plan views are inspected.

## Reproduce

See README for controls, direct-IP setup and Tools/run.py build/test/smoke/botsmoke. Tools/network_smoke.py accepts the packaged inner executable. Tools/package.py creates a Windows build. Tools/run.py assets rebuilds from checked-in art sources and replaces House map edits.

## Known limits

- The bot uses a small authored route graph and basic visibility/hearing decisions. It is playable opposition, not an advanced tactical AI. Its reactions and the sound balance need human headphone playtesting.
- Visuals are a first imported-asset pass with simple animation states and procedural weapon motion. Audio remains synthesized. No finished character customization, reload hand animation or polished effects.
- Networking is verified on loopback, not physical LAN, packet loss or internet latency. No lobby/matchmaking/relay.
- Penetration depends on convex primitive wall collision. Holes are cosmetic; walls do not become transparent or destructible. Furniture meshes are not a general layered penetration simulation.
- Acoustics use straight-line obstruction estimates rather than room portals/reverberation. Doors interpolate hinge rotation and may intersect a pawn while opening; closing has a simple occupancy guard.
- No thrown distractions, movable noisy props or tactical light switches. Players can walk during the five-second round end but cannot fire/reload/interact.
