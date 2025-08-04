import hou
import toolutils

# Get the current viewer
viewer = toolutils.sceneViewer()
# Get the flipbook settings from the viewer and create a stashed copy
fbsettings = viewer.flipbookSettings().stash()


#fbsettings.setResolution((1280, 720))
#fbsettings.setFrameRange((1, 100))


hip = hou.expandString("$HIP")
ver = 1
output_path = f"{hip}/flipbooks/flipbook_v{ver}.$F4.png"

fbsettings.output(output_path)

# Generate the flipbook
viewer.flipbook(viewport=None, settings=fbsettings, open_dialog=False)

print(f"Flipbook generated to: {output_path}")
import hou
import toolutils

# Get the current viewer
viewer = toolutils.sceneViewer()
# Get the flipbook settings from the viewer and create a stashed copy
fbsettings = viewer.flipbookSettings().stash()


#fbsettings.setResolution((1280, 720))
#fbsettings.setFrameRange((1, 100))


hip = hou.expandString("$HIP")
ver = 1
output_path = f"{hip}/flipbooks/flipbook_v{ver}.$F4.png"

fbsettings.output(output_path)

# Generate the flipbook
viewer.flipbook(viewport=None, settings=fbsettings, open_dialog=False)

print(f"Flipbook generated to: {output_path}")

