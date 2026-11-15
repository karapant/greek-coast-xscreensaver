# Image generation

## assets/fish.png

Generated with the built-in imagegen tool and imagegen skill, copied into the project; no runtime generation service is required. Exact prompt:

```text
Use case: photorealistic-natural
Asset type: transparent diffuse fish texture atlas for small underwater Mediterranean shoals in a realistic OpenGL Greek island screensaver. SQUARE image, THREE equal HORIZONTAL ROWS, one fish per row. Each fish lies horizontally facing RIGHT, exactly dorsal TOP-DOWN view as seen looking down into clear sea, not side profile. Full fish entirely within its own row, identical neutral diffuse illumination and size convention; fish length about 80 percent of image width, body thickness about one quarter of its length. TOP ROW: small Mediterranean bogue (Boops boops), slender silvery body, subtle pale olive back, fine realistic scales, forked translucent tail, restrained silver/gold longitudinal detail. MIDDLE ROW: salema (Sarpa salpa), slightly deeper silver body with delicate gold longitudinal stripes, olive dorsal ridge, pale fins and forked tail. BOTTOM ROW: small Mediterranean damselfish (Chromis chromis), charcoal brown gray body, subtle dark scales, modest rounded fins, forked tail. Anatomically believable ordinary small fish, 12 to 25 cm species, realistic dorsal silhouettes and tiny eyes toward sides of head, no monstrous exaggerated features. No bubbles, no water, no scene background, no ground shadows, no labels, no text, no outlines, no grid lines. Genuine transparent alpha outside each fish and between fins; no gray matte, no white backdrop, no checkerboard baked in. Natural photographic texture detail, neutrally lit so software can add underwater absorption and caustics. Tail extends left, head right, dorsal back toward viewer in every row.
```

## assets/sailing-materials.png

Generated with the built-in imagegen tool and imagegen skill, copied into the project; no runtime generation service is required. Exact prompt:

```text
Use case: product-mockup
Asset type: diffuse material atlas for a true 3D traditional Greek wooden sailing caique in an OpenGL coastal screensaver. WIDE 2:1 rectangle with FOUR exactly equal VERTICAL panels, each exactly one quarter of image width. Material swatches only, not a boat illustration. Panel 1 dark honey and warm walnut varnished wooden hull planks, beautiful fine horizontal grain, subtle Mediterranean salt weathering, well maintained. Panel 2 deep traditional oxblood/burgundy painted wooden gunwale trim, subtle straight grain and restrained age, not bright red. Panel 3 pale honey teak deck planks, horizontal grain, fine narrow dark caulking, sun bleached tactile wood. Panel 4 undyed warm IVORY LINEN SAIL CLOTH, subtly weathered flax canvas weave with a few fine long parallel sail-panel stitches, even diffuse color, soft delicate texture, no dark logos or emblems. All panels flat seamless material swatches with consistent neutral diffuse lighting, no directional shadows, no highlights, no perspective, no waves or sky, no boat silhouette, no text or labels or borders. Photographic realistic Mediterranean marine materials. Keep fourth panel light warm ivory suitable for a triangular lateen sail lit and deformed by software.
```

## assets/ferry-materials.png

Generated with the built-in imagegen tool and imagegen skill, copied into the project; no runtime generation service is required. Exact prompt:

```text
Use case: product-mockup
Asset type: diffuse material atlas for a true 3D small Greek inter-island passenger ferry in a realistic OpenGL screensaver. WIDE 2:1 rectangle with FOUR equal VERTICAL material panels, exactly one quarter of total width each, no gaps. This is a MATERIAL TEXTURE, not a photograph of a vessel. Panel 1 aged but well maintained warm white painted marine STEEL, fine subtle horizontal hull seams and restrained salt wear. Panel 2 classic rich Aegean royal blue painted marine steel, subtle tiny surface texture and restrained wear, no rust streaks. Panel 3 sun-bleached warm honey teak deck planks with fine dark caulking, horizontal consistent grain. Panel 4 pale warm ivory cabin wall and dashboard paint, very fine tactile surface. Flat orthographic material swatches, consistent scale, diffuse neutral daylight, no baked perspective or directional shadows, no glare, no boat silhouette, no water, no sky, no characters, no logos, no text, no labels, no borders. Photographic realistic weathered marine materials suitable for a small Greek island ferry.
```

