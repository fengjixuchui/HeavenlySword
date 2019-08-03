#ifndef COMMON_H
#define COMMON_H
//--------------------------------------------------
//!
//!	Common functions used by debugshaders
//!
//--------------------------------------------------


float4 _RGB_to_LogYUV(float4 rgb)
{
	float gamut = 1.613f;

	// define useful constants
	float3 epsilon = { 0.000000001f, 0.000000001f, 0.000000001f };
	float unit = 1.0f/256.0f;
	float halfunit = 1.0f/512.0f;

	float3x3 mRGB_to_CIE = {
		{ 0.497f, 0.212671f, 0.023f },
		{ 0.339f, 0.715160f, 0.113f },
		{ 0.164f, 0.072169f, 0.864f }
	};

	// Luv color space don't like pure black 
	// (as a nice side effect it's now always safe to computer luminance
	rgb.xyz = max(rgb.xyz, epsilon);

	// convert our colour from RGB space to CIE space
	float3 cie = mul(float3(rgb), mRGB_to_CIE);
	
	// get the luminance..
	float Le = (255.0f/32.0f)*min(log2(cie.y) + 16.0f, 32.0f);


	// pack the luminance into 2 channels..
	float Le_LSBs = frac(Le);
	float Le_MSBs = (Le - (floor(Le_LSBs*255.0f))/255.0f)/255.0f;

	// from YUV chrominance to Luv chromacity 
	float2 uv = float2(gamut * 4.0f, gamut * 9.0f ) * cie.xy / dot(cie, float3(1.0f, 15.0f, 3.0f));
	
	return float4(uv, Le_LSBs , Le_MSBs);
}


float get_logluminance_LogLuv( float4 linear_LogLuv )
{
	return (32.0f*(linear_LogLuv.w  + (linear_LogLuv.z / 255.0f)) - 16.0f);
}


float4 _LogYUV_to_RGB(float4 yuv)
{

	float gamut = 1.613f;

	// define useful constants
	float3 epsilon = { 0.00001f, 0.00001f, 0.00001f };
	float unit = 1.0f/256.0f;
	float halfunit = 1.0f/512.0f;


	float3x3 mCIE_to_RGB = {
		{  2.51380f, -0.75070f,  0.03126f },
		{  -1.13113f,  1.75478f, -0.19939f },
		{ -0.38267f, -0.00408f,   1.16813f }
	};

	float3 XYZ;
	XYZ.y = exp2( get_logluminance_LogLuv(yuv) );
	
	
	XYZ.x = (9.0f / gamut)* yuv.x * XYZ.y / ((4.0f / gamut) * yuv.y) ;
	
	XYZ.z = dot(float3(yuv.xy, 1.0f), float3(-3.0f / gamut, -20.0f / gamut, 12.0f)) * XYZ.y / ((4.0f / gamut) * yuv.y) ;

	return float4(mul(XYZ,mCIE_to_RGB) , 0.5f);
}


float4 _Lerp_NAO32(float4 incoming_RGB, float4 framebuffer_NAO32)
{
	float gamut = 1.613f;

	// define useful constants
	float3 epsilon = { 0.00001f, 0.00001f, 0.00001f };
	float unit = 1.0f/256.0f;
	float halfunit = 1.0f/512.0f;

	float3 framebuffer_XYZ;
	framebuffer_XYZ.y = exp2( get_logluminance_LogLuv(framebuffer_NAO32) );
	framebuffer_XYZ.x = (9.0f / gamut)* framebuffer_NAO32.x * framebuffer_XYZ.y / ((4.0f / gamut) * framebuffer_NAO32.y) ;
	framebuffer_XYZ.z = dot(float3(framebuffer_NAO32.xy, 1.0f), float3(-3.0f / gamut, -20.0f / gamut, 12.0f)) * framebuffer_XYZ.y / ((4.0f / gamut) * framebuffer_NAO32.y) ;

	float3x3 mRGB_to_CIE = {
		{ 0.497f, 0.212671f, 0.023f },
		{ 0.339f, 0.715160f, 0.113f },
		{ 0.164f, 0.072169f, 0.864f }
	};

	incoming_RGB.xyz = max(incoming_RGB.xyz, epsilon);

	// convert our colour from RGB space to CIE space
	float3 incoming_XYZ = mul(incoming_RGB.xyz, mRGB_to_CIE);
	
	float3 output = lerp( framebuffer_XYZ, incoming_XYZ, incoming_RGB.w );
	
		
	// get the luminance..
	float Le = (255.0f/32.0f)*min(log2(output.y) + 16.0f, 32.0f);


	// pack the luminance into 2 channels..
	float Le_LSBs = frac(Le);
	float Le_MSBs = (Le - (floor(Le_LSBs*255.0f))/255.0f)/255.0f;

	// from YUV chrominance to Luv chromacity 
	float2 uv = float2(gamut * 4.0f, gamut * 9.0f ) * output.xy / dot(output, float3(1.0f, 15.0f, 3.0f));
	
	return float4(uv, Le_LSBs , Le_MSBs);
}





