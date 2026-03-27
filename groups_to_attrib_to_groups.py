node = hou.pwd()
geo = node.geometry()

# create prim attributes from prim group names
pg = geo.primGroups()
for pg_ in pg:
    n = "group_"+pg_.name()
    geo.addAttrib(hou.attribType.Prim,n,0)

# set 1 if in group
for pr in geo.prims():
    for g in pr.groups():
        gn = "group_"+g.name()
        pr.setAttribValue(gn,1)

# ---------------------------------------------------------------------
node = hou.pwd()
geo = node.geometry()

# create prim groups
for pa in geo.primAttribs():
    n = pa.name()
    if n.startswith("group_"):
        print(n)
        n_ = n.split("group_")[1]
        geo.createPrimGroup(n_) 

# add prim to group if attrib=1
for pr in geo.prims():
    for pa in geo.primAttribs():
        n = pa.name()
        if n.startswith("group_"):
            n_ = n.split("group_")[1]
            v=pr.attribValue(pa)
            if v==1:             
                pg = geo.primGroups()
                for pg_ in pg:
                    if pg_.name()==n_:
                        pg_.add(pr)


