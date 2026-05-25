

#line 6 "c_superset_probes/p264_runtime_struct.sp"
typedef struct Vec3D Vec3D;
struct Vec3D {
    float x;
    float y;
    float z;
};

#line 12 "c_superset_probes/p264_runtime_struct.sp"
float Vec3D__length(Vec3D * this);
float Vec3D__length(Vec3D * this) {

#line 13 "c_superset_probes/p264_runtime_struct.sp"
    float xx = this->x * this->x;

#line 14 "c_superset_probes/p264_runtime_struct.sp"
    float yy = this->y * this->y;

#line 15 "c_superset_probes/p264_runtime_struct.sp"
    float zz = this->z * this->z;

#line 16 "c_superset_probes/p264_runtime_struct.sp"
    return xx + yy + zz;
}

#line 19 "c_superset_probes/p264_runtime_struct.sp"
int main() {

#line 20 "c_superset_probes/p264_runtime_struct.sp"
    Vec3D v;

#line 21 "c_superset_probes/p264_runtime_struct.sp"
    v.x = 3.0f;

#line 22 "c_superset_probes/p264_runtime_struct.sp"
    v.y = 4.0f;

#line 23 "c_superset_probes/p264_runtime_struct.sp"
    v.z = 0.0f;

#line 25 "c_superset_probes/p264_runtime_struct.sp"
    float len_sq = Vec3D__length(&v);

#line 27 "c_superset_probes/p264_runtime_struct.sp"
    if (len_sq != 25.0f) 
        return 1;

#line 30 "c_superset_probes/p264_runtime_struct.sp"
    v.z = 12.0f;

#line 31 "c_superset_probes/p264_runtime_struct.sp"
    float len_sq2 = Vec3D__length(&v);

#line 33 "c_superset_probes/p264_runtime_struct.sp"
    if (len_sq2 != 169.0f) 
        return 2;

#line 36 "c_superset_probes/p264_runtime_struct.sp"
    Vec3D w = v;

#line 37 "c_superset_probes/p264_runtime_struct.sp"
    if (w.x != 3.0f) 
        return 3;

#line 38 "c_superset_probes/p264_runtime_struct.sp"
    if (w.y != 4.0f) 
        return 4;

#line 39 "c_superset_probes/p264_runtime_struct.sp"
    if (w.z != 12.0f) 
        return 5;

#line 41 "c_superset_probes/p264_runtime_struct.sp"
    return 0;
}