// standard affine matrix mul on a position
float4 affine_mul_point( float4 point, float4x4 transform )
{
	return transform[0]*point.xxxx 
		+ transform[1]*point.yyyy
		+ transform[2]*point.zzzz
		+ transform[3];
}
float3 affine_mul_point( float3 point, float4x4 transform )
{
	return transform[0].xyz*point.xxx 
		+ transform[1].xyz*point.yyy
		+ transform[2].xyz*point.zzz
		+ transform[3].xyz;
}
float3 affine_mul_point( float3 point, float4x3 transform )
{
	return transform[0]*point.xxx 
		+ transform[1]*point.yyy
		+ transform[2]*point.zzz
		+ transform[3];
}

// standard affine matrix mul on a direction
float3 affine_mul_direction( float3 direction, float4x4 transform )
{
	return transform[0].xyz*direction.xxx 
		+ transform[1].xyz*direction.yyy
		+ transform[2].xyz*direction.zzz;
}
float3 affine_mul_direction( float3 direction, float3x3 transform )
{
	return transform[0]*direction.xxx 
		+ transform[1]*direction.yyy
		+ transform[2]*direction.zzz;
}

// linear RGB to luminance
float get_luminance( float3 linear_rgb )
{
	return dot( linear_rgb, float3( 0.212671f, 0.715160f, 0.072169f ) );
}


// tonemaps an exposed colour into a displayable range
// (this function assumes the exposed colour is still in linear RGB space)
float3 tonemap( float3 exposed_colour, float white_luminance_rcp_sq )
{
	float exposed_luminance = get_luminance( exposed_colour );
	float tonemap = ( 1.0f + exposed_luminance * white_luminance_rcp_sq ) / ( 1.0f + exposed_luminance );

	return exposed_colour * tonemap.xxx;
}

// reversed tone mapping
float3 reverse_tonemap( float3 ldr_color, float white_luminance_rcp_sq )
{
	return ( 1.0f.xxx - ldr_color ) / ( ldr_color - white_luminance_rcp_sq.xxx );
}


half encode_luminance( half luminance )
{
	half log_luminance = log2( luminance );
	half compress_luminance = (1.0f / (4.0f + 8.0f)) * ( log_luminance + 4.0f );
	
	return compress_luminance;
}

half decode_luminance( half encoded_luminance )
{
	half log_luminance = ( log_luminance * (4.0f + 8.0f) ) - 4.0f;
	half luminance = pow( log_luminance, 2.0f );
	
	return luminance;
}
 
half4 opaque_tonemap( half4 colour, half linear_scale, half white_luminance_rcp_sq )
{
	half3 exposed_colour = linear_scale.xxx * colour.xyz;
	half exposed_luminance = get_luminance( exposed_colour );
	
	half tonemap = ( 1.0f + exposed_luminance * white_luminance_rcp_sq ) / ( 1.0f + exposed_luminance );
	
	return ( exposed_colour * tonemap.xxx, encode_luminance( exposed_luminance ) );
}

half4 transparent_tonemap( half4 colour, half linear_scale, half white_luminance_rcp_sq )
{
	half3 exposed_colour = linear_scale.xxx * colour.xyz;
	half exposed_luminance = get_luminance( exposed_colour );
	
	half tonemap = ( 1.0f + exposed_luminance * white_luminance_rcp_sq ) / ( 1.0f + exposed_luminance );
	
	return ( exposed_colour * tonemap.xxx, colour.w );
}	

