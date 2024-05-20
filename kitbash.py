# update kitbash texture paths (imported in houdini as FBX)
'''
exec(hou.pwd().parm("mypy").rawValue())
'''

import os

# texture directory
# in this case, we save the .hip in the texture directory
new_texture_dir = os.getenv("HIP")

principledshaders = hou.vopNodeTypeCategory().nodeTypes()["principledshader::2.0"].instances()
for shader in principledshaders: 
    for parm in shader.parms():
        if parm.name().endswith("_texture"):
            path = parm.eval()
            if path:
                texture = path.split('\\')[-1]
                newpath=f"{new_texture_dir}/{texture}"
                parm.set(newpath)

