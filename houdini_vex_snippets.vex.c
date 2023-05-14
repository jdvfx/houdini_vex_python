/* --------------------
   Houdini VEX snippets
   -------------------- */

//* -------------------------------------------------------------- */
// keep point in proximity (or cull by proximity)
int pt_found = pcnumfound(pcopen(1,"P",@P,chf("radius"),1));
if(pt_found == chi("reverse_selection")){
        removepoint(0,@ptnum,1);
}
/* -------------------------------------------------------------- */
// rotate vel around axis
float angle = radians(chf("angle_in_degrees"));
vector axis = chv("axis");
v@v = qrotate(quaternion(angle, axis),v@v);
/* -------------------------------------------------------------- */
// create coordinate system X,Y,Z
// run over Detail
void line(vector pos1,pos2){
    int p1 = addpoint(0,pos1);
    int p2 = addpoint(0,pos2);
    int prim = addprim(0,"polyline");
    int v1 = addvertex(0,prim,p1);
    int v2 = addvertex(0,prim,p2);
    setattrib(0,"prim","Cd",prim,0,pos2,"set");

}
vector p0 = vector(0);
vector p1 = {1,0,0};
vector p2 = {0,1,0};
vector p3 = {0,0,1};
line(p0,p1);
line(p0,p2);
line(p0,p3);
/* -------------------------------------------------------------- */
// orient to N and up 
v@up = qrotate(p@orient, {0,1,0});
v@N = qrotate(p@orient, {0,0,1});
/* -------------------------------------------------------------- */
// rainbow XYZ
v@Cd = relbbox(0,@P);
/* -------------------------------------------------------------- */
// ramp by proximity to point (pointcloud)
int handle = pcopen(1,"P",@P,chf("radius"),chi("maxpoints"));
float d = 0;
if(pcnumfound(handle)>0){
    vector p = pcfilter(handle,"P");
    d = fit(distance(@P,p),0,chf("radius"),1,0);
}
@Cd=d+{0,0,1};
/* -------------------------------------------------------------- */
// ramp by proximity to surface
int prim;
vector uv;
float f = xyzdist(1,@P,prim,uv,chf("dist_max"));
@Cd = fit(f,chf("dist_min"),chf("dist_max"),1,0)+{0,0,1};
/* -------------------------------------------------------------- */
// cull points outside/inside bounds 
vector r = relbbox(1,@P);
if(r.x>1||r.y>1||r.z>1||r.x<0||r.y<0||r.z<0){
    removepoint(0,@ptnum);
}
/* -------------------------------------------------------------- */
// fade volume edges (bounds)
vector r = relbbox(0,@P);
float o = chf("padding");
float s = chf("fade_start");
float d = fit(r.x,s,o,0,1)*fit(r.x,1-s,1-o,0,1)*fit(r.y,s,o,0,1)*fit(r.y,1-s,1-o,0,1)*fit(r.z,s,o,0,1)*fit(r.z,1-s,1-o,0,1);
f@density *= d;





// --- TO DO ... test VEX below


// Sigmoid  function
// x  : input value
// k  : steepness of the sigmoid curve
// x0 : midpoint of the curve

float sigmoid(float x, float k, float x0){
    return 1.0 / (1.0 + exp(-k * (x - x0)));
}


/* -------------------------------------------------------------- */
// pop rotate orient using v
vector up = {0,1,0};
vector axis = cross(up,normalize(v@v));
vector4 rot = quaternion(radians(ch('rotation_speed')*length(v@v)),axis);
p@orient = qmultiply(p@orient, rot);

/* -------------------------------------------------------------- */
// random orient - TODO: need 360 distribution
vector axis = normalize(rand(v@P*chf("seed"))*vector(2)-1);
p@orient = quaternion(axis);

/* -------------------------------------------------------------- */
// 4D CurlX noise with FBM octaves
vector4 P4;
P4 = @P;
setcomp(P4, chf("noise_time"), 3);
float c =0;

float amplitude = 1;
float freq = chf("freq");
float maxval = 0;

for(int i=0;i<chi("octaves");i++){
    // 1.655 is a multiplier to get -2,2 range
    float f = float(curlxnoise(P4*freq+chv("offset")*(i+1))*1.655);
    c += pow(fit(f,-2,2,0,1),chf("contrast")) * amplitude ;
    maxval += amplitude;
    amplitude *= chf("roughness");
    freq*= 2;
}

@Cd=(c/maxval)*chf("mult");

