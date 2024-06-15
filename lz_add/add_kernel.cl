
kernel void vector_add(global int *src1, global int *src2, global int *dst) 
{
  const int id = get_global_id(0);
  dst[id] = src1[id] + src2[id];
}