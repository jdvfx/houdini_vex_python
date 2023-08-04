//* -------------------------------------------------------------- */
// keep point in #proximity (or #cull by proximity)
int pt_found = pcnumfound(pcopen(1,"P",v@P,chf("radius"),1));
if(pt_found == chi("reverse_selection")){
        removepoint(0,i@ptnum,1);
}
/* -------------------------------------------------------------- */
// #rotate vel around axis
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
// #orient to N and up 
v@up = qrotate(p@orient, {0,1,0});
v@N = qrotate(p@orient, {0,0,1});
/* -------------------------------------------------------------- */
// rainbow XYZ
v@Cd = relbbox(0,v@P);
/* -------------------------------------------------------------- */
// ramp by proximity to point (pointcloud)
int handle = pcopen(1,"P",v@P,chf("radius"),chi("maxpoints"));
float d = 0;
if(pcnumfound(handle)>0){
    vector p = pcfilter(handle,"P");
    d = fit(distance(v@P,p),0,chf("radius"),1,0);
}
v@Cd=d+{0,0,1};
/* -------------------------------------------------------------- */
// #ramp by #proximity to #surface
int prim;
vector uv;
float f = xyzdist(1,v@P,prim,uv,chf("dist_max"));
v@Cd = fit(f,chf("dist_min"),chf("dist_max"),1,0)+{0,0,1};
/* -------------------------------------------------------------- */
// #cull points outside/inside #bounds 
vector r = relbbox(1,v@P);
if(r.x>1||r.y>1||r.z>1||r.x<0||r.y<0||r.z<0){
    removepoint(0,i@ptnum);
}
/* -------------------------------------------------------------- */
// #fade #volume edges (bounds)
vector r = relbbox(0,v@P);
float o = chf("padding");
float s = chf("fade_start");
float d = fit(r.x,s,o,0,1)*fit(r.x,1-s,1-o,0,1)*fit(r.y,s,o,0,1)*fit(r.y,1-s,1-o,0,1)*fit(r.z,s,o,0,1)*fit(r.z,1-s,1-o,0,1);
f@density *= d;
/* -------------------------------------------------------------- */
// #point #decimate
int ptvar = i@ptnum;
if (chi("use_id")) ptvar = i@id;
if(rand(ptvar+chf("seed"))>chf("threshold"))removepoint(0,i@ptnum);
/* -------------------------------------------------------------- */
// #sigmoid  function
// x  : input value
// k  : steepness of the sigmoid curve
// x0 : midpoint of the curve
// good values for 0-1 range, k=10, x0=0.5
// WARNING: values at <=0 and >=1 are not exactly 0 or 1
float sigmoid(float x, k, x0){
    return 1.0 / (1.0 + exp(-k * (x - x0)));
    // TODO: lerp each to 0 and 1
}
/* -------------------------------------------------------------- */
// #rotate #orient using v (#pop wrangle)
if(length(v@v)>chf("min_speed_for_rotation")){
    vector up = {0,1,0};
    vector axis = cross(up,normalize(v@v));
    float r = fit01(pow(rand(i@id-chf("seed")),2),chf("rotation_speed_min"),chf("rotation_speed_max"));
    // random rotation direction (sign)
    //if(rand(i@id+chf("seed"))>0.5)r*=-1;
    vector4 rot = quaternion(radians(r*length(v@v)),axis);
    p@orient = qmultiply(p@orient, rot);
}
/* -------------------------------------------------------------- */
// #random #orient 
vector Xaxis = normalize(rand(i@ptnum*89.3)*vector(2)-1);
vector Yaxis = normalize(rand(i@ptnum*73.5)*vector(2)-1);
vector Zaxis = cross(Xaxis,Yaxis);
matrix myTransform = set(Xaxis,Yaxis,Zaxis,v@P);
p@orient = quaternion(matrix3(myTransform));
/* -------------------------------------------------------------- */
// randomize vel direction on impact (pop wrangle)
// add drag, stop when slow
if(f@hittime>(2/24.0) && abs(f@hittime-f@Time)<chf("threshold")){
    float speed = length(v@v) * clamp(1-chf("drag"),0,1);
    vector r = rand(v@P*chf("seed"))*vector(2)-1;
    v@v += r*chf("rand_direction")*speed;
    v@v = normalize(v@v)*speed;
    if(speed<chf("min_speed")){
        i@stopped = 1;
    }
}
/* -------------------------------------------------------------- */
// ####  TO DO ... test VEX below                            #####
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
// 4D CurlX #noise with FBM octaves
vector4 P4;
P4 = v@P;
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

