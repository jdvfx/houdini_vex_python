import glob
import os

"""
- for all installed Houdini versions in /opt/
- create ~/houdiniXX.X/pythonX.Xlibs/pythonrc.py
"""

home = os.getenv("HOME")

pythonvers=[]
for i in glob.glob("/opt/hfs*/python/lib/python*"):
  pythonvers.append(i)

for hou_home in glob.glob(f"{home}/houdini*"):
  houver = os.path.basename(hou_home).split("houdini")[-1]

  found_pythonver = None
  for j in pythonvers:
    if houver in hou_home:
      found_pythonver = j
      break

  if found_pythonver:
    ver = os.path.basename(found_pythonver)
    newdir = f"{hou_home}/{ver}libs"

    try:
      os.makedirs(newdir)
      
    except Exception:
      pass

    if os.path.exists(newdir):
      pyrcfile = f"{newdir}/pythonrc.py"
      if not os.path.exists(pyrcfile):
        with open(pyrcfile,"w") as f:
          f.write("# python startup script\n")
          print(f"created pythonrc.py in {newdir}")

