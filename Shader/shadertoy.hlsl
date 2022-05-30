#include "shadertoy_changer.hlsli"

#define SHADERTOY 5


#if SHADERTOY == 0
// Protean clouds
// https://www.shadertoy.com/view/3l23Rh
// Protean clouds by nimitz (twitter: @stormoid)
// Protean clouds by nimitz (twitter: @stormoid)
// https://www.shadertoy.com/view/3l23Rh
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// Contact the author for other licensing options

/*
	Technical details:

	The main volume noise is generated from a deformed periodic grid, which can produce
	a large range of noise-like patterns at very cheap evalutation cost. Allowing for multiple
	fetches of volume gradient computation for improved lighting.

	To further accelerate marching, since the volume is smooth, more than half the the density
	information isn't used to rendering or shading but only as an underlying volume	distance to
	determine dynamic step size, by carefully selecting an equation	(polynomial for speed) to
	step as a function of overall density (not necessarialy rendered) the visual results can be
	the	same as a naive implementation with ~40% increase in rendering performance.

	Since the dynamic marching step size is even less uniform due to steps not being rendered at all
	the fog is evaluated as the difference of the fog integral at each rendered step.

*/

mat2 rot(in float a) { float c = cos(a), s = sin(a); return mat2(c, s, -s, c); }
static const mat3 m3 = mat3(0.33338, 0.56034, -0.71817, -0.87887, 0.32651, -0.15323, 0.15162, 0.69596, 0.61339) * 1.93;
float mag2(vec2 p) { return dot(p, p); }
float linstep(in float mn, in float mx, in float x) { return clamp((x - mn) / (mx - mn), 0., 1.); }
static float prm1 = 0.;
static vec2 bsMo = (0);

vec2 disp(float t) { return vec2(sin(t * 0.22) * 1., cos(t * 0.175) * 1.) * 2.; }

vec2 map(vec3 p)
{
	vec3 p2 = p;
	p2.xy -= disp(p.z).xy;
	p.xy = mul(p.xy,rot(sin(p.z + iTime) * (0.1 + prm1 * 0.05) + iTime * 0.09));
	float cl = mag2(p2.xy);
	float d = 0.;
	p *= .61;
	float z = 1.;
	float trk = 1.;
	float dspAmp = 0.1 + prm1 * 0.2;
	for (int i = 0; i < 5; i++)
	{
		p += sin(p.zxy * 0.75 * trk + iTime * trk * .8) * dspAmp;
		d -= abs(dot(cos(p), sin(p.yzx)) * z);
		z *= 0.57;
		trk *= 1.4;
		p = mul(p , m3);
	}
	d = abs(d + prm1 * 3.) + prm1 * .3 - 2.5 + bsMo.y;
	return vec2(d + cl * .2 + 0.25, cl);
}

vec4 render(in vec3 ro, in vec3 rd, float time)
{
	vec4 rez = (0);
	const float ldst = 8.;
	vec3 lpos = vec3(disp(time + ldst) * 0.5, time + ldst);
	float t = 1.5;
	float fogT = 0.;
	for (int i = 0; i < 130; i++)
	{
		if (rez.a > 0.99)break;

		vec3 pos = ro + t * rd;
		vec2 mpv = map(pos);
		float den = clamp(mpv.x - 0.3, 0., 1.) * 1.12;
		float dn = clamp((mpv.x + 2.), 0., 3.);

		vec4 col = (0);
		if (mpv.x > 0.6)
		{

			col = vec4(sin(vec3(5., 0.4, 0.2) + mpv.y * 0.1 + sin(pos.z * 0.4) * 0.5 + 1.8) * 0.5 + 0.5, 0.08);
			col *= den * den * den;
			col.rgb *= linstep(4., -2.5, mpv.x) * 2.3;
			float dif = clamp((den - map(pos + .8).x) / 9., 0.001, 1.);
			dif += clamp((den - map(pos + .35).x) / 2.5, 0.001, 1.);
			col.xyz *= den * (vec3(0.005, .045, .075) + 1.5 * vec3(0.033, 0.07, 0.03) * dif);
		}

		float fogC = exp(t * 0.2 - 2.2);
		col.rgba += vec4(0.06, 0.11, 0.11, 0.1) * clamp(fogC - fogT, 0., 1.);
		fogT = fogC;
		rez = rez + col * (1. - rez.a);
		t += clamp(0.5 - dn * dn * .05, 0.09, 0.3);
	}
	return clamp(rez, 0.0, 1.0);
}

float getsat(vec3 c)
{
	float mi = min(min(c.x, c.y), c.z);
	float ma = max(max(c.x, c.y), c.z);
	return (ma - mi) / (ma + 1e-7);
}

//from my "Will it blend" shader (https://www.shadertoy.com/view/lsdGzN)
vec3 iLerp(in vec3 a, in vec3 b, in float x)
{
	vec3 ic = mix(a, b, x) + vec3(1e-6, 0., 0.);
	float sd = abs(getsat(ic) - mix(getsat(a), getsat(b), x));
	vec3 dir = normalize(vec3(2. * ic.x - ic.y - ic.z, 2. * ic.y - ic.x - ic.z, 2. * ic.z - ic.y - ic.x));
	float lgt = dot((1.0), ic);
	float ff = dot(dir, normalize(ic));
	ic += 1.5 * dir * sd * ff * lgt;
	return clamp(ic, 0., 1.);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 q = fragCoord.xy / iResolution.xy;
	vec2 p = (fragCoord.xy - 0.5 * iResolution.xy) / iResolution.y;
	bsMo = (iMouse.xy - 0.5 * iResolution.xy) / iResolution.y;

	float time = iTime * 3.;
	vec3 ro = vec3(0, 0, time);

	ro += vec3(sin(iTime) * 0.5, sin(iTime * 1.) * 0., 0);

	float dspAmp = .85;
	ro.xy += disp(ro.z) * dspAmp;
	float tgtDst = 3.5;

	vec3 target = normalize(ro - vec3(disp(time + tgtDst) * dspAmp, time + tgtDst));
	ro.x -= bsMo.x * 2.;
	vec3 rightdir = normalize(cross(target, vec3(0, 1, 0)));
	vec3 updir = normalize(cross(rightdir, target));
	rightdir = normalize(cross(updir, target));
	vec3 rd = normalize((p.x * rightdir + p.y * updir) * 1. - target);
	rd.xy = mul(rd.xy,rot(-disp(time + 3.5).x * 0.2 + bsMo.x));
	prm1 = smoothstep(-0.4, 0.4, sin(iTime * 0.3));
	vec4 scn = render(ro, rd, time);

	vec3 col = scn.rgb;
	col = iLerp(col.bgr, col.rgb, clamp(1. - prm1, 0.05, 1.));

	col = pow(col, vec3(.55, 0.65, 0.6)) * vec3(1., .97, .9);

	col *= pow(16.0 * q.x * q.y * (1.0 - q.x) * (1.0 - q.y), 0.12) * 0.7 + 0.3; //Vign

	fragColor = vec4(col, 1.0);
}


#endif

#if SHADERTOY == 1
// Flying fireflies
// https://www.shadertoy.com/view/WscBzX
#define radius 0.02
#define sphere_Count 60.0

float N21(vec2 p) {
	vec3 a = fract(vec3(p.xyx) * vec3(213.897, 653.453, 253.098));
	a += dot(a, a.yzx + 79.76);
	return fract((a.x + a.y) * a.z);
}

vec2 N22(vec2 p) {
	float n = N21(p);
	return vec2(n, N21(n + p));
	
}


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord / (iResolution.xy / 2.0) - 1.0;
	uv.x *= iResolution.x / iResolution.y;

	vec3 pointLight;
	for (float i = 0.0; i < sphere_Count; i += 1.0)
	{
		vec2 rnd = N22(vec2(i, i * 2.0));
		vec2 p = vec2(cos(iTime * rnd.x + i) * 2.0, sin(iTime * rnd.y + i));
		float distanceToPoint = distance(uv, p);
		pointLight += mul(radius / distanceToPoint ,sin(iTime + i) / 2.5 + 0.7);
	}
	pointLight *= vec3(0.3, 0.3, 1.0);

	/*
	vec3 pointLight2;
	for (float i=60.0; i<80.0;i+=1.0)
	{
		vec2 rnd = N22(vec2(i,i+2.0));
		vec2 point = vec2(cos(iTime*rnd.x+i)*1.5,sin(iTime*rnd.y+i));
		float distanceToPoint = distance(uv, point);
		pointLight2 += vec3(radius/distanceToPoint) * vec3(clamp(sin(iTime+i)/2.0+0.6,0.1,1.0));
	}
	pointLight2 *= vec3(0.5,0.8,0.5);
	pointLight += pointLight2;
	*/

	fragColor = vec4(pointLight, 1.0);
}




#endif

#if SHADERTOY == 2
// Rainier mood 
// https://www.shadertoy.com/view/ldfyzl

/*

A quick experiment with rain drop ripples.

This effect was written for and used in the launch scene of the
64kB intro "H - Immersion", by Ctrl-Alt-Test.

 > http://www.ctrl-alt-test.fr/productions/h-immersion/
 > https://www.youtube.com/watch?v=27PN1SsXbjM

--
Zavie / Ctrl-Alt-Test

*/

// Maximum number of cells a ripple can cross.
#define MAX_RADIUS 2

// Set to 1 to hash twice. Slower, but less patterns.
#define DOUBLE_HASH 0

// Hash functions shamefully stolen from:
// https://www.shadertoy.com/view/4djSRW
#define HASHSCALE1 .1031
#define HASHSCALE3 vec3(.1031, .1030, .0973)

float hash12(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * HASHSCALE1);
	p3 += dot(p3, p3.yzx + 19.19);
	return fract((p3.x + p3.y) * p3.z);
}

vec2 hash22(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * HASHSCALE3);
	p3 += dot(p3, p3.yzx + 19.19);
	return fract((p3.xx + p3.yz) * p3.zy);

}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	float resolution = 10. * exp2(-3. * iMouse.x / iResolution.x);
	vec2 uv = fragCoord.xy / iResolution.y * resolution;
	vec2 p0 = floor(uv);

	vec2 circles = (0.);
	for (int j = -MAX_RADIUS; j <= MAX_RADIUS; ++j)
	{
		for (int i = -MAX_RADIUS; i <= MAX_RADIUS; ++i)
		{
			vec2 pi = p0 + vec2(i, j);
#if DOUBLE_HASH
			vec2 hsh = hash22(pi);
#else
			vec2 hsh = pi;
#endif
			vec2 p = pi + hash22(hsh);

			float t = fract(0.3 * iTime + hash12(hsh));
			vec2 v = p - uv;
			float d = length(v) - (float(MAX_RADIUS) + 1.) * t;

			float h = 1e-3;
			float d1 = d - h;
			float d2 = d + h;
			float p1 = sin(31. * d1) * smoothstep(-0.6, -0.3, d1) * smoothstep(0., -0.3, d1);
			float p2 = sin(31. * d2) * smoothstep(-0.6, -0.3, d2) * smoothstep(0., -0.3, d2);
			circles += 0.5 * normalize(v) * ((p2 - p1) / (2. * h) * (1. - t) * (1. - t));
		}
	}
	circles /= float((MAX_RADIUS * 2 + 1) * (MAX_RADIUS * 2 + 1));

	float intensity = mix(0.01, 0.15, smoothstep(0.1, 0.6, abs(fract(0.05 * iTime + 0.5) * 2. - 1.)));
	vec3 n = vec3(circles, sqrt(1. - dot(circles, circles)));
	vec3 color = texture(iChannel0, uv / resolution - intensity * n.xy).rgb + 5. * pow(clamp(dot(n, normalize(vec3(1., 0.7, 0.5))), 0., 1.), 6.);
	fragColor = vec4(color, 1.0);
}


#endif

#if SHADERTOY == 3
// Cole Peterson
#define R iResolution
#define m vec2(R.x/R.y*(iMouse.x/R.x-.5),iMouse.y/R.y-.5)
mat2 rot(float a) {
	return mat2(cos(a), -sin(a), sin(a), cos(a));
}

void mainImage(out vec4 f, in vec2 u)
{
	vec3 col = /*vec3*/(0);

	float r = .47;
	float n = 20.;

	for (float i = n; i > 0.; i--)
	{
		vec2 uv = vec2(u.xy - 0.5*R.xy) / R.y;;
		uv *= (i*.007 + .2);
		//uv *= rot(i*.05);
		uv = mul(uv, rot(i*.05));
		uv.x += iTime * .075;
		if (iMouse.z > 0.)uv -= m.x*.5;
		vec2 id = floor(uv*8.);

		uv.y += sin(i*.5 + iTime * 4. + id.y*345. + id.x*883.)*0.007;
		uv.x += cos(i*.5 + iTime * 4. + id.y*845. + id.x*383.)*0.007;

		vec2 ruv = fract(uv*8.) - .5;
		id = floor(uv*8.);

		vec3 nc = .55 + .3*cos(vec3(2., 3., 0.4)*(id.x + id.y + i * 0.05 + iTime * .6)*3.);

		float s = max(pow(dot(ruv, vec2(-.8, .5))*4.4, 4.0), 0.001);
		nc *= abs(s) + .6;
		nc *= ((n - i) / n);

		col = mix(col, nc, smoothstep(r, r - .015, length(ruv)));
		col *= 1. - smoothstep(0.01, 0.003, abs(length(ruv) - r + .005));
		r -= .0215;
	}
	f = vec4(col, 1.0);
}
#endif

#if SHADERTOY == 4

//'Warp Speed' by David Hoskins 2013.
//I tried to find gaps and variation in the star cloud for a feeling of structure.
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	float time = (iTime + 29.) * 60.0;

	float s = 0.0, v = 0.0;
	vec2 uv = (-iResolution.xy + 2.0 * fragCoord) / iResolution.y;
	float t = time * 0.005;
	uv.x += sin(t) * 0.5;
	float si = sin(t + 2.17); // ...Squiffy rotation matrix!
	float co = cos(t);
	//uv *= mat2(co, si, -si, co);
	uv = mul(uv, mat2(co, si, -si, co));
	vec3 col = /*vec3*/(0.0);
	vec3 init = vec3(0.25, 0.25 + sin(time * 0.001) * 0.4, floor(time) * 0.0008);
	for (int r = 0; r < 100; r++)
	{
		vec3 p = init + s * vec3(uv, 0.143);
		p.z = mod(p.z, 2.0);
		for (int i = 0; i < 10; i++)	p = abs(p * 2.04) / dot(p, p) - 0.75;
		v += length(p * p) * smoothstep(0.0, 0.5, 0.9 - s) * .002;
		// Get a purple and cyan effect by biasing the RGB in different ways...
		col += vec3(v * 0.8, 1.1 - s * 0.5, .7 + v * 0.5) * v * 0.013;
		s += .01;
	}
	fragColor = vec4(col, 1.0);
}

#endif

#if SHADERTOY == 5
#define time iTime

static float ratio = 1;

#define PI2 6.28318530718
#define PI 3.1416

float vorocloud(vec2 p) {
	float f = 0.0;
	vec2 pp = cos(vec2(p.x * 14.0, (16.0 * p.y + cos(floor(p.x * 30.0)) + time * PI2)));
	p = cos(p * 12.1 + pp * 10.0 + 0.5 * cos(pp.x * 10.0));

	vec2 pts[4];

	pts[0] = vec2(0.5, 0.6);
	pts[1] = vec2(-0.4, 0.4);
	pts[2] = vec2(0.2, -0.7);
	pts[3] = vec2(-0.3, -0.4);

	float d = 5.0;

	for (int i = 0; i < 4; i++) {
		pts[i].x += 0.03 * cos(float(i)) + p.x;
		pts[i].y += 0.03 * sin(float(i)) + p.y;
		d = min(d, distance(pts[i], pp));
	}

	f = 2.0 * pow(1.0 - 0.3 * d, 13.0);

	f = min(f, 1.0);

	return f;
}

vec4 scene(vec2 UV) {
	float x = UV.x;
	float y = UV.y;

	vec2 p = vec2(x, y) - /*vec2*/(0.5);

	vec4 col = /*vec4*/(0.0);
	col.g += 0.02;

	float v = vorocloud(p);
	v = 0.2 * floor(v * 5.0);

	col.r += 0.1 * v;
	col.g += 0.6 * v;
	col.b += 0.5 * pow(v, 5.0);


	v = vorocloud(p * 2.0);
	v = 0.2 * floor(v * 5.0);

	col.r += 0.1 * v;
	col.g += 0.2 * v;
	col.b += 0.01 * pow(v, 5.0);

	col.a = 1.0;

	return col;
}


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord.xy / iResolution.xy;
	uv.y = 1.0 - uv.y;
	fragColor = scene(uv);
}
#endif

#if SHADERTOY == 6
//////////////////////////////////
//
//	 GLOWING WAVES
//
//   by Tech_
//
//////////////////////////////////

////////////////////////////////

#define GLOW_AMOUNT 1.3
#define SPEED 0.8

//////////////////////////////////

vec3 getWave(in vec2 uv, float curve, vec3 color)
{
	uv.x += curve * 1.6;
	float wave = pow(1. - distance(uv.x, 0.5), 38.0 / GLOW_AMOUNT);
	return wave * color;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord / iResolution.xy;
	uv /= 2.0;
	uv.x += 0.25;

	float speed = iTime * SPEED;

	vec3 red = vec3(1.0, 0.0, 0.0);
	vec3 blue = vec3(0.0, 0.0, 1.0);
	vec3 yellow = vec3(1.0, 0.8, 0.0);
	vec3 purple = vec3(0.5, 0.0, 1.0);
	vec3 orange = vec3(1.0, 0.5, 0.0);

	// Time varying pixel color
	vec3 col = 0;
	col += getWave(uv, sin((uv.y + speed * 0.05) * 10.) * 0.06, red);
	col += getWave(uv, cos((uv.y + speed * 0.025) * 10.) * 0.12, blue);
	col += getWave(uv, sin((uv.y + speed * -0.007 + cos(uv.y * 0.2)) * 8.) * 0.1, yellow);
	col += getWave(uv, cos((uv.y + speed * 0.035 + sin(uv.y * 0.13)) * 8.) * 0.05, purple);
	col += getWave(uv, sin((uv.y / 2.0 + speed * -0.05) * 10.) * 0.05, orange);

	col *= 0.8;

	// Output to screen
	fragColor = vec4(col, 1.0);
}
#endif

#if SHADERTOY == 7

/*
* "Seascape" by Alexander Alekseev aka TDM - 2014
* License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
* Contact: tdmaav@gmail.com
*/

static const int NUM_STEPS = 8;
static const float PI = 3.141592;
static const float EPSILON = 1e-3;
#define EPSILON_NRM (0.1 / iResolution.x)
#define AA

// sea
static const int ITER_GEOMETRY = 3;
static const int ITER_FRAGMENT = 5;
static const float SEA_HEIGHT = 0.6;
static const float SEA_CHOPPY = 4.0;
static const float SEA_SPEED = 0.8;
static const float SEA_FREQ = 0.16;
static const vec3 SEA_BASE = vec3(0.0, 0.09, 0.18);
static const vec3 SEA_WATER_COLOR = vec3(0.8, 0.9, 0.6)*0.6;
#define SEA_TIME (1.0 + iTime * SEA_SPEED)
static const mat2 octave_m = mat2(1.6, 1.2, -1.2, 1.6);