v@Cd=(c/maxval)*chf("mult");

/* -------------------------------------------------------------- */
// #filter by #size (max axis) when using packed primitives
vector b = primintrinsic(0,"bounds",i@primnum);
float sx = b[1]-b[0];
float sy = b[3]-b[2];
float sz = b[5]-b[4];
if(max(max(sx,sy),sz)<chf("max_axis"))removeprim(0,i@primnum,1);

/* -------------------------------------------------------------- */
// Average Neighbouring Normals
int n[] = neighbours(0, i@ptnum);
vector avgN = v@N;
foreach (int pt; n){
    avgN += point(0, "N", pt);
}
avgN /= len(n)+1;
v@N = avgN;

/* -------------------------------------------------------------- */
// #onoise 3D 
vector n3 =onoise (v@P*chf("n3_freq")+chv("n3_offset"),chi("n3_turb"),ch("n3_rough"),ch("n3_atten"))*vector(chf("n3_amp"));

/* -------------------------------------------------------------- */
// #onoise 1D
float n1 =onoise (v@P*chf("n1_freq")+chv("n1_offset"),chi("n1_turb"),ch("n1_rough"),ch("n1_atten"))*chf("n1_amp");

/* -------------------------------------------------------------- */
// #transform geo with extractTransform as input 2
vector pivot=point(1,"pivot",0);
vector p =point(1,"P",0);
p@orient = point(1,"orient",0);

matrix3 m = qconvert(p@orient);

v@P-=pivot;
v@P*=m;
v@P+=pivot;
v@P+=p-pivot;

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
v@Cd=rand(x);

/* -------------------------------------------------------------- */
// #minimun colour of neighbour points 
int n[] = neighbours(0, i@ptnum);
vector Cd = v@Cd;
foreach (int pt; n){
    Cd = min(Cd,point(0, "Cd", pt));
}
v@Cd = Cd;

/* -------------------------------------------------------------- */
// vex "carve" SOP (run over primitives)
// keep all the points and slide them along curve
float u = ch("u");
vector uv = set(0, 0, 0);
int prim = i@primnum;
int pts[] = primpoints(0, prim);

foreach(int pt; pts)
{
    uv.x = fit(pt, pts[0], pts[-1], 0, u);
    vector pos = primuv(0, "P", prim, uv);
    setpointattrib(0, "P", pt, pos);
}

/* -------------------------------------------------------------- */
// N + up to #orient
vector Xaxis = normalize(v@N);
vector Yaxis = normalize(v@up);
vector Zaxis = cross(Xaxis,Yaxis);
vector center = v@P;
matrix myTransform = set(Xaxis,Yaxis,Zaxis,center);
p@orient = quaternion(matrix3(myTransform));

/* -------------------------------------------------------------- */
// vel #along #curve
if(i@ptnum>0){
    v@v = @P-point(0,"P",i@ptnum-1);
}else{
    v@v = point(0,"P",i@ptnum+1)-v@P;
}

/* -------------------------------------------------------------- */
// keep first or last points on the curves (run over primitives)
int p[] = primpoints(0,i@primnum);
foreach (int num; p) {
    if(chi("keep_last_or_first_point_on_curve")==1){
        if(num!=p[0])removepoint(0,num,1);
    }else{
        if(num!=p[len(p)-1])removepoint(0,num,1);
    }
}

