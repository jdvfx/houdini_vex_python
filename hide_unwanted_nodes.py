# python startup script
# script executed on startup
# https://www.sidefx.com/docs/houdini/hom/locations.html
# to be saved in : ~/houdiniXX.X/pythonX.Xlibs/
# eg: ~/houdini19.0/python3.7libs/uiready.py

import hou

# hide some nodes I don't use.
hidelist =[
'wrinkle','onnx','gltf','ml','fem','unreal','unix',
'muscle','agent','crowd','kinefx','motion','groom','fem','feather',
'tissue','ris','apex','guide'
]

keeplist = [
]

hidden_nodes=[]

cats = hou.nodeTypeCategories()
for cat in cats:
    for type in cats[cat].nodeTypes():
        for node_to_hide in hidelist:
            if node_to_hide in type and type not in keeplist:
                hidden_nodes.append(f"{cat} {type}")
                cats[cat].nodeTypes()[type].setHidden(1)

with open("/tmp/houdini_hidden_nodes.txt","w") as file:
    for node in hidden_nodes:
        file.write(f"{node}\n")

