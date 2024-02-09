# script executed on startup
# https://www.sidefx.com/docs/houdini/hom/locations.html
# to be saved in : ~/houdiniXX.X/pythonX.Xlibs/
# eg: ~/houdini19.0/python3.7libs/

import hou

# hide some nodes not in use.

hidelist =[
'Groom','groom','heightfield',
'muscle','fem','unreal','game','unix',
'crowd','agent','tissue','feather','labs',
'gltf','usd','guide'
'motion','kinefx'
]

keeplist = [
'pyrobakevolume'
]

debug = False

if debug:
    hidelist=['kinefx']

cats = hou.nodeTypeCategories()
for cat in cats:
    for type in cats[cat].nodeTypes():
        for node_to_hide in hidelist:
            if node_to_hide in type and type not in keeplist:
                if debug:
                    print(cat , type)
                else:
                    cats[cat].nodeTypes()[type].setHidden(1)


