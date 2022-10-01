# get current camera (python)
def getCurrentCamera():
    for i in hou.ui.currentPaneTabs():
        if i.type().name()=="SceneViewer":
            return i.curViewport().camera().path()
            break
# -----------------------------------------------------
