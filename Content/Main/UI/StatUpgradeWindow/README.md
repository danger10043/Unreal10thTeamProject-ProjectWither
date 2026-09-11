# Stat Upgrade Window Frame

First deliverable: the complete outer frame and dark background only.

## Asset

- Current file: T_StatUpgradeWindow_Frame_v2.png
- Supersedes v1, which retained a black matte outside the silver frame.
- Generated using the built-in image_gen tool.
- Native raster dimensions: 1565 x 1005 pixels, RGBA PNG with transparent exterior.
- The filled dark center is retained; its sampled alpha is 252/255.
- UMG design dimensions: 1120 x 720 logical units.
- Source aspect ratio differs from the design by approximately 0.107%.
  Drawing into 1120 x 720 introduces approximately 1.2 design pixels of width adjustment.
  This is not a pixel-exact 1120 x 720 source export.
- Existing Border padding: 30 on all sides.
- Style: dark charcoal, narrow worn silver rim, small restrained corner details.
- No text, buttons, title plates, or stat rows are baked into this image.

## Apply to the existing widget

1. Import the PNG into the Unreal Content Browser, for example under
   /Game/Jihun/StatusUpgrade/Textures.
2. Select the existing outer Border under SizeBox in WBP_StatUpgradeWindow.
3. Assign the imported texture to the Border's Background brush image.
4. Set brush Draw As to Image, Tiling to No Tile, and brush Tint to white.
   This design is intended for proportional scaling of the complete window.
5. Set brush Image Size to 1120 x 720 and keep the SizeBox overrides at 1120 x 720.
6. Keep Border Padding at 30 on all sides.
7. Set Border Brush Color to white with alpha 0.90. Keep Content Color and Opacity
   white with alpha 1.0, and keep widget Render Opacity at 1.0.
   Brush alpha affects the background and silver frame together; text remains
   independent. Exterior transparency is already encoded in the PNG. The center
   is nearly opaque in the source; brush alpha controls its in-game darkness.
8. Keep the outer ScaleBox on Scale To Fit as already configured.

Version 2 removes the external black matte with actual alpha. An intermediate
edit painted a checkerboard and was rejected. Runtime brush alpha gives an
adjustable dark overlay while preserving the exterior transparency.
Independent opacity for the metal rim and center would require separate layers;
this first deliverable uses one background brush in the existing hierarchy.

## Validation

- Inspected the final image visually.
- Checked decoded dimensions and pixel format with System.Drawing.
- Confirmed that exterior sample pixels have alpha 0, the canvas perimeter
  alpha is at most 1/255, and the center sample alpha is 252/255.
- Inspected that the dark interior remains filled and the rim is retained.
  Keep the existing 30-unit content padding instead of reducing it.
- Actual Unreal import and in-game rendering have not been performed.
- Row and button assets are deferred until their geometry is established.

## References

- Border background brush and separate content color:
  https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/Border?application_version=5.6
- Slate brush draw types:
  https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/SlateBrushDrawType?application_version=5.2

See generation-prompts-v2.json for the current exterior-removal prompts.
generation-prompts.json records the original v1 generation.