// math
mat3 fromEuler(vec3 ang) {
	vec2 a1 = vec2(sin(ang.x), cos(ang.x));
	vec2 a2 = vec2(sin(ang.y), cos(ang.y));
	vec2 a3 = vec2(sin(ang.z), cos(ang.z));
	mat3 m;
	m[0] = vec3(a1.y*a3.y + a1.x*a2.x*a3.x, a1.y*a2.x*a3.x + a3.y*a1.x, -a2.y*a3.x);
	m[1] = vec3(-a2.y*a1.x, a1.y*a2.y, a2.x);
	m[2] = vec3(a3.y*a1.x*a2.x + a1.y*a3.x, a1.x*a3.x - a1.y*a3.y*a2.x, a2.y*a3.y);
	return m;
}
float hash(vec2 p) {
	float h = dot(p, vec2(127.1, 311.7));
	return fract(sin(h)*43758.5453123);
}
float noise(in vec2 p) {
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f*(3.0 - 2.0*f);
	return -1.0 + 2.0*mix(mix(hash(i + vec2(0.0, 0.0)),
		hash(i + vec2(1.0, 0.0)), u.x),
		mix(hash(i + vec2(0.0, 1.0)),
			hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

// lighting
float diffuse(vec3 n, vec3 l, float p) {
	return pow(dot(n, l) * 0.4 + 0.6, p);
}
float specular(vec3 n, vec3 l, vec3 e, float s) {
	float nrm = (s + 8.0) / (PI * 8.0);
	return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

// sky
vec3 getSkyColor(vec3 e) {
	e.y = (max(e.y, 0.0)*0.8 + 0.2)*0.8;
	return vec3(pow(1.0 - e.y, 2.0), 1.0 - e.y, 0.6 + (1.0 - e.y)*0.4) * 1.1;
}

// sea
float sea_octave(vec2 uv, float choppy) {
	uv += noise(uv);
	vec2 wv = 1.0 - abs(sin(uv));
	vec2 swv = abs(cos(uv));
	wv = mix(wv, swv, wv);
	return pow(1.0 - pow(wv.x * wv.y, 0.65), choppy);
}

float map(vec3 p) {
	float freq = SEA_FREQ;
	float amp = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2 uv = p.xz; uv.x *= 0.75;

	float d, h = 0.0;
	for (int i = 0; i < ITER_GEOMETRY; i++) {
		d = sea_octave((uv + SEA_TIME)*freq, choppy);
		d += sea_octave((uv - SEA_TIME)*freq, choppy);
		h += d * amp;
		//uv *= octave_m; freq *= 1.9; amp *= 0.22;
		uv = mul(uv, octave_m); freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.y - h;
}

float map_detailed(vec3 p) {
	float freq = SEA_FREQ;
	float amp = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2 uv = p.xz; uv.x *= 0.75;

	float d, h = 0.0;
	for (int i = 0; i < ITER_FRAGMENT; i++) {
		d = sea_octave((uv + SEA_TIME)*freq, choppy);
		d += sea_octave((uv - SEA_TIME)*freq, choppy);
		h += d * amp;
		//uv *= octave_m; freq *= 1.9; amp *= 0.22;
		uv = mul(uv, octave_m); freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.y - h;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {
	float fresnel = clamp(1.0 - dot(n, -eye), 0.0, 1.0);
	fresnel = pow(fresnel, 3.0) * 0.5;

	vec3 reflected = getSkyColor(reflect(eye, n));
	vec3 refracted = SEA_BASE + diffuse(n, l, 80.0) * SEA_WATER_COLOR * 0.12;

	vec3 color = mix(refracted, reflected, fresnel);

	float atten = max(1.0 - dot(dist, dist) * 0.001, 0.0);
	color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;

	color += /*vec3*/(specular(n, l, eye, 60.0));

	return color;
}

// tracing
vec3 getNormal(vec3 p, float eps) {
	vec3 n;
	n.y = map_detailed(p);
	n.x = map_detailed(vec3(p.x + eps, p.y, p.z)) - n.y;
	n.z = map_detailed(vec3(p.x, p.y, p.z + eps)) - n.y;
	n.y = eps;
	return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {
	float tm = 0.0;
	float tx = 1000.0;
	float hx = map(ori + dir * tx);
	if (hx > 0.0) return tx;
	float hm = map(ori + dir * tm);
	float tmid = 0.0;
	for (int i = 0; i < NUM_STEPS; i++) {
		tmid = mix(tm, tx, hm / (hm - hx));
		p = ori + dir * tmid;
		float hmid = map(p);
		if (hmid < 0.0) {
			tx = tmid;
			hx = hmid;
		}
		else {
			tm = tmid;
			hm = hmid;
		}
	}
	return tmid;
}

vec3 getPixel(in vec2 coord, float time) {
	vec2 uv = coord / iResolution.xy;
	uv = uv * 2.0 - 1.0;
	uv.x *= iResolution.x / iResolution.y;

	// ray
	vec3 ang = vec3(sin(time*3.0)*0.1, sin(time)*0.2 + 0.3, time);
	vec3 ori = vec3(0.0, 3.5, time*5.0);
	vec3 dir = normalize(vec3(uv.xy, -2.0)); dir.z += length(uv) * 0.14;
	//dir = normalize(dir) * fromEuler(ang);
	dir = mul(normalize(dir), fromEuler(ang));

	// tracing
	vec3 p;
	heightMapTracing(ori, dir, p);
	vec3 dist = p - ori;
	vec3 n = getNormal(p, dot(dist, dist) * EPSILON_NRM);
	vec3 light = normalize(vec3(0.0, 1.0, 0.8));

	// color
	return mix(
		getSkyColor(dir),
		getSeaColor(p, n, light, dir, dist),
		pow(smoothstep(0.0, -0.02, dir.y), 0.2));
}

// main
void mainImage(out vec4 fragColor, in vec2 fragCoord) {

	fragCoord.y = 1 - fragCoord.y;

	float time = iTime * 0.3 + iMouse.x*0.01;

#ifdef AA
	vec3 color = /*vec3*/(0.0);
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			vec2 uv = fragCoord + vec2(i, j) / 3.0;
			color += getPixel(uv, time);
		}
	}
	color /= 9.0;
#else
	vec3 color = getPixel(fragCoord, time);
#endif

	// post
	fragColor = vec4(pow(color, /*vec3*/(0.65)), 1.0);
}
#endif


#if SHADERTOY == 8
// Antialiasing: number of samples in x and y dimensions
#define AA 2

#define MIN_DIST 0.001
#define MAX_DIST 10.

#define PI 3.1415926
#define TAU 6.2831853


float opIntersection(float d1, float d2) {
	return max(d1, d2);
}

// from https://github.com/doxas/twigl
mat3 rotate3D(float angle, vec3 axis) {
	vec3 a = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float r = 1.0 - c;
	return mat3(
		a.x * a.x * r + c,
		a.y * a.x * r + a.z * s,
		a.z * a.x * r - a.y * s,
		a.x * a.y * r - a.z * s,
		a.y * a.y * r + c,
		a.z * a.y * r + a.x * s,
		a.x * a.z * r + a.y * s,
		a.y * a.z * r - a.x * s,
		a.z * a.z * r + c
	);
}

float sdSphere(vec3 p, float radius) {
	return length(p) - radius;
}

float sdGyroid(vec3 p, float scale, float thickness, float bias) {
	p *= scale;
	return abs(dot(sin(p*.5), cos(p.zxy * 1.23)) - bias) / scale - thickness;
}

vec2 sceneSDF(vec3 p) {
	p = mul(p, rotate3D(iTime * .2, vec3(0, 1, 0)));

	float gyroid = sdGyroid(p, 10., .01, 0.) * .55;
	float d = opIntersection(sdSphere(p, 1.5), gyroid);

	return vec2(d, 1.);
}

// Compute camera-to-world transformation.
mat3 setCamera(in vec3 ro, in vec3 ta, float cr) {
	vec3 cw = normalize(ta - ro);
	vec3 cp = vec3(sin(cr), cos(cr), 0.0);
	vec3 cu = normalize(cross(cw, cp));
	vec3 cv = normalize(cross(cu, cw));
	return mat3(cu, cv, cw);
}

// Cast a ray from origin ro in direction rd until it hits an object.
// Return (t,m) where t is distance traveled along the ray, and m
// is the material of the object hit.
vec2 castRay(in vec3 ro, in vec3 rd) {
	float tmin = MIN_DIST;
	float tmax = MAX_DIST;

#if 0
	// bounding volume
	float tp1 = (0.0 - ro.y) / rd.y;
	if (tp1 > 0.0) tmax = min(tmax, tp1);
	float tp2 = (1.6 - ro.y) / rd.y;
	if (tp2 > 0.0) {
		if (ro.y > 1.6) tmin = max(tmin, tp2);
		else tmax = min(tmax, tp2);
	}
#endif

	float t = tmin;
	float m = -1.0;
	for (int i = 0; i < 100; i++) {
		float precis = 0.0005 * t;
		vec2 res = sceneSDF(ro + rd * t);
		if (res.x < precis || t > tmax) break;
		t += res.x;
		m = res.y;
	}

	if (t > tmax) m = -1.0;
	return vec2(t, m);
}

// Compute normal vector to surface at pos, using central differences method?
vec3 calcNormal(in vec3 pos) {
	// epsilon = a small number
	vec2 e = vec2(1.0, -1.0) * 0.5773 * 0.0005;

	return normalize(
		e.xyy * sceneSDF(pos + e.xyy).x +
		e.yyx * sceneSDF(pos + e.yyx).x +
		e.yxy * sceneSDF(pos + e.yxy).x +
		e.xxx * sceneSDF(pos + e.xxx).x
	);
}

vec3 computeColor(vec3 ro, vec3 rd, vec3 pos, float d, float m) {
	vec3 nor = calcNormal(pos);
	return nor;
}

// Figure out color value when casting ray from origin ro in direction rd.
vec3 render(in vec3 ro, in vec3 rd) {
	// cast ray to nearest object
	vec2 res = castRay(ro, rd);
	float distance = res.x; // distance
	float materialID = res.y; // material ID

	vec3 col = vec3(245, 215, 161) / 255.;
	if (materialID > 0.0) {
		vec3 pos = ro + distance * rd;
		col = computeColor(ro, rd, pos, distance, materialID);
	}
	return vec3(clamp(col, 0.0, 1.0));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	// Ray Origin)\t
	vec3 ro = /*vec3*/(2.3);
	vec3 ta = /*vec3*/(0.0);
	// camera-to-world transformation
	mat3 ca = setCamera(ro, ta, 0.0);

	vec3 color = /*vec3*/(0.0);

#if AA>1
	for (int m = 0; m < AA; m++)
		for (int n = 0; n < AA; n++) {
			// pixel coordinates
			vec2 o = vec2(float(m), float(n)) / float(AA) - 0.5;
			vec2 p = (-iResolution.xy + 2.0 * (fragCoord.xy + o)) / iResolution.y;
#else
	vec2 p = (-iResolution.xy + 2.0 * fragCoord.xy) / iResolution.y;
#endif

	// ray direction
	vec3 rd = mul(ca, normalize(vec3(p.xy, 2.0)));

	// render\t
	vec3 col = render(ro, rd);

	color += col;
#if AA>1
		}
color /= float(AA*AA);
#endif


fragColor = vec4(color, 1.0);
}
#endif

#if SHADERTOY == 9
//Sci-fi radar based on the work of gmunk for Oblivion
//http://work.gmunk.com/OBLIVION-GFX

#define SMOOTH(r,R) (1.0-smoothstep(R-1.0,R+1.0, r))
#define RANGE(a,b,x) ( step(a,x)*(1.0-step(b,x)) )
#define RS(a,b,x) ( smoothstep(a-1.0,a+1.0,x)*(1.0-smoothstep(b-1.0,b+1.0,x)) )
#define M_PI 3.1415926535897932384626433832795

#define blue1 vec3(0.74,0.95,1.00)
#define blue2 vec3(0.87,0.98,1.00)
#define blue3 vec3(0.35,0.76,0.83)
#define blue4 vec3(0.953,0.969,0.89)
#define red   vec3(1.00,0.38,0.227)

#define MOV(a,b,c,d,t) (vec2(a*cos(t)+b*cos(0.1*(t)), c*sin(t)+d*cos(0.1*(t))))

float movingLine(vec2 uv, vec2 center, float radius)
{
	//angle of the line
	float theta0 = 90.0 * iTime;
	vec2 d = uv - center;
	float r = sqrt(dot(d, d));
	if (r < radius)
	{
		//compute the distance to the line theta=theta0
		vec2 p = radius * vec2(cos(theta0*M_PI / 180.0),
			-sin(theta0*M_PI / 180.0));
		float l = length(d - p * clamp(dot(d, p) / dot(p, p), 0.0, 1.0));
		d = normalize(d);
		//compute gradient based on angle difference to theta0
		float theta = mod(180.0*atan(d.y, d.x) / M_PI + theta0, 360.0);
		float gradient = clamp(1.0 - theta / 90.0, 0.0, 1.0);
		return SMOOTH(l, 1.0) + 0.5*gradient;
	}
	else return 0.0;
}

float circle(vec2 uv, vec2 center, float radius, float width)
{
	float r = length(uv - center);
	return SMOOTH(r - width / 2.0, radius) - SMOOTH(r + width / 2.0, radius);
}

float circle2(vec2 uv, vec2 center, float radius, float width, float opening)
{
	vec2 d = uv - center;
	float r = sqrt(dot(d, d));
	d = normalize(d);
	if (abs(d.y) > opening)
		return SMOOTH(r - width / 2.0, radius) - SMOOTH(r + width / 2.0, radius);
	else
		return 0.0;
}
float circle3(vec2 uv, vec2 center, float radius, float width)
{
	vec2 d = uv - center;
	float r = sqrt(dot(d, d));
	d = normalize(d);
	float theta = 180.0*(atan(d.y, d.x) / M_PI);
	return smoothstep(2.0, 2.1, abs(mod(theta + 2.0, 45.0) - 2.0)) *
		mix(0.5, 1.0, step(45.0, abs(mod(theta, 180.0) - 90.0))) *
		(SMOOTH(r - width / 2.0, radius) - SMOOTH(r + width / 2.0, radius));
}

float triangles(vec2 uv, vec2 center, float radius)
{
	vec2 d = uv - center;
	return RS(-8.0, 0.0, d.x - radius) * (1.0 - smoothstep(7.0 + d.x - radius, 9.0 + d.x - radius, abs(d.y)))
		+ RS(0.0, 8.0, d.x + radius) * (1.0 - smoothstep(7.0 - d.x - radius, 9.0 - d.x - radius, abs(d.y)))
		+ RS(-8.0, 0.0, d.y - radius) * (1.0 - smoothstep(7.0 + d.y - radius, 9.0 + d.y - radius, abs(d.x)))
		+ RS(0.0, 8.0, d.y + radius) * (1.0 - smoothstep(7.0 - d.y - radius, 9.0 - d.y - radius, abs(d.x)));
}

float _cross(vec2 uv, vec2 center, float radius)
{
	vec2 d = uv - center;
	int x = int(d.x);
	int y = int(d.y);
	float r = sqrt(dot(d, d));
	if ((r < radius) && ((x == y) || (x == -y)))
		return 1.0;
	else return 0.0;
}
float dots(vec2 uv, vec2 center, float radius)
{
	vec2 d = uv - center;
	float r = sqrt(dot(d, d));
	if (r <= 2.5)
		return 1.0;
	if ((r <= radius) && ((abs(d.y + 0.5) <= 1.0) && (mod(d.x + 1.0, 50.0) < 2.0)))
		return 1.0;
	else if ((abs(d.y + 0.5) <= 1.0) && (r >= 50.0) && (r < 115.0))
		return 0.5;
	else
		return 0.0;
}
float bip1(vec2 uv, vec2 center)
{
	return SMOOTH(length(uv - center), 3.0);
}
float bip2(vec2 uv, vec2 center)
{
	float r = length(uv - center);
	float R = 8.0 + mod(87.0*iTime, 80.0);
	return (0.5 - 0.5*cos(30.0*iTime)) * SMOOTH(r, 5.0)
		+ SMOOTH(6.0, r) - SMOOTH(8.0, r)
		+ smoothstep(max(8.0, R - 20.0), R, r) - SMOOTH(R, r);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec3 finalColor;
	vec2 uv = fragCoord.xy;
	//center of the image
	vec2 c = iResolution.xy / 2.0;
	finalColor = /*vec3*/(0.3*_cross(uv, c, 240.0));
	finalColor += (circle(uv, c, 100.0, 1.0)
		+ circle(uv, c, 165.0, 1.0)) * blue1;
	finalColor += (circle(uv, c, 240.0, 2.0));//+ dots(uv,c,240.0)) * blue4;
	finalColor += circle3(uv, c, 313.0, 4.0) * blue1;
	finalColor += triangles(uv, c, 315.0 + 30.0*sin(iTime)) * blue2;
	finalColor += movingLine(uv, c, 240.0) * blue3;
	finalColor += circle(uv, c, 10.0, 1.0) * blue3;
	finalColor += 0.7 * circle2(uv, c, 262.0, 1.0, 0.5 + 0.2*cos(iTime)) * blue3;
	if (length(uv - c) < 240.0)
	{
		//animate some bips with random movements
		vec2 p = 130.0*MOV(1.3, 1.0, 1.0, 1.4, 3.0 + 0.1*iTime);
		finalColor += bip1(uv, c + p) * vec3(1, 1, 1);
		p = 130.0*MOV(0.9, -1.1, 1.7, 0.8, -2.0 + sin(0.1*iTime) + 0.15*iTime);
		finalColor += bip1(uv, c + p) * vec3(1, 1, 1);
		p = 50.0*MOV(1.54, 1.7, 1.37, 1.8, sin(0.1*iTime + 7.0) + 0.2*iTime);
		finalColor += bip2(uv, c + p) * red;
	}

	fragColor = vec4(finalColor, 1.0);
}
#endif

#if SHADERTOY == 10
// Star Nest by Pablo Roman Andrioli

// This content is under the MIT License.

#define iterations 17
#define formuparam 0.53

#define volsteps 20
#define stepsize 0.1

#define zoom   0.800
#define tile   0.850
#define speed  0.010 

#define brightness 0.0015
#define darkmatter 0.300
#define distfading 0.730
#define saturation 0.850


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	//get coords and direction
	vec2 uv = fragCoord.xy / iResolution.xy - .5;
	uv.y *= iResolution.y / iResolution.x;
	vec3 dir = vec3(uv*zoom, 1.);
	float time = iTime * speed + .25;

	//mouse rotation
	float a1 = .5 + iMouse.x / iResolution.x*2.;
	float a2 = .8 + iMouse.y / iResolution.y*2.;
	mat2 rot1 = mat2(cos(a1), sin(a1), -sin(a1), cos(a1));
	mat2 rot2 = mat2(cos(a2), sin(a2), -sin(a2), cos(a2));
	dir.xz = mul(dir.xz, rot1);
	dir.xy = mul(dir.xy, rot2);
	vec3 from = vec3(1., .5, 0.5);
	from += vec3(time*2., time, -2.);
	from.xz = mul(from.xz, rot1);
	from.xy = mul(from.xy, rot2);

	//volumetric rendering
	float s = 0.1, fade = 1.;
	vec3 v = /*vec3*/(0.);
	for (int r = 0; r < volsteps; r++) {
		vec3 p = from + s * dir*.5;
		p = abs(/*vec3*/(tile)-mod(p, /*vec3*/(tile*2.))); // tiling fold
		float pa, a = pa = 0.;
		for (int i = 0; i < iterations; i++) {
			p = abs(p) / dot(p, p) - formuparam; // the magic formula
			a += abs(length(p) - pa); // absolute sum of average change
			pa = length(p);
		}
		float dm = max(0., darkmatter - a * a*.001); //dark matter
		a *= a * a; // add contrast
		if (r > 6) fade *= 1. - dm; // dark matter, don't render near
								  //v+=vec3(dm,dm*.5,0.);
		v += fade;
		v += vec3(s, s*s, s*s*s*s)*a*brightness*fade; // coloring based on distance
		fade *= distfading; // distance fading
		s += stepsize;
	}
	v = mix(/*vec3*/(length(v)), v, saturation); //color adjust
	fragColor = vec4(v*.01, 1.);

}
#endif

#if SHADERTOY == 11

//CBS
//Parallax scrolling fractal galaxy.
//Inspired by JoshP's Simplicity shader: https://www.shadertoy.com/view/lslGWr

// http://www.fractalforums.com/new-theories-and-research/very-simple-formula-for-fractal-patterns/
float field(in vec3 p, float s) {
	float strength = 7. + .03 * log(1.e-6 + fract(sin(iTime) * 4373.11));
	float accum = s / 4.;
	float prev = 0.;
	float tw = 0.;
	for (int i = 0; i < 26; ++i) {
		float mag = dot(p, p);
		p = abs(p) / mag + vec3(-.5, -.4, -1.5);
		float w = exp(-float(i) / 7.);
		accum += w * exp(-strength * pow(abs(mag - prev), 2.2));
		tw += w;
		prev = mag;
	}
	return max(0., 5. * accum / tw - .7);
}

// Less iterations for second layer
float field2(in vec3 p, float s) {
	float strength = 7. + .03 * log(1.e-6 + fract(sin(iTime) * 4373.11));
	float accum = s / 4.;
	float prev = 0.;
	float tw = 0.;
	for (int i = 0; i < 18; ++i) {
		float mag = dot(p, p);
		p = abs(p) / mag + vec3(-.5, -.4, -1.5);
		float w = exp(-float(i) / 7.);
		accum += w * exp(-strength * pow(abs(mag - prev), 2.2));
		tw += w;
		prev = mag;
	}
	return max(0., 5. * accum / tw - .7);
}

vec3 nrand3(vec2 co)
{
	vec3 a = fract(cos(co.x*8.3e-3 + co.y)*vec3(1.3e5, 4.7e5, 2.9e5));
	vec3 b = fract(sin(co.x*0.3e-3 + co.y)*vec3(8.1e5, 1.0e5, 0.1e5));
	vec3 c = mix(a, b, 0.5);
	return c;
}


void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	vec2 uv = 2. * fragCoord.xy / iResolution.xy - 1.;
	vec2 uvs = uv * iResolution.xy / max(iResolution.x, iResolution.y);
	vec3 p = vec3(uvs / 4., 0) + vec3(1., -1.3, 0.);
	p += .2 * vec3(sin(iTime / 16.), sin(iTime / 12.), sin(iTime / 128.));

	float freqs[4];
	//Sound
	freqs[0] = texture(iChannel0, vec2(0.01, 0.25)).x;
	freqs[1] = texture(iChannel0, vec2(0.07, 0.25)).x;
	freqs[2] = texture(iChannel0, vec2(0.15, 0.25)).x;
	freqs[3] = texture(iChannel0, vec2(0.30, 0.25)).x;

	float t = field(p, freqs[2]);
	float v = (1. - exp((abs(uv.x) - 1.) * 6.)) * (1. - exp((abs(uv.y) - 1.) * 6.));

	//Second Layer
	vec3 p2 = vec3(uvs / (4. + sin(iTime*0.11)*0.2 + 0.2 + sin(iTime*0.15)*0.3 + 0.4), 1.5) + vec3(2., -1.3, -1.);
	p2 += 0.25 * vec3(sin(iTime / 16.), sin(iTime / 12.), sin(iTime / 128.));
	float t2 = field2(p2, freqs[3]);
	vec4 c2 = mix(.4, 1., v) * vec4(1.3 * t2 * t2 * t2, 1.8  * t2 * t2, t2* freqs[0], t2);


	//Let's add some stars
	//Thanks to http://glsl.heroku.com/e#6904.0
	vec2 seed = p.xy * 2.0;
	seed = floor(seed * iResolution.x);
	vec3 rnd = nrand3(seed);
	vec4 starcolor = /*vec4*/(pow(rnd.y, 40.0));

	//Second Layer
	vec2 seed2 = p2.xy * 2.0;
	seed2 = floor(seed2 * iResolution.x);
	vec3 rnd2 = nrand3(seed2);
	starcolor += /*vec4*/(pow(rnd2.y, 40.0));

	fragColor = mix(freqs[3] - .3, 1., v) * vec4(1.5*freqs[2] * t * t* t, 1.2*freqs[1] * t * t, freqs[3] * t, 1.0) + c2 + starcolor;
}
#endif

#if SHADERTOY == 12
// Xyptonjtroz by nimitz (twitter: @stormoid)
// https://www.shadertoy.com/view/4ts3z2
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// Contact the author for other licensing options

//Audio by Dave_Hoskins

#define ITR 100
#define FAR 30.
#define time iTime

/*
Believable animated volumetric dust storm in 7 samples,
blending each layer in based on geometry distance allows to
render it without visible seams. 3d Triangle noise is
used for the dust volume.

Also included is procedural bump mapping and glow based on
curvature*fresnel. (see: https://www.shadertoy.com/view/Xts3WM)


Further explanation of the dust generation (per Dave's request):

The basic idea is to have layers of gradient shaded volumetric
animated noise. The problem is when geometry is intersected
before the ray reaches the far plane. A way to smoothly blend
the low sampled noise is needed.  So I am blending (smoothstep)
each dust layer based on current ray distance and the solid
interesction distance. I am also scaling the noise taps	as a
function of the current distance so that the distant dust doesn't
appear too noisy and as a function of current height to get some
"ground hugging" effect.

*/

mat2 mm2(in float a) { float c = cos(a), s = sin(a); return mat2(c, s, -s, c); }

float height(in vec2 p)
{
	p *= 0.2;
	return sin(p.y)*0.4 + sin(p.x)*0.4;
}

//smooth min (http://iquilezles.org/www/articles/smin/smin.htm)
float smin(float a, float b)
{
	float h = clamp(0.5 + 0.5*(b - a) / 0.7, 0.0, 1.0);
	return mix(b, a, h) - 0.7*h*(1.0 - h);
}


vec2 nmzHash22(vec2 q)
{
	uvec2 p = uvec2(ivec2(q));
	p = p * uvec2(3266489917U, 668265263U) + p.yx;
	p = p * (p.yx ^ (p >> 15U));
	return vec2(p ^ (p >> 16U))*(1.0 / /*vec2*/(0xffffffffU));
}

float vine(vec3 p, in float c, in float h)
{
	p.y += sin(p.z*0.2625)*2.5;
	p.x += cos(p.z*0.1575)*3.;
	vec2 q = vec2(mod(p.x, c) - c / 2., p.y);
	return length(q) - h - sin(p.z*2. + sin(p.x*7.)*0.5 + time * 0.5)*0.13;
}

float map(vec3 p)
{
	p.y += height(p.zx);

	vec3 bp = p;
	vec2 hs = nmzHash22(floor(p.zx / 4.));
	p.zx = mod(p.zx, 4.) - 2.;

	float d = p.y + 0.5;
	p.y -= hs.x*0.4 - 0.15;
	p.zx += hs * 1.3;
	d = smin(d, length(p) - hs.x*0.4);

	d = smin(d, vine(bp + vec3(1.8, 0., 0), 15., .8));
	d = smin(d, vine(bp.zyx + vec3(0., 0, 17.), 20., 0.75));

	return d * 1.1;
}

float march(in vec3 ro, in vec3 rd)
{
	float precis = 0.002;
	float h = precis * 2.0;
	float d = 0.;
	for (int i = 0; i < ITR; i++)
	{
		if (abs(h) < precis || d > FAR) break;
		d += h;
		float res = map(ro + rd * d);
		h = res;
	}
	return d;
}

float tri(in float x) { return abs(fract(x) - .5); }
vec3 tri3(in vec3 p) { return vec3(tri(p.z + tri(p.y*1.)), tri(p.z + tri(p.x*1.)), tri(p.y + tri(p.x*1.))); }

static mat2 m2 = mat2(0.970, 0.242, -0.242, 0.970);

float triNoise3d(in vec3 p, in float spd)
{
	float z = 1.4;
	float rz = 0.;
	vec3 bp = p;
	for (float i = 0.; i <= 3.; i++)
	{
		vec3 dg = tri3(bp*2.);
		p += (dg + time * spd);

		bp *= 1.8;
		z *= 1.5;
		p *= 1.2;
		//p.xz*= m2;

		rz += (tri(p.z + tri(p.x + tri(p.y)))) / z;
		bp += 0.14;
	}
	return rz;
}

float fogmap(in vec3 p, in float d)
{
	p.x += time * 1.5;
	p.z += sin(p.x*.5);
	return triNoise3d(p*2.2 / (d + 20.), 0.2)*(1. - smoothstep(0., .7, p.y));
}

vec3 fog(in vec3 col, in vec3 ro, in vec3 rd, in float mt)
{
	float d = .5;
	for (int i = 0; i < 7; i++)
	{
		vec3  pos = ro + rd * d;
		float rz = fogmap(pos, d);
		float grd = clamp((rz - fogmap(pos + .8 - float(i)*0.1, d))*3., 0.1, 1.);
		vec3 col2 = (vec3(.1, 0.8, .5)*.5 + .5*vec3(.5, .8, 1.)*(1.7 - grd))*0.55;
		col = mix(col, col2, clamp(rz*smoothstep(d - 0.4, d + 2. + d * .75, mt), 0., 1.));
		d *= 1.5 + 0.3;
		if (d > mt)break;
	}
	return col;
}

vec3 normal(in vec3 p)
{
	vec2 e = vec2(-1., 1.)*0.005;
	return normalize(e.yxx*map(p + e.yxx) + e.xxy*map(p + e.xxy) +
		e.xyx*map(p + e.xyx) + e.yyy*map(p + e.yyy));
}

float bnoise(in vec3 p)
{
	float n = sin(triNoise3d(p*.3, 0.0)*11.)*0.6 + 0.4;
	n += sin(triNoise3d(p*1., 0.05)*40.)*0.1 + 0.9;
	return (n*n)*0.003;
}

vec3 bump(in vec3 p, in vec3 n, in float ds)
{
	vec2 e = vec2(.005, 0);
	float n0 = bnoise(p);
	vec3 d = vec3(bnoise(p + e.xyy) - n0, bnoise(p + e.yxy) - n0, bnoise(p + e.yyx) - n0) / e.x;
	n = normalize(n - d * 2.5 / sqrt(ds));
	return n;
}

float shadow(in vec3 ro, in vec3 rd, in float mint, in float tmax)
{
	float res = 1.0;
	float t = mint;
	for (int i = 0; i < 10; i++)
	{
		float h = map(ro + rd * t);
		res = min(res, 4.*h / t);
		t += clamp(h, 0.05, .5);
		if (h<0.001 || t>tmax) break;
	}
	return clamp(res, 0.0, 1.0);

}

float curv(in vec3 p, in float w)
{
	vec2 e = vec2(-1., 1.)*w;

	float t1 = map(p + e.yxx), t2 = map(p + e.xxy);
	float t3 = map(p + e.xyx), t4 = map(p + e.yyy);

	return .125 / (e.x*e.x) *(t1 + t2 + t3 + t4 - 4. * map(p));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 p = fragCoord.xy / iResolution.xy - 0.5;
	vec2 q = fragCoord.xy / iResolution.xy;
	p.x *= iResolution.x / iResolution.y;
	vec2 mo = iMouse.xy / iResolution.xy - .5;
	mo = (mo == /*vec2*/(-.5)) ? mo = vec2(-0.1, 0.07) : mo;
	mo.x *= iResolution.x / iResolution.y;

	vec3 ro = vec3(smoothstep(0., 1., tri(time*.45)*2.)*0.1, smoothstep(0., 1., tri(time*0.9)*2.)*0.07, -time * 0.6);
	ro.y -= height(ro.zx) + 0.05;
	mo.x += smoothstep(0.6, 1., sin(time*.6)*0.5 + 0.5) - 1.5;
	vec3 eyedir = normalize(vec3(cos(mo.x), mo.y*2. - 0.2 + sin(time*0.45*1.57)*0.1, sin(mo.x)));
	vec3 rightdir = normalize(vec3(cos(mo.x + 1.5708), 0., sin(mo.x + 1.5708)));
	vec3 updir = normalize(cross(rightdir, eyedir));
	vec3 rd = normalize((p.x*rightdir + p.y*updir)*1. + eyedir);

	vec3 ligt = normalize(vec3(.5, .05, -.2));
	vec3 ligt2 = normalize(vec3(.5, -.1, -.2));

	float rz = march(ro, rd);

	vec3 fogb = mix(vec3(.7, .8, .8)*0.3, vec3(1., 1., .77)*.95, pow(dot(rd, ligt2) + 1.2, 2.5)*.25);
	fogb *= clamp(rd.y*.5 + .6, 0., 1.);
	vec3 col = fogb;

	if (rz < FAR)
	{
		vec3 pos = ro + rz * rd;
		vec3 nor = normal(pos);
		float d = distance(pos, ro);
		nor = bump(pos, nor, d);
		float crv = clamp(curv(pos, .4), .0, 10.);
		float shd = shadow(pos, ligt, 0.1, 3.);
		float dif = clamp(dot(nor, ligt), 0.0, 1.0)*shd;
		float spe = pow(clamp(dot(reflect(rd, nor), ligt), 0.0, 1.0), 50.)*shd;
		float fre = pow(clamp(1.0 + dot(nor, rd), 0.0, 1.0), 1.5);
		vec3 brdf = vec3(0.10, 0.11, 0.13);
		brdf += 1.5*dif*vec3(1.00, 0.90, 0.7);
		col = mix(vec3(0.1, 0.2, 1), vec3(.3, .5, 1), pos.y*.5)*0.2 + .1;
		col *= (sin(bnoise(pos)*900.)*0.2 + 0.8);
		col = col * brdf + col * spe*.5 + fre * vec3(.7, 1., 0.2)*.3*crv;
	}

	//ordinary distance fog first
	col = mix(col, fogb, smoothstep(FAR - 7., FAR, rz));

	//then volumetric fog
	col = fog(col, ro, rd, rz);

	//post
	col = pow(col, /*vec3*/(0.8));
	col *= 1. - smoothstep(0.1, 2., length(p));

	fragColor = vec4(col, 1.0);
}
#endif

#if SHADERTOY == 13
// Plasma Globe by nimitz (twitter: @stormoid)
// https://www.shadertoy.com/view/XsjXRm
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// Contact the author for other licensing options

//looks best with around 25 rays
#define NUM_RAYS 13.

#define VOLUMETRIC_STEPS 19

#define MAX_ITER 35
#define FAR 6.

#define time iTime*1.1


mat2 mm2(in float a) { float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }
float noise(in float x) { return textureLod(iChannel0, vec2(x*.01, 1.), 0.0).x; }

float hash(float n) { return fract(sin(n)*43758.5453); }

float noise(in vec3 p)
{
	vec3 ip = floor(p);
	vec3 fp = fract(p);
	fp = fp * fp*(3.0 - 2.0*fp);

	vec2 tap = (ip.xy + vec2(37.0, 17.0)*ip.z) + fp.xy;
	vec2 rg = textureLod(iChannel0, (tap + 0.5) / 256.0, 0.0).yx;
	return mix(rg.x, rg.y, fp.z);
}

mat3 m3 = mat3(0.00, 0.80, 0.60,
	-0.80, 0.36, -0.48,
	-0.60, -0.48, 0.64);


//See: https://www.shadertoy.com/view/XdfXRj
float flow(in vec3 p, in float t)
{
	float z = 2.;
	float rz = 0.;
	vec3 bp = p;
	for (float i = 1.; i < 5.; i++)
	{
		p += time * .1;
		rz += (sin(noise(p + t * 0.8)*6.)*0.5 + 0.5) / z;
		p = mix(bp, p, 0.6);
		z *= 2.;
		p *= 2.01;
		p = mul(p, m3);
	}
	return rz;
}

//could be improved
float sins(in float x)
{
	float rz = 0.;
	float z = 2.;
	for (float i = 0.; i < 3.; i++)
	{
		rz += abs(fract(x*1.4) - 0.5) / z;
		x *= 1.3;
		z *= 1.15;
		x -= time * .65*z;
	}
	return rz;
}

float segm(vec3 p, vec3 a, vec3 b)
{
	vec3 pa = p - a;
	vec3 ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.);
	return length(pa - ba * h)*.5;
}

vec3 path(in float i, in float d)
{
	vec3 en = vec3(0., 0., 1.);
	float sns2 = sins(d + i * 0.5)*0.22;
	float sns = sins(d + i * .6)*0.21;
	en.xz = mul(en.xz, mm2((hash(i*10.569) - .5)*6.2 + sns2));
	en.xy = mul(en.xy, mm2((hash(i*4.732) - .5)*6.2 + sns));
	return en;
}

vec2 map(vec3 p, float i)
{
	float lp = length(p);
	vec3 bg = /*vec3*/(0.);
	vec3 en = path(i, lp);

	float ins = smoothstep(0.11, .46, lp);
	float outs = .15 + smoothstep(.0, .15, abs(lp - 1.));
	p *= ins * outs;
	float id = ins * outs;

	float rz = segm(p, bg, en) - 0.011;
	return vec2(rz, id);
}

float march(in vec3 ro, in vec3 rd, in float startf, in float maxd, in float j)
{
	float precis = 0.001;
	float h = 0.5;
	float d = startf;
	for (int i = 0; i < MAX_ITER; i++)
	{
		if (abs(h) < precis || d > maxd) break;
		d += h * 1.2;
		float res = map(ro + rd * d, j).x;
		h = res;
	}
	return d;
}

//volumetric marching
vec3 vmarch(in vec3 ro, in vec3 rd, in float j, in vec3 orig)
{
	vec3 p = ro;
	vec2 r = /*vec2*/(0.);
	vec3 sum = /*vec3*/(0);
	float w = 0.;
	for (int i = 0; i < VOLUMETRIC_STEPS; i++)
	{
		r = map(p, j);
		p += rd * .03;
		float lp = length(p);

		vec3 col = sin(vec3(1.05, 2.5, 1.52)*3.94 + r.y)*.85 + 0.4;
		col.rgb *= smoothstep(.0, .015, -r.x);
		col *= smoothstep(0.04, .2, abs(lp - 1.1));
		col *= smoothstep(0.1, .34, lp);
		sum += abs(col)*5. * (1.2 - noise(lp*2. + j * 13. + time * 5.)*1.1) / (log(distance(p, orig) - 2.) + .75);
	}
	return sum;
}

//returns both collision dists of unit sphere
vec2 iSphere2(in vec3 ro, in vec3 rd)
{
	vec3 oc = ro;
	float b = dot(oc, rd);
	float c = dot(oc, oc) - 1.;
	float h = b * b - c;
	if (h < 0.0) return /*vec2*/(-1.);
	else return vec2((-b - sqrt(h)), (-b + sqrt(h)));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 p = fragCoord.xy / iResolution.xy - 0.5;
	p.x *= iResolution.x / iResolution.y;
	vec2 um = iMouse.xy / iResolution.xy - .5;

	//camera
	vec3 ro = vec3(0., 0., 5.);
	vec3 rd = normalize(vec3(p*.7, -1.5));
	mat2 mx = mm2(time*.4 + um.x*6.);
	mat2 my = mm2(time*0.3 + um.y*6.);
	ro.xz = mul(ro.xz, mx); rd.xz = mul(rd.xz, mx);
	ro.xy = mul(ro.xy, my); rd.xy = mul(rd.xy, my);

	vec3 bro = ro;
	vec3 brd = rd;

	vec3 col = vec3(0.0125, 0., 0.025);
#if 1
	for (float j = 1.; j < NUM_RAYS + 1.; j++)
	{
		ro = bro;
		rd = brd;
		mat2 mm = mm2((time*0.1 + ((j + 1.)*5.1))*j*0.25);
		ro.xy = mul(ro.xy, mm); rd.xy = mul(rd.xy, mm);
		ro.xz = mul(ro.xz, mm); rd.xz = mul(rd.xz, mm);
		float rz = march(ro, rd, 2.5, FAR, j);
		if (rz >= FAR)continue;
		vec3 pos = ro + rz * rd;
		col = max(col, vmarch(pos, rd, j, bro));
	}
#endif

	ro = bro;
	rd = brd;
	vec2 sph = iSphere2(ro, rd);

	if (sph.x > 0.)
	{
		vec3 pos = ro + rd * sph.x;
		vec3 pos2 = ro + rd * sph.y;
		vec3 rf = reflect(rd, pos);
		vec3 rf2 = reflect(rd, pos2);
		float nz = (-log(abs(flow(rf*1.2, time) - .01)));
		float nz2 = (-log(abs(flow(rf2*1.2, -time) - .01)));
		col += (0.1*nz*nz* vec3(0.12, 0.12, .5) + 0.05*nz2*nz2*vec3(0.55, 0.2, .55))*0.8;
	}

	fragColor = vec4(col*1.3, 1.0);
}
#endif

#if SHADERTOY == 14
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// This is the code for this video from 2009: https://www.youtube.com/watch?v=iWr5kSZQ7jk


vec2 isphere(in vec4 sph, in vec3 ro, in vec3 rd)
{
	vec3 oc = ro - sph.xyz;

	float b = dot(oc, rd);
	float c = dot(oc, oc) - sph.w*sph.w;
	float h = b * b - c;
	if (h < 0.0) return /*vec2*/(-1.0);
	h = sqrt(h);
	return -b + vec2(-h, h);
}

float map(in vec3 p, in vec3 c, out vec4 resColor)
{
	vec3 z = p;
	float m = dot(z, z);

	vec4 trap = vec4(abs(z), m);
	float dz = 1.0;


	for (int i = 0; i < 4; i++)
	{
		dz = 8.0*pow(m, 3.5)*dz;

#if 0
		float x1 = z.x; float x2 = x1 * x1; float x4 = x2 * x2;
		float y1 = z.y; float y2 = y1 * y1; float y4 = y2 * y2;
		float z1 = z.z; float z2 = z1 * z1; float z4 = z2 * z2;

		float k3 = x2 + z2;
		float k2 = inversesqrt(k3*k3*k3*k3*k3*k3*k3);
		float k1 = x4 + y4 + z4 - 6.0*y2*z2 - 6.0*x2*y2 + 2.0*z2*x2;
		float k4 = x2 - y2 + z2;

		z.x = c.x + 64.0*x1*y1*z1*(x2 - z2)*k4*(x4 - 6.0*x2*z2 + z4)*k1*k2;
		z.y = c.y + -16.0*y2*k3*k4*k4 + k1 * k1;
		z.z = c.z + -8.0*y1*k4*(x4*x4 - 28.0*x4*x2*z2 + 70.0*x4*z4 - 28.0*x2*z2*z4 + z4 * z4)*k1*k2;
#else

		float r = length(z);
		float b = 8.0*acos(clamp(z.y / r, -1.0, 1.0));
		float a = 8.0*atan(z.x, z.z);
		z = c + pow(r, 8.0) * vec3(sin(b)*sin(a), cos(b), sin(b)*cos(a));
#endif        

		trap = min(trap, vec4(abs(z), m));

		m = dot(z, z);
		if (m > 2.0)
			break;
	}

	resColor = trap;

	return 0.25*log(m)*sqrt(m) / dz;
}

float intersect(in vec3 ro, in vec3 rd, out vec4 rescol, float fov, vec3 c)
{
	float res = -1.0;

	// bounding volume
	vec2 dis = isphere(vec4(0.0, 0.0, 0.0, 1.25), ro, rd);
	if (dis.y < 0.0)
		return -1.0;
	dis.x = max(dis.x, 0.0);

	vec4 trap;

	// raymarch
	float fovfactor = 1.0 / sqrt(1.0 + fov * fov);
	float t = dis.x;
	for (int i = 0; i < 128; i++)
	{
		vec3 p = ro + rd * t;

		float surface = clamp(0.0015*t*fovfactor, 0.0001, 0.1);

		float dt = map(p, c, trap);
		if (t > dis.y || dt < surface) break;

		t += dt;
	}


	if (t < dis.y)
	{
		rescol = trap;
		res = t;
	}

	return res;
}

float softshadow(in vec3 ro, in vec3 rd, float mint, float k, vec3 c)
{
	float res = 1.0;
	float t = mint;
	for (int i = 0; i < 80; i++)
	{
		vec4 kk;
		float h = map(ro + rd * t, c, kk);
		res = min(res, k*h / t);
		if (res < 0.001) break;
		t += clamp(h, 0.002, 0.1);
	}
	return clamp(res, 0.0, 1.0);
}

vec3 calcNormal(in vec3 pos, in float t, in float fovfactor, vec3 c)
{
	vec4 tmp;
	float surface = clamp(0.3 * 0.0015*t*fovfactor, 0.0001, 0.1);
	vec2 eps = vec2(surface, 0.0);
	return normalize(vec3(
		map(pos + eps.xyy, c, tmp) - map(pos - eps.xyy, c, tmp),
		map(pos + eps.yxy, c, tmp) - map(pos - eps.yxy, c, tmp),
		map(pos + eps.yyx, c, tmp) - map(pos - eps.yyx, c, tmp)));

}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 p = (-iResolution.xy + 2.0*fragCoord) / iResolution.y;

	float time = iTime * .15;

	vec3 light1 = vec3(0.577, 0.577, -0.577);
	vec3 light2 = vec3(-0.707, 0.000, 0.707);


	float r = 1.3 + 0.1*cos(.29*time);
	vec3  ro = vec3(r*cos(.33*time), 0.8*r*sin(.37*time), r*sin(.31*time));
	vec3  ta = vec3(0.0, 0.1, 0.0);
	float cr = 0.5*cos(0.1*time);

	float fov = 1.5;
	vec3 cw = normalize(ta - ro);
	vec3 cp = vec3(sin(cr), cos(cr), 0.0);
	vec3 cu = normalize(cross(cw, cp));
	vec3 cv = normalize(cross(cu, cw));
	vec3 rd = normalize(p.x*cu + p.y*cv + fov * cw);


	vec3 cc = vec3(0.9*cos(3.9 + 1.2*time) - .3, 0.8*cos(2.5 + 1.1*time), 0.8*cos(3.4 + 1.3*time));
	if (length(cc) < 0.50) cc = 0.50*normalize(cc);
	if (length(cc) > 0.95) cc = 0.95*normalize(cc);

	vec3 col;
	vec4 tra;
	float t = intersect(ro, rd, tra, fov, cc);
	if (t < 0.0)
	{
		col = 1.3*vec3(0.8, .95, 1.0)*(0.7 + 0.3*rd.y);
		col += vec3(0.8, 0.7, 0.5)*pow(clamp(dot(rd, light1), 0.0, 1.0), 32.0);
	}
	else
	{
		vec3 pos = ro + t * rd;
		vec3 nor = calcNormal(pos, t, fov, cc);
		vec3 hal = normalize(light1 - rd);
		vec3 ref = reflect(rd, nor);

		col = vec3(1.0, 1.0, 1.0)*0.3;
		col = mix(col, vec3(0.7, 0.3, 0.3), sqrt(tra.x));
		col = mix(col, vec3(1.0, 0.5, 0.2), sqrt(tra.y));
		col = mix(col, vec3(1.0, 1.0, 1.0), tra.z);
		col *= 0.4;


		float dif1 = clamp(dot(light1, nor), 0.0, 1.0);
		float dif2 = clamp(0.5 + 0.5*dot(light2, nor), 0.0, 1.0);
		float occ = clamp(1.2*tra.w - 0.6, 0.0, 1.0);
		float sha = softshadow(pos, light1, 0.0001, 32.0, cc);
		float fre = 0.04 + 0.96*pow(clamp(1.0 - dot(-rd, nor), 0.0, 1.0), 5.0);
		float spe = pow(clamp(dot(nor, hal), 0.0, 1.0), 12.0) * dif1 * fre*8.0;

		vec3 lin = 1.0*vec3(0.15, 0.20, 0.23)*(0.6 + 0.4*nor.y)*(0.1 + 0.9*occ);
		lin += 4.0*vec3(1.00, 0.90, 0.60)*dif1*sha;
		lin += 4.0*vec3(0.14, 0.14, 0.14)*dif2*occ;
		lin += 2.0*vec3(1.00, 1.00, 1.00)*spe*sha * occ;
		lin += 0.3*vec3(0.20, 0.30, 0.40)*(0.02 + 0.98*occ);
		col *= lin;
		col += spe * 1.0*occ*sha;
	}

	col = sqrt(col);

	fragColor = vec4(col, 1.0);
}
#endif


#if SHADERTOY == 15
// Panteleymonov A K 2015

//
// procedural noise from https://www.shadertoy.com/view/4sfGzS
// for first variant

// animated noise
static vec4 NC0 = vec4(0.0, 157.0, 113.0, 270.0);
static vec4 NC1 = vec4(1.0, 158.0, 114.0, 271.0);
//vec4 WS=vec4(10.25,32.25,15.25,3.25);
static vec4 WS = vec4(0.25, 0.25, 0.25, 0.25);

// mix noise for alive animation, full source
vec4 hash4(vec4 n) { return fract(sin(n)*1399763.5453123); }
vec3 hash3(vec3 n) { return fract(sin(n)*1399763.5453123); }
vec3 hpos(vec3 n) { return hash3(vec3(dot(n, vec3(157.0, 113.0, 271.0)), dot(n, vec3(271.0, 157.0, 113.0)), dot(n, vec3(113.0, 271.0, 157.0)))); }
//vec4 hash4( vec4 n ) { return fract(n*fract(n*0.5453123)); }
//vec4 hash4( vec4 n ) { n*=1.987654321; return fract(n*fract(n)); }
float noise4q(vec4 x)
{
	vec4 n3 = vec4(0, 0.25, 0.5, 0.75);
	vec4 p2 = floor(x.wwww + n3);
	vec4 b = floor(x.xxxx + n3) + floor(x.yyyy + n3)*157.0 + floor(x.zzzz + n3)*113.0;
	vec4 p1 = b + fract(p2*0.00390625)*vec4(164352.0, -164352.0, 163840.0, -163840.0);
	p2 = b + fract((p2 + 1.0)*0.00390625)*vec4(164352.0, -164352.0, 163840.0, -163840.0);
	vec4 f1 = fract(x.xxxx + n3);
	vec4 f2 = fract(x.yyyy + n3);
	f1 = f1 * f1*(3.0 - 2.0*f1);
	f2 = f2 * f2*(3.0 - 2.0*f2);
	vec4 n1 = vec4(0, 1.0, 157.0, 158.0);
	vec4 n2 = vec4(113.0, 114.0, 270.0, 271.0);
	vec4 vs1 = mix(hash4(p1), hash4(n1.yyyy + p1), f1);
	vec4 vs2 = mix(hash4(n1.zzzz + p1), hash4(n1.wwww + p1), f1);
	vec4 vs3 = mix(hash4(p2), hash4(n1.yyyy + p2), f1);
	vec4 vs4 = mix(hash4(n1.zzzz + p2), hash4(n1.wwww + p2), f1);
	vs1 = mix(vs1, vs2, f2);
	vs3 = mix(vs3, vs4, f2);
	vs2 = mix(hash4(n2.xxxx + p1), hash4(n2.yyyy + p1), f1);
	vs4 = mix(hash4(n2.zzzz + p1), hash4(n2.wwww + p1), f1);
	vs2 = mix(vs2, vs4, f2);
	vs4 = mix(hash4(n2.xxxx + p2), hash4(n2.yyyy + p2), f1);
	vec4 vs5 = mix(hash4(n2.zzzz + p2), hash4(n2.wwww + p2), f1);
	vs4 = mix(vs4, vs5, f2);
	f1 = fract(x.zzzz + n3);
	f2 = fract(x.wwww + n3);
	f1 = f1 * f1*(3.0 - 2.0*f1);
	f2 = f2 * f2*(3.0 - 2.0*f2);
	vs1 = mix(vs1, vs2, f1);
	vs3 = mix(vs3, vs4, f1);
	vs1 = mix(vs1, vs3, f2);
	float r = dot(vs1, /*vec4*/(0.25));
	//r=r*r*(3.0-2.0*r);
	return r * r*(3.0 - 2.0*r);
}

// body of a star
float noiseSpere(vec3 ray, vec3 pos, float r, mat3 mr, float zoom, vec3 subnoise, float anim)
{
	float b = dot(ray, pos);
	float c = dot(pos, pos) - b * b;

	vec3 r1 = /*vec3*/(0.0);

	float s = 0.0;
	float d = 0.03125;
	float d2 = zoom / (d*d);
	float ar = 5.0;

	for (int i = 0; i < 3; i++) {
		float rq = r * r;
		if (c < rq)
		{
			float l1 = sqrt(rq - c);
			r1 = ray * (b - l1) - pos;
			r1 = mul(r1, mr);
			s += abs(noise4q(vec4(r1*d2 + subnoise * ar, anim*ar))*d);
		}
		ar -= 2.0;
		d *= 4.0;
		d2 *= 0.0625;
		r = r - r * 0.02;
	}
	return s;
}

// glow ring
float ring(vec3 ray, vec3 pos, float r, float size)
{
	float b = dot(ray, pos);
	float c = dot(pos, pos) - b * b;

	float s = max(0.0, (1.0 - size * abs(r - sqrt(c))));

	return s;
}

// rays of a star
float ringRayNoise(vec3 ray, vec3 pos, float r, float size, mat3 mr, float anim)
{
	float b = dot(ray, pos);
	vec3 pr = ray * b - pos;

	float c = length(pr);

	pr = mul(pr, mr);

	pr = normalize(pr);

	float s = max(0.0, (1.0 - size * abs(r - c)));

	float nd = noise4q(vec4(pr*1.0, -anim + c))*2.0;
	nd = pow(nd, 2.0);
	float n = 0.4;
	float ns = 1.0;
	if (c > r) {
		n = noise4q(vec4(pr*10.0, -anim + c));
		ns = noise4q(vec4(pr*50.0, -anim * 2.5 + c * 2.0))*2.0;
	}
	n = n * n*nd*ns;

	return pow(s, 4.0) + s * s*n;
}

vec4 noiseSpace(vec3 ray, vec3 pos, float r, mat3 mr, float zoom, vec3 subnoise, float anim)
{
	float b = dot(ray, pos);
	float c = dot(pos, pos) - b * b;

	vec3 r1 = /*vec3*/(0.0);

	float s = 0.0;
	float d = 0.0625*1.5;
	float d2 = zoom / d;

	float rq = r * r;
	float l1 = sqrt(abs(rq - c));
	r1 = mul((ray*(b - l1) - pos), mr);

	r1 *= d2;
	s += abs(noise4q(vec4(r1 + subnoise, anim))*d);
	s += abs(noise4q(vec4(r1*0.5 + subnoise, anim))*d*2.0);
	s += abs(noise4q(vec4(r1*0.25 + subnoise, anim))*d*4.0);
	//return s;
	return vec4(s*2.0, abs(noise4q(vec4(r1*0.1 + subnoise, anim))), abs(noise4q(vec4(r1*0.1 + subnoise * 6.0, anim))), abs(noise4q(vec4(r1*0.1 + subnoise * 13.0, anim))));
}

float sphereZero(vec3 ray, vec3 pos, float r)
{
	float b = dot(ray, pos);
	float c = dot(pos, pos) - b * b;
	float s = 1.0;
	if (c < r*r) s = 0.0;
	return s;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 p = (-iResolution.xy + 2.0*fragCoord.xy) / iResolution.y;

	float time = iTime * 1.0;

	float mx = iMouse.z > 0.0 ? iMouse.x / iResolution.x*10.0 : time * 0.025;
	float my = iMouse.z > 0.0 ? iMouse.y / iResolution.y*4.0 - 2.0 : -0.6;
	vec2 rotate = vec2(mx, my);

	vec2 sins = sin(rotate);
	vec2 coss = cos(rotate);
	mat3 mr = mat3(vec3(coss.x, 0.0, sins.x), vec3(0.0, 1.0, 0.0), vec3(-sins.x, 0.0, coss.x));
	mr = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, coss.y, sins.y), vec3(0.0, -sins.y, coss.y))*mr;

	mat3 imr = mat3(vec3(coss.x, 0.0, -sins.x), vec3(0.0, 1.0, 0.0), vec3(sins.x, 0.0, coss.x));
	imr = imr * mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, coss.y, -sins.y), vec3(0.0, sins.y, coss.y));

	vec3 ray = normalize(vec3(p, 2.0));
	vec3 pos = vec3(0.0, 0.0, 3.0);

	float s1 = noiseSpere(ray, pos, 1.0, mr, 0.5, /*vec3*/(0.0), time);
	s1 = pow(min(1.0, s1*2.4), 2.0);
	float s2 = noiseSpere(ray, pos, 1.0, mr, 4.0, vec3(83.23, 34.34, 67.453), time);
	s2 = min(1.0, s2*2.2);
	fragColor = vec4(mix(vec3(1.0, 1.0, 0.0), /*vec3*/(1.0), pow(s1, 60.0))*s1, 1.0);
	fragColor += vec4(mix(mix(vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 1.0), pow(s2, 2.0)), /*vec3*/(1.0), pow(s2, 10.0))*s2, 1.0);

	fragColor.xyz -= /*vec3*/(ring(ray, pos, 1.03, 11.0))*2.0;
	fragColor = max(/*vec4*/(0.0), fragColor);

	float s3 = ringRayNoise(ray, pos, 0.96, 1.0, mr, time);
	fragColor.xyz += mix(vec3(1.0, 0.6, 0.1), vec3(1.0, 0.95, 1.0), pow(s3, 3.0))*s3;

	float zero = sphereZero(ray, pos, 0.9);
	if (zero > 0.0) {
		//float s4=noiseSpace(ray,pos,100.0,mr,0.5,vec3(0.0),time*0.01);
		vec4 s4 = noiseSpace(ray, pos, 100.0, mr, 0.05, vec3(1.0, 2.0, 4.0), 0.0);
		//float s5=noiseSpace(ray,pos,100.0,vec3(mx,my,0.5),vec3(83.23,34.34,67.453),time*0.01);
		//s4=pow(s4*2.0,6.0);
		//s4=pow(s4*1.8,5.7);
		s4.x = pow(s4.x, 3.0);
		//s5=pow(s5*2.0,6.0);
		//fragColor.xyz += (vec3(0.0,0.0,1.0)*s4*0.6+vec3(0.9,0.0,1.0)*s5*0.3)*sphereZero(ray,pos,0.9);
		fragColor.xyz += mix(mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), s4.y*1.9), vec3(0.9, 1.0, 0.1), s4.w*0.75)*s4.x*pow(s4.z*2.5, 3.0)*0.2*zero;
		//fragColor.xyz += (mix(mix(vec3(1.0,0.0,0.0),vec3(0.0,0.0,1.0),s4*3.0),vec3(1.0),pow(s4*2.0,4.0))*s4*0.6)*sphereZero(ray,pos,0.9);


		/*float b = dot(ray,pos);
		float c = dot(pos,pos) - b*b;
		float l1 = sqrt(abs(10.0-c));
		vec3 spos = (ray*(b-l1))*mr;
		vec3 sposr=ceil(spos)+spos/abs(spos)*0.5;
		//sposr+=hpos(sposr)*0.2;

		float ss3=max(0.0,ringRayNoise(ray,(sposr)*imr,0.001,10.0,mr,time));
		fragColor.xyz += vec3(ss3);*/
	}

	//fragColor = max( vec4(0.0), fragColor );
	//s+=noiseSpere(ray,vec3(0.0,0.0,3.0),0.96,vec2(mx+1.4,my),vec3(83.23,34.34,67.453));
	//s+=noiseSpere(ray,vec3(0.0,0.0,3.0),0.90,vec2(mx,my),vec3(123.223311,956.34,7.45333))*0.6;

	fragColor = max(/*vec4*/(0.0), fragColor);
	fragColor = min(/*vec4*/(1.0), fragColor);
}
#endif

