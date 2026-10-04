# Current status — playable v0.3.2

Unreal Engine 5.8.2, Windows Development. Branch codex/spycho-playable-v0.1; draft PR #1 remains unmerged.

## This milestone

The v0.3 handling, sound clues, bot intentions and deception/replay systems are retained. The dim residential hallway pass is retained. This patch corrects the pistol floating away from the fingers: both view and opponent weapons follow the hand's grip socket with a calibrated palm offset at native scale. Animation-finalized anchoring keeps the entire first-person arm assembly together; recoil no longer translates the weapon separately. A mesh-space additive firing clip is no longer played as a complete pose; procedural recoil uses the stable gripping pose. ADS and hip placement are adjusted for the corrected weapon dimensions.

- Deliberate movement retains 2.1 m/s normal, 0.95 m/s Ctrl/Alt slow and 4.2 m/s Shift sprint, with faster acceleration/braking. Correct mouse look, responsive 0.16-second semi-auto firing and one queued click while ADS remain.
- A stable gripping pose during shots, animated reload hands, restrained sway/bob, recoil recovery, cosmetic ejected cases and a close-wall weapon lowering trace. Six rounds loaded, twelve spare; 2.1-second reload.
- E door prompts, handles/insets, forgiving selection, swing away from the operator, pawn clearance checks and authoritative replicated panel angle. Closed panels seal side/header gaps and remain penetrable.
- Shared acoustic obstruction rules for humans/bot: multiple walls and closed doors muffle and reduce volume; open door routes preserve more detail. Modest room-dependent reverb; recorded CC0 material footsteps (four each), pistol/reload and wood/metal foley. No music or enemy indicators.
- Bot patrol/listen/investigate/hold/relocate intentions. Hearing gives an uncertain area estimate; it waits before checking it. Visible chest/head checks alone supply exact target updates. Loud gunfire can provoke one inferred wall shot, then relocation. New coins can redirect older clues; later bounces preserve the initial estimate. F3 alone shows bot intention/debug information.
- Q tosses a server-owned bouncing coin. Two per round; up to three audible bounces. The bot hears the impact location, never a hidden thrower's position. Coins and their count replicate; reset removes old coins and restores two.
- First to three / best of five, a small score display, alternating sides and three paired start layouts, five-second next rounds, decisive match finish and Enter rematch for solo/listen host. F5 restarts a match. Connecting/disconnecting a second human starts a fresh match.
- Same 14 x 9 m reference layout. Pale desaturated plaster with normal-map relief; dark varnished wood with grain-dependent roughness; cool end-of-hall sash window; alternating frosted-glass/bronze wall sconces; stepped cornices, tall skirting and door casings; raised three-panel doors; original aged landscape paintings in dark layered frames. Ceiling fixtures and oversized room signs are removed. Lumen global illumination/reflections and TSR replace the previous unlit/reflection-free rendering setup. Capsule sweeps verify all authored bot routes remain clear.
- Existing opaque thin-wall lethal penetration, masonry protection, persistent entry/exit evidence, replicated authoritative damage/ammo/death/round flow and direct-IP two-human play remain.

## Verification / reproduction

Editor build; three Unreal rule tests; saved-map firing/movement/door checks; bot silent-hearing/doorway/wall-shot/combat checks; and new polish checks run. Equivalent packaged checks, loopback host/client replication and captured rendered views are recorded in the deliverable VALIDATION.md.

The roofless overhead layout capture adds a presentation-only directional fill for readability. Hallway and first-person captures use the authored game lighting.

Use Tools/run.py build, test, smoke, botsmoke, polishsmoke or play. Tools/network_smoke.py accepts a packaged inner executable. Tools/package.py creates the Windows build. Tools/run.py assets reimports checked-in audio/art, regenerates House and applies materials; it replaces manual map edits. Tools/run.py capture supports fire alongside aim/reload captures. Optional audio processing needs numpy + soundfile. Tools/style_reference_house.py reapplies the atmosphere after the base map/art build. Decorative geometry has no collision; panel collision and route positions are retained. No runtime Python or new external engine plugins.

## Limits / next validation

- This is a closer atmosphere/material match, not a claim of photographic parity. Stock low-poly furnishings and template character/weapon silhouettes remain. Lumen adds GPU cost; performance on other PCs is unmeasured.
- Human playtesting must assess responsiveness, tension, perceived direction and headphone balance. Automated passes establish behavior, not fun.
- Furnishings remain lightweight low-poly CC0 meshes; mannequin opponent and template pistol use simple animation switching. No bespoke character art or skinned pistol slide/magazine animation. Reload hands are animated, but gun parts are static.
- Acoustic propagation is an authored six-room open-door approximation plus straight-line blockers, not a physical sound simulation. Reverb has two short room profiles. Long sound filters are sampled when the event starts.
- Bot decisions use a small route graph. It has imperfect hearing/sight, not advanced tactical navigation. Its visible fire is still highly lethal.
- Penetration requires convex collision; holes are cosmetic. Walls do not become traversable. No noisy movable furniture or tactical light switches yet; the coin is the one distraction tool.
- Networking is verified on one computer with two processes, not two-machine LAN, internet latency or packet loss. No lobby/matchmaking/relay.

Next: play several full matches on headphones, then a two-human LAN session. Tune audible range, investigation timing and recoil around observed failures before adding more features.