/* -------------------------------------------------------------- */
// filter by size (max axis) when using packed primitives
vector b = primintrinsic(0,"bounds",@primnum);
float sx = b[1]-b[0];
float sy = b[3]-b[2];
float sz = b[5]-b[4];
if(max(max(sx,sy),sz)<chf("max_axis"))removeprim(0,@primnum,1);

/* -------------------------------------------------------------- */
// Average Neighbouring Normals
int n[] = neighbours(0, @ptnum);
vector avgN = v@N;
foreach (int pt; n){
    avgN += point(0, "N", pt);
}
avgN /= len(n)+1;
v@N = avgN;




/* -------------------------------------------------------------- */
// onoise 3D 
vector n3 =onoise (@P*chf("n3_freq")+chv("n3_offset"),chi("n3_turb"),ch("n3_rough"),ch("n3_atten"))*vector(chf("n3_amp"));

/* -------------------------------------------------------------- */
// onpoise 1D
float n1 =onoise (@P*chf("n1_freq")+chv("n1_offset"),chi("n1_turb"),ch("n1_rough"),ch("n1_atten"))*chf("n1_amp");

/* -------------------------------------------------------------- */
// transform geo with extractTransform as input 2
vector pivot=point(1,"pivot",0);
vector p =point(1,"P",0);
p@orient = point(1,"orient",0);

matrix3 m = qconvert(p@orient);

@P-=pivot;
@P*=m;
@P+=pivot;
@P+=p-pivot;



/* -------------------------------------------------------------- */
// random color from name attribute
// run on primitives or points
//
string attribType = "prim";
//string attribType = "point";

int n_=nuniqueval(0,attribType,"name");
int x=-1;
for(int i=0;i<n_;i++){
    string a = uniqueval(0,attribType,"name",i);
    if(a==s@name){
        x=i;
        break;
    }
}
i@nameindex=x;
@Cd=rand(x);

/* -------------------------------------------------------------- */
// minimun colour of neighbour points 
int n[] = neighbours(0, @ptnum);
vector Cd = @Cd;
foreach (int pt; n){
    Cd = min(Cd,point(0, "Cd", pt));
}
@Cd = Cd;

/* -------------------------------------------------------------- */
// vex "carve" SOP (run over primitives)
// keep all the points and slide them along curve
float u = ch("u");
vector uv = set(0, 0, 0);
int prim = @primnum;
int pts[] = primpoints(0, prim);

foreach(int pt; pts)
{
    uv.x = fit(pt, pts[0], pts[-1], 0, u);
    vector pos = primuv(0, "P", prim, uv);
    setpointattrib(0, "P", pt, pos);
}

/* -------------------------------------------------------------- */
// v@N and v@up to p@orient
vector Xaxis = normalize(v@N);
vector Yaxis = normalize(v@up);
vector Zaxis = cross(Xaxis,Yaxis);
vector center = v@P;
matrix myTransform = set(Xaxis,Yaxis,Zaxis,center);
p@orient = quaternion(matrix3(myTransform));

/* -------------------------------------------------------------- */
// vel along curve
if(@ptnum>0){
    v@v = @P-point(0,"P",@ptnum-1);
}else{
    v@v = point(0,"P",@ptnum+1)-@P;
}

/* -------------------------------------------------------------- */
// keep first or last points on the curves (run over primitives)
int p[] = primpoints(0,@primnum);
foreach (int num; p) {
    if(chi("keep_last_or_first_point_on_curve")==1){
        if(num!=p[0])removepoint(0,num,1);
    }else{
        if(num!=p[len(p)-1])removepoint(0,num,1);
    }
}

/* -------------------------------------------------------------- */
// keep first and last points on the curves (run over primitives)
int p[] = primpoints(0,@primnum);
foreach (int num; p) {   
    if(num!=p[0] && num!=p[len(p)-1])removepoint(0,num,1);  
}

/* -------------------------------------------------------------- */
// ramp from distance to points
int handle = pcopen(1,"P",@P,chf("radius"),1);
float d = 0;
if(pcnumfound(handle)>0){
    vector p = pcfilter(handle,"P");
    d = fit(distance(@P,p),0,chf("radius"),1,0);
}
@Cd=pow(d,chf("exp"))+{0,0,1};

/* -------------------------------------------------------------- */
// remove prims with less than 3 points
int pts[] = primpoints( 0, @primnum );
if( len(pts) < 3 ) removeprim( 0, @primnum, 1 );