#if SHADERTOY == 16
// Created by inigo quilez - iq/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// An edge antialising experiment (not multisampling used)
//
// If slow_antialias is disabled, then only the 4 closest hit points are used for antialising, 
// otherwise all found partial-intersections are considered.

#define ANTIALIASING
//#define SLOW_ANTIALIAS

vec2 sincos(float x) { return vec2(sin(x), cos(x)); }

vec2 sdCylinder(in vec3 p)
{
	return vec2(length(p.xz), (p.y + 50.0) / 100.0);
}

vec3 opU(vec3 d1, vec3 d2) { return (d1.x < d2.x) ? d1 : d2; }

vec3 map(vec3 p)
{
	vec2 id = floor((p.xz + 1.0) / 2.0);
	p.xz = mod(p.xz + 1.0, 2.0) - 1.0;

	float ph = sin(0.5 + 3.1*id.x + sin(7.1*id.y));

	p.xz += 0.5*sincos(1.0 + 0.5*iTime + (p.y + 11.0*ph)*0.8);

	vec3 p1 = p; p1.xz += 0.15*sincos(1.0*p.y - 1.0*iTime + 0.0);
	vec3 p2 = p; p2.xz += 0.15*sincos(1.0*p.y - 1.0*iTime + 2.0);
	vec3 p3 = p; p3.xz += 0.15*sincos(1.0*p.y - 1.0*iTime + 4.0);

	vec2 h1 = sdCylinder(p1);
	vec2 h2 = sdCylinder(p2);
	vec2 h3 = sdCylinder(p3);

	return opU(opU(vec3(h1.x - 0.12, ph + 0.0 / 3.0, h1.y),
		vec3(h2.x - 0.12 - 0.05*cos(500.0*h2.y - iTime * 4.0), ph + 1.0 / 3.0, h2.y)),
		vec3(h3.x - 0.12 - 0.02*cos(2000.0*h3.y - iTime * 4.0), ph + 2.0 / 3.0, h3.y));
}

//-------------------------------------------------------

vec3 calcNormal(in vec3 pos, in float dt)
{
	vec2 e = vec2(1.0, -1.0)*dt;
	return normalize(e.xyy*map(pos + e.xyy).x +
		e.yyx*map(pos + e.yyx).x +
		e.yxy*map(pos + e.yxy).x +
		e.xxx*map(pos + e.xxx).x);
}

float calcOcc(in vec3 pos, in vec3 nor)
{
	const float h = 0.15;
	float ao = 0.0;
	for (int i = 0; i < 8; i++)
	{
		vec3 dir = sin(float(i)*vec3(1.0, 7.13, 13.71) + vec3(0.0, 2.0, 4.0));
		dir = dir + 2.5*nor*max(0.0, -dot(nor, dir));
		float d = map(pos + h * dir).x;
		ao += max(0.0, h - d);
	}
	return clamp(1.0 - 0.7*ao, 0.0, 1.0);
}

//-------------------------------------------------------
vec3 shade(in float t, in float m, in float v, in vec3 ro, in vec3 rd)
{
	float px = 0.0001;//(2.0/iResolution.y)*(1.0/3.0);
	float eps = px * t;

	vec3  pos = ro + t * rd;
	vec3  nor = calcNormal(pos, eps);
	float occ = calcOcc(pos, nor);

	vec3 col = 0.5 + 0.5*cos(m*vec3(1.4, 1.2, 1.0) + vec3(0.0, 1.0, 2.0));
	col += 0.05*nor;
	col = clamp(col, 0.0, 1.0);
	col *= 1.0 + 0.5*nor.x;
	col += 0.2*clamp(1.0 + dot(rd, nor), 0.0, 1.0);
	col *= 1.4;
	col *= occ;
	col *= exp(-0.15*t);
	col *= 1.0 - smoothstep(15.0, 35.0, t);

	return col;
}

//-------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 p = (-iResolution.xy + 2.0*fragCoord.xy) / iResolution.y;

	vec3 ro = 0.6*vec3(2.0, -3.0, 4.0);
	vec3 ta = 0.5*vec3(0.0, 4.0, -4.0);

	float fl = 1.0;
	vec3 ww = normalize(ta - ro);
	vec3 uu = normalize(cross(vec3(1.0, 0.0, 0.0), ww));
	vec3 vv = normalize(cross(ww, uu));
	vec3 rd = normalize(p.x*uu + p.y*vv + fl * ww);

	float px = (2.0 / iResolution.y)*(1.0 / fl);

	vec3 col = /*vec3*/(0.0);

	//---------------------------------------------
	// raymach loop
	//---------------------------------------------
	const float maxdist = 32.0;

	vec3 res = /*vec3*/(-1.0);
	float t = 0.0;
#ifdef ANTIALIASING
	vec3 oh = /*vec3*/(0.0);
	mat4 hit = mat4(-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);
	vec4 tmp = /*vec4*/(0.0);
#endif

	for (int i = 0; i < 128; i++)
	{
		vec3 h = map(ro + t * rd);
		float th1 = px * t;
		res = vec3(t, h.yz);
		if (h.x<th1 || t>maxdist) break;


#ifdef ANTIALIASING
		float th2 = px * t*3.0;
		if ((h.x < th2) && (h.x > oh.x))
		{
			float lalp = 1.0 - (h.x - th1) / (th2 - th1);
#ifdef SLOW_ANTIALIAS
			vec3  lcol = shade(t, oh.y, oh.z, ro, rd);
			tmp.xyz += (1.0 - tmp.w)*lalp*lcol;
			tmp.w += (1.0 - tmp.w)*lalp;
			if (tmp.w > 0.99) break;
#else
			if (hit[0].x < 0.0)
			{
				hit[0] = hit[1]; hit[1] = hit[2]; hit[2] = hit[3]; hit[3] = vec4(t, oh.yz, lalp);
			}
#endif
		}
		oh = h;
#endif

		t += min(h.x, 0.5)*0.5;
	}

	if (t < maxdist)
		col = shade(res.x, res.y, res.z, ro, rd);

#ifdef ANTIALIASING
#ifdef SLOW_ANTIALIAS
	col = mix(col, tmp.xyz / (0.001 + tmp.w), tmp.w);
#else
	for (int i = 0; i < 4; i++) // blend back to front
		if (hit[3 - i].x > 0.0)
			col = mix(col, shade(hit[3 - i].x, hit[3 - i].y, hit[3 - i].z, ro, rd), hit[3 - i].w);
#endif
#endif

	//---------------------------------------------

	col = pow(col, vec3(0.5, 0.7, 0.5));

	vec2 q = fragCoord.xy / iResolution.xy;
	col *= pow(16.0*q.x*q.y*(1.0 - q.x)*(1.0 - q.y), 0.1);

	fragColor = vec4(col, 1.0);
}
#endif

#if SHADERTOY == 17
// https://www.shadertoy.com/view/4sSSWz
// starDust - shadertoy intro
// Created by Dmitry Andreev - and'2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

#define SPEED           (1.7)
#define WARMUP_TIME     (2.0)

// Shadertoy's sound is a bit out of sync every time you run it :(
#define SOUND_OFFSET    (-0.0)

float saturate(float x)
{
	return clamp(x, 0.0, 1.0);
}

float isectPlane(vec3 n, float d, vec3 org, vec3 dir)
{
	float t = -(dot(org, n) + d) / dot(dir, n);

	return t;
}

float drawLogo(in vec2 fragCoord)
{
	float res = max(iResolution.x, iResolution.y);
	vec2  pos = vec2(floor((fragCoord.xy / res) * 128.0));

	float val = 0.0;

	// AND'14 bitmap
	if (pos.y == 2.0) val = 4873761.5;
	if (pos.y == 3.0) val = 8049199.5;
	if (pos.y == 4.0) val = 2839721.5;
	if (pos.y == 5.0) val = 1726633.5;
	if (pos.x > 125.0) val = 0.0;

	float bit = floor(val * exp2(pos.x - 125.0));

	return bit != floor(bit / 2.0) * 2.0 ? 1.0 : 0.0;
}

vec3 drawEffect(vec2 coord, float time)
{
	vec3 clr = (0.0);
	const float far_dist = 10000.0;

	float mtime = SOUND_OFFSET + time * 2.0 / SPEED;
	vec2 uv = coord.xy / iResolution.xy;
	uv.y = 1.0 - uv.y;

	vec3 org = (0.0);
	vec3 dir = vec3(uv.xy * 2.0 - 1.0, 1.0);

	// Animate tilt
	float ang = sin(time * 0.2) * 0.2;
	vec3 odir = dir;
	dir.x = cos(ang) * odir.x + sin(ang) * odir.y;
	dir.y = sin(ang) * odir.x - cos(ang) * odir.y;

	// Animate FOV and aspect ratio
	dir.x *= 1.5 + 0.5 * sin(time * 0.125);
	dir.y *= 1.5 + 0.5 * cos(time * 0.25 + 0.5);

	// Animate view direction
	dir.x += 0.25 * sin(time * 0.3);
	dir.y += 0.25 * sin(time * 0.7);

	// Bend it like this
	dir.xy = mix(vec2(dir.x + 0.2 * cos(dir.y) - 0.1, dir.y), dir.xy,
		smoothstep(0.0, 1.0, saturate(0.5 * abs(mtime - 50.0))));

	// Bend it like that
	dir.xy = mix(vec2(dir.x + 0.1 * sin(4.0 * (dir.x + time)), dir.y), dir.xy,
		smoothstep(0.0, 1.0, saturate(0.5 * abs(mtime - 58.0))));

	// Cycle between long blurry and short sharp particles
	vec2 param = mix(vec2(60.0, 0.8), vec2(800.0, 3.0),
		pow(0.5 + 0.5 * sin(time * 0.2), 2.0));

	float lt = fract(mtime / 4.0) * 4.0;
	vec2 mutes = (0.0);

	if (mtime >= 32.0 && mtime < 48.0)
	{
		mutes = max((0.0), 1.0 - 4.0 * abs(lt - vec2(3.25, 3.50)));
	}

	for (int k = 0; k < 2; k++)
		for (int i = 0; i < 64; i++)
		{
			// Draw only few layers during prologue
			if (mtime < 16.0 && i >= 16) break;

			vec3 pn = vec3(k > 0 ? -1.0 : 1.0, 0.0, 0.0);
			float t = isectPlane(pn, 100.0 + float(i) * 20.0, org, dir);

			if (t <= 0.0 || t >= far_dist) continue;

			vec3 p = org + dir * t;
			vec3 vdir = normalize(-p);

			// Create particle lanes by quantizing position
			vec3 pp = ceil(p / 100.0) * 100.0;

			// Pseudo-random variables
			float n = pp.y + float(i) + float(k) * 123.0;
			float q = fract(sin(n * 123.456) * 234.345);
			float q2 = fract(sin(n * 234.123) * 345.234);

			q = sin(p.z * 0.0003 + 1.0 * time * (0.25 + 0.75 * q2) + q * 12.0);

			// Smooth particle edges out
			q = saturate(q * param.x - param.x + 1.0) * param.y;
			q *= saturate(4.0 - 8.0 * abs(-50.0 + pp.y - p.y) / 100.0);

			// Fade out based on distance
			q *= 1.0 - saturate(pow(t / far_dist, 5.0));

			// Fade out based on view angle
			float fn = 1.0 - pow(1.0 - dot(vdir, pn), 2.0);
			q *= 2.0 * smoothstep(0.0, 1.0, fn);

			// Flash fade left or right plane
			q *= 1.0 - 0.9 * (k == 0 ? mutes.x : mutes.y);

			// Cycle palettes
			const vec3 orange = vec3(1.0, 0.7, 0.4);
			const vec3 blue = vec3(0.4, 0.7, 1.0);
			clr += q * mix(orange, blue, 0.5 + 0.5 * sin(time * 0.5 + q2));

			// Flash some particles in sync with bass drum
			float population = mtime < 16.0 ? 0.0 : 0.97;

			if (mtime >= 8.0 && q2 > population)
			{
				float a = mtime >= 62.0 ? 8.0 : 1.0;
				float b = mtime < 16.0 ? 2.0 : a;

				clr += q * (mtime < 16.0 ? 2.0 : 8.0)
					* max(0.0, fract(-mtime * b) * 2.0 - 1.0);
			}
		}

	clr *= 0.2;

	// Cycle gammas
	clr.r = pow(clr.r, 0.75 + 0.35 * sin(time * 0.5));
	clr.b = pow(clr.b, 0.75 - 0.35 * sin(time * 0.5));

	// Initial fade-in
	clr *= pow(min(mtime / 4.0, 1.0), 2.0);

	// Fade-out shortly after initial fade-in right before drums kick in
	if (mtime < 8.0) clr *= 1.0 - saturate((mtime - 5.0) / 3.0);

	// Flash horizon in sync with snare drum
	if (mtime >= 15.0)
	{
		float h = normalize(dir).x;
		clr *= 1.0 + 2.0 * pow(saturate(1.0 - abs(h)), 8.0)
			* max(0.0, fract(-mtime + 0.5) * 4.0 - 3.0);
	}

	// The end
	if (mtime >= 64.0) clr = (0.0);

	// Initial flash
	if (mtime >= 16.0) clr += max(0.0, 1.0 - (mtime - 16.0) * 1.0);

	// Final flash
	if (mtime >= 64.0) clr += max(0.0, 1.0 - (mtime - 64.0) * 0.5) * vec3(0.8, 0.9, 1.0);

	// Desaturate prologue
	if (mtime < 16.0) clr = mix((dot(clr, (0.33))), clr, min(1.0, mtime / 32.0));

	// Vignette in linear space (looks better)
	clr *= clr;
	clr *= 1.4;
	clr *= 1.0 - 1.5 * dot(uv - 0.5, uv - 0.5);
	clr = sqrt(max((0.0), clr));

	return clr;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	float time = max(0.0, iTime - WARMUP_TIME);
	vec3  clr = (0.0);

	clr = drawEffect(fragCoord.xy, time)/* * float3(1.0, 0.2, 0.1)*/;
	//clr = mix(clr, vec3(0.8, 0.9, 1.0), 0.3 * drawLogo(fragCoord));

	fragColor = vec4(clr, 1.0);
}
#endif

