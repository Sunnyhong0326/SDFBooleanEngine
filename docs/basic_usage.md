# Basic Usage
To create your own 3D assets, you need to follow the format of the json file and 
## CSG Json file format

## SDF Primitives
### Sphere
```glsl
float sdSphere( vec3 p, float s )
{
  return length(p)-s;
}
```
### Box
```glsl
float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}
```
### Triangle Prism
```glsl
float sdTriPrism( vec3 p, vec2 h )
{
  vec3 q = abs(p);
  return max(q.z-h.y,max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5);
}
```

## SDF Operations
### Union
```glsl
float opUnion( float d1, float d2 )
{
    return min(d1,d2);
}
```
### Subtract
```glsl
float opSubtraction( float d1, float d2 )
{
    return max(-d1,d2);
}
```
### Intersection
```glsl
float opIntersection( float d1, float d2 )
{
    return max(d1,d2);
}
```