/* -------------------------------------------------------------- */
// ramp from distance to surface 
int prim;
vector uv;
float dmin = chf("dist_min");
float dmax = chf("dist_max");
float d = xyzdist(1,@P,prim,uv,dmax);
float f = fit(d,dmin,dmax,0,1);
if(chi("reverse_direction")==1)f=fit01(f,1,0);
@Cd = f + {0,0,1};

/* -------------------------------------------------------------- */
// delete by proximity to surface (run over points)
int prim;
vector uv;
float d = xyzdist(1,@P,prim,uv,chf("maxdist"));
if(chi("invert_selection")==1){
     if(d<chf("maxdist")){
         removepoint(0,@ptnum);
     }
}else{
    if(d>=chf("maxdist")){
        removepoint(0,@ptnum);
    }
}

/* -------------------------------------------------------------- */
// create u attribute along one curve (run over points)
f@u = float(i@ptnum)/float(npoints(0));
if(chi("display_u")==1){
	@Cd=f@u+{0,0,1}; 
}

/* -------------------------------------------------------------- */
// create u attribute along curves (run over primitives)
int p[] = primpoints(0,@primnum);
float u;
int l = len(p);
for(int i=0;i<l;i++){
    u=float(i)/float(l-1);
    if(chi("reverse_direction")==1)u=(1-u);    
    setpointattrib(0,"u",p[i],u,"set");
    if(chi("display_u")==1){
        vector c=u+{0,0,1};
        setpointattrib(0,"Cd",p[i],c,"set");    
    }
}

/* -------------------------------------------------------------- */
// velocity along one curve (run over points)
if(@ptnum>0){
    v@v = @P-point(0,"P",@ptnum-1);
}else{
    v@v = point(0,"P",@ptnum+1)-@P;
}
if(chi("reverse_direction")==1)v@v*=-1;

/* -------------------------------------------------------------- */
// vel along multiple curves (run over primitives)
int p[] = primpoints(0,@primnum);
for(int i=0;i<len(p);i++)  {
    vector v=vector(0);
    if(i>0){
        v = point(0,"P",p[i])- point(0,"P",p[i-1]);
    }else{
        v = point(0,"P",p[i+1])- point(0,"P",p[i]);
    }
    setpointattrib(0,"v",p[i],v,"set");
}

/* -------------------------------------------------------------- */
// clouds noise
#include < voplib.h>
// density mask (input 2)
float d = 1;
if(chi("dMask_useMask")==1){
    vector mask_pos_noise = anoise(@P*chf("dMask_pn_freq")+chv("dMask_pn_offset"), chi("dMask_pn_turb"), chf("dMask_pn_rough"), chf("dMask_pn_atten"))*chf("dMask_pn_mult");
    d=pow(fit(volumesample(1,0,@P+mask_pos_noise),0,chf("dMask_max_density"),1,0),chf("dMask_density_exp"));
}
// noise mask (aligator)
float mask = 1;
if(chi("nMask_useMask")==1){
    mask =  clamp(pow(anoise(@P*chf("nMask_freq")+chv("nMask_offset"), chi("nMask_turb"), chf("nMask_rough"), chf("nMask_atten")),chf("nMask_exp"))*chf("nMask_mult"),0,1);
} 
// noise 1 (fbm)
vector n1p = vop_fbmNoiseVV(@P*chf("n1_pn_Freq")+chv("n1_pn_Offset"),chf("n1_pn_rough"),chi("n1_pn_turb"),"noise");
vector n1 = vop_fbmNoiseVV(n1p*chf("n1_Freq")+chv("n1_Offset"),chf("n1_rough"),chi("n1_turb"),"noise");
// noise 2 (fbm)
vector n2p = vop_fbmNoiseVV(@P*chf("n2_pn_Freq")+chv("n2_pn_Offset"),chf("n2_pn_rough"),chi("n2_pn_turb"),"noise");
vector n2 = vop_fbmNoiseVV(n2p*chf("n2_Freq")+chv("n2_Offset"),chf("n2_rough"),chi("n2_turb"),"noise");
// sample displaced density
@density = volumesample(0,0,@P+(n1*chf("n1_mult")+n2*chf("n2_mult"))*mask*d*chf("advection_length"));