#if SHADERTOY == 18

#if HW_PERFORMANCE==0
#define AA 1
#else
#define AA 2
#endif

#define numballs 8

// undefine this for numerical normals
#define ANALYTIC_NORMALS

//----------------------------------------------------------------

float hash1(float n)
{
	return fract(sin(n)*43758.5453123);
}

vec2 hash2(float n)
{
	return fract(sin(vec2(n, n + 1.0))*vec2(43758.5453123, 22578.1459123));
}

vec3 hash3(float n)
{
	return fract(sin(vec3(n, n + 1.0, n + 2.0))*vec3(43758.5453123, 22578.1459123, 19642.3490423));
}

//----------------------------------------------------------------

static vec4 blobs[numballs];

float sdMetaBalls(vec3 pos)
{
	float m = 0.0;
	float p = 0.0;
	float dmin = 1e20;

	float h = 1.0; // track Lipschitz constant

	for (int i = 0; i < numballs; i++)
	{
		// bounding sphere for ball
		float db = length(blobs[i].xyz - pos);
		if (db < blobs[i].w)
		{
			float x = db / blobs[i].w;
			p += 1.0 - x * x*x*(x*(x*6.0 - 15.0) + 10.0);
			m += 1.0;
			h = max(h, 0.5333*blobs[i].w);
		}
		else // bouncing sphere distance
		{
			dmin = min(dmin, db - blobs[i].w);
		}
	}
	float d = dmin + 0.1;

	if (m > 0.5)
	{
		float th = 0.2;
		d = h * (th - p);
	}

	return d;
}


vec3 norMetaBalls(vec3 pos)
{
	vec3 nor = vec3(0.0, 0.0001, 0.0);

	for (int i = 0; i < numballs; i++)
	{
		float db = length(blobs[i].xyz - pos);
		float x = clamp(db / blobs[i].w, 0.0, 1.0);
		float p = x * x*(30.0*x*x - 60.0*x + 30.0);
		nor += normalize(pos - blobs[i].xyz) * p / blobs[i].w;
	}

	return normalize(nor);
}


float map(in vec3 p)
{
	return sdMetaBalls(p);
}


static const float precis = 0.01;

vec2 intersect(in vec3 ro, in vec3 rd)
{
	float maxd = 10.0;
	float h = precis * 2.0;
	float t = 0.0;
	float m = 1.0;
	for (int i = 0; i < 75; i++)
	{
		if (h<precis || t>maxd) continue;//break;
		t += h;
		h = map(ro + rd * t);
	}

	if (t > maxd) m = -1.0;
	return vec2(t, m);
}

