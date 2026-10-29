#pragma OPENCL EXTENSION cl_khr_fp16 : enable

__kernel void vector_add( __global const float *B, __global float *C, int n) {
const int i = get_global_id(0);
const int j = get_global_id(1);

if(j>=n){
if (j - n == i) { C[i*n*2+j] = 1;
}
else { C[i*n*2+j]=0;
}
}
else if (j<n){C[i*n*2+j]=(float)B[i*n+j];
}
}

__kernel void vector_add2( __global float *C, int n, __global float *C_copy,float oporn, int ii) {
const int i = get_global_id(0);
const int j = get_global_id(1);

if(i!=ii&&oporn!=0)
C_copy[i*n*2+j]=(C[i*n*2+j]*C[ii*n*2+ii]-C[ii*n*2+j]*C[i*n*2+ii])/oporn;
}

__kernel void vector_add3( __global float *C_copy, int n,float determinamt) {
const int i = get_global_id(0);
const int j = get_global_id(1);

C_copy[i*n*2+j]=C_copy[i*n*2+j]/determinamt;
}