// used by several shaders now so seperated into its own funciton
float4 effects_scale_and_filter_hdr( half2 texcoord, 
								uniform half4 offset, 
								uniform half4 scale, 
								uniform half4 filter_offsets, 
								uniform sampler2D source)
{
	half4 colour = tex2D( source, texcoord + filter_offsets.xy )
		+ tex2D( source, texcoord + filter_offsets.xw )
		+ tex2D( source, texcoord + filter_offsets.zy )
		+ tex2D( source, texcoord + filter_offsets.zw );

	return ( colour - offset )*scale;
}

float4 effects_scale_and_filter_LogLuv( half2 texcoord, 
								uniform half4 offset, 
								uniform half4 scale, 
								uniform half4 filter_offsets, 
								uniform sampler2D source)
{
	half4 colour = _LogYUV_to_RGB(tex2D( source, texcoord + filter_offsets.xy ))
		+ _LogYUV_to_RGB(tex2D( source, texcoord + filter_offsets.xw ))
		+ _LogYUV_to_RGB(tex2D( source, texcoord + filter_offsets.zy ))
		+ _LogYUV_to_RGB(tex2D( source, texcoord + filter_offsets.zw ));

	//colour = _LogYUV_to_RGB( colour );

	return ( colour - offset)*scale ;
}

// depth_haze used by skybox effect, and others
void depth_haze(	in float4 objectPos, 
					out float3 extinction,
					out float3 inscattering,
					uniform float4x4 worldViewMatrix,
					uniform float3 eyePosInObj,
					uniform float3 aConsts,				// <1, log_2 e, termMultiplier >
					uniform float3 gConsts,				// < ((1-g)^2), SunPower, SunMultipler>
					uniform float3 sunDir,
					uniform float3 beta1PlusBeta2,
					uniform float3 betaDash1,
					uniform float3 betaDash2, 
					uniform float3 oneOverBeta1PlusBeta2,
					uniform float4 sunColour	)
{
	// distance s
	float s = mul( objectPos, worldViewMatrix ).z;

	// extinction term E
	float3 Fex = -(beta1PlusBeta2 * s.xxx) ;
	Fex = exp( Fex );// * aConsts.yyy;
	// reflanctance to total net effective E
	extinction = Fex; // do multiple in pixel shader * surfaceReflectance;
	
	// Calc V
	float3 V = eyePosInObj - objectPos.xyz;
	V = normalize( V );
	
	// angle between sun and view direction (both in object space)
	float cosTheta = dot(V, sunDir );
	float phase1Theta = aConsts.x + pow(cosTheta,2); 
	
	// ATI phaseTheta seems to produce a ring rather than a disc which looks bad very very bad
	// I've replaced it with a simple phong style exponent seems to do a better job, considering
	// we want to put a sun/moon graphic over the spot anyway
	float phase2Theta = pow( max(cosTheta,0.00001), gConsts.y);
	float val = phase2Theta * gConsts.z;
	phase2Theta = gConsts.x * val;

	// inscattering (I)
	float3 I = (betaDash1 * phase1Theta) + (betaDash2 * phase2Theta);
	inscattering = I * oneOverBeta1PlusBeta2 * aConsts.zzz * (aConsts.xxx - Fex) * sunColour.xyz * sunColour.w;

	// extinction can be low precision (diffuse channel)
	// inscattering is needed at high dynamic range
}

// pixel shader combines depth_haze with shader colour (all RGB)
float3 depth_haze_combine(	float3 shaderColour,
							float3 extinction,
							float3 inScatter )
{
	return ( (extinction * shaderColour) + inScatter );
}

// computes the colour from ray and a spherical harmonic environment
float3 sh_lookup( float3 ray, float4x4 sharmonics[3] )
{
	// FIXME: remove the temp
	float4 temp;
	temp.xyz = ray;
	temp.w = 1.0f;

	float3 result;
	result.x = dot( mul( sharmonics[0], temp ), temp );
	result.y = dot( mul( sharmonics[1], temp ), temp );
	result.z = dot( mul( sharmonics[2], temp ), temp );
	return result;
}