vec3 calcNormal(in vec3 pos)
{
#ifdef ANALYTIC_NORMALS	
	return norMetaBalls(pos);
#else	
	vec3 eps = vec3(precis, 0.0, 0.0);
	return normalize(vec3(
		map(pos + eps.xyy) - map(pos - eps.xyy),
		map(pos + eps.yxy) - map(pos - eps.yxy),
		map(pos + eps.yyx) - map(pos - eps.yyx)));
#endif
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	//-----------------------------------------------------
	// input
	//-----------------------------------------------------

	vec2 q = fragCoord.xy / iResolution.xy;

	vec2 m = (0.5);
	if (iMouse.z > 0.0) m = iMouse.xy / iResolution.xy;

	//-----------------------------------------------------
	// montecarlo (over time, image plane and lens) (5D)
	//-----------------------------------------------------

	vec3 tot = (0.0);
#if AA>1
	for (int ax = 0; ax < AA; ax++)
		for (int ay = 0; ay < AA; ay++)
		{
			vec2  poff = vec2(float(ax), float(ay)) / float(AA);
			float toff = 0.0*(float(AA*ay + ax) / float(AA*AA)) * (0.5 / 24.0); // shutter time of half frame
#else
			{
				vec2  poff = (0.0);
				float toff = 0.0;
#endif		
				//-----------------------------------------------------
				// animate scene
				//-----------------------------------------------------
				float time = iTime + toff;

				// move metaballs
				for (int i = 0; i < numballs; i++)
				{
					float h = float(i) / 8.0;
					blobs[i].xyz = 2.0*sin(6.2831*hash3(h*1.17) + hash3(h*13.7)*time);
					blobs[i].w = 1.7 + 0.9*sin(6.28*hash1(h*23.13));
				}

				// move camera		
				float an = 0.5*time - 6.2831*(m.x - 0.5);
				vec3 ro = vec3(5.0*sin(an), 2.5*cos(0.4*an), 5.0*cos(an));
				vec3 ta = vec3(0.0, 0.0, 0.0);


				//-----------------------------------------------------
				// camera
				//-----------------------------------------------------
				// image plane		
				vec2 p = (2.0*(fragCoord + poff) - iResolution.xy) / iResolution.y;

				// camera matrix
				vec3 ww = normalize(ta - ro);
				vec3 uu = normalize(cross(ww, vec3(0.0, 1.0, 0.0)));
				vec3 vv = normalize(cross(uu, ww));
				// create view ray
				vec3 rd = normalize(p.x*uu + p.y*vv + 2.0*ww);
				// dof
#if AA > 4
				vec3 fp = ro + rd * 5.0;
				vec2 le = -1.0 + 2.0*hash2(dot(fragCoord.xy, vec2(131.532, 73.713)) + float(AA*ay + ax)*121.41);
				ro += (uu*le.x + vv * le.y)*0.1;
				rd = normalize(fp - ro);
#endif		

				//-----------------------------------------------------
				// render
				//-----------------------------------------------------

				// background
				vec3 col = pow(texture(iChannel0, rd).xyz, (2.2));

				// raymarch
				vec2 tmat = intersect(ro, rd);
				if (tmat.y > -0.5)
				{
					// geometry
					vec3 pos = ro + tmat.x*rd;
					vec3 nor = calcNormal(pos);
					vec3 ref = reflect(rd, nor);

					// materials
					vec3 mate = (0.0);
					float w = 0.01;
					for (int i = 0; i < numballs; i++)
					{
						float h = float(i) / 8.0;

						// metaball color
						vec3 ccc = (1.0);
						ccc = mix(ccc, vec3(1.0, 0.60, 0.05), smoothstep(0.65, 0.66, sin(30.0*h)));
						ccc = mix(ccc, vec3(0.3, 0.45, 0.25), smoothstep(0.65, 0.66, sin(15.0*h)));

						float x = clamp(length(blobs[i].xyz - pos) / blobs[i].w, 0.0, 1.0);
						float p = 1.0 - x * x*(3.0 - 2.0*x);
						mate += p * ccc;
						w += p;
					}
					mate /= w;

					// lighting
					vec3 lin = (0.0);
					lin += mix(vec3(0.05, 0.02, 0.0), 1.2*vec3(0.8, 0.9, 1.0), 0.5 + 0.5*nor.y);
					lin *= 1.0 + 1.5*vec3(0.7, 0.5, 0.3)*pow(clamp(1.0 + dot(nor, rd), 0.0, 1.0), 2.0);
					lin += 1.5*clamp(0.3 + 2.0*nor.y, 0.0, 1.0)*pow(texture(iChannel0, ref).xyz, (2.2))*(0.04 + 0.96*pow(clamp(1.0 + dot(nor, rd), 0.0, 1.0), 4.0));

					// surface-light interacion
					col = lin * mate;
				}
				// gamma
				col = pow(clamp(col, 0.0, 1.0), (0.45));
				tot += col;
			}
			tot /= float(AA*AA);

			//-----------------------------------------------------
			// postprocessing
			//-----------------------------------------------------

			// vigneting
			tot *= 0.5 + 0.5*pow(16.0*q.x*q.y*(1.0 - q.x)*(1.0 - q.y), 0.15);

			fragColor = vec4(tot, 1.0);
		}
#endif

#if SHADERTOY == 19
/*
Biomine
-------

A biocooling system for a futuristic, off-world mine... or a feeding mechanisn for an alien
hatchery? I wasn't really sure what I was creating when I started, and I'm still not. :) I at
least wanted to create the sense that the tubes were pumping some form of biomatter around
without having to resort to full reflective and refractive passes... I kind of got there. :)

All things considered, there's not a lot to this. Combine a couple of gyroid surfaces, ID them,
then add their respective material properties. The scene is simple to create, and explained in
the distance function. There's also some function based, 2nd order cellular bump mapping, for
anyone interested.

The fluid pumped through the tubes was created by indexing the reflected and refracted rays
into a basic environment mapping function. Not accurate, but simple, effective and way cheaper
than the real thing.

I'd just finished watching some of the Assembly 2016 entries on YouTube, so for better or
worse, wanted to produce the scene without the help of any in-house textures.

Related examples:

Cellular Tiling - Shane
https://www.shadertoy.com/view/4scXz2

Cellular Tiled Tunnel - Shane
https://www.shadertoy.com/view/MscSDB

*/

// Max ray distance.
#define FAR 50. 

// Variables used to identify the objects. In this case, there are just two - the biotubes and
// the tunnel walls.
static float objID = 0.; // Biotubes: 0, Tunnel walls: 1.
static float saveID = 0.;


// Standard 1x1 hash functions. Using "cos" for non-zero origin result.
float hash(float n) { return fract(cos(n)*45758.5453); }



// 2x2 matrix rotation. Note the absence of "cos." It's there, but in disguise, and comes courtesy
// of Fabrice Neyret's "ouside the box" thinking. :)
mat2 rot2(float a) { vec2 v = sin(vec2(1.570796, 0) + a);	return mat2(v, -v.y, v.x); }


// Compact, self-contained version of IQ's 3D value noise function. I have a transparent noise
// example that explains it, if you require it.
float noise3D(in vec3 p) {

	const vec3 s = vec3(7, 157, 113);
	vec3 ip = floor(p); p -= ip;
	vec4 h = vec4(0., s.yz, s.y + s.z) + dot(ip, s);
	p = p * p*(3. - 2.*p); //p *= p*p*(p*(p * 6. - 15.) + 10.);
	h = mix(fract(sin(h)*43758.5453), fract(sin(h + s.x)*43758.5453), p.x);
	h.xy = mix(h.xz, h.yw, p.y);
	return mix(h.x, h.y, p.z); // Range: [0, 1].
}

////////
// The cellular tile routine. Draw a few objects (four spheres, in this case) using a minumum
// blend at various 3D locations on a cubic tile. Make the tile wrappable by ensuring the 
// objects wrap around the edges. That's it.
//
// Believe it or not, you can get away with as few as three spheres. If you sum the total 
// instruction count here, you'll see that it's way, way lower than 2nd order 3D Voronoi.
// Not requiring a hash function provides the biggest benefit, but there is also less setup.
// 
// The result isn't perfect, but 3D cellular tiles can enable you to put a Voronoi looking 
// surface layer on a lot of 3D objects for little cost.
//
float drawSphere(in vec3 p) {

	p = fract(p) - .5;
	return dot(p, p);

	//p = abs(fract(p)-.5);
	//return dot(p, vec3(.5));  
}


float cellTile(in vec3 p) {

	// Draw four overlapping objects (spheres, in this case) at various positions throughout the tile.
	vec4 v, d;
	d.x = drawSphere(p - vec3(.81, .62, .53));
	p.xy = vec2(p.y - p.x, p.y + p.x)*.7071;
	d.y = drawSphere(p - vec3(.39, .2, .11));
	p.yz = vec2(p.z - p.y, p.z + p.y)*.7071;
	d.z = drawSphere(p - vec3(.62, .24, .06));
	p.xz = vec2(p.z - p.x, p.z + p.x)*.7071;
	d.w = drawSphere(p - vec3(.2, .82, .64));

	v.xy = min(d.xz, d.yw), v.z = min(max(d.x, d.y), max(d.z, d.w)), v.w = max(v.x, v.y);

	d.x = min(v.z, v.w) - min(v.x, v.y); // First minus second order, for that beveled Voronoi look. Range [0, 1].
										 //d.x =  min(v.x, v.y); // Minimum, for the cellular look.

	return d.x*2.66; // Normalize... roughly.

}

// The path is a 2D sinusoid that varies over time, depending upon the frequencies, and amplitudes.
vec2 path(in float z) {
	//return vec2(0);
	float a = sin(z * 0.11);
	float b = cos(z * 0.14);
	return vec2(a*4. - b * 1.5, b*1.7 + a * 1.5);
}


// Smooth maximum, based on IQ's smooth minimum function.
float smaxP(float a, float b, float s) {

	float h = clamp(0.5 + 0.5*(a - b) / s, 0., 1.);
	return mix(b, a, h) + h * (1.0 - h)*s;
}


// The distance function. It's a lot simpler than it looks: The biological tubes are just a gyroid lattice.
// The mine tunnel, is created by takoing the negative space, and bore out the center with a cylinder. The
// two are combined with a smooth maximum to produce the tunnel with biotube lattice. On top of that, the 
// whole scene is wrapped around a path and slightly mutated (the first two lines), but that's it.

float map(vec3 p) {

	p.xy -= path(p.z); // Wrap the scene around a path.

	p += cos(p.zxy*1.5707963)*.2; // Perturb slightly. The mutation gives it a bit more of an organic feel.

								  // If you're not familiar with a gyroid lattice, this is basically it. Not so great to hone in on, but
								  // pretty cool looking and simple to produce.
	float d = dot(cos(p*1.5707963), sin(p.yzx*1.5707963)) + 1.;

	// Biotube lattice. The final time-based term makes is heave in and out.
	float bio = d + .25 + dot(sin(p*1. + iTime * 6.283 + sin(p.yzx*.5)), (.033));

	// The tunnel. Created with a bit of trial and error. The smooth maximum against the gyroid rounds it off
	// a bit. The abs term at the end just adds some variation via the beveled edges. Also trial and error.
	float tun = smaxP(3.25 - length(p.xy - vec2(0, 1)) + .5*cos(p.z*3.14159 / 32.), .75 - d, 1.) - abs(1.5 - d)*.375;;// - sf*.25;


	objID = step(tun, bio); // Tunnel and biolattice IDs, for coloring, lighting, bumping, etc, later.

	return min(tun, bio); // Return the distance to the scene.


}


// Surface bump function. Cheap, but with decent visual impact.
float bumpSurf3D(in vec3 p) {

	float bmp;
	float noi = noise3D(p*96.);

	if (saveID > .5) {
		float sf = cellTile(p*.75);
		float vor = cellTile(p*1.5);

		bmp = sf * .66 + (vor*.94 + noi * .06)*.34;
	}
	else {
		p /= 3.;//
		float ct = cellTile(p*2. + sin(p*12.)*.5)*.66 + cellTile(p*6. + sin(p*36.)*.5)*.34;
		bmp = (1. - smoothstep(-.2, .25, ct))*.9 + noi * .1;


	}

	return bmp;

}

// Standard function-based bump mapping function.
vec3 doBumpMap(in vec3 p, in vec3 nor, float bumpfactor) {

	const vec2 e = vec2(0.001, 0);
	float ref = bumpSurf3D(p);
	vec3 grad = (vec3(bumpSurf3D(p - e.xyy),
		bumpSurf3D(p - e.yxy),
		bumpSurf3D(p - e.yyx)) - ref) / e.x;

	grad -= nor * dot(nor, grad);

	return normalize(nor + grad * bumpfactor);

}

// Basic raymarcher.
float trace(in vec3 ro, in vec3 rd) {

	float t = 0.0, h;
	for (int i = 0; i < 72; i++) {

		h = map(ro + rd * t);
		// Note the "t*b + a" addition. Basically, we're putting less emphasis on accuracy, as
		// "t" increases. It's a cheap trick that works in most situations... Not all, though.
		if (abs(h) < 0.002*(t*.125 + 1.) || t > FAR) break; // Alternative: 0.001*max(t*.25, 1.)        
		t += step(h, 1.)*h*.2 + h * .5;

	}

	return min(t, FAR);
}

// Standard normal function. It's not as fast as the tetrahedral calculation, but more symmetrical.
vec3 getNormal(in vec3 p) {
	const vec2 e = vec2(0.002, 0);
	return normalize(vec3(map(p + e.xyy) - map(p - e.xyy), map(p + e.yxy) - map(p - e.yxy), map(p + e.yyx) - map(p - e.yyx)));
}

// XT95's really clever, cheap, SSS function. The way I've used it doesn't do it justice,
// so if you'd like to really see it in action, have a look at the following:
//
// Alien Cocoons - XT95: https://www.shadertoy.com/view/MsdGz2
//
float thickness(in vec3 p, in vec3 n, float maxDist, float falloff)
{
	const float nbIte = 6.0;
	float ao = 0.0;

	for (float i = 1.; i < nbIte + .5; i++) {

		float l = (i*.75 + fract(cos(i)*45758.5453)*.25) / nbIte * maxDist;

		ao += (l + map(p - n * l)) / pow(1. + l, falloff);
	}

	return clamp(1. - ao / nbIte, 0., 1.);
}

/*
// Shadows.
float softShadow(vec3 ro, vec3 rd, float start, float end, float k){

float shade = 1.0;
const int maxIterationsShad = 20;

float dist = start;
//float stepDist = end/float(maxIterationsShad);

// Max shadow iterations - More iterations make nicer shadows, but slow things down.
for (int i=0; i<maxIterationsShad; i++){

float h = map(ro + rd*dist);
shade = min(shade, k*h/dist);

// +=h, +=clamp( h, 0.01, 0.25 ), +=min( h, 0.1 ), +=stepDist, +=min(h, stepDist*2.), etc.
dist += clamp( h, 0.01, 0.25);//min(h, stepDist);

// Early exits from accumulative distance function calls tend to be a good thing.
if (h<0.001 || dist > end) break;
}

// Shadow value.
return min(max(shade, 0.) + 0.5, 1.0);
}
*/


// Ambient occlusion, for that self shadowed look. Based on the original by XT95. I love this 
// function, and in many cases, it gives really, really nice results. For a better version, and 
// usage, refer to XT95's examples below:
//
// Hemispherical SDF AO - https://www.shadertoy.com/view/4sdGWN
// Alien Cocoons - https://www.shadertoy.com/view/MsdGz2
float calculateAO(in vec3 p, in vec3 n)
{
	float ao = 0.0, l;
	const float maxDist = 4.;
	const float nbIte = 6.0;
	//const float falloff = 0.9;
	for (float i = 1.; i < nbIte + .5; i++) {

		l = (i + hash(i))*.5 / nbIte * maxDist;

		ao += (l - map(p + n * l)) / (1. + l);// / pow(1.+l, falloff);
	}

	return clamp(1. - ao / nbIte, 0., 1.);
}

/*
/////
// Code block to produce some layers of smokey haze. Not sophisticated at all.
// If you'd like to see a much more sophisticated version, refer to Nitmitz's
// Xyptonjtroz example. Incidently, I wrote this off the top of my head, but
// I did have that example in mind when writing this.

// Hash to return a scalar value from a 3D vector.
float hash31(vec3 p){ return fract(sin(dot(p, vec3(127.1, 311.7, 74.7)))*43758.5453); }

// Four layers of cheap cell tile noise to produce some subtle mist.
// Start at the ray origin, then take four samples of noise between it
// and the surface point. Apply some very simplistic lighting along the
// way. It's not particularly well thought out, but it doesn't have to be.
float getMist(in vec3 ro, in vec3 rd, in vec3 lp, in float t){

float mist = 0.;
ro += rd*t/64.; // Edge the ray a little forward to begin.

for (int i = 0; i<8; i++){
// Lighting. Technically, a lot of these points would be
// shadowed, but we're ignoring that.
float sDi = length(lp-ro)/FAR;
float sAtt = min(1./(1. + sDi*0.25 + sDi*sDi*0.25), 1.);
// Noise layer.
//float n = trigNoise3D(ro/2.);//noise3D(ro/2.)*.66 + noise3D(ro/1.)*.34;
float n = cellTile(ro/2.);
mist += n*sAtt;//trigNoise3D
// Advance the starting point towards the hit point.
ro += rd*t/8.;
}

// Add a little noise, then clamp, and we're done.
return clamp(mist/4. + hash31(ro)*0.2-0.1, 0., 1.);

}
*/

//////
// Simple environment mapping. Pass the reflected vector in and create some
// colored noise with it. The normal is redundant here, but it can be used
// to pass into a 3D texture mapping function to produce some interesting
// environmental reflections.
//
// More sophisticated environment mapping:
// UI easy to integrate - XT95    
// https://www.shadertoy.com/view/ldKSDm
vec3 eMap(vec3 rd, vec3 sn) {


	// Add a time component, scale, then pass into the noise function.
	rd.y += iTime;
	rd /= 3.;

	// Biotube texturing.
	float ct = cellTile(rd*2. + sin(rd*12.)*.5)*.66 + cellTile(rd*6. + sin(rd*36.)*.5)*.34;
	vec3 texCol = (vec3(.25, .2, .15)*(1. - smoothstep(-.1, .3, ct)) + vec3(0.02, 0.02, 0.53) / 6.);
	return smoothstep(0., 1., texCol);

}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {

	// Screen coordinates.
	vec2 uv = (fragCoord - iResolution.xy*0.5) / iResolution.y;

	// Camera Setup.
	vec3 lookAt = vec3(0, 1, iTime*2. + 0.1);  // "Look At" position.
	vec3 camPos = lookAt + vec3(0.0, 0.0, -0.1); // Camera position, doubling as the ray origin.


												 // Light positioning. 
	vec3 lightPos = camPos + vec3(0, .5, 5);// Put it a bit in front of the camera.

											// Using the Z-value to perturb the XY-plane.
											// Sending the camera, "look at," and light vector down the tunnel. The "path" function is 
											// synchronized with the distance function.
	lookAt.xy += path(lookAt.z);
	camPos.xy += path(camPos.z);
	lightPos.xy += path(lightPos.z);

	// Using the above to produce the unit ray-direction vector.
	float FOV = 3.14159265 / 2.; // FOV - Field of view.
	vec3 forward = normalize(lookAt - camPos);
	vec3 right = normalize(vec3(forward.z, 0., -forward.x));
	vec3 up = cross(forward, right);

	// rd - Unit ray direction.
	vec3 rd = normalize(forward + FOV * uv.x*right + FOV * uv.y*up);

	// Lens distortion, if preferable.
	//vec3 rd = (forward + FOV*uv.x*right + FOV*uv.y*up);
	//rd = normalize(vec3(rd.xy, rd.z - dot(rd.xy, rd.xy)*.25));    

	// Swiveling the camera about the XY-plane (from left to right) when turning corners.
	// Naturally, it's synchronized with the path in some kind of way.
	rd.xy = mul(rot2(path(lookAt.z).x / 16.), rd.xy);

	// Standard ray marching routine. I find that some system setups don't like anything other than
	// a "break" statement (by itself) to exit. 
	float t = trace(camPos, rd);

	// Save the object ID just after the "trace" function, since other map calls can change it, which
	// will distort the results.
	saveID = objID;

	// Initialize the scene color.
	vec3 sceneCol = (0);

	// The ray has effectively hit the surface, so light it up.
	if (t < FAR) {


		// Surface position and surface normal.
		vec3 sp = t * rd + camPos;
		vec3 sn = getNormal(sp);


		// Function based bump mapping. Comment it out to see the under layer. It's pretty
		// comparable to regular beveled Voronoi... Close enough, anyway.
		if (saveID > .5) sn = doBumpMap(sp, sn, .2);
		else sn = doBumpMap(sp, sn, .008);

		// Ambient occlusion.
		float ao = calculateAO(sp, sn);

		// Light direction vectors.
		vec3 ld = lightPos - sp;

		// Distance from respective lights to the surface point.
		float distlpsp = max(length(ld), 0.001);

		// Normalize the light direction vectors.
		ld /= distlpsp;

		// Light attenuation, based on the distances above.
		float atten = 1. / (1. + distlpsp * 0.25); // + distlpsp*distlpsp*0.025

												   // Ambient light.
		float ambience = 0.5;

		// Diffuse lighting.
		float diff = max(dot(sn, ld), 0.0);

		// Specular lighting.
		float spec = pow(max(dot(reflect(-ld, sn), -rd), 0.0), 32.);


		// Fresnel term. Good for giving a surface a bit of a reflective glow.
		float fre = pow(clamp(dot(sn, rd) + 1., .0, 1.), 1.);



		// Object texturing and coloring. 
		vec3 texCol;

		if (saveID > .5) { // Tunnel walls.
						 // Two second texture algorithm. Terrible, but it's dark, so no one will notice. :)
			texCol = (.3)*(noise3D(sp*32.)*.66 + noise3D(sp*64.)*.34)*(1. - cellTile(sp*16.)*.75);
			// Darkening the crevices with the bump function. Cheap, but effective.
			texCol *= smoothstep(-.1, .5, cellTile(sp*.75)*.66 + cellTile(sp*1.5)*.34)*.85 + .15;
		}
		else { // The biotubes.
			   // Cheap, sinewy, vein-like covering. Smoothstepping Voronoi is main mechanism involved.
			vec3 sps = sp / 3.;
			float ct = cellTile(sps*2. + sin(sps*12.)*.5)*.66 + cellTile(sps*6. + sin(sps*36.)*.5)*.34;
			texCol = vec3(.35, .25, .2)*(1. - smoothstep(-.1, .25, ct)) + vec3(0.1, 0.01, 0.004);
		}


		/////////   
		// Translucency, courtesy of XT95. See the "thickness" function.
		vec3 hf = normalize(ld + sn);
		float th = thickness(sp, sn, 1., 1.);
		float tdiff = pow(clamp(dot(rd, -hf), 0., 1.), 1.);
		float trans = (tdiff + .0)*th;
		trans = pow(trans, 4.);
		////////        


		// Darkening the crevices. Otherwise known as cheap, scientifically-incorrect shadowing.	
		float shading = 1.;//crv*0.5+0.5; 


						   // Shadows - Better, but they really drain the GPU, so I ramped up the fake shadowing so 
						   // that it's not as noticeable.
						   //shading *= softShadow(sp, ld, 0.05, distlpsp, 8.);

						   // Combining the above terms to produce the final color. It was based more on acheiving a
						   // certain aesthetic than science.
		sceneCol = texCol * (diff + ambience) + vec3(.7, .9, 1.)*spec;// + vec3(.5, .8, 1)*spec2;
		if (saveID < .5) sceneCol += vec3(.7, .9, 1.)*spec*spec;
		sceneCol += texCol * vec3(.8, .95, 1)*pow(fre, 4.)*2.;
		sceneCol += vec3(1, .07, .15)*trans*1.5;


		// Fake reflection and refraction on the biotubes. Not a proper reflective and 
		// refractive pass, but it does a reasonable job, and is much cheaper.
		vec3 ref, em;

		if (saveID < .5) { // Biotubes.

						 // Fake reflection and refraction to give a bit of a fluid look, albeit
						 // in a less than physically correct fashion.
			ref = reflect(rd, sn);
			em = eMap(ref, sn);
			sceneCol += em * .5;
			ref = refract(rd, sn, 1. / 1.3);//svn*.5 + n*.5
			em = eMap(ref, sn);
			sceneCol += em * vec3(2, .2, .3)*1.5;
		}

		// Shading.
		sceneCol *= atten * shading*ao;


	}

	// Blend the scene and the background; It's commented out, but you could also integrate some some 
	// very basic, 8-layered smokey haze.
	//float mist = getMist(camPos, rd, lightPos, t);
	vec3 sky = vec3(2., .9, .8);//* mix(1., .75, mist);//*(rd.y*.25 + 1.);
	sceneCol = mix(sky, sceneCol, 1. / (t*t / FAR / FAR * 8. + 1.));

	// Clamp and present the pixel to the screen.
	fragColor = vec4(/*sqrt*/(clamp(sceneCol, 0., 1.)), 1.0);

}
#endif

#if SHADERTOY == 20
//Basic fractal by @paulofalcao

static const int maxIterations = 6;//a nice value for fullscreen is 8

static float circleSize = 1.0 / (3.0*pow(2.0, float(maxIterations)));

//generic rotation formula
vec2 rot(vec2 uv, float a) {
	return vec2(uv.x*cos(a) - uv.y*sin(a), uv.y*cos(a) + uv.x*sin(a));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	//normalize stuff
	vec2 uv = iResolution.xy; uv = -.5*(uv - 2.0*fragCoord.xy) / uv.x;

	//global rotation and zoom
	uv = rot(uv, iTime);
	uv *= sin(iTime)*0.5 + 1.5;

	//mirror, rotate and scale 6 times...
	float s = 0.3;
	for (int i = 0; i < maxIterations; i++) {
		uv = abs(uv) - s;
		uv = rot(uv, iTime);
		s = s / 2.1;
	}

	//draw a circle
	float c = length(uv) > circleSize ? 0.0 : 1.0;

	fragColor = vec4(c, c, c, 1.0);
}
#endif

#if SHADERTOY == 21
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// These are 316 gaussian points (201 brushes by using local symmetry) forming the picture
// of Audrey Hepburn. They pack down to 800 bytes (34 bits per point) before huffman/arithmetic
// compression.
//
// The points were chosen by (very quickly) runing this 
//
// http://www.iquilezles.org/www/articles/genetic/genetic.htm
//
// with some importance sampling for the eyes, nouse and mouth.

float brush(float col, vec2 p, in vec4 b, float an)
{
	p += an * cos(iTime + 100.0*b.yz);

	vec2 dd = p - b.yz;
	col = mix(col, b.x, exp(-b.w*b.w*dot(dd, dd)));
	if (abs(b.z - 0.5) < 0.251)
	{
		dd.x = p.x - 1.0 + b.y;
		col = mix(col, b.x, exp(-b.w*b.w*dot(dd, dd)));
	}
	return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord.xy / iResolution.y;
	//uv.y = 1.0 - uv.y;

	uv.x -= 0.5*(iResolution.x / iResolution.y - 1.0);

	float an = smoothstep(0.0, 1.0, cos(iTime));

	float col = 0.0;

	col = brush(col, uv, vec4(1.000, 0.371, 0.379, 11.770), an);
	col = brush(col, uv, vec4(0.992, 0.545, 0.551, 8.359), an);
	col = brush(col, uv, vec4(0.749, 0.623, 0.990, 36.571), an);
	col = brush(col, uv, vec4(1.000, 0.510, 0.395, 11.315), an);
	col = brush(col, uv, vec4(1.000, 0.723, 0.564, 15.170), an);
	col = brush(col, uv, vec4(0.953, 0.729, 0.750, 14.629), an);
	col = brush(col, uv, vec4(0.706, 0.982, 0.033, 16.254), an);
	col = brush(col, uv, vec4(1.000, 0.855, 0.652, 26.256), an);
	col = brush(col, uv, vec4(1.000, 0.664, 0.623, 81.920), an);
	col = brush(col, uv, vec4(0.000, 0.881, 0.750, 8.031), an);
	col = brush(col, uv, vec4(0.686, 0.682, 0.900, 27.676), an);
	col = brush(col, uv, vec4(1.000, 0.189, 0.684, 18.618), an);
	col = brush(col, uv, vec4(0.000, 0.904, 0.750, 8.031), an);
	col = brush(col, uv, vec4(1.000, 0.422, 0.195, 44.522), an);
	col = brush(col, uv, vec4(1.000, 0.779, 0.750, 16.787), an);
	col = brush(col, uv, vec4(1.000, 0.645, 0.330, 14.222), an);
	col = brush(col, uv, vec4(1.000, 0.197, 0.648, 22.505), an);
	col = brush(col, uv, vec4(0.702, 0.512, 0.393, 35.310), an);
	col = brush(col, uv, vec4(1.000, 0.744, 0.621, 14.949), an);
	col = brush(col, uv, vec4(0.671, 0.885, 0.092, 24.675), an);
	col = brush(col, uv, vec4(0.000, 0.344, 0.750, 8.031), an);
	col = brush(col, uv, vec4(1.000, 0.760, 0.465, 40.960), an);
	col = brush(col, uv, vec4(0.008, 0.908, 0.311, 8.031), an);
	col = brush(col, uv, vec4(0.016, 0.959, 0.750, 10.039), an);
	col = brush(col, uv, vec4(0.004, 0.930, 0.750, 12.800), an);
	col = brush(col, uv, vec4(1.000, 0.555, 0.250, 19.883), an);
	col = brush(col, uv, vec4(1.000, 0.770, 1.018, 15.876), an);
	col = brush(col, uv, vec4(0.000, 0.828, 0.756, 36.571), an);
	col = brush(col, uv, vec4(0.580, 0.566, 0.424, 89.043), an);
	col = brush(col, uv, vec4(0.988, 0.162, 0.691, 40.157), an);
	col = brush(col, uv, vec4(0.000, 0.314, 0.750, 8.031), an);
	col = brush(col, uv, vec4(0.000, 0.947, 0.125, 32.000), an);
	col = brush(col, uv, vec4(0.914, 0.844, 0.725, 52.513), an);
	col = brush(col, uv, vec4(1.000, 0.313, 0.762, 42.667), an);
	col = brush(col, uv, vec4(0.996, 0.676, 0.689, 85.333), an);
	col = brush(col, uv, vec4(0.980, 0.346, 0.559, 24.675), an);
	col = brush(col, uv, vec4(1.000, 0.553, 0.250, 18.789), an);
	col = brush(col, uv, vec4(0.004, 0.258, 0.248, 8.031), an);
	col = brush(col, uv, vec4(1.000, 0.420, 0.742, 30.567), an);
	col = brush(col, uv, vec4(0.906, 0.543, 0.250, 22.756), an);
	col = brush(col, uv, vec4(0.863, 0.674, 0.322, 20.078), an);
	col = brush(col, uv, vec4(0.753, 0.357, 0.686, 78.769), an);
	col = brush(col, uv, vec4(0.906, 0.795, 0.705, 37.236), an);
	col = brush(col, uv, vec4(0.933, 0.520, 0.365, 38.642), an);
	col = brush(col, uv, vec4(0.996, 0.318, 0.488, 14.734), an);
	col = brush(col, uv, vec4(0.337, 0.486, 0.281, 81.920), an);
	col = brush(col, uv, vec4(0.965, 0.691, 0.516, 16.650), an);
	col = brush(col, uv, vec4(0.808, 0.582, 0.973, 52.513), an);
	col = brush(col, uv, vec4(0.012, 0.240, 0.928, 8.063), an);
	col = brush(col, uv, vec4(1.000, 0.496, 0.217, 31.508), an);
	col = brush(col, uv, vec4(0.000, 0.658, 0.953, 34.133), an);
	col = brush(col, uv, vec4(0.871, 0.582, 0.172, 62.061), an);
	col = brush(col, uv, vec4(0.855, 0.346, 0.342, 17.504), an);
	col = brush(col, uv, vec4(0.878, 0.787, 0.648, 28.845), an);
	col = brush(col, uv, vec4(0.000, 0.984, 0.111, 35.310), an);
	col = brush(col, uv, vec4(0.855, 0.514, 0.965, 66.065), an);
	col = brush(col, uv, vec4(0.561, 0.613, 0.350, 81.920), an);
	col = brush(col, uv, vec4(0.992, 0.818, 0.902, 21.558), an);
	col = brush(col, uv, vec4(0.914, 0.746, 0.615, 40.157), an);
	col = brush(col, uv, vec4(0.557, 0.580, 0.125, 60.235), an);
	col = brush(col, uv, vec4(0.475, 0.547, 0.414, 70.621), an);
	col = brush(col, uv, vec4(0.843, 0.680, 0.793, 20.277), an);
	col = brush(col, uv, vec4(1.000, 0.230, 0.758, 56.889), an);
	col = brush(col, uv, vec4(1.000, 0.299, 0.691, 68.267), an);
	col = brush(col, uv, vec4(0.737, 0.518, 0.100, 68.267), an);
	col = brush(col, uv, vec4(0.996, 0.227, 0.514, 41.796), an);
	col = brush(col, uv, vec4(0.929, 0.850, 0.770, 62.061), an);
	col = brush(col, uv, vec4(0.682, 0.834, 0.111, 30.118), an);
	col = brush(col, uv, vec4(0.996, 0.854, 0.793, 58.514), an);
	col = brush(col, uv, vec4(0.490, 0.736, 0.889, 19.321), an);
	col = brush(col, uv, vec4(0.980, 0.465, 0.725, 16.126), an);
	col = brush(col, uv, vec4(0.992, 0.484, 1.010, 23.273), an);
	col = brush(col, uv, vec4(0.008, 0.949, 0.727, 23.540), an);
	col = brush(col, uv, vec4(0.012, 0.086, 0.086, 8.031), an);
	col = brush(col, uv, vec4(1.000, 0.121, 0.750, 44.522), an);
	col = brush(col, uv, vec4(0.427, 0.617, 0.891, 27.676), an);
	col = brush(col, uv, vec4(0.804, 0.693, 0.633, 78.769), an);
	col = brush(col, uv, vec4(0.012, 0.711, 0.084, 13.745), an);
	col = brush(col, uv, vec4(0.082, 0.584, 0.338, 107.789), an);
	col = brush(col, uv, vec4(0.929, 0.613, 0.268, 19.692), an);
	col = brush(col, uv, vec4(0.200, 0.549, 0.420, 128.000), an);
	col = brush(col, uv, vec4(1.000, 0.402, 0.717, 26.947), an);
	col = brush(col, uv, vec4(0.000, 0.551, 0.168, 45.511), an);
	col = brush(col, uv, vec4(0.992, 0.627, 0.621, 56.889), an);
	col = brush(col, uv, vec4(0.902, 0.361, 0.748, 40.960), an);
	col = brush(col, uv, vec4(0.984, 0.344, 0.754, 38.642), an);
	col = brush(col, uv, vec4(0.902, 0.203, 0.818, 51.200), an);
	col = brush(col, uv, vec4(1.000, 0.230, 0.803, 52.513), an);
	col = brush(col, uv, vec4(0.922, 0.738, 0.691, 47.628), an);
	col = brush(col, uv, vec4(0.000, 0.385, 0.797, 43.574), an);
	col = brush(col, uv, vec4(0.000, 0.725, 0.305, 62.061), an);
	col = brush(col, uv, vec4(0.000, 0.150, 0.750, 45.511), an);
	col = brush(col, uv, vec4(1.000, 0.742, 0.408, 47.628), an);
	col = brush(col, uv, vec4(0.000, 0.645, 0.643, 60.235), an);
	col = brush(col, uv, vec4(1.000, 0.645, 0.438, 35.310), an);
	col = brush(col, uv, vec4(0.510, 0.564, 0.789, 18.450), an);
	col = brush(col, uv, vec4(0.863, 0.211, 0.781, 30.567), an);
	col = brush(col, uv, vec4(0.106, 0.508, 0.328, 89.043), an);
	col = brush(col, uv, vec4(0.012, 0.410, 0.875, 14.629), an);
	col = brush(col, uv, vec4(1.000, 0.871, 0.877, 48.762), an);
	col = brush(col, uv, vec4(1.000, 0.258, 0.779, 37.926), an);
	col = brush(col, uv, vec4(0.000, 0.436, 0.807, 28.845), an);
	col = brush(col, uv, vec4(0.918, 0.861, 0.836, 49.951), an);
	col = brush(col, uv, vec4(1.000, 0.291, 0.770, 40.960), an);
	col = brush(col, uv, vec4(0.000, 0.750, 0.283, 27.676), an);
	col = brush(col, uv, vec4(0.965, 0.596, 0.572, 28.055), an);
	col = brush(col, uv, vec4(0.902, 0.803, 0.953, 24.976), an);
	col = brush(col, uv, vec4(0.957, 0.498, 0.600, 16.126), an);
	col = brush(col, uv, vec4(0.914, 0.322, 0.432, 15.634), an);
	col = brush(col, uv, vec4(0.008, 0.025, 0.621, 17.809), an);
	col = brush(col, uv, vec4(0.000, 0.916, 0.713, 56.889), an);
	col = brush(col, uv, vec4(0.914, 0.547, 0.971, 47.628), an);
	col = brush(col, uv, vec4(0.000, 0.207, 0.432, 37.926), an);
	col = brush(col, uv, vec4(0.875, 0.176, 0.793, 46.545), an);
	col = brush(col, uv, vec4(0.000, 0.646, 0.668, 41.796), an);
	col = brush(col, uv, vec4(1.000, 0.721, 0.691, 51.200), an);
	col = brush(col, uv, vec4(0.451, 0.559, 0.754, 49.951), an);
	col = brush(col, uv, vec4(0.969, 0.846, 0.750, 58.514), an);
	col = brush(col, uv, vec4(0.000, 0.900, 0.146, 36.571), an);
	col = brush(col, uv, vec4(1.000, 0.613, 0.635, 85.333), an);
	col = brush(col, uv, vec4(0.596, 0.807, 0.150, 58.514), an);
	col = brush(col, uv, vec4(0.898, 0.330, 0.760, 40.157), an);
	col = brush(col, uv, vec4(0.694, 0.594, 0.012, 51.200), an);
	col = brush(col, uv, vec4(0.698, 0.592, 0.055, 53.895), an);
	col = brush(col, uv, vec4(0.902, 0.268, 0.773, 39.385), an);
	col = brush(col, uv, vec4(0.925, 0.838, 0.660, 58.514), an);
	col = brush(col, uv, vec4(0.843, 0.670, 0.242, 28.444), an);
	col = brush(col, uv, vec4(0.243, 0.465, 0.285, 85.333), an);
	col = brush(col, uv, vec4(0.816, 0.588, 0.674, 44.522), an);
	col = brush(col, uv, vec4(0.008, 0.283, 0.115, 8.031), an);
	col = brush(col, uv, vec4(0.247, 0.414, 0.691, 60.235), an);
	col = brush(col, uv, vec4(1.000, 0.104, 0.781, 60.235), an);
	col = brush(col, uv, vec4(0.000, 0.619, 0.660, 60.235), an);
	col = brush(col, uv, vec4(0.584, 0.650, 0.994, 46.545), an);
	col = brush(col, uv, vec4(0.000, 0.219, 0.393, 36.571), an);
	col = brush(col, uv, vec4(1.000, 0.307, 0.645, 97.524), an);
	col = brush(col, uv, vec4(0.953, 0.639, 0.771, 38.642), an);
	col = brush(col, uv, vec4(0.000, 0.238, 0.357, 34.712), an);
	col = brush(col, uv, vec4(0.922, 0.713, 0.352, 53.895), an);
	col = brush(col, uv, vec4(0.965, 0.387, 0.748, 43.574), an);
	col = brush(col, uv, vec4(0.000, 0.898, 0.633, 41.796), an);
	col = brush(col, uv, vec4(0.941, 0.352, 0.488, 14.734), an);
	col = brush(col, uv, vec4(0.933, 0.439, 0.725, 30.567), an);
	col = brush(col, uv, vec4(0.310, 0.541, 0.906, 47.628), an);
	col = brush(col, uv, vec4(0.941, 0.502, 0.689, 24.094), an);
	col = brush(col, uv, vec4(0.094, 0.527, 0.330, 85.333), an);
	col = brush(col, uv, vec4(0.000, 0.090, 0.688, 55.351), an);
	col = brush(col, uv, vec4(0.000, 0.652, 0.713, 75.852), an);
	col = brush(col, uv, vec4(0.949, 0.320, 0.623, 107.789), an);
	col = brush(col, uv, vec4(0.890, 0.775, 0.750, 22.505), an);
	col = brush(col, uv, vec4(0.012, 0.918, 0.490, 14.322), an);
	col = brush(col, uv, vec4(1.000, 0.871, 0.967, 58.514), an);
	col = brush(col, uv, vec4(0.000, 0.324, 0.676, 64.000), an);
	col = brush(col, uv, vec4(0.008, 0.141, 0.248, 8.031), an);
	col = brush(col, uv, vec4(0.000, 0.633, 0.707, 75.852), an);
	col = brush(col, uv, vec4(0.910, 0.385, 0.207, 44.522), an);
	col = brush(col, uv, vec4(0.012, 0.703, 0.182, 31.508), an);
	col = brush(col, uv, vec4(0.000, 0.617, 0.703, 73.143), an);
	col = brush(col, uv, vec4(0.890, 0.352, 0.225, 45.511), an);
	col = brush(col, uv, vec4(0.933, 0.826, 0.604, 44.522), an);
	col = brush(col, uv, vec4(0.914, 0.777, 0.574, 25.924), an);
	col = brush(col, uv, vec4(0.631, 0.781, 0.182, 68.267), an);
	col = brush(col, uv, vec4(1.000, 0.873, 0.916, 48.762), an);
	col = brush(col, uv, vec4(0.694, 0.520, 0.113, 81.920), an);
	col = brush(col, uv, vec4(0.000, 0.900, 0.926, 58.514), an);
	col = brush(col, uv, vec4(0.184, 0.598, 0.344, 146.286), an);
	col = brush(col, uv, vec4(0.863, 0.678, 0.250, 35.310), an);
	col = brush(col, uv, vec4(0.090, 0.566, 0.332, 78.769), an);
	col = brush(col, uv, vec4(0.420, 0.445, 0.301, 56.889), an);
	col = brush(col, uv, vec4(0.973, 0.617, 0.516, 18.124), an);
	col = brush(col, uv, vec4(0.000, 0.191, 0.500, 39.385), an);
	col = brush(col, uv, vec4(0.000, 0.240, 0.326, 31.508), an);
	col = brush(col, uv, vec4(0.000, 0.264, 0.322, 55.351), an);
	col = brush(col, uv, vec4(0.000, 0.604, 0.699, 70.621), an);
	col = brush(col, uv, vec4(0.000, 0.113, 0.604, 43.574), an);
	col = brush(col, uv, vec4(0.894, 0.760, 0.697, 49.951), an);
	col = brush(col, uv, vec4(0.914, 0.725, 0.383, 55.351), an);
	col = brush(col, uv, vec4(0.000, 0.199, 0.467, 48.762), an);
	col = brush(col, uv, vec4(0.000, 0.904, 0.660, 52.513), an);
	col = brush(col, uv, vec4(0.922, 0.611, 0.191, 45.511), an);
	col = brush(col, uv, vec4(0.059, 0.789, 0.869, 30.118), an);
	col = brush(col, uv, vec4(0.976, 0.641, 0.213, 40.960), an);
	col = brush(col, uv, vec4(0.918, 0.402, 0.742, 47.628), an);
	col = brush(col, uv, vec4(0.945, 0.717, 0.582, 40.157), an);
	col = brush(col, uv, vec4(0.000, 0.299, 0.672, 58.514), an);
	col = brush(col, uv, vec4(0.000, 0.719, 0.666, 48.762), an);
	col = brush(col, uv, vec4(0.882, 0.697, 0.271, 58.514), an);
	col = brush(col, uv, vec4(0.929, 0.752, 0.436, 64.000), an);
	col = brush(col, uv, vec4(1.000, 0.867, 0.813, 56.889), an);
	col = brush(col, uv, vec4(0.643, 0.588, 0.090, 64.000), an);
	col = brush(col, uv, vec4(0.012, 0.063, 0.922, 10.952), an);
	col = brush(col, uv, vec4(0.878, 0.186, 0.750, 31.508), an);
	col = brush(col, uv, vec4(0.953, 0.648, 0.613, 120.471), an);
	col = brush(col, uv, vec4(0.973, 0.180, 0.576, 45.511), an);
	col = brush(col, uv, vec4(0.741, 0.943, 0.076, 52.513), an);
	col = brush(col, uv, vec4(0.059, 0.545, 0.332, 89.043), an);
	col = brush(col, uv, vec4(0.094, 0.295, 0.734, 85.333), an);
	col = brush(col, uv, vec4(0.008, 0.676, 0.721, 85.333), an);
	col = brush(col, uv, vec4(0.550, 0.350, 0.650, 85.000), an);

	fragColor = vec4(col, col, col, 1.0);
}

#endif

#if SHADERTOY == 22
// Source edited by David Hoskins - 2013.

// I took and completed this http://glsl.heroku.com/e#9743.20 - just for fun! 8|
// Locations in 3x7 font grid, inspired by http://www.claudiocc.com/the-1k-notebook-part-i/
// Had to edit it to remove some duplicate lines.
// ABC  a:GIOMJL b:AMOIG c:IGMO d:COMGI e:OMGILJ f:CBN g:OMGIUS h:AMGIO i:EEHN j:GHTS k:AMIKO l:BN m:MGHNHIO n:MGIO
// DEF  o:GIOMG p:SGIOM q:UIGMO r:MGI s:IGJLOM t:BNO u:GMOI v:GJNLI w:GMNHNOI x:GOKMI y:GMOIUS z:GIMO
// GHI
// JKL 
// MNO
// PQR
// STU

static vec2 coord;

#define font_size 20. 
#define font_spacing .05
#define STROKEWIDTH 0.05
#define PI 3.14159265359

#define A_ vec2(0.,0.)
#define B_ vec2(1.,0.)
#define C_ vec2(2.,0.)

//#define D_ vec2(0.,1.)
#define E_ vec2(1.,1.)
//#define F_ vec2(2.,1.)

#define G_ vec2(0.,2.)
#define H_ vec2(1.,2.)
#define I_ vec2(2.,2.)

#define J_ vec2(0.,3.)
#define K_ vec2(1.,3.)
#define L_ vec2(2.,3.)

#define M_ vec2(0.,4.)
#define N_ vec2(1.,4.)
#define O_ vec2(2.,4.)

//#define P_ vec2(0.,5.)
//#define Q_ vec2(1.,5.)
//#define R_ vec2(1.,5.)

#define S_ vec2(0.,6.)
#define T_ vec2(1.,6.)
#define U_ vec2(2.0,6.)

#define A(p) t(G_,I_,p) + t(I_,O_,p) + t(O_,M_, p) + t(M_,J_,p) + t(J_,L_,p)
#define B(p) t(A_,M_,p) + t(M_,O_,p) + t(O_,I_, p) + t(I_,G_,p)
#define C(p) t(I_,G_,p) + t(G_,M_,p) + t(M_,O_,p) 
#define D(p) t(C_,O_,p) + t(O_,M_,p) + t(M_,G_,p) + t(G_,I_,p)
#define E(p) t(O_,M_,p) + t(M_,G_,p) + t(G_,I_,p) + t(I_,L_,p) + t(L_,J_,p)
#define F(p) t(C_,B_,p) + t(B_,N_,p) + t(G_,I_,p)
#define G(p) t(O_,M_,p) + t(M_,G_,p) + t(G_,I_,p) + t(I_,U_,p) + t(U_,S_,p)
#define H(p) t(A_,M_,p) + t(G_,I_,p) + t(I_,O_,p) 
#define I(p) t(E_,E_,p) + t(H_,N_,p) 
#define J(p) t(E_,E_,p) + t(H_,T_,p) + t(T_,S_,p)
#define K(p) t(A_,M_,p) + t(M_,I_,p) + t(K_,O_,p)
#define L(p) t(B_,N_,p)
#define M(p) t(M_,G_,p) + t(G_,I_,p) + t(H_,N_,p) + t(I_,O_,p)
#define N(p) t(M_,G_,p) + t(G_,I_,p) + t(I_,O_,p)
#define O(p) t(G_,I_,p) + t(I_,O_,p) + t(O_,M_, p) + t(M_,G_,p)
#define P(p) t(S_,G_,p) + t(G_,I_,p) + t(I_,O_,p) + t(O_,M_, p)
#define Q(p) t(U_,I_,p) + t(I_,G_,p) + t(G_,M_,p) + t(M_,O_, p)
#define R(p) t(M_,G_,p) + t(G_,I_,p)
#define S(p) t(I_,G_,p) + t(G_,J_,p) + t(J_,L_,p) + t(L_,O_,p) + t(O_,M_,p)
#define T(p) t(B_,N_,p) + t(N_,O_,p) + t(G_,I_,p)
#define U(p) t(G_,M_,p) + t(M_,O_,p) + t(O_,I_,p)
#define V(p) t(G_,J_,p) + t(J_,N_,p) + t(N_,L_,p) + t(L_,I_,p)
#define W(p) t(G_,M_,p) + t(M_,O_,p) + t(N_,H_,p) + t(O_,I_,p)
#define X(p) t(G_,O_,p) + t(I_,M_,p)
#define Y(p) t(G_,M_,p) + t(M_,O_,p) + t(I_,U_,p) + t(U_,S_,p)
#define Z(p) t(G_,I_,p) + t(I_,M_,p) + t(M_,O_,p)
#define STOP(p) t(N_,N_,p)

static vec2 caret_origin = vec2(3.0, .7);
static vec2 caret;

//-----------------------------------------------------------------------------------
float minimum_distance(vec2 v, vec2 w, vec2 p)
{	// Return minimum distance between line segment vw and point p
	float l2 = (v.x - w.x)*(v.x - w.x) + (v.y - w.y)*(v.y - w.y); //length_squared(v, w);  // i.e. |w-v|^2 -  avoid a sqrt
	if (l2 == 0.0) {
		return distance(p, v);   // v == w case
	}

	// Consider the line extending the segment, parameterized as v + t (w - v).
	// We find projection of point p onto the line.  It falls where t = [(p-v) . (w-v)] / |w-v|^2
	float t = dot(p - v, w - v) / l2;
	if (t < 0.0) {
		// Beyond the 'v' end of the segment
		return distance(p, v);
	}
	else if (t > 1.0) {
		return distance(p, w);  // Beyond the 'w' end of the segment
	}
	vec2 projection = v + t * (w - v);  // Projection falls on the segment
	return distance(p, projection);
}

//-----------------------------------------------------------------------------------
float textColor(vec2 from, vec2 to, vec2 p)
{
	p *= font_size;
	float inkNess = 0., nearLine, corner;
	nearLine = minimum_distance(from, to, p); // basic distance from segment, thanks http://glsl.heroku.com/e#6140.0
	inkNess += smoothstep(0., 1., 1. - 14.*(nearLine - STROKEWIDTH)); // ugly still
	inkNess += smoothstep(0., 2.5, 1. - (nearLine + 5. * STROKEWIDTH)); // glow
	return inkNess;
}

//-----------------------------------------------------------------------------------
vec2 grid(vec2 letterspace)
{
	return (vec2((letterspace.x / 2.) * .65, 1.0 - ((letterspace.y / 2.) * .95)));
}

//-----------------------------------------------------------------------------------
static float count = 0.0;
static float gtime;
float t(vec2 from, vec2 to, vec2 p)
{
	count++;
	if (count > gtime*20.0) return 0.0;
	return textColor(grid(from), grid(to), p);
}

//-----------------------------------------------------------------------------------
vec2 r()
{
	vec2 pos = coord.xy / iResolution.xy;
	pos.y -= caret.y;
	pos.x -= font_spacing * caret.x;
	return pos;
}

//-----------------------------------------------------------------------------------
void add()
{
	caret.x += 1.0;
}

//-----------------------------------------------------------------------------------
void space()
{
	caret.x += 1.5;
}

//-----------------------------------------------------------------------------------
void newline()
{
	caret.x = caret_origin.x;
	caret.y -= .18;
}

//-----------------------------------------------------------------------------------
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	float time = mod(iTime, 11.0);
	gtime = time;

	float d = 0.;
	vec3 col = vec3(0.1, .07 + 0.07*(.5 + sin(fragCoord.y*3.14159*1.1 + time * 2.0)) + sin(fragCoord.y*.01 + time + 2.5)*0.05, 0.1);

	coord = fragCoord;

	caret = caret_origin;

	// the quick brown fox jumps over the lazy dog...
	d += T(r()); add(); d += H(r()); add(); d += E(r()); space();
	d += Q(r()); add(); d += U(r()); add(); d += I(r()); add(); d += C(r()); add(); d += K(r()); space();
	d += B(r()); add(); d += R(r()); add(); d += O(r()); add(); d += W(r()); add(); d += N(r()); space();
	newline();
	d += F(r()); add(); d += O(r()); add(); d += X(r()); space();
	d += J(r()); add(); d += U(r()); add(); d += M(r()); add(); d += P(r()); add(); d += S(r()); space();
	d += O(r()); add(); d += V(r()); add(); d += E(r()); add(); d += R(r()); space();
	newline();
	d += T(r()); add(); d += H(r()); add(); d += E(r()); space();
	d += L(r()); add(); d += A(r()); add(); d += Z(r()); add(); d += Y(r()); space();
	d += D(r()); add(); d += O(r()); add(); d += G(r()); add(); d += STOP(r()); add(); d += STOP(r()); add(); d += STOP(r());
	d = clamp(d* (.75 + sin(fragCoord.x*PI*.5 - time * 4.3)*.5), 0.0, 1.0);

	col += vec3(d*.5, d, d*.85);
	vec2 xy = fragCoord.xy / iResolution.xy;
	col *= vec3(.4, .4, .3) + 0.5*pow(100.0*xy.x*xy.y*(1.0 - xy.x)*(1.0 - xy.y), .4);
	fragColor = vec4(col, 1.0);
}
#endif

#if SHADERTOY == 23
static const float rings = 5.0;	//exactly the number of complete white rings at any moment.
static const float velocity = 4.;
static const float b = 0.003;		//size of the smoothed border

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 position = fragCoord.xy / iResolution.xy;
	float aspect = iResolution.x / iResolution.y;
	position.x *= aspect;
	float dist = distance(position, vec2(aspect*0.5, 0.5));
	float offset = iTime * velocity;
	float conv = rings * 4.;
	float v = dist * conv - offset;
	float ringr = floor(v);
	float color = smoothstep(-b, b, abs(dist - (ringr + float(fract(v) > 0.5) + offset) / conv));
	if (mod(ringr, 2.) == 1.)
		color = 1. - color;
	fragColor = vec4(color, color, color, 1.);
}
#endif

#if SHADERTOY == 24
// Blobs by @paulofalcao

#define time iTime

