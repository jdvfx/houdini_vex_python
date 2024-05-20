
# 1) update kitbash texture paths (imported in houdini as FBX)
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



# 2) convert FBX hierarchy to USD in LOPs

# walk down connected nodes and get the last one
def branch_down(node):
    out = node.outputs()
    if len(out)==0:
        return node
    else:      
        return branch_down(out[0])

# copy transform values between nodes
def copy_xform(source,dest):
    axis=['x','y','z']
    trans=['t','r','s']
    for ax in axis:
        for tr in trans:
            parm = f"{tr}{ax}"
            val = source.evalParm(parm)
            dest.parm(parm).set(val)
    dest.parm("scale").set(source.evalParm("scale"))

# connect LOP nodes in a chain
nodes = []
def chain(node):
    nodes.append(node)
    if len(nodes)>1:
        nodes[-1].setInput(0,nodes[-2])


        
stage = hou.node("/stage")
nulls = hou.objNodeTypeCategory().nodeTypes()["null"].instances()

'''
- find all FBX group nodes
- find child nodes
- find file SOPs
- walk down node chain (until material SOP for example)
- create sopimport LOP node, transform, group transform and USD ROP nodes
'''
for null in nulls:
    if null.name().endswith("_grp"):
        for out in null.outputs():
            for child in out.children():
                if child.type().name()=="file":
                
                    last_node = branch_down(child)
                
                    sopimport = stage.createNode("sopimport")
                    chain(sopimport)
                    
                    sopimport.parm("soppath").set(last_node.path())
                    
                    prefix = f"{null.path()}/{out.name()}/{child.name()}"
                    sopimport.parm("pathprefix").set(prefix)
                    
                    trans = stage.createNode("xform")
                    chain(trans)
                    
                    trans.parm("primpattern").set(prefix)
                    copy_xform(out,trans)
                    
        # group (_grp) level transform
        trans = stage.createNode("xform")
        chain(trans)
        trans.parm("primpattern").set(null.path())
        copy_xform(null,trans)
        trans.setDisplayFlag("on")
        
usd_rop = stage.createNode("usd_rop")
chain(usd_rop)
usd_rop.parm("savestyle").set("flattenstage")

"""
TO DO:
    - cleanup shop_materialpath prim attribs
    - create materialX network
    - create materialX nodes
    - assign materials to groups
"""
        