/* -------------------------------------------------------------- */
// keep first and last points on the curves (run over primitives)
int p[] = primpoints(0,i@primnum);
foreach (int num; p) {   
    if(num!=p[0] && num!=p[len(p)-1])removepoint(0,num,1);  
}

/* -------------------------------------------------------------- */
// #ramp from #distance to #points
int handle = pcopen(1,"P",v@P,chf("radius"),1);
float d = 0;
if(pcnumfound(handle)>0){
    vector p = pcfilter(handle,"P");
    d = fit(distance(v@P,p),0,chf("radius"),1,0);
}
v@Cd=pow(d,chf("exp"))+{0,0,1};

/* -------------------------------------------------------------- */
// remove prims with less than 3 points
int pts[] = primpoints( 0, i@primnum );
if( len(pts) < 3 ) removeprim( 0, i@primnum, 1 );

/* -------------------------------------------------------------- */
// #ramp from #distance to #surface 
int prim;
vector uv;
float dmin = chf("dist_min");
float dmax = chf("dist_max");
float d = xyzdist(1,v@P,prim,uv,dmax);
float f = fit(d,dmin,dmax,0,1);
if(chi("reverse_direction")==1)f=fit01(f,1,0);
v@Cd = f + {0,0,1};

/* -------------------------------------------------------------- */
// #delete by #proximity to #surface (run over points)
int prim;
vector uv;
float d = xyzdist(1,v@P,prim,uv,chf("maxdist"));
if(chi("invert_selection")==1){
     if(d<chf("maxdist")){
         removepoint(0,i@ptnum);
     }
}else{
    if(d>=chf("maxdist")){
        removepoint(0,i@ptnum);
    }
}

/* -------------------------------------------------------------- */
// create u attribute along one curve (run over points)
f@u = float(i@ptnum)/float(npoints(0));
if(chi("display_u")==1){
	v@Cd=f@u+{0,0,1}; 
}

/* -------------------------------------------------------------- */
// create u attribute along curves (run over primitives)
int p[] = primpoints(0,i@primnum);
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
if(i@ptnum>0){
    v@v = v@P-point(0,"P",i@ptnum-1);
}else{
    v@v = point(0,"P",i@ptnum+1)-v@P;
}
if(chi("reverse_direction")==1)v@v*=-1;

/* -------------------------------------------------------------- */
// vel along multiple curves (run over primitives)
int p[] = primpoints(0,i@primnum);
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
// #clouds #noise
#include < voplib.h>
// density mask (input 2)
float d = 1;
if(chi("dMask_useMask")==1){
    vector mask_pos_noise = anoise(v@P*chf("dMask_pn_freq")+chv("dMask_pn_offset"), chi("dMask_pn_turb"), chf("dMask_pn_rough"), chf("dMask_pn_atten"))*chf("dMask_pn_mult");
    d=pow(fit(volumesample(1,0,v@P+mask_pos_noise),0,chf("dMask_max_density"),1,0),chf("dMask_density_exp"));
}
// noise mask (aligator)
float mask = 1;
if(chi("nMask_useMask")==1){
    mask =  clamp(pow(anoise(v@P*chf("nMask_freq")+chv("nMask_offset"), chi("nMask_turb"), chf("nMask_rough"), chf("nMask_atten")),chf("nMask_exp"))*chf("nMask_mult"),0,1);
} 
// noise 1 (fbm)
vector n1p = vop_fbmNoiseVV(v@P*chf("n1_pn_Freq")+chv("n1_pn_Offset"),chf("n1_pn_rough"),chi("n1_pn_turb"),"noise");
vector n1 = vop_fbmNoiseVV(n1p*chf("n1_Freq")+chv("n1_Offset"),chf("n1_rough"),chi("n1_turb"),"noise");
// noise 2 (fbm)
vector n2p = vop_fbmNoiseVV(v@P*chf("n2_pn_Freq")+chv("n2_pn_Offset"),chf("n2_pn_rough"),chi("n2_pn_turb"),"noise");
vector n2 = vop_fbmNoiseVV(n2p*chf("n2_Freq")+chv("n2_Offset"),chf("n2_rough"),chi("n2_turb"),"noise");
// sample displaced density
f@density = volumesample(0,0,v@P+(n1*chf("n1_mult")+n2*chf("n2_mult"))*mask*d*chf("advection_length"));

