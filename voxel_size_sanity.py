"""
limit volume div_size based on a max voxelcount
avoid freeze/crashes with div_size close to zero
"""
import math

# user parms
div_size_A = 0.02
max_voxels = 100_000_0

# bounding box size (in SOP)
bbox_size = [1.5,5.1,2.1]

def create_volume(size:list[float],div_size:float) -> list[float]:
    dx = math.ceil(size[0]/div_size)
    dy = math.ceil(size[1]/div_size)
    dz = math.ceil(size[2]/div_size)
    return [dx,dy,dz]

def voxel_count(vol:list[int]) -> int:
    return vol[0]*vol[1]*vol[2]

# check voxel count (for non-sparse volume)
voxels_A = create_volume(bbox_size,div_size_A)
voxels_A_count = voxel_count(voxels_A)

# safe div size so voxels<max_voxels
ratio = min(max_voxels/voxels_A_count,1)
div_size_B = div_size_A / ratio

# check new voxel count (for non-sparse volume)
voxels_B = create_volume(bbox_size,div_size_B)
voxels_B_count = voxel_count(voxels_B)

# display stats
print("{:<20} {:>5}".format(str(voxels_A), voxels_A_count))
print("{:<20} {:>5}".format(str(voxels_B), voxels_B_count))