// depth blur effect
float ComputeDepthBlur( float depth, float nearBlurDepth, float focalPlaneDepth, float farBlurDepth )
{
	float f;

	if (depth < focalPlaneDepth)
	{
		// scale depth value between near blur distance and focal distance to
		// [-1, 0] range
		f = (depth - focalPlaneDepth)/(focalPlaneDepth - nearBlurDepth);
	}
	else
	{
		// scale dept value between focal distance and far blur distance to
		// [0, 1] range
		f = (depth - focalPlaneDepth)/(farBlurDepth - focalPlaneDepth);
	}
	// scale and bias into [0, 1] range
	f = (f * 0.5f) + 0.5f;
	return clamp( f, 0, 1 );
}

#define DIAG 0.707f
static const half4 shadowCheckBoundaries[4] = { {	 0.0f,	 1.0f,
													-1.0f,	 0.0f		},
												{	 0.0f,	-1.0f,
													 1.0f,	 0.0f		},
												{	-DIAG,	 DIAG,
													-DIAG,	-DIAG		},
												{	 DIAG,	-DIAG,
													 DIAG,	 DIAG		}	};
													
 

#define NUM_SHADOW_FILTER_TAPS 16
static const half4 shadowFilterTaps[NUM_SHADOW_FILTER_TAPS/2] = {	{-0.326212f, -0.405805f,
															 -0.840144f, -0.07358f  },
															{-0.695914f,  0.457137f,
															 -0.203345f,  0.620716f },
															{ 0.96234f,  -0.194983f,
															  0.473434f, -0.480026f },
															{ 0.519456f,  0.767022f,
															  0.185461f, -0.893124f },
															{ 0.507431f,  0.064425f,
															  0.89642f,   0.412458f },
															{-0.32194f,  -0.932615f,
															 -0.1791559f, -0.597705f } ,
															 {-0.132194f,  -0.932615f,
															 -0.2791559f, -0.1597705f } ,
															 {-0.232194f,  -0.932615f,
															 -0.2791559f, -0.597705f } 






};

static const half2 shadowFilterTaps2[24] =	{	
												{ -0.326212f, -0.405805f },
												{ -0.840144f, -0.07358f  },
												{ -0.695914f,  0.457137f },
												{ -0.203345f,  0.620716f },
												{  0.96234f,  -0.194983f },
												{  0.473434f, -0.480026f },
												{  0.519456f,  0.767022f },	
												{  0.185461f, -0.893124f },
												{  0.507431f,  0.064425f },
												{  0.89642f,   0.412458f },
												{ -0.32194f,  -0.932615f },
												{ -0.1791559f, -0.597705f},

												{ -0.326212f*0.6f, -0.405805f*0.6f },
												{ -0.840144f*0.6f, -0.07358f*0.6f  },
												{ -0.695914f*0.6f,  0.457137f*0.6f },
												{ -0.203345f*0.6f,  0.620716f*0.6f },
												{  0.96234f*0.6f,  -0.194983f*0.6f },
												{  0.473434f*0.6f, -0.480026f*0.6f },
												{  0.519456f*0.6f,  0.767022f*0.6f },	
												{  0.185461f*0.6f, -0.893124f*0.6f },
												{  0.507431f*0.6f,  0.064425f*0.6f },
												{  0.89642f*0.6f,   0.412458f*0.6f },
												{ -0.32194f*0.6f,  -0.932615f*0.6f },
												{ -0.1791559f*0.6f, -0.597705f*0.6f}
											};

half filtered_shadow_receiveSMH( sampler2D shadowMap, float3 texcoord, half4 shadowRadii, half spot_size )
{
	float shadowTestDepth = (texcoord.z);
	float2 projCoord = (texcoord.xy);

	half colour = 0;

	half4 scaleFactor = shadowRadii.xyxy * spot_size;

#define nsamples 12

	for(int i=0;i < nsamples;i++)
	{


			colour += tex2D(shadowMap, float3(projCoord + shadowFilterTaps2[i]*scaleFactor,shadowTestDepth ) );

	}

	
	half finalcolour;
	
	finalcolour  =  (colour * (1.0f/nsamples) ); 
	
	return (1.0f -finalcolour);   
}