## assets/people-idle.png

Generated with the built-in imagegen tool and imagegen skill. References were `assets/people.png` and `assets/waiters.png`. Original saved as `.codex/generated_images/01a1db07-ffbe-76e2-a1c9-d68c5643a35f/exec-b70461e5-71df-4b6c-b007-9099bfdb7eaa.png`; copied into the project. Exact prompt:

```text
Use case: photorealistic-natural
Asset type: transparent animation atlas for distant people in a Greek island screensaver. References show the identities and clothing to match; do NOT copy their gray background. Generate six separate full-body cutouts in exactly 3 COLUMNS by 2 ROWS, wide 3:2 aspect. Six equal square cells, generous transparent margins, all feet at same baseline, all figures facing LEFT in side profile, consistent neutral daylight and camera.
TOP ROW left: same adult woman as reference 1, coral short sleeve top, navy trousers, dark shoes, standing naturally still with both feet planted near each other, arms relaxed. TOP middle: same child in yellow shirt blue shorts and trainers, standing still with feet planted and arms relaxed. TOP right: same male Greek waiter as reference 2, white shirt, navy waistcoat and black trousers, standing with both feet planted, holding a small tray of Greek food level at waist, relaxed professional posture.
BOTTOM ROW left: same woman standing conversing, one hand making a small natural gesture, both feet planted. BOTTOM middle: same child paused between games, standing turned slightly toward left, small conversational hand gesture, feet planted. BOTTOM right: same waiter standing at rest with tray, head turned slightly toward customer, feet planted. Match each character's identity and clothing across its two cells. Ordinary realistic anatomy and photographic realism, age-appropriate child proportions, no oversized heads. Every character fully within own cell, never overlap neighboring cells. No walking, no running, no legs frozen mid-stride, no ground, no environment, no shadows outside figures, no text, no labels, no grid lines. Genuine transparent alpha outside all figures and between their arms and legs, absolutely no gray/white opaque backdrop or checkerboard baked in. These are stationary/idling counterparts to existing walking and serving sprites.
```

## Hillside taverna redesign

The active renderer uses `assets/taverna-hill-day.png` and `assets/taverna-hill-night.png`. Built-in imagegen was used with the imagegen skill; originals remain in `.codex/generated_images`. The superseded foreground plates have been removed. Day reference: `assets/coast.png`, used only for perspective and lighting. Night edit target: the new day cutout, preserving placement and transparency.

### assets/taverna-hill-day.png — exact prompt

```text
Use case: photorealistic-natural
Asset type: transparent-background distant architectural cutout for an OpenGL Greek coastline screensaver.
Input image: the supplied coastline is a perspective, building-scale and lighting REFERENCE ONLY, not the edit target. Generate ONLY an isolated small hillside taverna with its narrow stone terrace, no coastline photograph or surrounding scene.
A modest traditional Cycladic taverna on a dry rocky village hillside, seen from the sea at the same distant slightly upward viewpoint as the white buildings in the reference. Wide 3:1 composition. One low whitewashed cubic building with blue door and small windows on the BACK LEFT, a low simple cane pergola over a narrow outdoor dining terrace to the RIGHT, restrained bougainvillea, six tiny blue-chair tables with white cloths, low irregular limestone retaining wall and natural rock foundation underneath. Whole building and terrace fit in image, detached isolated architectural group. Horizontal terrace floor at about 70% down image. Building roof at about 20% down image. Front retaining wall from 75% to 95%. Door about one quarter of total image height; tables and chairs correctly much smaller than door. An unobstructed horizontal service aisle runs between the building entrance on left and terrace tables on right, at 68% down image. View from across the bay, slightly below terrace level, primarily front elevation with a little visible terrace depth, NOT an elevated overhead view or a close-up restaurant interior. This artwork will be shown only about 95 pixels wide beside distant village houses, so keep restrained coherent shapes and realistic architectural proportions. Pale cream local stone with a few dry grasses at the base to blend into the reference hillside. Neutral summer Mediterranean daylight from upper right, naturally soft distant contrast. Daytime terrace mostly empty, NO people or waiters, no lighting switched on. No sea, no sky backdrop, no foreground, no huge flowers, no text, no labels, no borders. Genuine transparent alpha outside the building, pergola, furniture and low rock base, including gaps under pergola.
```