float makePoint(float x, float y, float fx, float fy, float sx, float sy, float t) {
	float xx = x + sin(t*fx)*sx;
	float yy = y + cos(t*fy)*sy;
	return 1.0 / sqrt(xx*xx + yy * yy);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {

	vec2 p = (fragCoord.xy / iResolution.x)*2.0 - vec2(1.0, iResolution.y / iResolution.x);

	p = p * 2.0;

	float x = p.x;
	float y = p.y;

	float a =
		makePoint(x, y, 3.3, 2.9, 0.3, 0.3, time);
	a = a + makePoint(x, y, 1.9, 2.0, 0.4, 0.4, time);
	a = a + makePoint(x, y, 0.8, 0.7, 0.4, 0.5, time);
	a = a + makePoint(x, y, 2.3, 0.1, 0.6, 0.3, time);
	a = a + makePoint(x, y, 0.8, 1.7, 0.5, 0.4, time);
	a = a + makePoint(x, y, 0.3, 1.0, 0.4, 0.4, time);
	a = a + makePoint(x, y, 1.4, 1.7, 0.4, 0.5, time);
	a = a + makePoint(x, y, 1.3, 2.1, 0.6, 0.3, time);
	a = a + makePoint(x, y, 1.8, 1.7, 0.5, 0.4, time);

	float b =
		makePoint(x, y, 1.2, 1.9, 0.3, 0.3, time);
	b = b + makePoint(x, y, 0.7, 2.7, 0.4, 0.4, time);
	b = b + makePoint(x, y, 1.4, 0.6, 0.4, 0.5, time);
	b = b + makePoint(x, y, 2.6, 0.4, 0.6, 0.3, time);
	b = b + makePoint(x, y, 0.7, 1.4, 0.5, 0.4, time);
	b = b + makePoint(x, y, 0.7, 1.7, 0.4, 0.4, time);
	b = b + makePoint(x, y, 0.8, 0.5, 0.4, 0.5, time);
	b = b + makePoint(x, y, 1.4, 0.9, 0.6, 0.3, time);
	b = b + makePoint(x, y, 0.7, 1.3, 0.5, 0.4, time);

	float c =
		makePoint(x, y, 3.7, 0.3, 0.3, 0.3, time);
	c = c + makePoint(x, y, 1.9, 1.3, 0.4, 0.4, time);
	c = c + makePoint(x, y, 0.8, 0.9, 0.4, 0.5, time);
	c = c + makePoint(x, y, 1.2, 1.7, 0.6, 0.3, time);
	c = c + makePoint(x, y, 0.3, 0.6, 0.5, 0.4, time);
	c = c + makePoint(x, y, 0.3, 0.3, 0.4, 0.4, time);
	c = c + makePoint(x, y, 1.4, 0.8, 0.4, 0.5, time);
	c = c + makePoint(x, y, 0.2, 0.6, 0.6, 0.3, time);
	c = c + makePoint(x, y, 1.3, 0.5, 0.5, 0.4, time);

	vec3 d = vec3(a, b, c) / 32.0;

	fragColor = vec4(d.x, d.y, d.z, (d.x + d.y + d.z) / 3.0);
}
#endif

#if SHADERTOY == 25
#define SCALE 20.
#define SPEED 9.
#define FREQUENCY .3

static float d;
#define C(p)  min(1., sqrt(10.*abs(length(p-.5)-.4)))
#define D(p,o)  ( (d=length(p-o)*5.)<=.6 ? d:1. )
/*
void mainImage( out vec4 O, in vec2 U )    // compact version by FabriceNeyret2 (having problems with it on windows)
{
vec2 R = iResolution.xy,
p = SCALE*(U+U/R)/R.y,
i = ceil(p);
O -= O - C(i-p) * D(i-p, .5 + .4 * sin( iDate.w*SPEED + (i.x+i.y)*FREQUENCY + vec2(1.6,0) ));
}*/

void mainImage(out vec4 O, in vec2 U)
{
	vec2 R = iResolution.xy,
		p = SCALE * (U + U / R) / R.y,
		f = fract(p);
	p = floor(p);
	float t = (p.x + p.y)*FREQUENCY
		+ iTime * SPEED;
	vec2 o = vec2(cos(t), sin(t))*.4 + .5;
	O.xyz = (C(f)*D(f, o));
	O.w = 1;
}
#endif

#if SHADERTOY == 26
// Created by Daniel Burke - burito/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// Inspiration from Dr Who (2005) S7E13 - The Name of the Doctor

vec2 rot(vec2 p, float a)
{
	float c = cos(a);
	float s = sin(a);
	return vec2(p.x*c + p.y*s,
		-p.x*s + p.y*c);
}

float circle(vec2 pos, float radius)
{
	return clamp(((1.0 - abs(length(pos) - radius)) - 0.99)*100.0, 0.0, 1.0);

}

float circleFill(vec2 pos, float radius)
{
	return clamp(((1.0 - (length(pos) - radius)) - 0.99)*100.0, 0.0, 1.0);
}

// Thanks Inigo Quilez!
float Line(in vec2 p, in vec2 a, in vec2 b)
{
	vec2 pa = -p - a;
	vec2 ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	float d = length(pa - ba * h);

	return clamp(((1.0 - d) - 0.99)*100.0, 0.0, 1.0);
}

// for posterity, the original evil function
float EvilLine(vec2 pos, vec2 start, vec2 finish)
{
	vec2 delta = finish - start;
	vec2 n = normalize(delta);
	float l = length(delta);
	float d = sign(n.x);

	float angle = atan(n.y, n.x);
	vec2 t = rot(-pos - start, angle);

	float s = d < 0.0 ? 0.0 : d * l;
	float f = d < 0.0 ? d * l : 0.0;
	if (t.x > s || t.x < f)return 0.0;

	return clamp(((1.0 - abs(t.y)) - 0.99)*100.0, 0.0, 1.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord.xy / iResolution.xy;
	vec2 p = -1.0 + 2.0 * uv;
	p.x *= iResolution.x / iResolution.y;

	vec3 colour = (0);
	vec3 white = (1);



	float c = circle(p, 0.2);
	c += circle(p, 0.1);
	c += circle(p, 0.18);
	c += circleFill(p, 0.005);

	//    c += circle(p, 1.3);
	c += circle(p, 1.0);
	if (p.x > 0.0)c += circle(p, 0.4);
	if (p.x > 0.0)c += circle(p, 0.42);
	if (p.x < 0.0)c += circle(p, 0.47);
	c += circleFill(p + vec2(0.47, 0.0), 0.02);
	c += circleFill(p + vec2(0.84147*0.47, 0.54030*0.47), 0.02);
	c += circleFill(p + vec2(0.84147*0.47, -0.54030*0.47), 0.02);
	c += circleFill(p + vec2(0.41614*0.47, 0.90929*0.47), 0.02);
	c += circleFill(p + vec2(0.41614*0.47, -0.90929*0.47), 0.02);

	float t = iTime;
	float t2 = t * -0.01;
	float t3 = t * 0.03;

	vec2 angle1 = vec2(sin(t), cos(t));
	vec2 a = angle1 * 0.7;

	t *= 0.5;
	vec2 angle2 = vec2(sin(t), cos(t));
	vec2 b = angle2 * 0.8;

	vec2 angle3 = vec2(sin(t2), cos(t2));
	vec2 d = b + angle3 * 0.4;

	vec2 angle4 = vec2(sin(t3), cos(t3));
	vec2 e = angle4 * 0.9;

	vec2 angle5 = vec2(sin(t3 + 4.0), cos(t3 + 4.0));
	vec2 f = angle5 * 0.8;

	vec2 angle6 = vec2(sin(t*-0.1 + 5.0), cos(t*-0.1 + 5.0));
	vec2 h = angle6 * 0.8;





	float tt = t * 1.4;

	float tm = mod(tt, 0.5);
	float tmt = tt - tm;
	if (tm > 0.4) tmt += (tm - 0.4)*5.0;
	vec2 tangle1 = vec2(sin(tmt), cos(tmt));

	tt *= 0.8;
	tm = mod(tt, 0.6);
	float tmt2 = tt - tm;
	if (tm > 0.2) tmt2 += (tm - 0.2)*1.5;

	vec2 tangle2 = vec2(sin(tmt2*-4.0), cos(tmt2*-4.0));

	vec2 tangle3 = vec2(sin(tmt2), cos(tmt2));

	tt = t + 3.0;
	tm = mod(tt, 0.2);
	tmt = tt - tm;
	if (tm > 0.1) tmt += (tm - 0.1)*2.0;
	vec2 tangle4 = vec2(sin(-tmt), cos(-tmt)); tmt += 0.9;
	vec2 tangle41 = vec2(sin(-tmt), cos(-tmt)); tmt += 0.5;
	vec2 tangle42 = vec2(sin(-tmt), cos(-tmt)); tmt += 0.5;
	vec2 tangle43 = vec2(sin(-tmt), cos(-tmt)); tmt += 0.5;
	vec2 tangle44 = vec2(sin(-tmt), cos(-tmt)); tmt += 0.5;
	vec2 tangle45 = vec2(sin(-tmt), cos(-tmt));

	tt = iTime + 0.001;
	tm = mod(tt, 1.0);
	tmt = tt - tm;
	if (tm > 0.9) tmt += (tm - 0.9)*10.0;

	vec2 tangle51 = 0.17*vec2(sin(-tmt), cos(-tmt)); tmt += 1.0471975511965976;
	vec2 tangle52 = 0.17*vec2(sin(-tmt), cos(-tmt)); tmt += 1.0471975511965976;
	vec2 tangle53 = 0.17*vec2(sin(-tmt), cos(-tmt));

	c += Line(p, tangle51, -tangle53);
	c += Line(p, tangle52, tangle51);
	c += Line(p, tangle53, tangle52);
	c += Line(p, -tangle51, tangle53);
	c += Line(p, -tangle52, -tangle51);
	c += Line(p, -tangle53, -tangle52);

	c += circleFill(p + tangle51, 0.01);
	c += circleFill(p + tangle52, 0.01);
	c += circleFill(p + tangle53, 0.01);
	c += circleFill(p - tangle51, 0.01);
	c += circleFill(p - tangle52, 0.01);
	c += circleFill(p - tangle53, 0.01);



	c += circle(p + a, 0.2);
	c += circle(p + a, 0.14);
	c += circle(p + a, 0.1);
	c += circleFill(p + a, 0.04);
	c += circleFill(p + a + tangle3 * 0.2, 0.025);


	c += circle(p + a, 0.14);


	c += circle(p + b, 0.2);
	c += circle(p + b, 0.03);
	c += circle(p + b, 0.15);
	c += circle(p + b, 0.45);
	c += circleFill(p + b + tangle1 * 0.05, 0.01);
	c += circleFill(p + b + tangle1 * 0.09, 0.02);
	c += circleFill(p + b + tangle1 * 0.15, 0.03);
	c += circle(p + b + tangle1 * -0.15, 0.03);
	c += circle(p + b + tangle1 * -0.07, 0.015);

	c += circle(p + d, 0.08);


	c += circle(p + e, 0.08);


	c += circle(p + f, 0.12);
	c += circle(p + f, 0.10);
	c += circleFill(p + f + tangle2 * 0.05, 0.01);
	c += circleFill(p + f + tangle2 * 0.10, 0.01);
	c += circle(p + f - tangle2 * 0.03, 0.01);
	c += circleFill(p + f + (0.085), 0.005);
	c += circleFill(p + f, 0.005);


	vec2 g = tangle4 * 0.16;
	c += circle(p + h, 0.05);
	c += circle(p + h, 0.1);
	c += circle(p + h, 0.17);
	c += circle(p + h, 0.2);
	c += circleFill(p + h + tangle41 * 0.16, 0.01);
	c += circleFill(p + h + tangle42 * 0.16, 0.01);
	c += circleFill(p + h + tangle43 * 0.16, 0.01);
	c += circleFill(p + h + tangle44 * 0.16, 0.01);
	c += circleFill(p + h + tangle45 * 0.16, 0.01);
	c += circleFill(p + h + angle1 * 0.06, 0.02);
	c += circleFill(p + h + tangle43 * -0.16, 0.01);


	c += Line(p, (0.0), a);
	c += circleFill(p + b, 0.005);
	c += circleFill(p + d, 0.005);
	c += circleFill(p + e, 0.005);

	c += Line(p, b, a);
	c += Line(p, d, e);
	c += Line(p, b + tangle1 * 0.15, e);
	c += Line(p, e, f + (0.085));

	c += Line(p, h + angle1 * 0.06, f);
	c += Line(p, h + tangle43 * -0.16, d);
	c += Line(p, h + tangle42 * 0.16, e);


	// of course I'd write a line function that
	// doesn't handle perfectly vertical lines
	c += Line(p, vec2(0.001, -0.5), vec2(0.0001, 0.5));
	c += circleFill(p + vec2(0.001, -0.5), 0.005);
	c += circleFill(p + vec2(0.001, 0.5), 0.005);

	c = clamp(c, 0.0, 1.0);
	colour = white * c;


	fragColor = vec4(colour, 1.0);
}
#endif

#if SHADERTOY == 27
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec4 color;

	vec2 uv = fragCoord.xy / iResolution.xy;

	float distanceFromCenter = length(uv - vec2(0.5, 0.5));

	float vignetteAmount;

	float lum;

	vignetteAmount = 1.0 - distanceFromCenter;
	vignetteAmount = smoothstep(0.1, 1.0, vignetteAmount);

	color = texture(iChannel0, uv) * 2;

	// luminance hack, responses to red channel most
	lum = dot(color.rgb, vec3(0.85, 0.30, 0.10));

	color.rgb = vec3(0.0, lum, 0.0);

	// scanlines
	color += 0.1*sin(uv.y*iResolution.y*2.0);

	// screen flicker
	color += 0.005 * sin(iTime*16.0);

	// vignetting
	color *= vignetteAmount * 1.0;

	fragColor = color;
}
#endif

#if SHADERTOY == 28
float rand(vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453) * 2.0 - 1.0;
}

float offset(float blocks, vec2 uv) {
	return rand(vec2(iTime, floor(uv.y * blocks)));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord.xy / iResolution.xy;

	fragColor = texture(iChannel0, uv);

	fragColor.r = texture(iChannel0, uv + vec2(offset(16.0, uv) * 0.03, 0.0)).r;
	fragColor.g = texture(iChannel0, uv + vec2(offset(8.0, uv) * 0.03 * 0.16666666, 0.0)).g;
	fragColor.b = texture(iChannel0, uv + vec2(offset(8.0, uv) * 0.03, 0.0)).b;
}

#endif

#if SHADERTOY == 29

// Basic sobel filter implementation
// Jeroen Baert - jeroen.baert@cs.kuleuven.be
// 
// www.forceflow.be


// Use these parameters to fiddle with settings
//static float step = 1.0;
float rand(vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453) * 2.0 - 1.0;
}
float intensity(in vec4 color) {
	return sqrt((color.x*color.x) + (color.y*color.y) + (color.z*color.z));
}

vec3 sobel(float stepx, float stepy, vec2 center) {
	// get samples around pixel
	float tleft = intensity(texture(iChannel0, center + vec2(-stepx, stepy)));
	float left = intensity(texture(iChannel0, center + vec2(-stepx, 0)));
	float bleft = intensity(texture(iChannel0, center + vec2(-stepx, -stepy)));
	float top = intensity(texture(iChannel0, center + vec2(0, stepy)));
	float bottom = intensity(texture(iChannel0, center + vec2(0, -stepy)));
	float tright = intensity(texture(iChannel0, center + vec2(stepx, stepy)));
	float right = intensity(texture(iChannel0, center + vec2(stepx, 0)));
	float bright = intensity(texture(iChannel0, center + vec2(stepx, -stepy)));

	// Sobel masks (see http://en.wikipedia.org/wiki/Sobel_operator)
	//        1 0 -1     -1 -2 -1
	//    X = 2 0 -2  Y = 0  0  0
	//        1 0 -1      1  2  1

	// You could also use Scharr operator:
	//        3 0 -3        3 10   3
	//    X = 10 0 -10  Y = 0  0   0
	//        3 0 -3        -3 -10 -3

	float x = tleft + 2.0*left + bleft - tright - 2.0*right - bright;
	float y = -tleft - 2.0*top - tright + bleft + 2.0 * bottom + bright;
	float color = sqrt((x*x) + (y*y));
	return vec3(color, color, color);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	float step = rand(iTime);
	vec2 uv = fragCoord.xy / iResolution.xy;
	vec4 color = texture(iChannel0, uv.xy);
	fragColor.xyz = sobel(step / iResolution[0], step / iResolution[1], uv);
	fragColor.w = 1;
}
#endif

#if SHADERTOY == 30
// aji's amazing scanline shader

static const float linecount = 120.0;
static const vec4 gradA = vec4(0.0, 0.0, 0.0, 1.0);
static const vec4 gradB = vec4(0.5, 0.7, 0.6, 1.0);
static const vec4 gradC = vec4(1.0, 1.0, 1.0, 1.0);

static vec2 pos, uv;

float noise(float factor)
{
	vec4 v = texture(iChannel1, uv + iTime * vec2(9.0, 7.0));
	return factor * v.x + (1.0 - factor);
}

vec4 base(void)
{
	return texture(iChannel0, uv + .1 * noise(1.0) * vec2(0.02, 0.0));
}

float Triangle(float phase)
{
	//phase *= 2.0;
	//return 1.0 - abs(mod(phase, 2.0) - 1.0);
	// sin is not really a triangle.. but it's easier to do bandlimited
	float y = sin(phase * 3.14159);
	// if you want something brighter but more aliased, change 1.0 here to something like 0.3
	return pow(y * y, 1.0);
}

float scanline(float factor, float contrast)
{
	vec4 v = base();
	float lum = .2 * v.x + .5 * v.y + .3  * v.z;
	lum *= noise(0.3);
	float tri = Triangle(pos.y * linecount);
	tri = pow(tri, contrast * (1.0 - lum) + .5);
	return tri * lum;
}

vec4 gradient(float i)
{
	i = clamp(i, 0.0, 1.0) * 2.0;
	if (i < 1.0) {
		return (1.0 - i) * gradA + i * gradB;
	}
	else {
		i -= 1.0;
		return (1.0 - i) * gradB + i * gradC;
	}
}

vec4 vignette(vec4 at)
{
	float dx = 1.3 * abs(pos.x - .5);
	float dy = 1.3 * abs(pos.y - .5);
	return at * (1.0 - dx * dx - dy * dy);
}

vec4 gamma(vec4 x, float f)
{
	return pow(x, (1. / f));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	pos = uv = (fragCoord.xy - vec2(0.0, 0.5)) / iResolution.xy;
	uv.y = floor(uv.y * linecount) / linecount;
	fragColor = gamma(vignette(gradient(scanline(0.8, 2.0))), 1.5);
}
#endif

#if SHADERTOY == 31
/*
// Marginally longer, but saner:
void mainImage(out vec4 c,vec2 z)
{
z /= iResolution.xy;
z *= exp2(ceil(-log2(1.-z.y)));
c = texture(iChannel0,fract(z));
}*/

void mainImage(out vec4 c, vec2 z)
{
	z /= iResolution.xy;
	c = texture(iChannel0, fract(z*exp2(ceil(-log2(1. - z.y)))));
}
#endif

#if SHADERTOY == 32
/*
Bad analog television reception fragment shader
Bastiaan de Waard
defcon8
*/

#define noiselevel 0.4
#define rgbshiftlevel 0.01
#define ghostreflectionlevel 0.03
#define bypass false

float rand(vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord.xy / iResolution.xy;

	float randomValue = rand(vec2(uv.x + sin(iTime), uv.y + cos(iTime)));
	float rgbShift = sin(iTime + randomValue)*rgbshiftlevel;

	if (randomValue > 0.95 - ghostreflectionlevel)
		uv.x += sin(iTime / 5.0)*0.5;

	uv.y += (cos(iTime*randomValue) + 0.5) * (randomValue*0.01);

	float colorr = texture(iChannel0, vec2(uv.x + rgbShift, uv.y)).r;
	float colorg = texture(iChannel0, vec2(uv.x, uv.y)).g;
	float colorb = texture(iChannel0, vec2(uv.x - rgbShift, uv.y)).b;

	vec4 movieColor = vec4(colorr, colorg, colorb, 1.0);
	vec4 noiseColor = vec4(randomValue, randomValue, randomValue, 1.0);

	if (randomValue > 0.55 - ghostreflectionlevel)
		noiseColor = abs(noiseColor - 0.2);

	if (bypass)
		fragColor = texture(iChannel0, fragCoord.xy / iResolution.xy);
	else
		fragColor = mix(movieColor, noiseColor, noiselevel);


}
#endif

#if SHADERTOY == 33
#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 15

#define precomputedKernel false


//sigma 10.0, MSIZE 15
//in GLSL ES 3.0 you could use this (instead of calling init kernel) to make it even faster
//const float kernel[MSIZE] = float[MSIZE](0.031225216, 0.033322271, 0.035206333, 0.036826804, 0.038138565, 0.039104044, 0.039695028, 0.039894000, 0.039695028, 0.039104044, 0.038138565, 0.036826804, 0.035206333, 0.033322271, 0.031225216);
static float kernel[MSIZE];

void initKernel()
{
	if (!precomputedKernel)
	{
		for (int i = 0; i < MSIZE; i++)
			kernel[i] = .0;
		return;
	}

	kernel[0] = 0.031225216;
	kernel[1] = 0.033322271;
	kernel[2] = 0.035206333;
	kernel[3] = 0.036826804;
	kernel[4] = 0.038138565;
	kernel[5] = 0.039104044;
	kernel[6] = 0.039695028;
	kernel[7] = 0.039894000;
	kernel[8] = 0.039695028;
	kernel[9] = 0.039104044;
	kernel[10] = 0.038138565;
	kernel[11] = 0.036826804;
	kernel[12] = 0.035206333;
	kernel[13] = 0.033322271;
	kernel[14] = 0.031225216;
}

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x / (sigma*sigma)) / sigma;
}

float normpdf3(in vec3 v, in float sigma)
{
	return 0.39894*exp(-0.5*dot(v, v) / (sigma*sigma)) / sigma;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	initKernel();
	vec3 c = texture(iChannel0, fragCoord.xy / iResolution.xy).rgb;
	//declare stuff
	const int ksize = (MSIZE - 1) / 2;
	vec3 final_colour = (0.0);

	if (!precomputedKernel)
	{
		//create the 1-d kernel
		for (int j = 0; j <= ksize; ++j)
		{
			kernel[ksize + j] = kernel[ksize - j] = normpdf(float(j), SIGMA);
		}
	}

	float Z = 0.0;
	vec3 cc;
	float factor;
	float bZ = 0.39894 / BSIGMA; // == 1.0/normpdf(0.0, BSIGMA);

								 //read out the texels
	for (int i = -ksize; i <= ksize; ++i)
	{
		for (int j = -ksize; j <= ksize; ++j)
		{
			vec2 offset;
			offset.x = float(i) / iResolution.x;
			offset.y = float(j) / iResolution.y;
			cc = texture(iChannel0, fragCoord.xy / iResolution.xy + offset).rgb;
			factor = normpdf3(cc - c, BSIGMA)*bZ*kernel[ksize + j] * kernel[ksize + i];
			Z += factor;
			final_colour += factor * cc;
		}
	}

	if (fragCoord.x > iMouse.x)
	{
		vec3 currFragment = texture(iChannel0, (fragCoord.xy / iResolution.xy)).rgb;
		fragColor = vec4(currFragment, 1.0);
	}
	else
		fragColor = vec4(final_colour / Z, 1.0);
}
#endif

#if SHADERTOY == 34
// The MIT License
// Copyright ? 2017 Michael Schuresko
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord / iResolution.xy;

	mat3 to_yuvish = mat3(0.299, -0.14713, 0.615,
		0.587, -0.28886, -0.51499,
		0.114, 0.436, -0.10001);
	mat3 from_yuvish = mat3(1.0, 1.0, 1.0,
		0.0, -0.39465, 2.03211,
		1.13983, -0.58060, 0.0);

	// Output to screen
	fragColor = texture(iChannel0, uv);
	float rot_amount = smoothstep(0.2, 0.6, 0.95 * length(fragColor.rgb)) *
		smoothstep(1.0, 0.6, 0.95 * length(fragColor.rgb));
	fragColor.r += 0.2 * rot_amount;
	rot_amount *= 6.0 * sin(12.0 * iTime + 100.0 * length(fragColor.rgb));
	float stheta = sin(rot_amount);
	float ctheta = cos(rot_amount);
	mat3 rot = mat3(1.0, 0.0, 0.0,
		0.0, ctheta, -stheta,
		0.0, stheta, ctheta);
	fragColor.rgb = mul(mul(from_yuvish, mul(rot, to_yuvish)), smoothstep(0.3, 0.7, fragColor.rgb));
}

#endif

#if SHADERTOY == 35
#define red(mix)   vec4(mix.r > .5, 0, 0, mix.a)
#define green(mix) vec4(0, mix.g > .5, 0, mix.a)
#define blue(mix)  vec4(0, 0, mix.b > .5, mix.a)
#define yellow(mix)  vec4(mix.r>.5 && mix.g>.5, mix.r>.5 && mix.g>.5, 0, mix.a)
#define magneta(mix) vec4(mix.r>.5 && mix.b>.5, 0, mix.r>.5 && mix.b>.5, mix.a)
#define cyan(mix)    vec4(0, mix.g>.5 && mix.b>.5, mix.g>.5 && mix.b>.5, mix.a)
#define value(mix)    vec4(mix.r>.5 && mix.g>.5 && mix.b>.5, mix.r>.5 && mix.g>.5 && mix.b>.5, mix.r>.5 && mix.g>.5 && mix.b>.5, mix.a)
#define anypure(mix)  vec4(mix.r > .5, mix.g > .5, mix.b > .5, mix.a)

#define SUBCELLS 4

int which_cell(vec2 w)
{
	// returns an int between 0 and (SUBCELLS*SUBCELLS)-1
	w = floor(w*float(SUBCELLS));
	int i = int(w.x), j = int(w.y);
	return i + SUBCELLS * j;
}

//iChannel0: texture (e.g. Claude Van Damme)
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	const float fSUB = float(SUBCELLS);
#if 1
	// good default
	float CELL_1 = iResolution.y / 3.;
#else
	// use mouse.x to fine-tune
	float CELL_1 = iResolution.y / iMouse.x*10.;
#endif
	float CELL_2 = CELL_1 * fSUB;

	vec2 xy = CELL_1 * fragCoord.xy / iResolution.y;
	vec2 s1 = floor(xy); // sample bottom-left corner of CELL_1
	vec2 T = iChannelResolution[0].xy;
	vec4 t = texture(iChannel0, s1 / CELL_1 * T.y / T); // target color

	vec2 s2 = floor(xy * fSUB); // represent the target color with (SUBCELLS*SUBCELLS) cells

	vec4 c = t; // sampled color

	switch (which_cell(s2 / fSUB - s1))
	{
	case 0: c = cyan(t);	break;	case 1: c = anypure(t);	break;	case 2: c = value(t);	break;
	case 3: c = value(t);	break;	case 4: c = cyan(t);	break;	case 5: c = anypure(t);	break;
	case 6: c = anypure(t);	break;	case 7: c = value(t);	break;	case 8: c = cyan(t);	break;
	default: break;
	}


	vec2 d = xy * fSUB - s2 - 0.5; // transform coordinates to unit circle

	if (length(d) < 0.443)	fragColor = c;
	else           			fragColor = value(t);
}
#endif

#if SHADERTOY == 36
// Center at 0, Right at 1. Integration between 0 and 1/2.
float BoxNearestRight(float OneCenter, float OneRight, float TwoCenter, float TwoRight)
{
	return 0.5*OneCenter*TwoCenter;
}

// Left at -1, Center at 0. Integration between -1/2 and 0.
float BoxNearestLeft(float OneLeft, float OneCenter, float TwoLeft, float TwoCenter)
{
	return 0.5*OneCenter*TwoCenter;
}


// Left at -1, Center at 0. Integration between -1/2 and 0.
float TentNearestLeft(float OneLeft, float OneCenter, float TwoLeft, float TwoCenter)
{
	float Result = 0.0;

	Result += (1.0 / 8.0)*OneLeft*TwoLeft;
	Result += (3.0 / 8.0)*OneCenter*TwoCenter;

	return Result;
}

// Center at 0, Right at 1. Integration between 0 and 1/2.
float TentNearestRight(float OneCenter, float OneRight, float TwoCenter, float TwoRight)
{
	float Result = 0.0;

	Result = TentNearestLeft(OneRight, OneCenter, TwoRight, TwoCenter);

	return Result;
}


// Left at -1, Center at 0. Integration between -1/2 and 0.
float TentLinearLeft(float OneLeft, float OneCenter, float TwoLeft, float TwoCenter)
{
	float Result = 0.0;

	Result += (1.0 / 12.0)*OneLeft*TwoLeft;
	Result += (1.0 / 12.0)*OneLeft*TwoCenter;
	Result += (1.0 / 12.0)*OneCenter*TwoLeft;
	Result += (3.0 / 12.0)*OneCenter*TwoCenter;

	return Result;
}

// Center at 0, Right at 1. Integration between 0 and 1/2.
float TentLinearRight(float OneCenter, float OneRight, float TwoCenter, float TwoRight)
{
	float Result = 0.0;

	Result = TentLinearLeft(OneRight, OneCenter, TwoRight, TwoCenter);

	return Result;
}

// This works more in the sense of a "spectrum", which RGB colors are NOT.
vec3 TentLinearMultiplyColors(vec3 ColorOne, vec3 ColorTwo)
{
	vec3 Result = (0.0);

	// Left of Red.
	Result.r += BoxNearestLeft(ColorOne.r, ColorOne.r, ColorTwo.r, ColorTwo.r);

	// Right of Red.
	Result.r += TentLinearRight(ColorOne.r, ColorOne.g, ColorTwo.r, ColorTwo.g);

	// Left of Green.
	Result.g += TentLinearLeft(ColorOne.r, ColorOne.g, ColorTwo.r, ColorTwo.g);

	// Right of Green.
	Result.g += TentLinearRight(ColorOne.g, ColorOne.b, ColorTwo.g, ColorTwo.b);

	// Left of Blue.
	Result.b += TentLinearLeft(ColorOne.g, ColorOne.b, ColorTwo.g, ColorTwo.b);

	// Right of Blue.
	Result.b += BoxNearestRight(ColorOne.b, ColorOne.b, ColorTwo.b, ColorTwo.b);

	return Result;
}