half4 filtered_quad_shadow_receiveSMH_lowquality( sampler2D shadowMap0,
									half4 shadowRadii0,
									float4 texcoord0,
									half4 posInScreenS)
									
{
	half scrZ = saturate( posInScreenS.z/posInScreenS.w - 0.991f);
	half spot_size = (scrZ*20.f) + 0.05f;
	
	return filtered_shadow_receiveSMH( shadowMap0, texcoord0, shadowRadii0, spot_size ).xxxx;
}

half4 filtered_quad_shadow_receiveSMH( sampler2D shadowMap0,
									half4 clip0,
									half4 clip1,
									half4 clip2,
									half4 shadowRadii0,
									half4 shadowRadii1,
									half4 shadowRadii2,
									float4 texcoord0,
									float4 texcoord1,
									float4 texcoord2,
									half4 posInScreenS)
									
{
	half4 regionD;
	
	// dot 4 planes into a register
	regionD.x = dot( posInScreenS, clip0 );
	regionD.y = dot( posInScreenS, clip1 );
	regionD.z = dot( posInScreenS, clip2 );
	//regionD.w = dot( posInScreenS, clip3 );
	
	half scrZ = saturate( posInScreenS.z/posInScreenS.w - 0.991f);
	half spot_size = (scrZ*20.f) + 0.05f;
	
	// classify each plane simulatously and
	// get a unique number indicated which plane this pixel is in (it can only every be in one region at any one time)
	//half region = dot( (regionD >= 0.f), float4(1.f, 2.f, 4.f, 8.f) );
	
	
	// 'quick' shadow maps selection (it appears to be 30% faster than shadow maps selection via dynamic branching)
	half4 mask = 0.0f.xxxx;

	if (regionD.x >= 0) mask.x = 1;
	if (regionD.x < 0 && regionD.y >= 0) mask.y = 1;	
	if (regionD.y < 0 && regionD.z >= 0) mask.z = 1;	
	//if (regionD.z < 0 && regionD.w >= 0) mask.w = 1;	

	float4 texcoord = texcoord0*mask.x + texcoord1*mask.y + texcoord2*mask.z;
	float4 radii = shadowRadii0*mask.x + shadowRadii1*mask.y + shadowRadii2*mask.z;
	
	return filtered_shadow_receiveSMH( shadowMap0, texcoord, radii, spot_size );


	// FULL DYNAMIC BRANCHING SHADOW MAP SELECTION
	/*
	half shadowterm = 0.1f;

	// these are float compares with the numbers being ... in last plane will be 8 = 8, 8+4 = 12, 8+4+2 = 14, 8+4+2+1 = 15
	if( region < 8.5f )
	{
		shadowterm = filtered_shadow_receiveSMH( shadowMap0, texcoord3, shadowRadii3, spot_size ); 
	} else if( region < 12.5f )
	{
		shadowterm = filtered_shadow_receiveSMH( shadowMap0, texcoord2, shadowRadii2, spot_size );
	} else if( region < 14.5f )
	{
		shadowterm = filtered_shadow_receiveSMH( shadowMap0, texcoord1, shadowRadii1, spot_size );
	} else if( region < 15.5f )
	{
		shadowterm = filtered_shadow_receiveSMH( shadowMap0, texcoord0, shadowRadii0, spot_size );
	}

	return shadowterm; */


	// FULL STATIC BRANCHING SHADOW MAPS SELECTION
	/*
	half4 color;
	color.x = 	filtered_shadow_receiveSMH( shadowMap3, texcoord3, shadowRadii3, spot_size );
	color.y = 	filtered_shadow_receiveSMH( shadowMap2, texcoord2, shadowRadii2, spot_size );
	color.z = 	filtered_shadow_receiveSMH( shadowMap1, texcoord1, shadowRadii1, spot_size );
	color.w = 	filtered_shadow_receiveSMH( shadowMap0, texcoord0, shadowRadii0, spot_size );
	
	half4 select = half4( 0.0f, 0.0f, 0.0f, 0.0f);
	
	if (region < 8.5f) select.x = 1.0f;
	if (region >= 8.5f && region < 12.5f) select.y = 1.0f;
	if (region >= 12.5f && region < 14.5f) select.z = 1.0f;
	if (region >= 14.5f && region < 15.5f) select.w = 1.0f;
	
	half4 color0 = {1.0f, 0.0f, 1.0f, 0.0f};
	half4 color1 = {1.0f, 0.0f, 0.0f, 0.0f};
	half4 color2 = {0.0f, 1.0f, 0.0f, 0.0f};
	half4 color3 = {0.0f, 0.0f, 1.0f, 0.0f};
		
	//return (dot(color, select))*(color0*select.x  + color1*select.y + color2*select.z + color3*select.w);
	
	return (dot(color, select));
	
	//return (texcoord1.xyzw/texcoord1.wwww); */


			
}	