### assets/taverna-hill-night.png — exact prompt

```text
Use case: lighting-weather
Asset type: matching transparent nighttime taverna cutout for a distant hillside village in an OpenGL screensaver.
Edit target: supplied transparent daytime taverna artwork. Preserve EXACT image size, camera, complete building silhouette, white walls, blue doors/windows, pergola beams, all table and chair locations, stone retaining wall, rocks, and transparent alpha outside. Convert only lighting and dining occupancy to a realistic Greek island summer evening. Same isolated group, same front elevation, distant viewpoint. Switch on soft warm amber interior light through the small windows and blue door glazing, several discreet small lanterns on building and pergola posts, and a restrained warm string of tiny bulbs under the existing roof. Natural local warm illumination on walls, food, patrons and stone terrace; cool deep navy shadow detail on rocks and unlit walls. Full lively small taverna: about twelve realistic adult diners seated in the existing blue chairs at the tables, chatting over small plates of Greek food, wine glasses and water; proportionate adults no taller than the existing doorway when standing. All patrons are SEATED, heads just above chair backs. Keep the clear horizontal service aisle in FRONT of the tables entirely unobstructed. No standing waiters: animated waiters will be rendered separately. No people on the rocks. No new architecture or furniture, no relocated posts, no huge light glows, no candles dominating, no sea or sky backdrop, no opaque background, no text. Preserve genuine transparent alpha in gaps and outside silhouette. This must align pixel-for-pixel with the day version for smooth blending; limit changes to plausible evening illumination and seated diners.
```

## assets/clouds.png

Generated with the built-in imagegen tool and the imagegen skill, with a transparent background, then copied into this project. Exact prompt:

```text
Use case: photorealistic-natural
Asset type: transparent cloud sprite atlas for a Greek island sky in an OpenGL screensaver.
Wide landscape 3:1 atlas, three equal square cells in a single horizontal row. In each cell a separate isolated small fair-weather Mediterranean cumulus cloud, front view from ground. Cloud 1: small uneven white puffy cumulus with soft cauliflower lobes. Cloud 2: wispy broken white cloud with delicate translucent edges. Cloud 3: broad softly puffed small cumulus with a few uneven wisps. All clouds fully inside their own cells with generous empty margins. Cloud width roughly 80 percent of cell width and height about 35 percent of cell height; cloud centers aligned to middle of cells. Natural realistic photographic sunlit white tops, subtle cool pale gray undersides, detailed wispy semi-transparent boundaries, soft volume and light scattering. Genuine transparent background outside the clouds including between the cells. No blue sky, no gray backdrop, no dark outlines, no oval lens shapes, no storm clouds, no sunset tint, no text or labels. Neutral daylight illumination so software can tint the clouds for day, dusk and night. Delicate realistic quiet summer clouds, not dense cloud coverage.
```

## assets/celestial.png

Generated with the built-in imagegen tool and the imagegen skill, with a transparent background, then copied into this project. Exact prompt:

```text
Use case: scientific-educational
Asset type: photorealistic transparent celestial sprite atlas for a realistic Greek coastal screensaver.
Wide 2:1 landscape canvas, two equal square cells side by side. LEFT CELL: realistic photograph-like visible solar disc seen through clear atmosphere, brilliant warm white core with subtle pale golden limb, restrained very soft warm-white corona-like camera bloom immediately outside the disc; no spiky starburst, no cartoon orange outline, no fantasy flames. The sun is a bright round luminous body, fine natural luminous texture only, no black sunspots that would be large enough to distract. RIGHT CELL: realistic full Moon, natural neutral gray lunar maria and fine craters in recognizable lunar geography, orthographic whole circular lunar disk, softly detailed photographic telescopic appearance, fully sunlit full-moon texture, no halo beyond a tiny optical softness. Same circular body diameter in both cells: 76 percent of cell height, each centered exactly at its cell center. Empty margins around each body. Actual transparent background across the whole atlas outside the discs and subtle solar bloom. No black rectangle or backdrop, no sky, no stars, no text, no labels. No Earth. These assets will be tiny at accurate angular size; a shader applies lunar phases and atmospheric solar colors, so keep Sun near white and Moon neutral gray.
```

## assets/activity.png

Generated with the built-in imagegen tool and the imagegen skill, with a transparent background, then copied into this project. Exact prompt:

```text
Use case: product-mockup
Asset type: transparent photorealistic sprite atlas for distant ambient life in a Greek island coastal screensaver. Exact atlas layout: 3 equal columns by 2 equal rows, wide landscape 3:2 canvas, six separate isolated cutouts with genuinely transparent background. Each cutout entirely within its own cell with 15 percent empty padding. No labels, no text, no borders, no background, no ground plane.
Top row: three successive natural walking poses of the SAME casually dressed adult island resident, pale linen shirt, muted beige shorts, small dark hair, normal human anatomy, realistic photographic person, facing LEFT in side profile, camera looking slightly down from about 15 degrees. Poses: left foot forward, mid-step, right foot forward. Same person and outfit in all three cells, relaxed realistic walk, full body including shoes. No waving.
Bottom row: three different compact modern Mediterranean hatchback cars, respectively white, muted blue, light silver, all pointing LEFT, side profile with slight elevated top view matching the people, realistic wheels, correct everyday car proportions, windows and small mirrors. No brand logo, no visible license text. Natural neutral Mediterranean daylight from upper right, soft material shading, no hard ground shadow. Objects will appear very small on distant village roads, so clean photographic silhouettes and believable proportions are essential. Avoid cartoon, illustration, pixel art, toy cars, oversized heads, heavy outlines.
```

## assets/coast-night.png

Built-in imagegen edit of `assets/coast.png`, preserving the camera and shoreline, saved as a separate asset. Exact edit prompt:

```text
Use case: lighting-weather
Asset type: exact matching night environment texture for an animated Greek coastline screensaver.
Edit the supplied daytime photograph into a beautiful realistic clear Greek summer night. Preserve EXACTLY every building, the blue-domed church, roads, hills, shoreline geometry, camera, horizon, composition and image dimensions. Do not add or move structures. Deep navy starry sky with subtle sparse stars, NO moon, no sunset, no sunlight. The whitewashed village on the left has warm amber interior lights visible ONLY in its existing windows and doors, a few discreet practical lights by entrances, and warm street lamps along the existing winding road/path among the houses. Street lighting creates believable small warm pools on nearby ground and subtle warm light spill onto adjacent walls. Church modestly lit with restrained warm architectural lighting. Distant empty rocky headland remains unlit and dark. Water is dark deep blue with subdued believable reflected warm village lights near the left shoreline. Overall nocturnal exposure with visible shadow detail but genuinely dark, warm lights balanced against cool sky. Photorealistic natural lighting, no neon, no giant glows, no new buildings, no enlarged windows, no text. This is a pixel-aligned night plate to crossfade with the original: preserving geometry is essential.
```

Mode: built-in `image_gen.imagegen`, using the imagegen skill. Both selected outputs were copied into this project. No CLI API fallback was used.

## assets/coast.png

Exact prompt:

```text
Use case: photorealistic-natural
Asset type: wide environment plate for an OpenGL Greek coast screensaver, 16:9 landscape.
Scene: quiet rocky cove on a Greek Cycladic island in early summer, exquisite natural photographic realism. Camera stands at low headland overlooking exceptionally clear turquoise Aegean water. Whitewashed village with a very small blue-domed church on the distant left hillside, pale rugged limestone coast wrapping along far horizon, dry muted olive scrub, hazy distant islands toward right. Clear pale blue Mediterranean sky, late afternoon sunlight from upper right, no orange sunset. Sea horizon is precisely 40 percent down from top of image; all land and buildings restricted to the upper 50 percent, absolutely no foreground land in the lower half. Bottom half is entirely uninterrupted clear shallow turquoise sea with submerged pale sand and gentle ripples, deepening to sapphire toward horizon. No boats, no people, no text, no watermark. Crisp restrained colors, realistic landscape photography, not an illustration. This plate will supply the distant shoreline and sky above a live physically animated water surface; water below it is replaced by live waves.
```

## assets/seabed.png

Exact prompt:

```text
Use case: product-mockup
Asset type: seamless realistic terrain albedo texture, square, for an OpenGL Greek coastal screensaver.
A perfectly top-down orthographic macro photograph of pale warm ivory Greek limestone seabed, fine cream sand mixed with small smooth rounded limestone pebbles and subtle weathered stone patches. Seamless tiling material with balanced non-directional detail and consistent neutral diffuse lighting, no cast shadows or highlights, no perspective. Subtle natural calcium streaks, pale buff and ivory palette with tiny muted gray mineral flecks. Fine tactile realism, not a wall, not paving, not repeating bricks. No water baked into texture, no plants, no large objects, no text, no borders. Texture will be seen underwater through animated caustic lighting.
```