/* -------------------------------------------------------------- */
// rotate quaternion around axis
matrix3 m;
matrix3 rm;
m = qconvert(@orient);
vector axis = chv("axis");
rotate(m, radians(chf("angle_in_degrees"), axis);    
@orient = quaternion(m);

/* -------------------------------------------------------------- */
// intersection of 2 bounds
vector m1,M1,m2,M2;
getbbox(0,m1,M1);
getbbox(1,m2,M2);
if(m2.x>M1.x || M2.x< m1.x || m2.y>M1.y || M2.y< m1.y || m2.z>M1.z || M2.z< m1.z){
    removepoint(0,@ptnum);
}else{
    @P.x=min(M1.x,max(@P.x,m2.x),M2.x);
    @P.y=min(M1.y,max(@P.y,m2.y),M2.y);
    @P.z=min(M1.z,max(@P.z,m2.z),M2.z);
}

/* -------------------------------------------------------------- */
// divscale disturbance
if(@density< chf("max_density")){

    float speed = length(@vel);
    if(speed>chf("min_speed")){

        vector4 hvec;
        vector _div = (v@P + rand(f@Frame-5) * chf("divscale"))/chf("divscale");
        hvec = _div;
        setcomp(hvec, f@Frame+9, 3);
        float r1 = float(random(hvec));
       
        if(r1>chf("threshold")){
       
            vector4 hvec2;
            vector _div2 = (v@P + rand(f@Frame+55) * chf("divscale"))/chf("divscale");
            hvec2 = _div2;
            setcomp(hvec2, f@Frame, 3);
            vector r3 = random(hvec2);
            vector d = vector(chf("disturbance"));
            vector r3_ = fit(r3,vector(0),vector(1),-d,d);
       
            v@vel += r3_ * fit(speed,0,1,0,1);
        }
    }   
}

/* -------------------------------------------------------------- */
// volume motion blur using vel from second input (not VDB)
float vx=volumesample(1,0,@P);
float vy=volumesample(1,1,@P);
float vz=volumesample(1,2,@P);
vector v = set(vx,vy,vz);
if(length(v)>0){
    float dsum=0;
    for(int i=0;i< chi("steps");i++){
        float f = float(i)/float(chi("steps")-1);
        dsum += volumesample(0,0,@P+v*chf("displacement")*f);
    }
    f@density = dsum /float(chi("steps"));
}

/* -------------------------------------------------------------- */
// 3d fbm noise - for vel
#include 
// noise 1
vector n1p = vop_fbmNoiseVV(@P*chf("noise1Pos_Freq")+chv("noise1Pos_Offset"),chf("noise1Pos_Roughness"),chi("noise1Pos_Turbulence"),"noise");
vector n1 = vop_fbmNoiseVV(n1p*chf("noise1_Freq")+chv("noise1_Offset"),chf("noise1_Roughness"),chi("noise1_Turbulence"),"noise");
// noise 2
v@vel = n1;

//////////////////////////////////////////////////////////////////// 
// remap density
f@density = efit(pow(efit(f@density,0,chf("max_density"),0,1),chf("exp")),0,1,0,chf("out_density"));

//////////////////////////////////////////////////////////////////// 
// mask
f@density =  clamp(pow(anoise(@P*chf("mask_freq")+chv("mask_offset"), chi("mask_turbulence"), chf("mask_roughness"), chf("mask_attenuation")),chf("mask_exp"))*chf("mask_mult"),0,1);



/* -------------------------------------------------------------- */
//EXTRACTING TRANSFORMS
//Depending on the value of c,
//returns the translate (c=0), rotate (c=1), or scale (c=2)
//component of the transform (xform)
//
matrix xform = primintrinsic(1, "packedfulltransform", @ptnum);
//matrix xform = primintrinsic(1, "transform", @ptnum);

#define XFORM_SRT       0  // Scale Rotate Translate
#define XFORM_XYZ       0 // Rx Ry Rz

int trs = XFORM_XYZ; //Transform Order
int xyz = XFORM_SRT; //Rotation Order
vector p = @P; //pivot for crack/extracting transforms
vector translate = cracktransform(trs, xyz, 0 , p, xform);
vector rotate    = cracktransform(trs, xyz, 1 , p, xform);
vector scale    = cracktransform(trs, xyz, 2 , p, xform);

/* -------------------------------------------------------------- */
//MAKE TRANSFORM
matrix newTrans = maketransform(trs, xyz, translate, rotate,{1,1,1});
@orient = quaternion(matrix3(newTrans));

//rotation in degrees
matrix matx = qconvert(@orient);
vector extracted = cracktransform(0, 0, 1, @P, matx);
v@rotation = extracted;

s@filename = primintrinsic(1, "filename", @ptnum);

v@scale = scale;

/* -------------------------------------------------------------- */
// basic point clustering
int handle = pcopen(0,"P",@P,chf("radius"),chi("maxpoints"));
if(pcnumfound(handle)>0){
    while(pciterate(handle)){
        vector c;
        vector p;
        pcimport(handle,"P",p);         
        pcimport(handle,"Cd",c);        
        float dist = distance(@P,p);
        int steps  = int(dist / chf("stepsize"));
        for(int i=0;i< steps;i++){
          float d = float(i)/float(steps);
            vector r = (rand(@P*23424+i*55)*vector(2)-1);
            vector p_ = lerp(@P,p,d)+r*chf("jitter");
            vector c_ = lerp(@Cd,c,d);            
            int n = addpoint(0,p_);
            setpointattrib(0,"Cd",n,c_);
        }
    }
}

/* -------------------------------------------------------------- */
// volume camera cull
vector pndc = toNDC(chs("camera_name"), @P);
// padding
float pad = chf("padding");
if(pndc.x< 0-pad || pndc.x>1+pad || pndc.y< 0-pad || pndc.y>1+pad || pndc.z>=0 ){
    f@density=0;    
}

/* -------------------------------------------------------------- */
// bunker bullet auto freeze
if(length(@v)< chf("min_vel")){
    f@s+=1;
    if(f@s>ch("min_vel_frames")){
        i@bullet_sleeping=1;
    }
}else{
    f@s=0;
}

if(distance(@P,v@oldP)< chf("min_dist")){
    f@d=f@d+1;
    if(f@d>ch("min_dist_frames")){
        v@v *= clamp(1-chf("drag"),0,1);
        v@w *= clamp(1-chf("torque_drag"),0,1);
    }
}else{
    f@d=0;
}
// stash P
v@oldP = @P;

/* -------------------------------------------------------------- */
// connect adjacent pieces
void line(vector pos1,pos2){
    int p1 = addpoint(0,pos1);
    int p2 = addpoint(0,pos2);
    int prim = addprim(0,"polyline");
    int v1 = addvertex(0,prim,p1);
    int v2 = addvertex(0,prim,p2);
}

int handle = pcopen(0,"P",@P,chf("radius"),chi("maxpoints"));
if(pcnumfound(handle)>0){
    while(pciterate(handle)){
        int id;
        pcimport(handle,"pieceid",id);
        if(id!=i@pieceid){            
            vector p;
            pcimport(handle,"P",p);        
            line(@P,p);                    
        }     
    }
}

/* -------------------------------------------------------------- */
// remove lone points (unconnected)
if(neighbourcount(0,@ptnum)==0){
    removepoint(0,@ptnum);
}else{
    // collapse lines into points (centroids)
    @P = (@P+point(0,"P",neighbour(0,@ptnum,0)))*.5;
}

/* -------------------------------------------------------------- */
// vorticity
// input 2 has vel.* volumes
vector dx = volumegradient(1,"vel.x",@P);
vector dy = volumegradient(1,"vel.y",@P);
vector dz = volumegradient(1,"vel.z",@P);
vector w = set(dz.y-dy.z,dx.z-dz.x,dy.x-dx.y);
@Cd = fit(length(w),0,chf("max_vorticity"),0,1)+{0,0,1};




/* -------------------------------------------------------------- */
// SOP ambient occlusion
// code from: Labs calculate occlusion
#include <voplib.h>

vector hit, raydir;
float u, v, conewidth; 
float occluded; 
int hitprim, hitprim_2;
float tempOcc = 0;
 
float maxdist = ch('maxdist'); // 2 
conewidth = radians(ch('conewidth')); //90
int rays = chi('rays'); // 50 
float bias = chf('bias'); // 0.5

for (int i = 0; i<rays; i++ ) {
    raydir = sample_direction_cone(v@N,conewidth,vector2(rand(i*234524)));
    hitprim = intersect(0,@P+(v@N*0.001), raydir*maxdist, hit, u, v);
    hitprim_2 = intersect(1,@P+(v@N*0.001), raydir*maxdist, hit, u, v);
    if (hitprim!=-1 || hitprim_2!=-1) tempOcc+=1;
}

float occ = clamp(vop_bias(1.0-(tempOcc / rays), bias), 0, 1);
@Cd=occ;

/* -------------------------------------------------------------- */