// Code below assumes Weights sum up to 1.
vec3 LuminancePreservingClamp(vec3 Color, vec3 Weights) // !! LuminancePreservingMax
{
	// We need two passes to fully spread.
	for (int i = 0; i<2; i++)
	{
		// How much a channel is abve 1.0.
		vec3 Extend = max((0.0), Color - (1.0));

		// Relative luminance a component can surrender.
		vec3 LuminanceExtend = mul(Extend, Weights);

		//
		// We will be distributing channel a into the two other
		// channels b and c. We will be using the Weight b on
		// channel c and vice versa. Before applying the weights
		// we need to normalize, as the sum of weight b and c
		// is not one.
		//
		// Note that Weight_a+Weight_b equals 1-Weights_c
		//
		vec3 NormalizedExtend = LuminanceExtend / ((1.0) - Weights);

#if 1
		Color += Weights.brg*NormalizedExtend.gbr + Weights.gbr*NormalizedExtend.brg; // Cross product, but with +.
#else
		// Spread Red into Green and Blue.
		Color.g += Weights.b / (Weights.g + Weights.b)*LuminanceExtend.r;
		Color.b += Weights.g / (Weights.g + Weights.b)*LuminanceExtend.r;

		// Spread Green into Blue and Red.
		Color.b += Weights.r / (Weights.b + Weights.r)*LuminanceExtend.g;
		Color.r += Weights.b / (Weights.b + Weights.r)*LuminanceExtend.g;

		// Spread Blue into Red and Green.
		Color.r += Weights.g / (Weights.r + Weights.g)*LuminanceExtend.b;
		Color.g += Weights.r / (Weights.r + Weights.g)*LuminanceExtend.b;
#endif

	}

	return clamp(Color, 0.0, 1.0);
}

static const vec3 Rec709LuminanceWeights = vec3(0.2126, 0.7152, 0.0722);

vec3 RelativeLuminancePreservingClampRec709(vec3 Color)
{
	return LuminancePreservingClamp(Color, Rec709LuminanceWeights);
}

void mainImage(out vec4 FragColor, in vec2 FragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
	vec2 NormalizedScreenCoords = FragCoord / iResolution.xy;

	vec3 ColorOne = vec3(0.0, 1.0, 0.0);

	vec3 ColorTwo = texture(iChannel0, NormalizedScreenCoords).rgb;

	vec3 Result = TentLinearMultiplyColors(ColorOne, ColorTwo);

	Result *= 5.0 + 4.0*sin(iTime);

	Result = RelativeLuminancePreservingClampRec709(Result);

	FragColor = vec4(Result, 1.0);
}
#endif

#if SHADERTOY == 37
// "USE_TILE_BORDER" creates a border around each tile.
// "USE_ROUNDED_CORNERS" gives each tile a rounded effect.
// If neither are defined, it is a basic pixelization filter.
#define USE_TILE_BORDER
//#define USE_ROUNDED_CORNERS

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	const float minTileSize = 1.0;
	const float maxTileSize = 32.0;
	const float textureSamplesCount = 3.0;
	const float textureEdgeOffset = 0.005;
	const float borderSize = 1.0;
	const float speed = 1.0;

	float time = pow(sin(iTime * speed), 2.0);
	float tileSize = minTileSize + floor(time * (maxTileSize - minTileSize));
	tileSize += mod(tileSize, 2.0);
	vec2 tileNumber = floor(fragCoord / tileSize);

	vec4 accumulator = (0.0);
	for (float y = 0.0; y < textureSamplesCount; ++y)
	{
		for (float x = 0.0; x < textureSamplesCount; ++x)
		{
			vec2 textureCoordinates = (tileNumber + vec2((x + 0.5) / textureSamplesCount, (y + 0.5) / textureSamplesCount)) * tileSize / iResolution.xy;
			textureCoordinates.y = 1 - textureCoordinates.y;

			textureCoordinates.y = 1.0 - textureCoordinates.y;
			textureCoordinates = clamp(textureCoordinates, 0.0 + textureEdgeOffset, 1.0 - textureEdgeOffset);
			accumulator += texture(iChannel0, textureCoordinates);
		}
	}

	fragColor = accumulator / (textureSamplesCount * textureSamplesCount);

#if defined(USE_TILE_BORDER) || defined(USE_ROUNDED_CORNERS)
	vec2 pixelNumber = floor(fragCoord - (tileNumber * tileSize));
	pixelNumber = mod(pixelNumber + borderSize, tileSize);

#if defined(USE_TILE_BORDER)
	float pixelBorder = step(min(pixelNumber.x, pixelNumber.y), borderSize) * step(borderSize * 2.0 + 1.0, tileSize);
#else
	float pixelBorder = step(pixelNumber.x, borderSize) * step(pixelNumber.y, borderSize) * step(borderSize * 2.0 + 1.0, tileSize);
#endif
	fragColor *= pow(fragColor, (pixelBorder));
#endif
}
#endif

#if SHADERTOY == 38
static float size = 100.0; // length of edges
static const float sin30 = 0.5;
static const float cos30 = 0.86602540378; // = sin60 = sqrt(3) / 2.0
static const float sqrt3 = sqrt(3.0);

ivec2 hexCell(vec2 xy) {
	vec2 n = xy / vec2(sqrt3 * size, size * 1.5);

	ivec2 i;
	i.y = int(floor(n.y));

	bool odd = i.y % 2 == 1;
	if (odd) // odd rows
		n.x -= 0.5;

	i.x = int(floor(n.x));
	vec2 r = fract(n);

	if (r.y > 2.0 / 3.0) {
		// diagonal areas
		float y = (r.y - 2.0 / 3.0) * 3.0;
		if (r.x < 0.5) {
			// top-left/bottom-right
			float x = r.x * 2.0;
			if (y > x) {
				i.x += odd ? 0 : -1;
				i.y += 1;
			}
		}
		else {
			// top-right/bottom-left
			float x = (r.x - 0.5) * 2.0;
			if (x > 1.0 - y) {
				i.x += odd ? 1 : 0;
				i.y += 1;
			}
		}
	}

	return i;
}

vec2 hexGrid(vec2 xy) {
	ivec2 cell = hexCell(xy);

	vec2 result = vec2(cell) * vec2(sqrt3 * size, 1.5 * size);
	if (cell.y % 1 == 0)
		result.x += cos30 * size;
	return result;
}

bool feq(float a, float b) {
	return abs(a - b) < 1e-5;
}

vec2 triangleCell(vec2 xy) {
	vec2 pos = xy / vec2(size * sin30, size * cos30);
	vec2 i = floor(pos);
	vec2 r = fract(pos);
	//if (feq(mod(i.x, 2.), 1.) ^^ feq(mod(i.y, 2.), 1.)) {
	bool P = feq(mod(i.x, 2.), 1.);
	bool Q = feq(mod(i.y, 2.), 1.);
	if ((P && !Q) || (!P && Q)) {
		if (r.x + r.y < 1.0)
			i.x--;
	}
	else {
		if (r.x - r.y < 0.0)
			i.x--;
	}
	return i;
}

vec2 triangleGrid(vec2 xy) {
	vec2 i = triangleCell(xy);
	vec2 origin = vec2(i) * vec2(size * sin30, size * cos30);
	float c = cos30 * 2.0 / 3.0;
	//if (feq(mod(i.y, 2.), 1.) ^^ feq(mod(i.x, 2.), 1.))
	bool P = feq(mod(i.x, 2.), 1.);
	bool Q = feq(mod(i.y, 2.), 1.);
	if ((P && !Q) || (!P && Q))
		origin.y += size * cos30 * 2.0 / 3.0;
	else
		origin.y += size * cos30 - (0.5 * size) / cos30;
	origin.x += 0.5 * size;
	return origin;
}

vec2 rectCell(vec2 xy) {
	return floor(xy / size);
}

vec2 rectGrid(vec2 xy) {
	vec2 i = rectCell(xy);
	return i * size + size / 2.0;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	if (iMouse.z > 0.5)
		size = 1.0 + floor(iMouse.x * 50.0 / iResolution.x);
	else
		size = 1.0 + (1.0 + sin(iTime)) * 50.0;

	vec2 xy = fragCoord;

	float t = fract(iTime / 4.0);
	if (t < 0.33)
		xy = hexGrid(xy);
	else if (t < 0.66)
		xy = rectGrid(xy);
	else
		xy = triangleGrid(xy);

	vec2 uv = xy / iResolution.xy;

	// vec3 col = texelFetch(iChannel0, ivec2(uv*vec2(textureSize(iChannel0, 0).xy)), 0).rgb; // for textures with mipmaps
	vec3 col = texture(iChannel0, uv).rgb;

	// Output to screen
	fragColor = vec4(col, 1.0);
}
#endif

#if SHADERTOY == 39

#define MOVING_CENTER
//#define DISCARD_ON_CONTRAST

float hash21(vec2 p)
{
	float v = fract(sin(p.x*1234.68 + p.y * 98765.543)*753.159);
	return v;
}

vec4 circularMosaic(in vec2 uv, vec2 Center, float band_size, float angle_nb, float deplacement)
{
	vec2 PtToCenter = uv - Center;

	float angle = atan(PtToCenter.y, PtToCenter.x);
	float angle_step = floor(angle * angle_nb);

	float radius = length(PtToCenter);
	float radius_step = floor(radius * band_size);

	float randVal = hash21(vec2(radius_step, angle_step));
	angle += randVal * deplacement;

	vec2 uvFromPolar = vec2(cos(angle), sin(angle)) * radius_step / band_size + Center;

	vec2 realUv = uvFromPolar;

	vec4 texColor = texture(iChannel0, realUv);

	return  texColor;
}

vec4 circularMosaic2(in vec2 uv, vec2 Center, float band_size, float angle_nb, float deplacement)
{
	vec2 PtToCenter = uv - Center;


	float radius = length(PtToCenter);
	float radius_step = floor(radius * band_size);



	float randV = hash21(vec2(radius_step, 0.));

	float angle;
	float angle_step;

	float randEffect = 10.;

	angle = atan(PtToCenter.y, PtToCenter.x) + randV / randEffect;
	angle_step = floor(angle * angle_nb) - randV / randEffect;

	/*
	angle = atan( PtToCenter.y, PtToCenter.x );
	angle_step = floor(angle * angle_nb);
	//*/

	vec2 tgtVect = vec2(-PtToCenter.y, PtToCenter.x);

	//angle += radius_step/10.*sin(iTime);
	float randVal = hash21(vec2(radius_step, angle_step));

	vec2 uvFromPolar = vec2(cos(angle), sin(angle)) * radius_step / band_size + Center;

	vec2 uvTgt = uvFromPolar + tgtVect * randVal * deplacement;

	vec4 texColor2 = texture(iChannel0, uvTgt);


	return texColor2;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord / iResolution.xy;

	vec2 Center = vec2(.25, .25);

#ifdef MOVING_CENTER
	float myTime = iTime * .1;
	Center = vec2(.6 + .2 * cos(myTime), .4 + .3 * sin(myTime));
#endif // MOVING_CENTER    

	float band_size = 50.;
	float angle_nb = 10.;
	float deplacement = 0.01;

	vec4 newColor = circularMosaic(uv, Center, band_size, angle_nb, deplacement);
	vec4 realColor = texture(iChannel0, uv);
	fragColor = newColor;
#ifdef DISCARD_ON_CONTRAST
	fragColor = realColor;
	if (distance(newColor, realColor) < .45)
		fragColor = newColor;
#endif // DISCARD_ON_CONTRAST

}
#endif

#if SHADERTOY == 40
//uniform float time;
static const vec2 resolution = 0;
//uniform vec2 mouse;

float rnd(float x) { return fract(sin(x * 1100.082) * 13485.8372); }
mat2 rot(float a) { float s = sin(a), c = cos(a); return mat2(c, -s, s, c); }
float sdTorus(vec3 p, vec2 t) { vec2 q = vec2(length(p.xz) - t.x, p.y); return length(q) - t.y; }

vec2 Rot2D(vec2 q, float a) { return q * cos(a) + q.yx * sin(a) * vec2(-1., 1.); }
vec3 IcosSym(vec3 p)
{
	float dihedIcos = 0.5 * acos(sqrt(5.) / 3.);
	float a, w;
	w = 2. * 3.1415 / 3.;
	p.z = abs(p.z);
	p.yz = Rot2D(p.yz, -dihedIcos);
	p.x = -abs(p.x);
	for (int k = 0; k < 4; k++) {
		p.zy = Rot2D(p.zy, -dihedIcos);
		p.y = -abs(p.y);
		p.zy = Rot2D(p.zy, dihedIcos);
		if (k < 3) p.xy = Rot2D(p.xy, -w);
	}
	p.z = -p.z;
	a = mod(atan(p.x, p.y) + 0.5 * w, w) - 0.5 * w;
	p.yx = vec2(cos(a), sin(a)) * length(p.xy);
	p.x -= 2. * p.x * step(0., p.x);
	return p;
}
float mp(vec3 p) {
	p.z += 1.5;
	float scale = 1.;
	float t = iTime - 0.15*rnd(p.y*3.3 + p.x*7.7);
	p.xy = mul(p.xy, rot(floor(t) + smoothstep(0., .4, fract(t))));
	p.xz = mul(p.xz, rot(floor(t) + smoothstep(.5, .9, fract(t))));
	for (int i = 0; i<1; i++) {
		scale *= 3.;
		p *= 3.;
		p = IcosSym(p);
		p.z += 3.;
		p.y -= .7;
	}
	return sdTorus(p, vec2(.5, .2)) / scale;
}


vec3 nor(vec3 p) {
	vec2 e = vec2(0, .001);
	return normalize(vec3(
		mp(p + e.xxy) - mp(p - e.xxy),
		mp(p + e.xyx) - mp(p - e.xyx),
		mp(p + e.yxx) - mp(p - e.yxx)
	));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	float ii, d = 0., rm, i; vec3 p, n;
	float mx = max(resolution.x, iResolution.y);
	vec2 uv = (2.*fragCoord.xy - iResolution.xy) / mx;
	for (float i = 0.; i<90.; i++) {
		ii = float(i);
		p = d * vec3(uv, .9);
		p.z -= 3.5;
		rm = mp(p);
		if (rm<.001)break;
		d += rm;
	}
	n = nor(p).bgr;
	vec3 col1 = vec3(63, 232, 130) / 255.;
	vec3 col2 = vec3(0, 71, 255) / 255.;
	vec3 col = mix(col1, col2, (n.x + n.y) / 2. + .5);
	fragColor = vec4(col*10. / (ii*d*d), 1.);
}
#endif


#if SHADERTOY == 41
static const int samples = 35,
LOD = 2,    // gaussian done on MIPmap at scale LOD
sLOD = 1 << LOD;  // tile size = 2^LOD
static const float sigma = float(samples) * .25;
// used in grayScale
static const vec3 W = vec3(0.2125, 0.7154, 0.0721);

float gaussian(vec2 i) { return exp(-.5 * dot(i /= sigma, i)) / (6.28 * sigma * sigma); }

vec4 blur(sampler2D sp, vec2 U, vec2 scale) {
	vec4 O = (0);
	int s = samples / sLOD;

	for (int i = 0; i < s * s; i++) {
		vec2 d = vec2(i % s, i / s) * float(sLOD) - float(samples) / 2.;
		vec4 temp = textureLod(sp, U + scale * d, float(LOD));
		O += vec4(gaussian(d) * temp);
	}

	return O / O.a;
}

// XDoG parameters
static const float Epsilon = 0.01;
static const float Phi = 200.0;
static const float Sigma = 0.2;
static const float k = 1.5;
static const float p = 0.97;

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord / iResolution.xy;

	// Two Gaussian blur
	vec3 blurImage1 = blur(iChannel0, uv, (k * Sigma / iResolution)).rgb;
	vec3 blurImage2 = blur(iChannel0, uv, (Sigma / iResolution)).rgb;
	blurImage1 = ((blurImage1 * W).g);
	blurImage2 = ((blurImage2 * W).g);
	float diff = blurImage1.r - p * blurImage2.r;

	if (diff < Epsilon) {
		diff = 0.0;
	}
	else {
		diff = 0.5 + tanh(Phi * diff);
	}

	// XDoG

	fragColor = vec4(13.0 * (diff), 13.0 * (diff), 13.0 * (diff), 1.0);

	// Just DoG
	// fragColor = vec4(20.0*(blurImage1-blurImage2),1.0);
}
#endif

#if SHADERTOY == 42
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec4 c = texture(iChannel0, fragCoord / iResolution.xy);
	fragColor = vec4(c.r + c.g + c.b - vec3(0, 1, 2), c.a);
}
#endif


#if SHADERTOY == 43
// https://www.shadertoy.com/view/WsdfRB
// Shader under CC BY 3.0 license
// Created by piootrek / Piotr Ruszkowski
// 2020

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = (fragCoord * 2.0 - iResolution.xy) / iResolution.y;
	uv += (distance(uv, (0.))) * 0.25 * vec2(sin(iTime * 1.73), cos(iTime * 2.67));


	vec3 color_offset = vec3(0.2, 0.1, 0.6);
	vec3 circle_col = (1.0 + 0.5 * sin(iTime + color_offset * 3.14)) * exp(distance(uv, (0.)));

	uv /= exp(fract(iTime * 2.0));

	float dist = distance(uv, (0.0));
	dist = fract(log(dist));

	float out_circle = smoothstep(1.0, 0.9, dist);
	float in_circle = smoothstep(0.95, 0.89, dist);
	vec3 circle = circle_col * (out_circle - in_circle);

	fragColor = vec4(circle * (1.), 1.0);
}
#endif

#if SHADERTOY == 44
// https://www.shadertoy.com/view/3s3BD7
vec4 overlay(vec4 target, vec4 blend) {
	float gray = dot(target.xyz, vec3(0.21, 0.71, 0.07));

	return (gray > 0.5) ? (1. - (1. - 2.*(gray - 0.5)) * (1. - blend))
		: ((2.*gray) * blend);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord / iResolution.xy;

	float t = cos(iTime) * 0.5 + 0.5;

	// Time varying pixel color
	vec4 col = vec4(0.5 + 0.5*cos(iTime + uv.xyx + vec3(0, 2, 4)), 1.0);
	vec4 text = texture(iChannel0, uv);

	vec4 target = text;
	vec4 blend = col;

	// Output to screen
	fragColor = overlay(target, blend);
}
#endif

#if SHADERTOY == 45
// https://www.shadertoy.com/view/WdtyDM
struct ray {
	vec3 o, d;
};
float N(float n) {
	return fract(sin(2385.*n)*5812.);
}
vec4 N14(float t) {
	return fract(sin(t*vec4(2385., 586., 3258., 747.))*vec4(5893., 3256., 234., 5812.));
}
ray getRay(vec2 uv, vec3 camPos, vec3 lookat, float zoom) {
	ray a;
	a.o = camPos;
	vec3 f = normalize(lookat - a.o);
	vec3 r = cross(vec3(0., 1., 0.), f);
	vec3 u = cross(f, r);
	vec3 c = a.o + zoom * f;
	vec3 i = c + uv.x*r + uv.y*u;
	a.d = normalize(i - a.o);
	return a;
}
float dis(ray r, vec3 p) {
	vec3 ro = r.o;
	vec3 rd = r.d;
	return length(cross(p - ro, rd)) / length(rd);
}
float Bokeh(ray r, vec3 center, float raduis, float blur)
{

	float d = dis(r, center);
	raduis *= length(center);
	float sphere = smoothstep(raduis, raduis*(1. - blur), d);
	sphere *= mix(.6, 1., smoothstep(raduis*.8, raduis, d));
	return sphere;
}
vec3 strightLight(ray r, float t, vec3 color) {
	float side = step(r.d.x, 0.);
	r.d.x = abs(r.d.x);

	float d = 0.;
	for (float i = 0.; i<1.; i += .1) {
		float ti = fract(t + i + side * 0.05);
		d += Bokeh(r, vec3(2., 2., 100. - 100.*ti), .05, .1)*ti*ti*ti;
	}
	vec3 col = d * color;
	return col;
}
vec3 headLight(ray r, float t, vec3 color) {
	t *= 2.;
	float w1 = .25;
	float w2 = w1 * 1.2;
	float d = 0.;
	float s = 1. / 30.;
	for (float i = 0.; i<1.; i += s) {
		if (N(i)>.1)continue;
		float ti = fract(t + i);
		float z = 100. - ti * 100.;
		float fade = ti * ti*ti*ti*ti;
		float focus = smoothstep(.9, 1., ti);

		float size = mix(.05, .03, focus);
		d += Bokeh(r, vec3(-1. - w1, .15, z), size, .1)*fade;
		d += Bokeh(r, vec3(-1. + w1, .15, z), size, .1)*fade;
		d += Bokeh(r, vec3(-1. - w2, .15, z), size, .1)*fade;
		d += Bokeh(r, vec3(-1. + w2, .15, z), size, .1)*fade;
		float ref = 0.;
		ref += Bokeh(r, vec3(-1. - w2, -.15, z), size*3., 1.)*fade;
		ref += Bokeh(r, vec3(-1. + w2, -.15, z), size*3., 1.)*fade;

		d += ref * focus;


	}
	vec3 col = d * color;
	return col;
}
vec3 tailLight(ray r, float t, vec3 color) {
	t *= .25;
	float w1 = .25;
	float w2 = w1 * 1.2;
	float d = 0.;
	float s = 1. / 15.;
	for (float i = 0.; i<1.; i += s) {
		float n = N(i);
		if (n>.5)continue;
		float lane = step(0.25, n);
		float ti = fract(t + i);
		float z = 100. - ti * 100.;
		float fade = ti * ti*ti*ti*ti;
		float focus = smoothstep(.9, 1., ti);

		float laneshift = smoothstep(1., .96, ti);
		float blink = step(0., sin(t*1000.))*7.*lane*step(.96, ti);
		float size = mix(.05, .03, focus);
		float x = 1.5 - lane * laneshift;
		d += Bokeh(r, vec3(x - w1, .15, z), size, .1)*fade;
		d += Bokeh(r, vec3(x + w1, .15, z), size, .1)*fade;
		d += Bokeh(r, vec3(x - w2, .15, z), size, .1)*fade;
		d += Bokeh(r, vec3(x + w2, .15, z), size, .1)*fade*(1. + blink);
		float ref = 0.;
		ref += Bokeh(r, vec3(x - w2, -.15, z), size*3., 1.)*fade;
		ref += Bokeh(r, vec3(x + w2, -.15, z), size*3., 1.)*fade*(1. + blink * .1);

		d += ref * focus;


	}
	vec3 col = d * color;
	return col;
}
vec3 envLight(ray r, float t, vec3 color) {
	float side = step(r.d.x, 0.);
	r.d.x = abs(r.d.x);

	vec3 col = (0.);
	for (float i = 0.; i<1.; i += .1) {
		float ti = fract(t + i + side * 0.05);

		vec4 n = N14(i + side * 123.);
		float x = mix(2.5, 10., n.x);
		float y = mix(.1, 1.5, n.y);
		float fade = sin(ti*3.14*20.*n.x)*0.5 + 0.5;
		vec3 p = vec3(x, y, 50. - 50.*ti);
		col += Bokeh(r, p, .05, .1)*fade*n.wzy*.5;
	}

	return col;
}
vec2 rain(vec2 uv, float t) {
	t *= 40.;



	vec2 a = vec2(3., 1.);
	vec2 suv = uv * a;

	vec2 id = floor(suv);
	suv.y += t * .22;
	float n = fract(sin(id.x*123.98)*965.25);
	suv.y += n;
	uv.y += n;
	vec2 st = fract(suv) - .5;

	id = floor(suv);
	t += fract(sin(id.x*155.48 + id.y*215.32)*9695.25)*7.576;
	float y = -sin(t + sin(t + sin(t)*.5))*.43;
	vec2 p1 = vec2(0., y);
	vec2 o1 = (st - p1) / a;
	float d = length(o1);
	float m1 = smoothstep(.07, .0, d);

	vec2 o2 = (fract(uv*a.x*vec2(1., 2.)) - .5) / vec2(1., 2.);
	d = length(o2);
	float m2 = smoothstep(.3*(.5 - st.y), .0, d)*smoothstep(-.1, .1, st.y - p1.y);
	//if(st.x>.46||st.y>.49) m1=1.;
	return vec2(m1*o1*30. + m2 * o2*10.);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord / iResolution.xy;
	uv.y = 1 - uv.y;

	uv -= .5;
	uv.x *= iResolution.x / iResolution.y;
	// Time varying pixel color

	vec2 m = iMouse.xy / iResolution.xy;
	float t = iTime * .05 + m.x;
	vec3 camPos = vec3(0.5, .2, 0.);
	vec3 lookat = vec3(0.5, .2, 1.);
	vec2 rainDistort = rain(uv*5., t)*.5;
	rainDistort += rain(uv*7., t)*.5;
	uv.x += sin(uv.y*70.)*0.002;
	uv.y += sin(uv.x*170.)*0.0015;
	ray r = getRay(uv - rainDistort * .5, camPos, lookat, 2.);

	vec3 col = strightLight(r, t, vec3(1., .7, .3));
	col += headLight(r, t, vec3(.9, .9, 1.));
	col += tailLight(r, t, vec3(1., .1, .03));
	col += envLight(r, t, vec3(0., 0., 0.));
	col += (r.d.y + .25)*vec3(.2, .1, .5);

	//col=vec3(rainDistort,0.);
	fragColor = vec4(col, 1.);
}
#endif

#if SHADERTOY == 46
// https://www.shadertoy.com/view/tddfDH
float Hash21(vec2 p) {
	p = fract(p*vec2(265.21, 232.23));
	p += dot(p, p + 200.23);
	return fract(p.x*p.y);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = (fragCoord - .5*iResolution.xy) / iResolution.y;

	vec2 UV = fragCoord / iResolution.xy;
	vec3 col = (0);

	uv += iTime * 0.1;
	uv *= 5.;
	vec2 gv = fract(uv) - .5;
	vec2 id = floor(uv);

	float width = .1*UV.y + 0.01 + sin(iTime) * .04 + .05;

	float n = Hash21(id); // Random 0...1

	if (n<.5) gv.x *= -1.;

	vec2 cUv = gv - sign(gv.x + gv.y + .001)*.5;
	float dFlat = abs(abs(gv.x + gv.y) - .5);
	float dCirc = length(cUv);
	float d = dCirc;
	float mask = smoothstep(.01, -.01, abs(d - .5) - width);

	float angle = atan(cUv.x, cUv.y);
	float checker = mod(id.x + id.y, 2.)*2. - 1.;
	//col += n;
	float flow = sin(iTime + angle * checker * 10.);
	float x = fract(flow + angle);
	float y = (d - (.5 - width)) / (2.*width);
	y = abs(y - .5)*2.;
	vec2 tUv = vec2(x, y);
	col += texture(iChannel1, tUv *.04).rgb*mask;
	col += y*mask;

	//if (gv.x > .48 || gv.y > .48) col = vec3(1,0,0);

	fragColor = vec4(col, 1.0);
}
#endif

#if SHADERTOY == 47
#define pi (acos(-1.))
#define tau (pi*2.)

mat2 rotate(float a)
{
	float c = cos(a);
	float s = sin(a);
	return mat2(c, -s, s, c);
}

float sdBox(vec3 p, vec3 b, float r)
{
	vec3 d = abs(p) - (b - r);
	return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0) - r;
}

float sdRect(vec2 p, vec2 b, float r)
{
	vec2 d = abs(p) - (b - r);
	return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - r;
}

static vec2 gl_FragCoord;
float scene(vec3 p)
{
	float flip = step(gl_FragCoord.x, iResolution.x*.5)*2. - 1.;
	float time = flip * iTime;

	p.yz = mul(p.yz, rotate(pi*.25*flip));

	vec3 a = vec3(
		length(p.xz),
		p.y,
		atan(p.x, p.z)
	);

	a.z = fract((a.z / tau)*8. + time) - .5;
	a.x -= 1.;
	a.xy = abs(a.xy) - .25;

	float r = .02;
	float d = min(
		sdBox(a, vec3(.2, .2, .4), r),
		sdRect(a.xy, (.15), r)
	);
	return d;
}

vec3 trace(vec3 cam, vec3 dir)
{
	vec3 color = vec3(.2, .1, .9);

	float t = 0.;
	float k = 0.;
	for (int i = 0; i<100; ++i)
	{
		k = scene(cam + dir * t);
		t += k;
		if (abs(k) < .001)
			break;
	}

	if (abs(k) < .001)
	{
		vec3 h = cam + dir * t;
		vec2 o = vec2(.001, 0);
		vec3 n = normalize(vec3(
			scene(h + o.xyy) - scene(h - o.xyy),
			scene(h + o.yxy) - scene(h - o.yxy),
			scene(h + o.yyx) - scene(h - o.yyx)
		));

		float A = .1;
		float B = scene(h + n * A);
		float fakeAO = clamp(B / A, 0., 1.);
		fakeAO = pow(fakeAO, .6)*.5 + .5;

		float light = 0.;
		vec3 lightDir = normalize(vec3(1, 4, -4));

		// wrap diffuse
		light += dot(n, lightDir)*.7 + .3;

		// specular
		vec3 H = normalize(lightDir + dir);
		float NdotH = dot(n, H);
		light += pow(max(NdotH, 0.), 3.)*30.;

		return light * fakeAO * color;
	}

	float gradient = pow(.8, dot(cam.xy, cam.xy));
	return mix(color, (1), gradient);
}

void mainImage(out vec4 out_color, vec2 fragCoord)
{
	gl_FragCoord = fragCoord;

	vec2 uv = fragCoord.xy / iResolution.xy - .5;
	uv.x *= iResolution.x / iResolution.y;

	vec3 cam = vec3(uv*3., -5.);
	vec3 dir = vec3(0, 0, 1);

	out_color.rgb = pow(trace(cam, dir), (.45));
	out_color.a = 1;
}
#endif

#if SHADERTOY == 48
// domain warping based on the master's notes at https://www.iquilezles.org/www/articles/warp/warp.htm

// NOISE ////
vec2 hash2(float n)
{
    return fract(sin(vec2(n, n + 1.0)) * vec2(13.5453123, 31.1459123));
}

float noise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    float a = textureLod(iChannel1, (p + vec2(0.5, 0.5)) / 256.0, 0.0).x;
    float b = textureLod(iChannel1, (p + vec2(1.5, 0.5)) / 256.0, 0.0).x;
    float c = textureLod(iChannel1, (p + vec2(0.5, 1.5)) / 256.0, 0.0).x;
    float d = textureLod(iChannel1, (p + vec2(1.5, 1.5)) / 256.0, 0.0).x;
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