/* -------------------------------------------------------------- */
// #rotate #quaternion around #axis
matrix3 m;
matrix3 rm;
m = qconvert(p@orient);
vector axis = chv("axis");
rotate(m, radians(chf("angle_in_degrees"), axis);    
p@orient = quaternion(m);

/* -------------------------------------------------------------- */
// #intersection of 2 #bounds
vector m1,M1,m2,M2;
getbbox(0,m1,M1);
getbbox(1,m2,M2);
if(m2.x>M1.x || M2.x< m1.x || m2.y>M1.y || M2.y< m1.y || m2.z>M1.z || M2.z< m1.z){
    removepoint(0,i@ptnum);
}else{
    v@P.x=min(M1.x,max(v@P.x,m2.x),M2.x);
    v@P.y=min(M1.y,max(v@P.y,m2.y),M2.y);
    v@P.z=min(M1.z,max(v@P.z,m2.z),M2.z);
}

/* -------------------------------------------------------------- */
// divscale #disturbance
if(f@density< chf("max_density")){

    float speed = length(v@vel);
    if(speed>chf("min_speed")){

        vector4 hvec;
        vector _div = (v@P + rand(f@Frame-32.47+chf("seed")) * chf("divscale"))/chf("divscale");
        hvec = _div;
        setcomp(hvec, f@Frame+94.42, 3);
        float r1 = float(random(hvec));
       
        if(r1>chf("threshold")){
       
            vector4 hvec2;
            vector _div2 = (v@P + rand(f@Frame+55.83+chf("seed")) * chf("divscale"))/chf("divscale");
            hvec2 = _div2;
            setcomp(hvec2, f@Frame+41.12, 3);
            vector r3 = random(hvec2);
            vector d = vector(chf("disturbance"));
            vector r3_ = fit(r3,vector(0),vector(1),-d,d);
       
            v@vel += r3_ * fit(speed,0,1,0,1);
        }
    }   
}

/* -------------------------------------------------------------- */
// #volume #motionblur using vel from second input (not VDB)
float vx=volumesample(1,0,v@P);
float vy=volumesample(1,1,v@P);
float vz=volumesample(1,2,v@P);
vector v = set(vx,vy,vz);
if(length(v)>0){
    float dsum=0;
    for(int i=0;i< chi("steps");i++){
        float f = float(i)/float(chi("steps")-1);
        dsum += volumesample(0,0,v@P+v*chf("displacement")*f);
    }
    f@density = dsum /float(chi("steps"));
}

/* -------------------------------------------------------------- */
// 3d fbm #noise - for vel
#include 
// noise 1
vector n1p = vop_fbmNoiseVV(v@P*chf("noise1Pos_Freq")+chv("noise1Pos_Offset"),chf("noise1Pos_Roughness"),chi("noise1Pos_Turbulence"),"noise");
vector n1 = vop_fbmNoiseVV(n1p*chf("noise1_Freq")+chv("noise1_Offset"),chf("noise1_Roughness"),chi("noise1_Turbulence"),"noise");
// noise 2
v@vel = n1;

//////////////////////////////////////////////////////////////////// 
// remap density
f@density = efit(pow(efit(f@density,0,chf("max_density"),0,1),chf("exp")),0,1,0,chf("out_density"));

//////////////////////////////////////////////////////////////////// 
// mask
f@density =  clamp(pow(anoise(v@P*chf("mask_freq")+chv("mask_offset"), chi("mask_turbulence"), chf("mask_roughness"), chf("mask_attenuation")),chf("mask_exp"))*chf("mask_mult"),0,1);

/* -------------------------------------------------------------- */
//EXTRACTING TRANSFORMS
//Depending on the value of c,
//returns the translate (c=0), rotate (c=1), or scale (c=2)
//component of the transform (xform)
//
matrix xform = primintrinsic(1, "packedfulltransform", i@ptnum);
//matrix xform = primintrinsic(1, "transform", i@ptnum);

#define XFORM_SRT       0  // Scale Rotate Translate
#define XFORM_XYZ       0 // Rx Ry Rz

int trs = XFORM_XYZ; //Transform Order
int xyz = XFORM_SRT; //Rotation Order
vector p = v@P; //pivot for crack/extracting transforms
vector translate = cracktransform(trs, xyz, 0 , p, xform);
vector rotate    = cracktransform(trs, xyz, 1 , p, xform);
vector scale    = cracktransform(trs, xyz, 2 , p, xform);

/* -------------------------------------------------------------- */
//MAKE TRANSFORM
matrix newTrans = maketransform(trs, xyz, translate, rotate,{1,1,1});
p@orient = quaternion(matrix3(newTrans));

//rotation in degrees
matrix matx = qconvert(p@orient);
vector extracted = cracktransform(0, 0, 1, v@P, matx);
v@rotation = extracted;

s@filename = primintrinsic(1, "filename", i@ptnum);

v@scale = scale;

/* -------------------------------------------------------------- */
// basic #point #clustering
// using point id (optional) and Cd (optional)

int use_Cd = chi("use_Cd");
int use_id = chi("use_id");
vector Cd = vector(0);
vector p = v@P;
int id = 0; 

int handle = pcopen(0,"P",v@P,chf("radius"),chi("maxpoints"));
if(pcnumfound(handle)>0){
    while(pciterate(handle)){
        
        pcimport(handle,"P",p);
        pcimport(handle,"id",id);
        if(use_Cd) pcimport(handle,"Cd",Cd);
        
        if(id>i@id || use_id==0){

            float dist = distance(v@P,p);
            int steps  = int(dist / chf("stepsize"));
            for(int i=0;i< steps;i++){
            
                float d = float(i)/float(steps);
                vector r = (rand(v@P*3927+i*537)*vector(2)-1);
                vector p_ = lerp(v@P,p,d)+r*chf("jitter");
                
                int newpoint = addpoint(0,p_);
                
                if(use_Cd){
                    vector Cd_ = lerp(v@Cd,Cd,d);            
                    setpointattrib(0,"Cd",newpoint,Cd_);
                }
            }
        }
    }
}

/* -------------------------------------------------------------- */
// #volume #camera #cull
vector pndc = toNDC(chs("camera_name"), v@P);
// padding
float pad = chf("padding");
if(pndc.x< 0-pad || pndc.x>1+pad || pndc.y< 0-pad || pndc.y>1+pad || pndc.z>=0 ){
    f@density=0;    
}

/* -------------------------------------------------------------- */
// bunker bullet auto freeze
if(length(v@v)< chf("min_vel")){
    f@s+=1;
    if(f@s>ch("min_vel_frames")){
        i@bullet_sleeping=1;
    }
}else{
    f@s=0;
}

if(distance(v@P,v@oldP)< chf("min_dist")){
    f@d=f@d+1;
    if(f@d>ch("min_dist_frames")){
        v@v *= clamp(1-chf("drag"),0,1);
        v@w *= clamp(1-chf("torque_drag"),0,1);
    }
}else{
    f@d=0;
}
// stash P
v@oldP = v@P;

/* -------------------------------------------------------------- */
// #connect #adjacent #pieces
void line(vector pos1,pos2){
    int p1 = addpoint(0,pos1);
    int p2 = addpoint(0,pos2);
    int prim = addprim(0,"polyline");
    int v1 = addvertex(0,prim,p1);
    int v2 = addvertex(0,prim,p2);
}

int handle = pcopen(0,"P",v@P,chf("radius"),chi("maxpoints"));
if(pcnumfound(handle)>0){
    while(pciterate(handle)){
        int id;
        pcimport(handle,"pieceid",id);
        if(id!=i@pieceid){            
            vector p;
            pcimport(handle,"P",p);        
            line(v@P,p);                    
        }     
    }
}

/* -------------------------------------------------------------- */
// remove #lone points (unconnected)
if(neighbourcount(0,i@ptnum)==0){
    removepoint(0,i@ptnum);
}else{
    // collapse lines into points (centroids)
    v@P = (v@P+point(0,"P",neighbour(0,i@ptnum,0)))*.5;
}

/* -------------------------------------------------------------- */
// #volume #vorticity (not VDB)
// input 2 has vel.* volumes
vector dx = volumegradient(1,"vel.x",v@P);
vector dy = volumegradient(1,"vel.y",v@P);
vector dz = volumegradient(1,"vel.z",v@P);
vector w = set(dz.y-dy.z,dx.z-dz.x,dy.x-dx.y);
v@Cd = fit(length(w),0,chf("max_vorticity"),0,1)+{0,0,1};

/* -------------------------------------------------------------- */
// SOP #ambient #occlusion
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
    hitprim = intersect(0,v@P+(v@N*0.001), raydir*maxdist, hit, u, v);
    hitprim_2 = intersect(1,v@P+(v@N*0.001), raydir*maxdist, hit, u, v);
    if (hitprim!=-1 || hitprim_2!=-1) tempOcc+=1;
}

