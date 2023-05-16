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
cache_file_parm = "file";
file_extension = "bgeo.sc"

def window(array, window_size):
    out = []
    for i in range(0,int(len(array)/window_size)):
        x = array[i*2]
        y = array[i*2+1]
        out.append([x,y])
    return out

def get_time(dir,file):
    return os.path.getmtime(os.path.join(dir, file))

if selected_nodes:
    node = selected_nodes[0]

    if node.type().name() == cache_node_type: 
        file = node.evalParm(cache_file_parm)

        dir = os.path.dirname(file)

        files = [file for file in os.listdir(dir) if file.endswith(file_extension)]
        files.sort(
            key=lambda x: os.path.getmtime(os.path.join(dir, x)), reverse=False
        )
        
        for fwin in window(files,2):
            t0 = get_time(dir,fwin[0])
            t1 = get_time(dir,fwin[1])
        
            # time difference in seconds
            time_diff_seconds = abs(t0 - t1)
            
            # convert to timedelta object
            time_diff = timedelta(seconds=time_diff_seconds)
            
            frame = fwin[0].split(".")[1]
            print(f"frame: {frame} > time: {str(time_diff)}")
        
        # creation times for first/last files
        t0 = get_time(dir,files[0])
        t1 = get_time(dir,files[-1])
        
        # time difference in seconds
        time_diff_seconds = abs(t0 - t1)
        
        # convert to timedelta object
        time_diff = timedelta(seconds=time_diff_seconds)
        
        print(f"\n> total cache time: {str(time_diff)}")
# -----------------------------------------------------
#
# ------------------------------------------------
# get lastest written bgeo.sc and go to that frame
# ------------------------------------------------
import os

selected_nodes = hou.selectedNodes()

cache_node_type = "file";
cache_file_parm = "file";
file_extension = "bgeo.sc"

if selected_nodes:
    node = selected_nodes[0]

    if node.type().name() == cache_node_type: 
        file = node.evalParm(cache_file_parm)

        dir = os.path.dirname(file)

        files = [file for file in os.listdir(dir) if file.endswith(file_extension)]
        files.sort(
            key=lambda x: os.path.getmtime(os.path.join(dir, x)), reverse=True
        )
        newest_file = os.path.basename(files[0])
        last_frame = float(newest_file.split(".")[1])
        hou.setFrame(last_frame)

