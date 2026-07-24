"""
limit volume div_size based on a max voxelcount
avoid freeze/crashes with div_size close to zero
"""
import math


# me = hou.pwd()

""" user parms """
div_size = 0.02
# div_size_A = me.parm("div_size").eval()
max_voxels = 1_000_000
# max_voxels = me.parm("max_voxels").eval()

""" bounding box size """
bbox_size = [1.5,5.1,2.1]
# me.parm("bbox_size").eval()

def create_volume(size:list[float],div_size:float) -> list[float]:
    dx = math.floor(size[0]/div_size)
    dy = math.floor(size[1]/div_size)
    dz = math.floor(size[2]/div_size)
    return [dx,dy,dz]

def voxel_count(vol:list[int]) -> int:
    return vol[0]*vol[1]*vol[2]

""" check voxel count (for non-sparse volume) """
voxels = create_volume(bbox_size,div_size)
voxels_count = voxel_count(voxels)

""" safe div size so voxels<max_voxels """
ratio = float(voxels_count)/float(max_voxels)
safe_voxels_count = voxels_count / ratio

ratio = (voxels_count/safe_voxels_count) ** (1/3);
safe_div_size = div_size * ratio

# set safe voxel size
# return safe_div_size

""" check new voxel count (for non-sparse volume) """
safe_voxels = create_volume(bbox_size,safe_div_size)
safe_voxels_count = voxel_count(safe_voxels)

""" display stats """
print(f"{voxels} {voxels_count:,.0f}")
print(f"{safe_voxels} {safe_voxels_count:,.0f}")