half filtered_shadow_receive_variable( sampler2D shadowMap, float3 texcoord, half4 shadowRadii, half spot_size, half weight, int iterations )
{
	half4 scaleFactor = shadowRadii.xyxy * spot_size;

	half colour = 0;
	for(int i=0;i < iterations ;i++)
		colour += tex2D(shadowMap, float3( texcoord.xy  + shadowFilterTaps2[i]*scaleFactor, texcoord.z ) );

	return colour * weight;
  
}

half4 filtered_quad_shadow_receiveSMHZ( sampler2D shadowMap,
									float3 shadowPlanes,
									half4 shadowRadii0,
									half4 shadowRadii1,
									half4 shadowRadii2,
									float3 texcoord0,
									float3 texcoord1,
									float3 texcoord2,
									float posZ)
									
{
	//half scrZ = saturate( posInScreenS.z/posInScreenS.w - 0.991f);
	//half spot_size = (scrZ*20.f) + 0.05f;


	half4 color;

	float3 texcoord;
	float4 radii;


	if (posZ <= shadowPlanes.x)
	{
		texcoord = texcoord0;
		radii = shadowRadii0;
	}
	else if (posZ > shadowPlanes.y)
	{
		texcoord = texcoord2;
		radii = shadowRadii2;
	}
	else
	{
		texcoord = texcoord1;
		radii = shadowRadii1;
	}
		
	color = filtered_shadow_receiveSMH( shadowMap, texcoord, radii, 0.04f);


	return  color;
}	

// skins a matrix
#define MAX_BONES					64

float3x4 skin_matrix( float4 weights, int4 indices, float4 skinmatrixrows[3*MAX_BONES] )
{
	float3x4 worldtransform;

	weights.x = 1.0f - dot( weights.yzw, 1.0f.xxx );

	worldtransform[0] = weights.xxxx*skinmatrixrows[indices.x];
	worldtransform[1] = weights.xxxx*skinmatrixrows[indices.x + 1];
	worldtransform[2] = weights.xxxx*skinmatrixrows[indices.x + 2];

	worldtransform[0] += weights.yyyy*skinmatrixrows[indices.y];
	worldtransform[1] += weights.yyyy*skinmatrixrows[indices.y + 1];
	worldtransform[2] += weights.yyyy*skinmatrixrows[indices.y + 2];

	worldtransform[0] += weights.zzzz*skinmatrixrows[indices.z];
	worldtransform[1] += weights.zzzz*skinmatrixrows[indices.z + 1];
	worldtransform[2] += weights.zzzz*skinmatrixrows[indices.z + 2];

	worldtransform[0] += weights.wwww*skinmatrixrows[indices.w];
	worldtransform[1] += weights.wwww*skinmatrixrows[indices.w + 1];
	worldtransform[2] += weights.wwww*skinmatrixrows[indices.w + 2];

	return worldtransform;
}

// skins a position
float4 skin_point( float3x4 skinmatrix, float4 point )
{
	return float4(
		dot( skinmatrix[0], point ), 
		dot( skinmatrix[1], point ), 
		dot( skinmatrix[2], point ), 
		point.w
	);
}

void computeFresnelTerm(in float3 in_world_view_dir,
						in float3 in_world_normal,
						in float in_fresnel_strength,
						out float out_fresnel_term)
{
//	float dummy = 1.0f + 0.5f * min( dot( in_world_view_dir, in_world_normal ), 0.0f);
	float dummy = 0.2f + 0.8f * min( dot( in_world_view_dir, in_world_normal ), 0.0f);
	out_fresnel_term = float( lerp( 1.0f, dummy, in_fresnel_strength ) );
}

#endif