""" 

> description:
when using the "File Cache" node,
creates a timestamped backup of the current scene in the Geo directory

> note:
this is the most basic way of tracking down what scene created some geometry
In production, a database would be used instead.

eg:

    base Name   : building
    baseFolde   : $HIP/geo
    version     : 1
    sceneFile   : myscene_002

backup file:

    $HIP/geo/building/v1/building_v1_bgeo_sc#myscene_002#250516151332.hipnc


> usage:
add to pre-render python script
!!! WARNING !!!
paste as plain text, the pre-render script background color should be BLACK! (not pink/cyan)
otherwize it will run as callback (runs on frame change or button press)


"""


import shutil
import datetime
import os
import re

node=hou.pwd().parent()

# increment version
ver=node.parm("version")
ver.set(ver.eval()+1)

# save current scene
hou.hipFile.save()

hipfile = hou.hipFile.name()
hipext = os.path.splitext(hipfile)[-1]

# filecache geo path
sopoutput = node.evalParm("sopoutput")

# hip name
hipname = re.sub(hipext,"",os.path.basename(hipfile))

# geo name (from path)
geoname = re.sub("\.","_",os.path.basename(sopoutput))

# geo dir (from path)
geodir = os.path.dirname(sopoutput)
# create dir if missing
os.makedirs(geodir, exist_ok=True)

# date
datestr = datetime.datetime.now().strftime("%y%m%d%H%M%S")

backuppath = f"{geodir}/{geoname}#{hipname}#{datestr}{hipext}"
shutil.copy(hipfile, backuppath)