## assets/boat-materials.png

Generated with the built-in imagegen tool and imagegen skill; original retained in `.codex/generated_images`. Exact prompt:

```text
Use case: product-mockup
Asset type: physically plausible material atlas for a real 3D traditional Greek kaiki fishing boat. This is a TEXTURE ATLAS, not an image of a boat.
Wide 2:1 rectangle, four precisely equal rectangular vertical panels from left to right, each a flat seamless top-down material swatch occupying one quarter of image width and entire image height. Panel 1: aged white painted wooden hull planks, fine horizontal grain, restrained salt weathering, narrow seams, mostly warm white. Panel 2: traditional Aegean cobalt-blue painted wooden gunwale and trim, subtle horizontal grain, restrained wear, clean rich blue. Panel 3: sun-bleached honey teak boat deck boards, fine straight horizontal grain, narrow dark caulking seams. Panel 4: aged warm ivory painted cabin timber with delicate texture and a few subtle salt streaks, no boards cutouts.
All four diffuse albedo textures evenly lit in neutral daylight, no directional shadows, no glare, no object perspective, no water, no sky, no boat silhouette, no labels, no logos, no text, no border around the swatches. Realistic photographic materials at consistent scale. Calm well-maintained working Greek fishing boat, not a wreck. These panels will be mapped onto curved 3D geometry, so only material surfaces should appear.
```


## assets/people.png

Generated with the built-in imagegen tool and imagegen skill; original retained in `.codex/generated_images`. Exact prompt:

```text
Use case: photorealistic-natural
Asset type: genuine transparent animation sprite atlas for distant people on a Greek island village lane. Exact 4 columns by 2 rows, landscape 2:1 layout, eight equally sized cells. Objects fully isolated, no ground, no backdrop, no text, no borders, generous consistent transparent margins. TOP ROW: four successive natural walking poses of the SAME Greek island adult woman, ordinary modest coral-red short sleeve top, navy trousers and dark shoes, small dark hair, facing LEFT in side profile, full body, normal realistic adult anatomy, calm walking, arms and feet move naturally, same outfit and identity in all four poses. BOTTOM ROW: four successive playful running/skipping poses of the SAME child aged about 9, yellow T-shirt, muted blue shorts, trainers, facing LEFT in side profile, full body, realistic age-appropriate proportions, ordinary casual summer clothing, natural movement. Camera about 10 degrees above horizontal, neutral Mediterranean daylight from upper right, photographic realistic cutouts, no cartoon outlines or oversized heads. Each figure is centered in its own cell, with feet aligned at the same baseline within the cell; neutral diffuse lighting allows a shader to tint day/night. The figures will be tiny on a distant village lane; retain clear contrasting coral/navy and yellow/blue clothing and clean believable silhouettes. Fully transparent alpha outside each person.
```


## assets/waiters.png

Generated with the built-in imagegen tool and imagegen skill; original retained in `.codex/generated_images`. Exact prompt:

```text
Use case: photorealistic-natural
Asset type: transparent waiter animation atlas for a Greek taverna terrace. Exact 4 columns by 2 rows, wide 2:1 landscape canvas, eight equal cells. Same realistic adult Greek waiter in every cell: white short-sleeve shirt, dark navy waistcoat, black trousers, neat dark shoes, holding a small round serving tray with two plates and a glass, ordinary restaurant work clothes. TOP ROW four successive natural walking poses facing LEFT, full body, tray kept level. BOTTOM ROW four natural serving poses facing LEFT: approaching table, slight reach forward with plate, placing plate, returning upright with tray. Camera looking slightly down from about 15 degrees. Feet aligned consistently, figures centered with generous transparent margins. Photorealistic proportions and details, natural Mediterranean neutral diffuse daylight so software can tint to warm evening light. Fully transparent actual alpha outside figures. No dining-room backdrop, no ground, no text, no labels, no borders, no logos, no cartoon or oversized heads. These figures will be composited into a small terrace restaurant, so believable stance, balanced trays, realistic hands, and clean clear silhouettes matter.
```
