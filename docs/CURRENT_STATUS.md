# Current status — playable v0.1

Built with Unreal Engine **5.8.2** on Windows. Development branch: `codex/spycho-playable-v0.1`. The empty repository received an initial project commit on `main` so the prototype can be reviewed in a draft PR. Do not merge automatically.

## Implemented

- Deliberate replicated first-person movement, crouch and careful walk, primitive handgun view and hold-to-aim iron sights.
- Saved seven-space graybox: six side rooms and a 1.6 m hall, connecting routes, furniture cover, wood/carpet/tile floors and a front entrance. House footprint 12 x 14 m.
- Six-round handgun with twelve reserve rounds, 0.55-second shot interval, audible timed reload, recoil, server-owned ammo and high lethality. No music, crosshair, hit markers or enemy UI.
- Separate penetration component: physical-material asset properties control energy cost, thickness limit and resistance. A reverse trace against the exact convex component finds the exit and actual oblique thickness. Up to six penetrations; unknown surfaces and masonry stop bullets. Direct damage 130, starting energy 100; ordinary 12 cm drywall costs 8.4 energy and remains lethal.
- Replicated entry/exit marks and short physics dust chips. Marks remain for the round; walls keep collision and cannot be walked through.
- Eight original procedural mono WAV assets: floor footsteps, loud gunshot, mechanical reload, doors, impact and sparse building creak. Footstep cadence follows traveled distance. Crouch/careful gains are quieter. Native spatial attenuation/occlusion plus additional wall-count filtering; gunshots use the reliable firing event. No ambient music or constant loop.
- Eleven replicated hinged doors, normal/careful opening, positional sound and blocking collision. Hall doors swing into rooms. Careful close refuses when a pawn is in the closed doorway region.
- Solo patrol pauses along a route in the northeast study, produces real footsteps, takes normal damage and dies. It does not fight back. A second human removes it.
- Two-player listen server/direct IP, authoritative shots/damage/death, replicated round state and automatic five-second reset. Reset restores pawns/ammo, closes doors and clears marks. Third connections are rejected.
- F3-only developer traces, material thickness/energy/damage labels and audio source/gain/obstruction displays. F5 host/practice reset.

## Verification

- SpychoEditor / Win64 / Development builds successfully with MSVC 14.51.36246 and SDK 10.0.26100.0. UE warns that this compiler is newer than its preferred version.
- Unreal automation: `Spycho.Rules.Penetration` and `Spycho.Rules.AmmoAndRound` both pass. Covers material energy costs, successive/oblique thickness, damage limits, ammo conservation and fire eligibility during reload/death/round end.
- Saved-map headless smoke passes actual simulated W movement (~92 cm in 0.6 s), Shift/Ctrl stance input, fire rate lock, reload timing, lethal drywall hit, masonry protection, persistent entry/exit marks, round cleanup, all audio references and careful door toggle/reset.
- Two separate editor game processes pass remote-client firing through drywall, replicated health/death, ammo, marks, round end, door state, removal of the test bot and automatic respawn/reset.
- Windows Development package builds/cooks/stages successfully. Cook reports zero errors/warnings. Packaged smoke verifies the same saved-map systems. Final packaged two-process networking also passes remote wall kill, replicated ammo/evidence/door/death and automatic round reset.
- Actual rendered capture reviewed; fixed exposure keeps lighting readable and dim. Material/map/audio assets exist and are checked in. Generated caches, binaries and Saved files are ignored. `git diff --check` passes.

## Run / reproduce

Open the project and Play the House map, or run `python Tools/run.py play`. For the local packaged output launch `Windows/Spycho.exe`.

Solo: open the entrance with E; move north along the central hallway. The patrol is in the northeast study. Listen near its east wall, then aim and fire through the opaque partition away from the doorway. F5 resets. See README for all controls and `Host` / `Join ADDRESS:7777`.

`python Tools/run.py build`, `test`, `smoke`; `python Tools/network_smoke.py` for real network processes. Pass a packaged inner executable path to that script to test cooking/replication together. `python Tools/package.py OUTPUT_DIRECTORY` packages Win64. Helpers set `COMSPEC` only in their own process; this avoids the machine's inherited missing PowerShell path. No system shell setting was changed.

## Known limits

- This is a primitive graybox with box furniture, cylinder silhouettes and a crude pistol. No hands, finished weapon animations, windows or environmental dressing. The sounds are original placeholders; directional clarity, headphone comfort and the quiet-to-gunshot balance still need human listening/playtests.
- The multiplayer checks run on loopback. Physical two-machine LAN, packet loss/latency and human duel feel have not been playtested. There is no lobby UI, matchmaking or internet relay; direct addressing uses Unreal's console.
- Penetration assumes the convex graybox collision. Concave meshes, layered complex collision and unknown art assets need further treatment before replacing the primitives. Cosmetic holes do not open sight lines or destroy walls.
- Audio wall counts are simple straight-line obstruction estimates, not room/portal acoustics. They preserve useful cues but do not simulate diffraction or reverberation. Sparse creaks are the only room tone.
- The patrol stays within one room and never shoots. No noisy movable props, throwable distraction or tactical light switches yet. Doors interpolate hinge rotation rather than simulate forces, and can intersect a pawn during opening; closing has a simple occupancy guard.
- Death is a simple falling silhouette, with no ragdoll or spectator flow. Remaining players can still walk during the five-second end state, but cannot fire/reload/interact.
- Default Unreal engine plugins remain at engine defaults; only Python/editor scripting tooling was explicitly enabled. No external gameplay dependencies or third-party art/audio were added.

## Next highest-value work

1. Play with two humans using headphones. Tune floor gains, obstruction filtering, gunshot range/dynamic contrast and penetration lethality around the listening fantasy.
2. Tighten sight alignment/feedback, doorway collision and the patrol route based on those sessions. Preserve silence and scarce ammo.
3. Try one bumpable noisy prop or one thrown coin only after the duel is convincing. Keep map size and weapon count fixed.
