# -----------------------------------------------------
# Freeze Bounds
# create box with hardcoded bounds values 
n = hou.selectedNodes()[0]

pos = n.position()
box = n.parent().createNode("box")
box.setPosition((pos[0],pos[1]-1))

axis = ['x','y','z']

for a in axis:
    s = 'bbox("'+n.path()+'",D_'+a.upper()+'SIZE)'
    box.parm("size"+a).setExpression(s)
    box.parm("size"+a).deleteAllKeyframes()

    t = 'centroid("'+n.path()+'",D_'+a.upper()+')'
    box.parm("t"+a).setExpression(t)
    box.parm("t"+a).deleteAllKeyframes()
# -----------------------------------------------------
# create an object merge with ref from sected node (Ctrl+!)
for i in hou.selectedNodes():
    m = i.parent().createNode("object_merge")
    o = "IN_"+i.name()+"_"    
    n = 0         
    for j in i.parent().children():
        if j.name().startswith(o):
            try:
                n = max(n,int(float(j.name().split("_")[-1])))        
            except:
                n=0
    m.setName(o + str(n+1))
    m.setPosition((i.position()[0],i.position()[1]-1))
    m.setColor(hou.Color((0.1,2,0.1)))
    m.parm("objpath1").set(m.relativePathTo(i))
# -----------------------------------------------------
# align nodes to grid (alt+a)
for i in hou.selectedNodes():
    p = i.position()
    x = p[0]
    y = p[1] 
    x2 = round(x*.5,0)*2-0.5
    x3 = round(x*.5+1,0)*2-0.5    
    y2= round(y)-0.15
    y3= round(y+1)-0.15    
    x_ = x3
    if(abs(x2-x)<abs(x3-x)):x_=x2   
    y_ = y3
    if(abs(y2-y)<abs(y3-y)):y_=y2
    i.setPosition((x_,y_))
# -----------------------------------------------------
def find_displayed_node():
    # find current network editor pane
    # find node with display flag ON
    for pane in hou.ui.currentPaneTabs():
        if isinstance(pane,hou.NetworkEditor):
            for node in pane.pwd().children():
                display_flag = None
                try:
                    display_flag = node.isDisplayFlagSet()
                except Exception:
                    pass
                if display_flag:
                    return node
    return None
# -----------------------------------------------------
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
# ------------------------------------------------
# get lastest written bgeo.sc and go to that frame
# 
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

