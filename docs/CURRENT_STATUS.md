# Current status — playable v0.4

Unreal Engine 5.8.2; Win64 Development. Branch codex/spycho-playable-v0.1; draft PR #1 stays unmerged.

## This milestone

- Three-second ready gate, 75-second duel clock, five-second result break and alternating paired room starts. Timeouts award no score and advance positions. Three consecutive stalemates finish as a draw. First to three wins; Enter rematches. Death feedback distinguishes clear shots from shots through a wall/door and explains a bot's sight or gunshot evidence after the round.
- Cautious, Standard and Sharp bot profiles tune reactions, sound uncertainty and aimed spread. Fresh visible contact begins with a warning miss; the bot backs off before slow lethal follow-ups. Hidden-player tracking still uses only remembered audible events. Silent hidden players provide no room knowledge. No living enemy UI or hit confirmation.
- Native animation blends gripping idle, a properly applied additive fire pose and reload. Skinned pistol slide/trigger/magazine move; empty slide locks back. Removed magazine follows the support hand; firing fingers retain their grip. Anchored two-hand assembly, responsive 0.16-second semi-auto fire and one buffered ADS click remain.
- Active sound filters and volume update with listener movement, door state and settings. Open-door routes pan toward their final doorway while retaining route attenuation distance. Four recorded footstep variants per surface and restrained room reverb remain. Left/right headphone test in settings does not create bot evidence.
- Start/pause menu with solo, direct-IP host/join, bot difficulty, sensitivity, master/action/ambience volume and brightness. Solo pauses; online continues. Returning clears held actions and restores movement/look. Settings save locally. Slightly brighter default exposure preserves the dim old-house style while improving readability.
- Smaller persistent entry holes, larger exit damage and varied thin dust chips. Opaque thin-wall penetration, masonry protection, cosmetic shell ejection and server-owned health/ammo/doors/evidence remain.
- Q coin deception, Ctrl/Alt standing slow walk, sprint, usable sealed doors, route clearance and the residential visual reference remain.

## Verify

Tools/run.py supports build, test, smoke, botsmoke, polishsmoke, experiencesmoke and captures including menu/fire/aim/reload. Tools/network_smoke.py runs two real processes and accepts a packaged inner executable. The legacy tests bypass opening grace; experiencesmoke checks the production gate. Automated settings do not overwrite the player's save. Packaged validation and actual rendered views are in the delivery report.

## Limits

Template mannequin, pistol and animations remain. The reload uses procedural part timing with template hand motion; bespoke animation polish is still needed. Acoustic routing is a six-room approximation with two short reverb profiles, not physical propagation. Bot navigation uses an authored graph. Bullet holes are cosmetic and penetration assumes convex collision. Direct-IP networking has no matchmaking/relay; loopback does not establish two-machine latency behavior. Lumen performance is unmeasured on other PCs. No human full-match or headphone playtest was performed; automated checks establish behavior, not fun.