float occ = clamp(vop_bias(1.0-(tempOcc / rays), bias), 0, 1);
v@Cd=occ;

// -----------------------------------------------------------------
// #hue #shift
vector hsv = rgbtohsv(@Cd);
hsv.x+=chf("hue_shift");
@Cd= hsvtorgb(hsv);
// -----------------------------------------------------------------
// #soft #clip (from pyro1 vfl)
float softclip (
      float f; // field value
      float s; // start val for compression
      float c; // compression
   )
{
   float out = f;
   if(f>s && c>0) {
      float ki = 1.0 / c;
      float w  = 1.0 / (c*log(10.0));
      float v  = log10(pow(w,ki));
      out = log10(pow((f-s)+w,ki)) - v + s;
   }
   return out;
}
// @P.y=softclip(@P.y,2,1); example usage
// -----------------------------------------------------------------
// delete polygons with only 2 points (lines)
int a = len(primpoints(0,@primnum));
if(a<3)removeprim(0,@primnum,1);
// -----------------------------------------------------------------
// #project on #sdf
for(int i=0;i<chi("steps");i++){
    @P.y-= chf("step_size");
    float vs = volumesample(1,0,@P);
    if(vs<0){
        break;
    }
    if(i==chi("steps")-1){
        removepoint(0,@ptnum,1);
    }
}
// -----------------------------------------------------------------
//convert #group to #attrib
string groups[] = detailintrinsic(0, "primitivegroups");
s[]@groups=groups;
int a=0;
foreach(string i; groups){   
    if(inprimgroup(0,i,@primnum) == 1){
        s@group=i;
        break;
    }
    a++;
}
// random color from group number
@Cd=rand(a);
i@groupnum=a;
// -----------------------------------------------------------------
// #point #density (normalized)
int pc = pcnumfound(pcopen(0,"P",@P,chf("radius"),chi("maxpoints")));
float density = float(pc)/float(chi("maxpoints"));
// -----------------------------------------------------------------
// #volume #diffusion 
float dsum = 0;
float mask = 1; // could be from ramp,volume,noise 

for(int i=0;i<chi("steps");i++){
    vector n = (rand(v@P*2389.42+@Frame+i)*vector(2)-1)*chf("diffusion")*mask;
    dsum += volumesample(0,0,@P+n);

}
f@density = dsum / float(chi("steps"));
// -----------------------------------------------------------------
// worley (#cell) #noise
float dist1,dist2;
wnoise(@P * chf("freq")+chv("offset"), chi("seed"), dist1, dist2);
float cellnoise = dist1;
// -----------------------------------------------------------------
//