static const mat2 mtx = mat2(0.80, 0.60, -0.60, 0.80);

float fbm(vec2 p)
{
    float f = 0.0;

    f += 0.500000 * noise(p);
    p = mul(mtx, p * 2.02);
    f += 0.250000 * noise(p);
    p = mul(mtx, p * 2.03);
    f += 0.125000 * noise(p);
    p = mul(mtx, p * 2.01);
    f += 0.062500 * noise(p);
    p = mul(mtx, p * 2.04);
    f += 0.031250 * noise(p);
    p = mul(mtx, p * 2.01);
    f += 0.015625 * noise(p);

    return f / 0.96875;
}

// -----------------------------------------------------------------------

float pattern(in vec2 p, in float t, in vec2 uv, out vec2 q, out vec2 r, out vec2 g)
{
    q = vec2(fbm(p), fbm(p + vec2(10, 1.3)));

    float s = dot(uv.x + 0.5, uv.y + 0.5);
    r = vec2(fbm(p + 4.0 * q + (t) + vec2(1.7, 9.2)), fbm(p + 4.0 * q + (t) + vec2(8.3, 2.8)));
    g = vec2(fbm(p + 2.0 * r + (t * 20.0) + vec2(2, 6)), fbm(p + 2.0 * r + (t * 10.0) + vec2(5, 3)));
    return fbm(p + 5.5 * g + (-t * 7.0));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord / iResolution.xy;

	// noise
    vec2 q, r, g;
    float noise = pattern(fragCoord * (.004), iTime * 0.007, uv, q, r, g);

	// base color based on main noise
    vec3 col = mix(vec3(0.1, 0.4, 0.4), vec3(0.5, 0.7, 0.0), smoothstep(0.0, 1.0, noise));

	// other lower-octave colors and mixes
    col = mix(col, vec3(0.35, 0.0, 0.1), dot(q, q) * 1.0);
    col = mix(col, vec3(0, 0.2, 1), 0.2 * g.y * g.y);
    col = mix(col, vec3(.3, 0, 0), smoothstep(0.0, .6, 0.6 * r.g * r.g));
    col = mix(col, vec3(0, .5, 0), 0.1 * g.x);

	// some dark outlines/contrast and different steps
    col = mix(col, (0), smoothstep(0.3, 0.5, noise) * smoothstep(0.5, 0.3, noise));
    col = mix(col, (0), smoothstep(0.7, 0.8, noise) * smoothstep(0.8, 0.7, noise));

	// contrast
    col *= noise * 2.0;

	// vignette
    col *= 0.70 + 0.65 * sqrt(70.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y));

	// Output to screen
    fragColor = vec4(col, 1.0);
}
#endif

#if SHADERTOY == 49

// Modification of https://www.shadertoy.com/view/4ssXRX
// Added a second way to do triangle noise without doing
// a second random sample.

static const int NUM_BUCKETS = 32;
static const int ITER_PER_BUCKET = 1024;
static const float HIST_SCALE = 8.0;

static const float NUM_BUCKETS_F = float(NUM_BUCKETS);
static const float ITER_PER_BUCKET_F = float(ITER_PER_BUCKET);


//note: uniformly distributed, normalized rand, [0;1[
float nrand(vec2 n)
{
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}
//note: remaps v to [0;1] in interval [a;b]
float remap(float a, float b, float v)
{
	return clamp((v - a) / (b - a), 0.0, 1.0);
}
//note: quantizes in l levels
float truncate(float a, float l)
{
	return floor(a*l) / l;
}

float n1rand(vec2 n)
{
	float t = fract(iTime);
	float nrnd0 = nrand(n + 0.07*t);
	return nrnd0;
}
float n2rand(vec2 n)
{
	float t = fract(iTime);
	float nrnd0 = nrand(n + 0.07*t);
	float nrnd1 = nrand(n + 0.11*t);
	return (nrnd0 + nrnd1) / 2.0;
}

float n2rand_faster(vec2 n)
{
	float t = fract(iTime);
	float nrnd0 = nrand(n + 0.07*t);

	// Convert uniform distribution into triangle-shaped distribution.
	float orig = nrnd0 * 2.0 - 1.0;
	nrnd0 = orig * inversesqrt(abs(orig));
	nrnd0 = max(-1.0, nrnd0); // Nerf the NaN generated by 0*rsqrt(0). Thanks @FioraAeterna!
	nrnd0 = nrnd0 - sign(orig) + 0.5;

	// Result is range [-0.5,1.5] which is
	// useful for actual dithering.
	// convert to [0,1] for histogram.
	return (nrnd0 + 0.5) * 0.5;
}
float n3rand(vec2 n)
{
	float t = fract(iTime);
	float nrnd0 = nrand(n + 0.07*t);
	float nrnd1 = nrand(n + 0.11*t);
	float nrnd2 = nrand(n + 0.13*t);
	return (nrnd0 + nrnd1 + nrnd2) / 3.0;
}
float n4rand(vec2 n)
{
	float t = fract(iTime);
	float nrnd0 = nrand(n + 0.07*t);
	float nrnd1 = nrand(n + 0.11*t);
	float nrnd2 = nrand(n + 0.13*t);
	float nrnd3 = nrand(n + 0.17*t);
	return (nrnd0 + nrnd1 + nrnd2 + nrnd3) / 4.0;
}
float n4rand_inv(vec2 n)
{
	float t = fract(iTime);
	float nrnd0 = nrand(n + 0.07*t);
	float nrnd1 = nrand(n + 0.11*t);
	float nrnd2 = nrand(n + 0.13*t);
	float nrnd3 = nrand(n + 0.17*t);
	float nrnd4 = nrand(n + 0.19*t);
	float v1 = (nrnd0 + nrnd1 + nrnd2 + nrnd3) / 4.0;
	float v2 = 0.5 * remap(0.0, 0.5, v1) + 0.5;
	float v3 = 0.5 * remap(0.5, 1.0, v1);
	return (nrnd4<0.5) ? v2 : v3;
}

/*
//alternate Gaussian,
//thanks to @self_shadow
float n4rand( vec2 n )
{
float nrnd0 = nrand( n + 0.07*fract( iTime ) );
float nrnd1 = nrand( n + 0.11*fract( iTime + 0.573953 ) );
return 0.23*sqrt(-log(nrnd0+0.00001))*cos(2.0*3.141592*nrnd1)+0.5;
}
*/
/*
//Mouse Y give you a curve distribution of ^1 to ^8
//thanks to Trisomie21
float n4rand( vec2 n )
{
float t = fract( iTime );
float nrnd0 = nrand( n + 0.07*t );

float p = 1. / (1. + iMouse.y * 8. / iResolution.y);
nrnd0 -= .5;
nrnd0 *= 2.;
if(nrnd0<0.)
nrnd0 = pow(1.+nrnd0, p)*.5;
else
nrnd0 = 1.-pow(nrnd0, p)*.5;
return nrnd0;
}
*/

float histogram(int iter, vec2 uv, vec2 interval, float height, float scale)
{
	float t = remap(interval.x, interval.y, uv.x);
	vec2 bucket = vec2(truncate(t, NUM_BUCKETS_F), truncate(t, NUM_BUCKETS_F) + 1.0 / NUM_BUCKETS_F);
	float bucketval = 0.0;
	for (int i = 0; i<ITER_PER_BUCKET; ++i)
	{
		float seed = float(i) / ITER_PER_BUCKET_F;

		float r;
		if (iter < 2)
			r = n1rand(vec2(uv.x, 0.5) + seed);
		else if (iter<3)
			r = n2rand(vec2(uv.x, 0.5) + seed);
		else if (iter<4)
			r = n2rand_faster(vec2(uv.x, 0.5) + seed);
		else if (iter<5)
			r = n3rand(vec2(uv.x, 0.5) + seed);
		else
			r = n4rand(vec2(uv.x, 0.5) + seed);

		bucketval += step(bucket.x, r) * step(r, bucket.y);
	}
	bucketval /= ITER_PER_BUCKET_F;
	bucketval *= scale;

	float v0 = step(uv.y / height, bucketval);
	float v1 = step((uv.y - 1.0 / iResolution.y) / height, bucketval);
	float v2 = step((uv.y + 1.0 / iResolution.y) / height, bucketval);
	return 0.5 * v0 + v1 - v2;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	fragCoord.y = iResolution.y - fragCoord.y;
	vec2 uv = fragCoord.xy / iResolution.xy;

	float o;
	int idx;
	vec2 uvrange;
	if (uv.x < 1.0 / 5.0)
	{
		o = n1rand(uv);
		idx = 1;
		uvrange = vec2(0.0 / 5.0, 1.0 / 5.0);
	}
	else if (uv.x < 2.0 / 5.0)
	{
		o = n2rand(uv);
		idx = 2;
		uvrange = vec2(1.0 / 5.0, 2.0 / 5.0);
	}
	else if (uv.x < 3.0 / 5.0)
	{
		o = n2rand_faster(uv);
		idx = 3;
		uvrange = vec2(2.0 / 5.0, 3.0 / 5.0);
	}
	else if (uv.x < 4.0 / 5.0)
	{
		o = n3rand(uv);
		idx = 4;
		uvrange = vec2(3.0 / 5.0, 4.0 / 5.0);
	}
	else
	{
		o = n4rand(uv);
		idx = 5;
		uvrange = vec2(4.0 / 5.0, 5.0 / 5.0);
	}

	//display histogram
	if (uv.y < 1.0 / 4.0)
		o = 0.125 + histogram(idx, uv, uvrange, 1.0 / 4.0, HIST_SCALE);

	//display lines
	if (abs(uv.x - 1.0 / 5.0) < 0.002) o = 0.0;
	if (abs(uv.x - 2.0 / 5.0) < 0.002) o = 0.0;
	if (abs(uv.x - 3.0 / 5.0) < 0.002) o = 0.0;
	if (abs(uv.x - 4.0 / 5.0) < 0.002) o = 0.0;
	if (abs(uv.y - 1.0 / 4.0) < 0.002) o = 0.0;


	fragColor = vec4(o, o, o, 1.0);
}




#endif

#if SHADERTOY == 50
// https://www.shadertoy.com/view/wslSRr

#define M_PI 3.1415926

#define MAT_BODY 1.0
#define MAT_FACE 2.0
#define MAT_HAND 3.0
#define MAT_BROW 4.0

#define SUN smoothstep(-0.5, 0.5, sunDir.y)
#define MOON smoothstep(-0.2, -0.5, sunDir.y)

float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(vec2 x) {
	vec2 i = floor(x), f = fract(x);

	float a = hash(i);
	float b = hash(i + vec2(1.0, 0.0));
	float c = hash(i + vec2(0.0, 1.0));
	float d = hash(i + vec2(1.0, 1.0));

	vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p) {
	const mat2 m2 = mat2(0.8, -0.6, 0.6, 0.8);

	float f = 0.5000 * noise(p); p = mul(m2, p) * 2.02;
	f += 0.2500 * noise(p); p = mul(m2, p) * 2.03;
	f += 0.1250 * noise(p); p = mul(m2, p) * 2.01;
	f += 0.0625 * noise(p);
	return f;
}

float sdSphere(vec3 p, float s)
{
	return length(p) - s;
}

float sdCapsule(vec3 p, vec3 a, vec3 b, float r)
{
	vec3 pa = p - a, ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	return length(pa - ba * h) - r;
}

vec2 opU(vec2 d1, vec2 d2)
{
	return (d1.x<d2.x) ? d1 : d2;
}

vec2 opS(vec2 d1, vec2 d2)
{
	return (-d1.x>d2.x) ? vec2(-d1.x, d1.y) : d2;
}

vec2 opSU(vec2 d1, vec2 d2, float k) {
	float h = clamp(0.5 + 0.5*(d2.x - d1.x) / k, 0.0, 1.0);
	return vec2(mix(d2.x, d1.x, h) - k * h*(1.0 - h), d1.y);
}

static mat2 rot(float th) { vec2 a = sin(vec2(1.5707963, 0) + th); return mat2(a.x, -a.y, a.y, a.x); }

vec2 thinkingFace(vec3 p)
{
	vec2 face = vec2(sdSphere(p, 1.0), MAT_BODY);

	vec3 q = p;
	q.x = abs(q.x);
	q.xz = mul(q.xz, rot(-.3));
	q.yz = mul(q.yz, rot(-0.25 + 0.05 * step(0.0, p.x)));
	q.y *= 0.8; q.z *= 2.0; q.z -= 2.0;
	vec2 eye = vec2(sdSphere(q, .11) * 0.5, MAT_FACE);

	q = p;
	q.x = abs(q.x);
	q.xz = mul(q.xz, rot(-.35));
	q.yz = mul(q.yz, rot(-0.62 + 0.26 * step(0.0, p.x) + pow(abs(q.x), 1.7) * 0.5));
	q.z -= 1.0;
	vec2 brow = vec2(sdCapsule(q, vec3(0.2, 0.0, 0.0), vec3(-.2, 0.0, 0.0), .05) * 0.5, MAT_BROW);

	q = p;
	q.yz = mul(q.yz, rot(0.2 + pow(abs(p.x), 1.8)));
	q.xy = mul(q.xy, rot(-0.25));
	q.z -= 1.0;
	vec2 mouth = vec2(sdCapsule(q, vec3(0.2, 0.0, 0.0), vec3(-.2, 0.0, 0.0), .045), MAT_BROW);

	p -= vec3(-.25, -.73, 1.0);
	p.xy = mul(p.xy, rot(0.2));
	q = p;
	q = (q * vec3(1.2, 1.0, 2.0));
	q -= vec3(0.0, 0.01, 0.0);
	vec2 hand = vec2(sdSphere(q, .3) * 0.5, MAT_HAND);

	q = p;

	vec2 finger1 = vec2(sdCapsule(q - vec3(0.3, 0.2, 0.02), vec3(0.2, 0.0, 0.0), vec3(-.2, 0.0, 0.0), .07), MAT_HAND);
	vec2 finger2 = vec2(sdCapsule(q * vec3(1.2, 1.0, .8) - vec3(0.2, 0.06, 0.02), vec3(0.1, 0.0, 0.0), vec3(-.1, 0.0, 0.0), .08), MAT_HAND);
	vec2 finger3 = vec2(sdCapsule(q * vec3(1.2, 1.0, .8) - vec3(0.15, -0.08, 0.015), vec3(0.1, 0.0, 0.0), vec3(-.1, 0.0, 0.0), .08), MAT_HAND);
	vec2 finger4 = vec2(sdCapsule(q * vec3(1.2, 1.0, .9) - vec3(0.1, -0.2, -0.01), vec3(0.1, 0.0, 0.0), vec3(-.1, 0.0, 0.0), .08), MAT_HAND);

	p -= vec3(-0.1, 0.3, 0.0);
	q = p;
	q.x -= q.y * 0.7;

	vec2 finger5 = vec2(sdCapsule(p, vec3(0.0, -0.2, 0.0) - q, vec3(0.0, 0.2, 0.0), .1 - p.y * 0.15), MAT_HAND);
	vec2 finger = opU(finger1, opU(finger5, opSU(finger2, opSU(finger3, finger4, 0.035), 0.035)));

	hand = opSU(hand, finger, 0.02);

	vec2 d = opU(eye, face);
	d = opU(brow, d);
	d = opS(mouth, d);
	d = opU(hand, d);
	return d;
}

// Starfield01 by xaot88
// https://www.shadertoy.com/view/Md2SR3
float Noise2d(in vec2 x)
{
	float xhash = cos(x.x * 37.0);
	float yhash = cos(x.y * 57.0);
	return fract(415.92653 * (xhash + yhash));
}

// Convert Noise2d() into a "star field" by stomping everthing below fThreshhold to zero.
float NoisyStarField(in vec2 vSamplePos, float fThreshhold)
{
	float StarVal = Noise2d(vSamplePos);
	if (StarVal >= fThreshhold)
		StarVal = pow((StarVal - fThreshhold) / (1.0 - fThreshhold), 6.0);
	else
		StarVal = 0.0;
	return StarVal;
}

// Stabilize NoisyStarField() by only sampling at integer values.
float StableStarField(in vec2 vSamplePos, float fThreshhold)
{
	// Linear interpolation between four samples.
	// Note: This approach has some visual artifacts.
	// There must be a better way to "anti alias" the star field.
	vSamplePos = mul(vSamplePos, rot(iTime*.5));
	float fractX = fract(vSamplePos.x);
	float fractY = fract(vSamplePos.y);
	vec2 floorSample = floor(vSamplePos);
	float v1 = NoisyStarField(floorSample, fThreshhold);
	float v2 = NoisyStarField(floorSample + vec2(0.0, 1.0), fThreshhold);
	float v3 = NoisyStarField(floorSample + vec2(1.0, 0.0), fThreshhold);
	float v4 = NoisyStarField(floorSample + vec2(1.0, 1.0), fThreshhold);

	float StarVal = v1 * (1.0 - fractX) * (1.0 - fractY)
		+ v2 * (1.0 - fractX) * fractY
		+ v3 * fractX * (1.0 - fractY)
		+ v4 * fractX * fractY;
	return StarVal;
}

vec2 map(vec3 p)
{
	vec2 think = thinkingFace(p - vec3(-0., 3.0, 0.));
	return think;
}

float hash(float n)
{
	return fract(sin(n)*43758.5453);
}

float noise(in vec3 x)
{
	vec3 p = floor(x);
	vec3 f = fract(x);

	f = f * f*(3.0 - 2.0*f);

	float n = p.x + p.y*57.0 + 113.0*p.z;

	float res = mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
		mix(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
		mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
			mix(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
	return res;
}

static const mat3 m = mat3(0.00, 0.80, 0.60,
	-0.80, 0.36, -0.48,
	-0.60, -0.48, 0.64);


float fbm(vec3 pos)
{
	float o = 0.;
	for (int i = 0; i < 5; i++)
	{
		o = o * 2. + abs(noise(pos)*2. - 1.);
		pos = mul(m, pos) * 2.;
	}
	return o / 40.0;
}

vec3 normal(in vec3 pos, float eps)
{
	vec2 e = vec2(1.0, -1.0)*0.5773*eps;
	float intensity = 0.02;
	float n1 = fbm(pos * 1.5 + e.xyy) * intensity;
	float n2 = fbm(pos * 1.5 + e.yyx) * intensity;
	float n3 = fbm(pos * 1.5 + e.yxy) * intensity;
	float n4 = fbm(pos * 1.5 + e.xxx) * intensity;
	return normalize(e.xyy*(map(pos + e.xyy).x + n1) +
		e.yyx*(map(pos + e.yyx).x + n2) +
		e.yxy*(map(pos + e.yxy).x + n3) +
		e.xxx*(map(pos + e.xxx).x + n4));
}

float shadow(in vec3 p, in vec3 l)
{
	float t = 0.15;
	float t_max = 20.0;

	float res = 1.0;
	for (int i = 0; i < 16; ++i)
	{
		if (t > t_max) break;

		float d = map(p + t * l).x;
		if (d < 0.01)
		{
			return 0.0;
		}
		t += d;
		res = min(res, 1.0 * d / t);
	}

	return res;
}

static vec3 sunDir;
vec3 fakeSky(vec3 rd)
{
	vec3 moning = mix(vec3(0.85, 0.5, 0.2)*1.1, vec3(0.2, 0.5, 0.85)*1.1, rd.y);
	vec3 noon = mix(vec3(0.2, 0.5, 0.85)*1.3, vec3(0.05, 0.2, 0.7), rd.y);
	vec3 night = mix(vec3(.5, 0.2, 0.05) * 2.0, vec3(0.3, 0.1, 0.5), clamp(pow(rd.y, 0.5), 0.0, 1.0));
	night = mix(night, vec3(0.0, 0.0, 0.0), MOON);

	// sky		
	vec3 col = noon;
	col = mix(moning, noon, SUN);
	col = mix(col, night, MOON);

	// stars
	col += (StableStarField(rd.xy * 1100.0, 0.99)) * 0.3;

	// Clouds
	vec3 cloud = mix(mix(vec3(1.0, 0.95, 1.0), vec3(1.0, 0.2, 0.1), 1.0 - SUN), (0.02), MOON);
	col = mix(col, cloud, 0.75 * step(0.0, rd.y) * smoothstep(0.4, 0.9, fbm(vec2(iTime, 0.0) + (rd.xz / rd.y) * 3.0)));

	return col * 3.0;
}

float ndfGGX(float NdotH, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (M_PI * denom * denom);
}

float gaSchlickG1(float theta, float k)
{
	return theta / (theta * (1.0 - k) + k);
}

float gaSchlickGGX(float NdotL, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	return gaSchlickG1(NdotL, k) * gaSchlickG1(NdotV, k);
}

vec3 fresnelSchlick_roughness(vec3 F0, float cosTheta, float roughness) {
	return F0 + (max((1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 shade(vec3 pos, vec3 albedo, float metalness, float roughness, vec3 N, vec3 V, vec3 L, vec3 Lradiance)
{
	vec3 H = normalize(L + V);
	float NdotV = max(0.0, dot(N, V));
	float NdotL = max(0.0, dot(N, L));
	float NdotH = max(0.0, dot(N, H));

	vec3 F0 = mix((0.04), albedo, metalness);

	vec3 F = fresnelSchlick_roughness(F0, max(0.0, dot(H, L)), roughness);
	float D = ndfGGX(NdotH, roughness);
	float G = gaSchlickGGX(NdotL, NdotV, roughness);

	vec3 kd = mix((1.0) - F, (0.0), metalness);

	float _shadow = shadow(pos, L);
	vec3 diffuseBRDF = kd * albedo / M_PI;
	vec3 specularBRDF = (F * D * G) / max(0.0001, 4.0 * NdotL * NdotV);

	return (diffuseBRDF + specularBRDF) * Lradiance * NdotL * _shadow;
}

// Unreal Engine Ambient BRDF Approx
// https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile?lang=en-US
vec3 EnvBRDFApprox(vec3 SpecularColor, float Roughness, float NoV)
{
	const vec4 c0 = vec4(-1, -0.0275, -0.572, 0.022);
	const vec4 c1 = vec4(1, 0.0425, 1.04, -0.04);
	vec4 r = Roughness * c0 + c1;
	float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
	vec2 AB = vec2(-1.04, 1.04) * a004 + r.zw;
	return SpecularColor * AB.x + AB.y;
}

// Specular Occlusion
// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/course-notes-moving-frostbite-to-pbr-v2.pdf
float so(float NoV, float ao, float roughness) {
	return clamp(pow(NoV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}

vec3 calcAmbient(vec3 pos, vec3 albedo, float metalness, float roughness, vec3 N, vec3 V, float t)
{
	vec3 F0 = mix((0.04), albedo, metalness);
	vec3 F = fresnelSchlick_roughness(F0, max(0.0, dot(N, V)), roughness);
	vec3 kd = mix((1.0) - F, (0.0), metalness);

	float aoRange = t / 40.0;
	float occlusion = max(0.0, 1.0 - map(pos + N * aoRange).x / aoRange);
	occlusion = min(exp2(-.8 * pow(occlusion, 2.0)), 1.0);

	vec3 ambientColor = mix(vec3(0.2, 0.5, 0.85)*0.5, vec3(0.2, 0.25, 0.8) * 0.75, 0.5 + 0.5*N.y);
	ambientColor = mix(vec3(0.3, 0.15, 0.05), ambientColor, 1.0 - smoothstep(0.2, -0.5, sunDir.y));
	ambientColor = mix((0.03), ambientColor, 1.0 - smoothstep(-0.2, -0.5, sunDir.y));

	vec3 diffuseAmbient = kd * albedo * ambientColor * min(1.0, 0.75 + 0.5*N.y) * 3.0;
	vec3 R = reflect(-V, N);

	vec3 col = mix(fakeSky(R) * pow(1.0 - max(-R.y, 0.0), 4.0), ambientColor, pow(roughness, 0.5));
	vec3 ref = EnvBRDFApprox(F0, roughness, max(dot(N, V), 0.0));
	vec3 specularAmbient = col * ref;

	diffuseAmbient *= occlusion;
	specularAmbient *= so(max(0.0, dot(N, V)), occlusion, roughness);

	return vec3(diffuseAmbient + specularAmbient);
}

vec3 materialize(vec3 p, vec3 ray, float depth, vec2 mat)
{
	vec3 col = (0.0);
	vec3 nor = normal(p, 0.001);
	vec3 sky = fakeSky(ray);
	if (depth > 2000.0) {
		col = sky;
	}
	else {
		float checker = mod(floor(p.x) + floor(p.z), 2.0);
		float roughness, metalness;
		if (mat.y == MAT_BODY) {
			col = pow(vec3(1.0, 204.0 / 255.0, 77.0 / 255.0), (2.2)) * 0.6;
			roughness = 0.5;
			metalness = 0.0;
		}
		else if (mat.y == MAT_FACE) {
			col = pow(vec3(102.0 / 255.0, 69.0 / 255.0, 0.0), (2.2)) * 0.6;
			roughness = 0.1;
			metalness = 0.0;
		}
		else if (mat.y == MAT_HAND) {
			col = pow(vec3(244.0 / 255.0, 144.0 / 255.0, 12.0 / 255.0), (2.2)) * 0.7;
			roughness = 0.8;
			metalness = 0.0;
		}
		else if (mat.y == MAT_BROW) {
			col = pow(vec3(102.0 / 255.0, 69.0 / 255.0, 0.0), (2.2)) * 0.8;
			roughness = 0.6;
			metalness = 0.0;
		}
		vec3 result = (0.);
		result += shade(p, col, metalness, roughness, nor, -ray, normalize(sunDir), vec3(1.0, 0.98, 0.95) * 100.) * SUN;
		result += shade(p, col, metalness, roughness, nor, -ray, normalize(-sunDir), vec3(1.0, 0.98, 0.95) * 2.) * MOON;
		result += calcAmbient(p, col, metalness, roughness, nor, -ray, depth);
		col = result;
	}
	float fo = 1.0 - exp(-0.0015*depth);
	col = mix(col, sky, fo);
	return col;
}

vec3 trace(vec3 p, vec3 ray)
{
	float t = 0.0;
	vec3 pos;
	vec2 mat;
	for (int i = 0; i < 100; i++) {
		pos = p + ray * t;
		mat = map(pos);
		if (mat.x < 0.00001) {
			break;
		}
		t += mat.x;
	}
	return materialize(pos, ray, t, mat);
}

mat3 camera(vec3 ro, vec3 ta, float cr)
{
	vec3 cw = normalize(ta - ro);
	vec3 cp = vec3(sin(cr), cos(cr), 0.);
	vec3 cu = normalize(cross(cw, cp));
	vec3 cv = normalize(cross(cu, cw));
	return mat3(cu, cv, cw);
}

float luminance(vec3 col)
{
	return dot(vec3(0.298912, 0.586611, 0.114478), col);
}

vec3 reinhard(vec3 col, float exposure, float white) {
	col *= exposure;
	white *= exposure;
	float lum = luminance(col);
	return (col * (lum / (white * white) + 1.0) / (lum + 1.0));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	fragCoord.y = iResolution.y - fragCoord.y;

	vec2 p = (fragCoord.xy * 2.0 - iResolution.xy) / min(iResolution.x, iResolution.y);

	float t = iTime * M_PI * 2.0 / 6.0 - M_PI * 0.5;
	sunDir = normalize(vec3(.5, sin(t), cos(t)));
	vec3 ro = vec3(-0.43 * 5.0, 1.3, 0.9 * 5.0);
	vec3 ta = vec3(2.4, 4.6, -0.3);
	mat3 c = camera(ro, ta, 0.0);
	vec3 ray = mul(c, normalize(vec3(p, 3.5)));
	vec3 col = trace(ro, ray);
	col = reinhard(col, .6, 30.0);
	col = pow(col, (1.0 / 2.2));
	fragColor = vec4(col, 1.0);
}
#endif

#if SHADERTOY == 51
static float cloudDensity = 0.4; // overall density [0,1]
static float noisiness = 0.35; // overall strength of the noise effect [0,1]
static float speed = 0.1; // controls the animation speed [0, 0.1 ish)
static float cloudHeight = 1.0; // (inverse) height of the input gradient [0,...)


// Simplex noise below = ctrl+c, ctrl+v:
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

vec3 mod289(vec3 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
    return mod289(((x * 34.0) + 1.0) * x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{
    const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
    vec3 i = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

// Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
    i = mod289(i);
    vec4 p = permute(permute(permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0/7.0
    vec3 ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z); //  mod(p,7*7)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_); // mod(j,N)

    vec4 x = x_ * ns.x + ns.yyyy;
    vec4 y = y_ * ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
    vec4 s0 = floor(b0) * 2.0 + 1.0;
    vec4 s1 = floor(b1) * 2.0 + 1.0;
    vec4 sh = -step(h, (0.0));

    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);

//Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

// Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, vec4(dot(p0, x0), dot(p1, x1),
                                dot(p2, x2), dot(p3, x3)));
}

/// Cloud stuff:
static const float maximum = 1.0 / 1.0 + 1.0 / 2.0 + 1.0 / 3.0 + 1.0 / 4.0 + 1.0 / 5.0 + 1.0 / 6.0 + 1.0 / 7.0 + 1.0 / 8.0;
// Fractal Brownian motion, or something that passes for it anyway: range [-1, 1]
float fBm(vec3 uv)
{
    float sum = 0.0;
    for (int i = 0; i < 8; ++i)
    {
        float f = float(i + 1);
        sum += snoise(uv * f) / f;
    }
    return sum / maximum;
}

// Simple vertical gradient:
float gradient(vec2 uv)
{
    return (1.0 - uv.y * uv.y * cloudHeight);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec3 p = vec3(uv, iTime * speed);
    vec3 someRandomOffset = vec3(0.1, 0.3, 0.2);
    vec2 duv = vec2(fBm(p), fBm(p + someRandomOffset)) * noisiness;
    float q = gradient(uv + duv) * cloudDensity;
    fragColor = vec4(q, q, q, 0.8);
}


#endif



// http://momentsingraphics.de/BlueNoise.html
float4 main(VS_OUT pin) : SV_TARGET
{
    float4 fragColor = 0;
    float2 fragCoord = pin.position.xy;

    mainImage(fragColor, fragCoord);
	
    return fragColor;
}

#undef texture
