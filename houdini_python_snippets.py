# create point with hardcoded centroid values
n = hou.selectedNodes()[0]

pos = n.position()
add = n.parent().createNode("add")

add.setPosition((pos[0],pos[1]-1))
add.parm("usept0").set(1)

axis = ['x','y','z']

for a in axis:
    t = 'centroid("'+n.path()+'",D_'+a.upper()+')'
    add.parm("pt0"+a).setExpression(t)
    add.parm("pt0"+a).deleteAllKeyframes()
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
# align nodes and dots to grid (alt+a)
for i in hou.selectedItems():

    p = i.position()
    x = p[0]
    y = p[1]

    if isinstance(i,hou.Node):
    
        x2 = round(x*.5,0)*2-0.5
        x3 = round(x*.5+1,0)*2-0.5    
        y2= round(y)-0.15
        y3= round(y+1)-0.15    
        x_ = x3
        if(abs(x2-x)<abs(x3-x)):x_=x2   
        y_ = y3
        if(abs(y2-y)<abs(y3-y)):y_=y2
     
    if isinstance(i,hou.NetworkDot):
    
        x1 = math.floor(x/2)*2
        x2 = math.ceil(x/2)*2
        x_ = x1
        if(abs(x-x2)<abs(x-x1)):
            x_ = x2
        y_ = round(y)
        
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
# walk up the node tree (recursive)
def branch_up(node):
    inputs = node.inputs()
    if len(inputs)==0:
        return node
    else:
        #default to first input unless switch node
        selected_input = inputs[0] 
        if node.type().name()=="switch":
            selected_input = inputs[node.evalParm("input")]
    return branch_up(selected_input)
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

# file SOP
# cache_node_type = "file"
# cache_file_parm = "file"

# fileCache SOP
cache_node_type = "filecache::2.0"
cache_file_parm = "sopoutput"

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

def print_sim_time() -> None:
    if selected_nodes:

        print(" ------------------ ")
        node = selected_nodes[0]

        if node.type().name() == cache_node_type: 
            file = node.evalParm(cache_file_parm)
            dir = os.path.dirname(file)
            
            try:
                files = [file for file in os.listdir(dir) if file.endswith(file_extension)]
            except Exception:
                return

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
                time_diff = str(time_diff).split(".")[0]
                
                frame = fwin[0].split(".")[1]
                print(f"frame: {frame} > time: {str(time_diff)}")
            
            # creation times for first/last files
            t0 = get_time(dir,files[0])
            t1 = get_time(dir,files[-1])
            
            # time difference in seconds
            time_diff_seconds = abs(t0 - t1)
            
            # convert to timedelta object
            time_diff = timedelta(seconds=time_diff_seconds)
            time_diff = str(time_diff).split(".")[0]
            
            print(f"\n> total cache time: {str(time_diff)}")

print_sim_time()
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

        try:
            files = [file for file in os.listdir(dir) if file.endswith(file_extension)]
        except Exception:
            files = []
        if len(files)>0:
            files.sort(
                key=lambda x: os.path.getmtime(os.path.join(dir, x)), reverse=True
            )
            newest_file = os.path.basename(files[0])
            last_frame = float(newest_file.split(".")[1])
            hou.setFrame(last_frame)


# ---------------------------------------------------
# bake mask values (from Paint SOP) as a wrangle node
#
import re

me = hou.pwd()
geo = me.inputs()[0].geometry()

# set floating point decimals
def prec(f:float,precision:int) -> float:
    p = pow(10,precision);
    return int(f*p)/p;

# store point positions and mask values: [[P.x,P.y,P.z,mask],...]
data=[]
for pt in geo.points():
    p=pt.attribValue("P")
    m=pt.attribValue("mask")

    dec_p = me.evalParm("dec_p")
    dec_m = me.evalParm("dec_m")
    
    x = [prec(p[0],dec_p),prec(p[1],dec_p),prec(p[2],dec_p),prec(m,dec_m)]
    data.append(x)

# create wrangle snippet string
s = str(data)
s = re.sub(r"\[","{",s)
s = re.sub(r"\]","}",s)

snippet = f"vector4 a[]={s};"

createpoints="""
foreach(vector4 v;a){
    vector p_ = set(v[0],v[1],v[2]);
    int n=addpoint(0,p_);
    setpointattrib(0,"mask",n,v[3]);
}"""

snippet+=f"\n{createpoints}"

# create wrangle and set snippet string
wrangle = me.parent().createNode("attribwrangle")
pos = me.position()
wrangle.setPosition((pos[0],pos[1]-1))
wrangle.parm("class").set(0)
wrangle.parm("snippet").set(snippet)
wrangle.setDisplayFlag("on")
wrangle.setSelected("on")
# -----------------------------------------------------
# get all SOP nodes of type "subnet"
subnets = hou.sopNodeTypeCategory().nodeTypes()["subnet"].instances()
# -----------------------------------------------------
# get all SOP nodes types containing a string
# used for nodes with version "baked in" the name: studio::mynode::5.0
def get_nodetypes_and_versions(node_type_name:str):
    node_types_list = []
    node_types = hou.nodeTypeCategories()["Sop"].nodeTypes()
    for node_type in node_types:
        if(node_type_name in node_type):
            node_types_list.append(node_type)
    return node_types_list
"""
>>>get_nodetypes_and_versions("voronoifracture")
voronoifracture
voronoifracture::2.0
voronoifracturepoints
"""
# -----------------------------------------------------
# set enviroment variables
env_vars = {
    "WEDGE":0,
    "CLUSTER":0
}

for varname,value in env_vars.items():
    hou.hscript(f"set -g {varname} = {value}; varchange")
    print(f"var: {varname}\tvalue:{value}")
# -----------------------------------------------------
# set cache nodes red
# set other nodes grey if they are already red
# > helps to quickly identify cache nodes
cachenodes=["rop_geometry","filemerge::2.0","filecache::2.0"]

red = hou.Color((1,0,0))
darkred = hou.Color((0.8,0.016,0.016))
grey = hou.Color((0.5,0.5,0.5))

for n in hou.node("/obj").children():
    if n.type().name()=="geo":


        for j in n.children():
            if j.type().name() in cachenodes:
                j.setColor(red)
            else:
                if j.color()==red or j.color()==darkred:
                    j.setColor(grey)
