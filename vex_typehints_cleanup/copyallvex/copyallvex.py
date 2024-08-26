# get all wrangles vex nodes, zip, write

import zlib, base64

out = ""
sop_wrangle_types = ["attribwrangle","volumewrangle"]
for sop_wrangle_type in sop_wrangle_types:
    for node in hou.sopNodeTypeCategory().nodeTypes()[sop_wrangle_type].instances():
        vexcode = node.parm("snippet").rawValue()
        out=f"{out}{vexcode}\n//--------\n"
        
with open("/tmp/out.txt","wb") as file:
    code = base64.b64encode(zlib.compress(out.encode('utf-8')))   
    file.write(code)
 
# ---------------------------------------------------

# read, unzip vex code

import zlib, base64, re
with open("/tmp/out.txt","rb") as file:
    unzipped = zlib.decompress(base64.urlsafe_b64decode(file.read()))
    lines = unzipped.splitlines()
    for line in lines:
        print(line.decode())

