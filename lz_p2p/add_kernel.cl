
kernel void vector_add(global int *src1, global int *src2) 
{
  const int id = get_global_id(0);
  src1[id] = src1[id] + src2[id];
}