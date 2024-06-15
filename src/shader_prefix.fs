float dot2( in vec2 v ) { return dot(v,v); }
float dot2( in vec3 v ) { return dot(v,v); }
float ndot( in vec2 a, in vec2 b ) { return a.x*b.x - a.y*b.y; }

float sdRoundBox( vec3 p, vec3 unused, vec3 b, float r )//a unused
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}

// float RoundBox( vec3 p, vec3 b, float r )
// {
//   vec3 q = abs(p) - b;
//   return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
// }
//https://iquilezles.org/
float sdSphere( vec3 p, vec3 unused, vec3 b, float s ) //b unused
{
  return length(p)-s;
}

float sdBoxFrame( vec3 p, vec3 unused, vec3 b, float e )
{
       p = abs(p)-b;
  vec3 q = abs(p+e)-e;
  return min(min(
      length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
      length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
      length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

float sdTorus( vec3 p, vec3 unused, vec3 t , float s) //t: vec2->vec3 s unused, a unused
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float sdBox( vec3 p, vec3 unused, vec3 b, float s) //s unused
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdCappedTorus( vec3 p, vec3 unused, vec3 sc, float ra) //float rb = sc.z
{
  p.x = abs(p.x);
  float k = (sc.y*p.x>sc.x*p.y) ? dot(p.xy,sc.xy) : length(p.xy);
  return sqrt( dot(p,p) + ra*ra - 2.0*ra*k ) - sc.z;
}

float sdLink( vec3 p, vec3 unused, vec3 a, float s) //vec3 a = float le, float r1, float r2 )
{
  vec3 q = vec3( p.x, max(abs(p.y)-a.x,0.0), p.z );
  return length(vec2(length(q.xy)-a.y,q.z)) - a.z;
}

float sdCylinder( vec3 p, vec3 a, vec3 c , float s)
{
  return length(p.xz-c.xy)-c.z;
}

float sdCone( vec3 p, vec3 unused, vec3 c, float h ) //c: vec2-> vec3
{
  // c is the sin/cos of the angle, h is height
  // Alternatively pass q instead of (c,h),
  // which is the point at the base in 2D
  vec2 q = h*vec2(c.x/c.y,-1.0);

  vec2 w = vec2( length(p.xz), p.y );
  vec2 a = w - q*clamp( dot(w,q)/dot(q,q), 0.0, 1.0 );
  vec2 b = w - q*vec2( clamp( w.x/q.x, 0.0, 1.0 ), 1.0 );
  float k = sign( q.y );
  float d = min(dot( a, a ),dot(b, b));
  float s = max( k*(w.x*q.y-w.y*q.x),k*(w.y-q.y)  );
  return sqrt(d)*sign(s);
}

float sdPlane( vec3 p, vec3 unused, vec3 n, float h )
{
  // n must be normalized
  return dot(p,n) + h;
}

float sdHexPrism( vec3 p, vec3 unused, vec3 h , float s) //h: vec2 -> vec3 , float s unused
{
  const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
  p = abs(p);
  p.xy -= 2.0*min(dot(k.xy, p.xy), 0.0)*k.xy;
  vec2 d = vec2(
       length(p.xy-vec2(clamp(p.x,-k.z*h.x,k.z*h.x), h.x))*sign(p.y-h.x),
       p.z-h.y );
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float sdTriPrism( vec3 p, vec3 unused, vec3 h , float s) //h: vec2 -> vec3, float s unused
{
  vec3 q = abs(p);
  return max(q.z-h.y,max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5);
}

float sdCapsule( vec3 p, vec3 a, vec3 b, float r )
{
  vec3 pa = p - a, ba = b - a;
  float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return length( pa - ba*h ) - r;
}

float sdVerticalCapsule( vec3 p, vec3 unused, vec3 h, float r )//h: float _> vec3
{
  p.y -= clamp( p.y, 0.0, h.x );
  return length( p ) - r;
}

// float sdCappedCylinder( vec3 p, vec3 unused, vec3 h, float r )//h: float _> vec3
// {
//   vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(r,h.x);
//   return min(max(d.x,d.y),0.0) + length(max(d,0.0));
// }

float sdCappedCylinder( vec3 p, vec3 a, vec3 b, float r )
{
  vec3  ba = b - a;
  vec3  pa = p - a;
  float baba = dot(ba,ba);
  float paba = dot(pa,ba);
  float x = length(pa*baba-ba*paba) - r*baba;
  float y = abs(paba-baba*0.5)-baba*0.5;
  float x2 = x*x;
  float y2 = y*y*baba;
  float d = (max(x,y)<0.0)?-min(x2,y2):(((x>0.0)?x2:0.0)+((y>0.0)?y2:0.0));
  return sign(d)*sqrt(abs(d))/baba;
}

float sdRoundedCylinder( vec3 p, vec3 unused, vec3 r, float s) //float ra, float rb, float h ), a unused
{
  vec2 d = vec2( length(p.xz)-2.0*r.x+r.y, abs(p.y) - r.z );
  return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - r.y;
}

float sdCappedCone( vec3 p, vec3 unused, vec3 r, float unused2) //float h, float r1, float r2, a unused, s unused
{
  vec2 q = vec2( length(p.xz), p.y );
  vec2 k1 = vec2(r.z,r.x);
  vec2 k2 = vec2(r.z-r.y,2.0*r.x);
  vec2 ca = vec2(q.x-min(q.x,(q.y<0.0)?r.y:r.z), abs(q.y)-r.x);
  vec2 cb = q - k1 + k2*clamp( dot(k1-q,k2)/dot2(k2), 0.0, 1.0 );
  float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
  return s*sqrt( min(dot2(ca),dot2(cb)) );
}

float sdSolidAngle( vec3 p, vec3 unused, vec3 c, float ra ) //c: vec2 -> vec3, a unused
{
  // c is the sin/cos of the angle
  vec2 q = vec2( length(p.xz), p.y );
  float l = length(q) - ra;
  float m = length(q - c.xy*clamp(dot(q,c.xy),0.0,ra) );
  return max(l,m*sign(c.y*q.x-c.x*q.y));
}

float sdCutSphere( vec3 p, vec3 unused, vec3 r, float unused2) //float r, float h ), s unused, a unused
{
  // sampling independent computations (only depend on shape)
  float w = sqrt(r.x*r.x-r.y*r.y);

  // sampling dependant computations
  vec2 q = vec2( length(p.xz), p.y );
  float s = max( (r.y-r.x)*q.x*q.x+w*w*(r.y+r.x-2.0*q.y), r.y*q.x-w*q.y );
  return (s<0.0) ? length(q)-r.x :
         (q.x<w) ? r.y - q.y     :
                   length(q-vec2(w,r.y));
}


float sdCutHollowSphere( vec3 p, vec3 unused , vec3 r, float s) //float r, float h, float t ), a unused, s unused
{
  // sampling independent computations (only depend on shape)
  float w = sqrt(r.x*r.x-r.y*r.y);

  // sampling dependant computations
  vec2 q = vec2( length(p.xz), p.y );
  return ((r.y*q.x<w*q.y) ? length(q-vec2(w,r.y)) :
                          abs(length(q)-r.x) ) - r.z;
}

float sdDeathStar( vec3 p2, vec3 unused , vec3 r, float s) // float ra, float rb, float d ) s unused
{
  // sampling independent computations (only depend on shape)
  float a = (r.x*r.x - r.y*r.y + r.z*r.z)/(2.0*r.z);
  float b = sqrt(max(r.x*r.x-a*a,0.0));

  // sampling dependant computations
  vec2 p = vec2( p2.x, length(p2.yz) );
  if( p.x*b-p.y*a > r.z*max(b-p.y,0.0) )
    return length(p-vec2(a,b));
  else
    return max( (length(p            )-r.x),
               -(length(p-vec2(r.z,0.0))-r.y));
}

float sdRoundCone( vec3 p, vec3 unused, vec3 r, float s) //float r1, float r2, float h )
{
  // sampling independent computations (only depend on shape)
  float b = (r.x-r.y)/r.z;
  float a = sqrt(1.0-b*b);

  // sampling dependant computations
  vec2 q = vec2( length(p.xz), p.y );
  float k = dot(q,vec2(-b,a));
  if( k<0.0 ) return length(q) - r.x;
  if( k>a*r.z ) return length(q-vec2(0.0,r.z)) - r.y;
  return dot(q, vec2(a,b) ) - r.x;
}

// float sdRoundCone( vec3 p, vec3 a, vec3 b, float r1, float r2 )
// {
//   // sampling independent computations (only depend on shape)
//   vec3  ba = b - a;
//   float l2 = dot(ba,ba);
//   float rr = r1 - r2;
//   float a2 = l2 - rr*rr;
//   float il2 = 1.0/l2;
//
//   // sampling dependant computations
//   vec3 pa = p - a;
//   float y = dot(pa,ba);
//   float z = y - l2;
//   float x2 = dot2( pa*l2 - ba*y );
//   float y2 = y*y*l2;
//   float z2 = z*z*l2;
//
//   // single square root!
//   float k = sign(rr)*rr*rr*x2;
//   if( sign(z)*a2*z2>k ) return  sqrt(x2 + z2)        *il2 - r2;
//   if( sign(y)*a2*y2<k ) return  sqrt(x2 + y2)        *il2 - r1;
//                         return (sqrt(x2*a2*il2)+y*rr)*il2 - r1;
// }

float sdEllipsoid( vec3 p, vec3 unused, vec3 r, float s)
{
  float k0 = length(p/r);
  float k1 = length(p/(r*r));
  return k0*(k0-1.0)/k1;
}

float sdVesicaSegment( in vec3 p, in vec3 a, in vec3 b, in float w )
{
    vec3  c = (a+b)*0.5;
    float l = length(b-a);
    vec3  v = (b-a)/l;
    float y = dot(p-c,v);
    vec2  q = vec2(length(p-c-y*v),abs(y));

    float r = 0.5*l;
    float d = 0.5*(r*r-w*w)/w;
    vec3  h = (r*q.x<d*(q.y-r)) ? vec3(0.0,r,0.0) : vec3(-d,0.0,d+w);

    return length(q-h.xy) - h.z;
}

float sdRhombus( vec3 p, vec3 unused, vec3 l, float ra) //float la, float lb, float h, float ra )
{
  p = abs(p);
  vec2 b = vec2(l.x,l.y);
  float f = clamp( (ndot(b,b-2.0*p.xz))/dot(b,b), -1.0, 1.0 );
  vec2 q = vec2(length(p.xz-0.5*b*vec2(1.0-f,1.0+f))*sign(p.x*b.y+p.z*b.x-b.x*b.y)-ra, p.y-l.z);
  return min(max(q.x,q.y),0.0) + length(max(q,0.0));
}

float sdOctahedron( vec3 p, vec3 unused, vec3 unused2, float s )
{
  p = abs(p);
  float m = p.x+p.y+p.z-s;
  vec3 q;
       if( 3.0*p.x < m ) q = p.xyz;
  else if( 3.0*p.y < m ) q = p.yzx;
  else if( 3.0*p.z < m ) q = p.zxy;
  else return m*0.57735027;

  float k = clamp(0.5*(q.z-q.y+s),0.0,s);
  return length(vec3(q.x,q.y-s+k,q.z-k));
}

float sdPyramid( vec3 p, vec3 unused, vec3 unused2, float h )
{
  float m2 = h*h + 0.25;

  p.xz = abs(p.xz);
  p.xz = (p.z>p.x) ? p.zx : p.xz;
  p.xz -= 0.5;

  vec3 q = vec3( p.z, h*p.y - 0.5*p.x, h*p.x + 0.5*p.y);

  float s = max(-q.x,0.0);
  float t = clamp( (q.y-0.5*p.z)/(m2+0.25), 0.0, 1.0 );

  float a = m2*(q.x+s)*(q.x+s) + q.y*q.y;
  float b = m2*(q.x+0.5*t)*(q.x+0.5*t) + (q.y-m2*t)*(q.y-m2*t);

  float d2 = min(q.y,-q.x*m2-q.y*0.5) > 0.0 ? 0.0 : min(a,b);

  return sqrt( (d2+q.z*q.z)/m2 ) * sign(max(q.z,-p.y));
}

// float udTriangle( vec3 p, vec3 a, vec3 b, vec3 c )
// {
//   vec3 ba = b - a; vec3 pa = p - a;
//   vec3 cb = c - b; vec3 pb = p - b;
//   vec3 ac = a - c; vec3 pc = p - c;
//   vec3 nor = cross( ba, ac );
//
//   return sqrt(
//     (sign(dot(cross(ba,nor),pa)) +
//      sign(dot(cross(cb,nor),pb)) +
//      sign(dot(cross(ac,nor),pc))<2.0)
//      ?
//      min( min(
//      dot2(ba*clamp(dot(ba,pa)/dot2(ba),0.0,1.0)-pa),
//      dot2(cb*clamp(dot(cb,pb)/dot2(cb),0.0,1.0)-pb) ),
//      dot2(ac*clamp(dot(ac,pc)/dot2(ac),0.0,1.0)-pc) )
//      :
//      dot(nor,pa)*dot(nor,pa)/dot2(nor) );
// }
//
// float udQuad( vec3 p, vec3 a, vec3 b, vec3 c, vec3 d )
// {
//   vec3 ba = b - a; vec3 pa = p - a;
//   vec3 cb = c - b; vec3 pb = p - b;
//   vec3 dc = d - c; vec3 pc = p - c;
//   vec3 ad = a - d; vec3 pd = p - d;
//   vec3 nor = cross( ba, ad );
//
//   return sqrt(
//     (sign(dot(cross(ba,nor),pa)) +
//      sign(dot(cross(cb,nor),pb)) +
//      sign(dot(cross(dc,nor),pc)) +
//      sign(dot(cross(ad,nor),pd))<3.0)
//      ?
//      min( min( min(
//      dot2(ba*clamp(dot(ba,pa)/dot2(ba),0.0,1.0)-pa),
//      dot2(cb*clamp(dot(cb,pb)/dot2(cb),0.0,1.0)-pb) ),
//      dot2(dc*clamp(dot(dc,pc)/dot2(dc),0.0,1.0)-pc) ),
//      dot2(ad*clamp(dot(ad,pd)/dot2(ad),0.0,1.0)-pd) )
//      :
//      dot(nor,pa)*dot(nor,pa)/dot2(nor) );
// }


// float opSmoothUnion( float d1, float d2, float k )
// {
//     float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
//     return mix( d2, d1, h ) - k*h*(1.0-h);
// }

vec4 opSmoothUnion( vec4 a, vec4 b)
{
    float h =  max( blend-abs(a.x-b.x), 0.0 )/blend;
    float m = h*h*0.5;
    float s = m*blend*(1.0/2.0);
    return (a.x<b.x) ? vec4(a.x-s,mix(a.gba,b.gba,m)) : vec4(b.x-s,mix(a.gba,b.gba,1.0-m));
}

vec4 opSmoothSubtraction( vec4 d1, vec4 d2 )
{
    float dist = opSmoothUnion(d1,vec4(-d2.x, d2.gba)).x;
    return vec4(-dist, d2.gba);
}

vec4 opSmoothIntersection( vec4 a, vec4 b)
{
    float h =  max( blend-abs(a.x-b.x), 0.0 )/blend;
    float m = h*h*0.5;
    float s = m*blend*(1.0/2.0);
    return (a.x<b.x) ? vec4(a.x-s,mix(a.gba,b.gba,m)) : vec4(b.x-s,mix(a.gba,b.gba,1.0-m));
}

// float opSmoothIntersection( float d1, float d2, float k )
//{
//         float h = clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
//         return mix( d2, d1, h ) + k*h*(1.0-h);
//     }


vec4 opRound( vec4 primitive3d, float rad )
{
    primitive3d.x = primitive3d.x - rad;
    return  primitive3d;
}


vec4 opOnion(vec4 primitive3d, in float thickness )
{
    primitive3d.x = abs(primitive3d.x) -thickness;
    return primitive3d;
}


vec4 opUnion( vec4 d1, vec4 d2 ) { vec4 res = (d1.x<d2.x?d1:d2); return res;}

vec4 opSubtraction( vec4 d1, vec4 d2 ) { vec4 res = ((-d1.x)>d2.x?-d1:d2); return res;}

vec4 opIntersection( vec4 d1, vec4 d2 ) { vec4 res = (d1.x>d2.x?d1:d2); return res; }

vec4 BlobbyMin( vec4 a, vec4 b, float blend )
{
    float h =  max( blend-abs(a.x-b.x), 0.0 )/blend;
    float m = h*h*0.5;
    float s = m*blend*(1.0/2.0);
    return (a.x<b.x) ? vec4(a.x-s,mix(a.gba,b.gba,m)) : vec4(b.x-s,mix(a.gba,b.gba,1.0-m));
}

vec4 Min( vec4 a, vec4 b )
{
    return (a.x<b.x) ? a : b;
}

vec4 opSmoothUnionSteppedColor( vec4 a, vec4 b, float blend )
{
    float h =  max( blend-abs(a.x-b.x), 0.0 )/blend;
    float m = h*h*0.5;
    float s = m*blend*(1.0/2.0);
    return (a.x<b.x) ? vec4(a.x-s,a.gba) : vec4(b.x-s,b.gba);
}



vec4 opS( vec4 d1, vec4 d2 )
{
    return vec4(max(-d2.x,d1.x), d1.gba);
}

vec4 opU( vec4 d1, vec4 d2 )
{
    return (d1.x<d2.x) ? d1 : d2;
}

vec3 opSymX( vec3 p )
{
    p.x = abs(p.x);
    return p;
}
vec3 opSymY( vec3 p )
{
    p.y = abs(p.y);
    return p;
}
vec3 opSymZ( vec3 p )
{
    p.z = abs(p.z);
    return p;
}
vec3 opSymXY( vec3 p )
{
    p.xy = abs(p.xy);
    return p;
}
vec3 opSymXZ( vec3 p )
{
    p.xz = abs(p.xz);
    return p;
}
vec3 opSymYZ( vec3 p )
{
    p.yz = abs(p.yz);
    return p;
}
vec3 opSymXYZ( vec3 p )
{
    p.xyz = abs(p.xyz);
    return p;
}

vec3 opRotateXYZ( vec3 p, vec3 theta)
{
    float cz = cos(theta.z);
    float sz = sin(theta.z);
    float cy = cos(theta.y);
    float sy = sin(theta.y);
    float cx = cos(theta.x);
    float sx = sin(theta.x);

    mat3 mat = mat3(
                cz*cy,
                cz*sy*sx - cx*sz,
                sz*sx + cz*cx*sy,

                cy*sz,
                cz*cx + sz*sy*sx,
                cx*sz*sy - cz*sx,

                -sy,
                cy*sx,
                cy*cx);

    return mat*p;
}


