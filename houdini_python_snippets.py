# get current camera (python)
def getCurrentCamera():
    for i in hou.ui.currentPaneTabs():
        if i.type().name()=="SceneViewer":
            return i.curViewport().camera().path()
            break
# -----------------------------------------------------

# walk down chain of nodes and get the last node
# useful to trigger a ROP job, from SOP
def branch_down(node):
    out = node.outputs()
    if len(out)==0:
        return node
    else:      
        return branch_down(out[0])
# -----------------------------------------------------
# --------
# sim time
# --------

import os
from datetime import datetime, timedelta

selected_nodes = hou.selectedNodes()

cache_node_type = "file";
cache_file_path = "path";
file_extension = "bgeo.sc"

if selected_nodes:
    node = selected_nodes[0]

    if node.type().name() == cache_node_type: 
        file = node.evalParm(cache_file_path)

        directory = os.path.dirname(file)

        files = [file for file in os.listdir(directory) if file.endswith(file_extension)]
        files.sort(
            key=lambda x: os.path.getmtime(os.path.join(directory, x)), reverse=True
        )

        # creation times for first/last files
        t0 = os.path.getmtime(os.path.join(directory, files[0]))
        t1 = os.path.getmtime(os.path.join(directory, files[-1]))

        # time delta to HH:MM:SS
        diff_seconds = int(t1 - t0)
        timedelta_obj = timedelta(seconds=diff_seconds)
        hours, remainder = divmod(timedelta_obj.seconds, 3600)
        minutes, seconds = divmod(remainder, 60)
        result = f"{hours:02}:{minutes:02}:{seconds:02}"

        # creation times for first/last files
        t0 = os.path.getmtime(os.path.join(directory, files[0]))
        t1 = os.path.getmtime(os.path.join(directory, files[-1]))

        # time difference in seconds
        time_diff_seconds = abs(t0 - t1)
        
        # convert to timedelta object
        time_diff = timedelta(seconds=time_diff_seconds)
        
        print(f"sim time: {str(time_diff)}")
# -----------------------------------------------------
#
# ------------------------------------------------
# get lastest written bgeo.sc and go to that frame
# ------------------------------------------------
import os

selected_nodes = hou.selectedNodes()

cache_node_type = "file";
cache_file_path = "path";
file_extension = "bgeo.sc"

if selected_nodes:
    node = selected_nodes[0]

    if node.type().name() == cache_node_type: 
        file = node.evalParm(cache_file_path)

        directory = os.path.dirname(file)

        files = [file for file in os.listdir(directory) if file.endswith(file_extension)]
        files.sort(
            key=lambda x: os.path.getmtime(os.path.join(directory, x)), reverse=True
        )
        newest_file = os.path.basename(files[0])
        last_frame = float(newest_file.split(".")[1])
        hou.setFrame(last_frame)

