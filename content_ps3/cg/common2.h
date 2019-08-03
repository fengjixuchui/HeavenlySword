void depth_haze( in float4 objectPos, 
			out half3 extinction,
			out half3 inscattering,
		
			uniform float4x4 worldviewMatrix,

			uniform float3 eyePosInObj,
			uniform float3 aConsts, // <1, log_2 e, termMultiplier >
			uniform float3 gConsts, // < ((1-g)^2), SunPower, SunMultipler>
			uniform float3 sunDir,
			uniform float3 beta1PlusBeta2,
			uniform float3 betaDash1,
			uniform float3 betaDash2, 
			uniform float3 oneOverBeta1PlusBeta2,
			uniform float4 sunColour	)
{
	// distance s
	float s = length( mul( objectPos, worldviewMatrix ) );

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
	float phase1Theta = aConsts.x + cosTheta * cosTheta; 

	// ATI phaseTheta seems to produce a ring rather than a disc which looks bad very very bad
	// I've replaced it with a simple phong style exponent seems to do a better job, considering
	// we want to put a sun/moon graphic over the spot anyway
		
	float phase2Theta = 0;
	if (cosTheta > 0)
	{
		phase2Theta = pow( cosTheta, gConsts.y );
		phase2Theta = phase2Theta * gConsts.x * gConsts.z;	
	}

	// inscattering (I)
	float3 I = (betaDash1 * phase1Theta) + (betaDash2 * phase2Theta);
	inscattering = I * oneOverBeta1PlusBeta2 * aConsts.zzz * (aConsts.xxx - Fex) * sunColour.xyz * sunColour.w;

	// extinction can be low precision (diffuse channel) because this is sometimes on SM3 we clamp here
	// inscattering is needed at high dynamic range but needs clamping to 0
	extinction = clamp( extinction, 0, 1 );
	inscattering = max(inscattering,0);
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

half3 irradiance_lookup( samplerCUBE irradiance_cache, float3 ray )
{
	return h3texCUBE( irradiance_cache, ray );
}


// pixel shader combines depth_haze with shader colour (all RGB)
float3 depth_haze_combine(	half3 shaderColour,
							float3 extinction,
							half3 inScatter )
{
	return ( (extinction * shaderColour) + inScatter );
}


// pixel shader combines depth_haze with shader colour (all RGB) for the unlit material.
// ok...it's just a quick hack for the sky box so that we can get some haze on the horizon
// but no haze on the azimuth (marco)
float3 unlit_depth_haze_combine(half3 shaderColour,
								half3 inScatter,
								float3 in_worldspace_normal,
								half in_haze_kill_power )
								
{
	half hazekiller = saturate(1 - abs(in_worldspace_normal.y));
	hazekiller = pow(hazekiller, in_haze_kill_power);
	
	return ( shaderColour + inScatter * hazekiller );
}
// skins a matrix
float3x4 skin_matrix( float4 weights, int4 indices, float4 skinmatrixrows[3*64] )
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

// skins a direction
float4 skin_direction( float3x4 skinmatrix, float4 direction )
{
	return float4(
		dot( skinmatrix[0].xyz, direction.xyz ), 
		dot( skinmatrix[1].xyz, direction.xyz ), 
		dot( skinmatrix[2].xyz, direction.xyz ), 
		direction.w
	);
}
float3 skin_direction( float3x4 skinmatrix, float3 direction )
{
	return float3(
		dot( skinmatrix[0].xyz, direction ), 
		dot( skinmatrix[1].xyz, direction ), 
		dot( skinmatrix[2].xyz, direction )
	);
}


// single sample of a depth map with clipping planes
float clipped_depth_compare( sampler2D depths, float4 texcoord, float4 posInScreenS, float4 clip0, float4 clip1 )
{
	float d0 = dot( posInScreenS, clip0 );
	float d1 = dot( posInScreenS, clip1 );
	if( d0 >= 0.f && d1 >= 0.f )
	{
		float  shadowTestDepth = (texcoord.z / texcoord.w);
		float  shadowDepth = tex2Dproj(depths, texcoord);
		float  sample  = (shadowTestDepth >= shadowDepth);

		return sample;
	}
	else
	{
		return 0.0f; // no shadowing
	}
}

#define NUM_SHADOW_FILTER_TAPS 12
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
															 -0.791559f, -0.597705f }};

half filtered_shadow_receive( sampler2D shadowMap, float4 texcoord, half4 shadowRadii, half spot_size )
{
	half shadowTestDepth = (texcoord.z / texcoord.w);
	half4 projCoord = (texcoord.xy / texcoord.w).xyxy; 
	
	half colour = 0;

	for(int i=0;i < NUM_SHADOW_FILTER_TAPS/4;i++)
	{
		half4 shadowDepths;
		half4 tap0 = shadowFilterTaps[(i*2)].xyzw * shadowRadii.xyxy * spot_size;
		half4 tap1 = shadowFilterTaps[(i*2)+1].xyzw * shadowRadii.xyxy * spot_size;

		half4 offsetCoord01 = projCoord + tap0;
		half4 offsetCoord23 = projCoord + tap1;

		shadowDepths.x = tex2Dlod(shadowMap, half4(offsetCoord01.xy,0,0) );
		shadowDepths.y = tex2Dlod(shadowMap, half4(offsetCoord01.zw,0,0) );
		shadowDepths.z = tex2Dlod(shadowMap, half4(offsetCoord23.xy,0,0) );
		shadowDepths.w = tex2Dlod(shadowMap, half4(offsetCoord23.zw,0,0) );
		
		half4  samples = (shadowTestDepth >= shadowDepths);
		colour += dot( samples, 1 );
	}
		
	return colour / NUM_SHADOW_FILTER_TAPS;
}

half filtered_quad_shadow_receive( sampler2D shadowMap0,
									sampler2D shadowMap1,
									sampler2D shadowMap2,
									sampler2D shadowMap3,
									half4 clip0,
									half4 clip1,
									half4 clip2,
									half4 clip3,
									half4 shadowRadii0,
									half4 shadowRadii1,
									half4 shadowRadii2,
									half4 shadowRadii3,
									half4 texcoord0,
									half4 texcoord1,
									half4 texcoord2,
									half4 texcoord3,
									half4 posInScreenS,
									half4 posInWorldS,
									half4 resolution )
{
	half4 regionD;
	
	// dot 4 planes into a register
	regionD.x = dot( posInScreenS, clip0 );
	regionD.y = dot( posInScreenS, clip1 );
	regionD.z = dot( posInScreenS, clip2 );
	regionD.w = dot( posInScreenS, clip3 );
	
//	half scrZ = saturate( (posInScreenS.z/posInScreenS.w) - 0.991f);
	half spot_size = /*(scrZ*250.f) + */0.05f;
	
	// classify each plane simulatously and
	// get a unique number indicated which plane this pixel is in (it can only every be in one region at any one time)
	half region = dot( (regionD >= 0.f), float4(1.f, 2.f, 4.f, 8.f) );

	// these are float compares with the numbers being ... in last plane will be 8 = 8, 8+4 = 12, 8+4+2 = 14, 8+4+2+1 = 15
	if( region < 8.5f )
	{
		return filtered_shadow_receive( shadowMap3, texcoord3, shadowRadii3, spot_size );
	} else if( region < 12.5f )
	{
		return filtered_shadow_receive( shadowMap2, texcoord2, shadowRadii2, spot_size );
	} else if( region < 14.5f )
	{
		return filtered_shadow_receive( shadowMap1, texcoord1, shadowRadii1, spot_size );
	} else if( region < 15.5f )
	{
		return filtered_shadow_receive( shadowMap0, texcoord0, shadowRadii0, spot_size );
	} else
	{
		// this case means that this misses all frustum could happen if the 4 shadow maps don't cover the entire scene
		return 0.f;
	}
}


half filtered_shadow_receiveSMH_SB( sampler2D shadowMap, float4 texcoord, half4 shadowRadii, half spot_size )
{
	float shadowTestDepth = (texcoord.z / texcoord.w);
	float4 projCoord = (texcoord.xy / texcoord.w).xyxy; 
	
	half colour = 0;
	
	for(int i=0;i < NUM_SHADOW_FILTER_TAPS/4;i++)
	{
		half4 tap0 = shadowFilterTaps[(i*2)].xyzw * shadowRadii.xyxy * spot_size;
		half4 tap1 = shadowFilterTaps[(i*2)+1].xyzw * shadowRadii.xyxy * spot_size;

		float4 offsetCoord01 = projCoord + tap0;
		float4 offsetCoord23 = projCoord + tap1;

		colour += tex2D(shadowMap, float3(offsetCoord01.xy,shadowTestDepth));
		colour += tex2D(shadowMap, float3(offsetCoord01.zw,shadowTestDepth));
		colour += tex2D(shadowMap, float3(offsetCoord23.xy,shadowTestDepth));
		colour += tex2D(shadowMap, float3(offsetCoord23.zw,shadowTestDepth));
	}
	
	
	for(int i=0;i < NUM_SHADOW_FILTER_TAPS/4;i++)
	{
		half4 tap0 = shadowFilterTaps[(i*2)].xyzw * shadowRadii.xyxy * spot_size * 0.7;
		half4 tap1 = shadowFilterTaps[(i*2)+1].xyzw * shadowRadii.xyxy * spot_size * 0.7;

		float4 offsetCoord01 = projCoord + tap0;
		float4 offsetCoord23 = projCoord + tap1;

		colour += tex2D(shadowMap, float3(offsetCoord01.xy,shadowTestDepth) );
		colour += tex2D(shadowMap, float3(offsetCoord01.zw,shadowTestDepth) );
		colour += tex2D(shadowMap, float3(offsetCoord23.xy,shadowTestDepth) );
		colour += tex2D(shadowMap, float3(offsetCoord23.zw,shadowTestDepth) );
	}


	half finalcolour =  1 - (colour / (NUM_SHADOW_FILTER_TAPS*1.5));
	
	return finalcolour;

}


half filtered_shadow_receiveSMH_DB( sampler2D shadowMap, float4 texcoord, half4 shadowRadii, half spot_size )
{
	float shadowTestDepth = (texcoord.z / texcoord.w);
	float4 projCoord = (texcoord.xy / texcoord.w).xyxy; 
	
	half colour = 0;
	
	for(int i=0;i < NUM_SHADOW_FILTER_TAPS/4;i++)
	{
		half4 tap0 = shadowFilterTaps[(i*2)].xyzw * shadowRadii.xyxy * spot_size;
		half4 tap1 = shadowFilterTaps[(i*2)+1].xyzw * shadowRadii.xyxy * spot_size;

		float4 offsetCoord01 = projCoord + tap0;
		float4 offsetCoord23 = projCoord + tap1;

		colour += tex2D(shadowMap, float3(offsetCoord01.xy,shadowTestDepth));
		colour += tex2D(shadowMap, float3(offsetCoord01.zw,shadowTestDepth));
		colour += tex2D(shadowMap, float3(offsetCoord23.xy,shadowTestDepth));
		colour += tex2D(shadowMap, float3(offsetCoord23.zw,shadowTestDepth));
	}
	

	float disc = dot(colour,colour);
	half finalcolour;


	if( disc < 0.5f || disc > ((NUM_SHADOW_FILTER_TAPS*NUM_SHADOW_FILTER_TAPS)-0.5f) )
		finalcolour =  1 - (colour / NUM_SHADOW_FILTER_TAPS);
	else
	{	
		for(int i=0;i < NUM_SHADOW_FILTER_TAPS/4;i++)
		{
			half4 tap0 = shadowFilterTaps[(i*2)].xyzw * shadowRadii.xyxy * spot_size * 0.7;
			half4 tap1 = shadowFilterTaps[(i*2)+1].xyzw * shadowRadii.xyxy * spot_size * 0.7;

			float4 offsetCoord01 = projCoord + tap0;
			float4 offsetCoord23 = projCoord + tap1;

			colour += tex2D(shadowMap, float3(offsetCoord01.xy,shadowTestDepth) );
			colour += tex2D(shadowMap, float3(offsetCoord01.zw,shadowTestDepth) );
			colour += tex2D(shadowMap, float3(offsetCoord23.xy,shadowTestDepth) );
			colour += tex2D(shadowMap, float3(offsetCoord23.zw,shadowTestDepth) );
		}

		finalcolour = 1 - (colour / (NUM_SHADOW_FILTER_TAPS*1.5));
	}

	return finalcolour;

}



half4 filtered_quad_shadow_receiveSMH_SB( sampler2D shadowMap0,
									sampler2D shadowMap1,
									sampler2D shadowMap2,
									sampler2D shadowMap3,
									half4 clip0,
									half4 clip1,
									half4 clip2,
									half4 clip3,
									half4 shadowRadii0,
									half4 shadowRadii1,
									half4 shadowRadii2,
									half4 shadowRadii3,
									float4 texcoord0,
									float4 texcoord1,
									float4 texcoord2,
									float4 texcoord3,
									half4 posInScreenS)
									
{
	half4 regionD;
	
	// dot 4 planes into a register
	regionD.x = dot( posInScreenS, clip0 );
	regionD.y = dot( posInScreenS, clip1 );
	regionD.z = dot( posInScreenS, clip2 );
	regionD.w = dot( posInScreenS, clip3 );
	
	half scrZ = saturate( (posInScreenS.z/posInScreenS.w) - 0.991f);
	half spot_size = (scrZ*20.f) + 0.05f;
	
	// classify each plane simulatously and
	// get a unique number indicated which plane this pixel is in (it can only every be in one region at any one time)
	half region = dot( (regionD >= 0.f), float4(1.f, 2.f, 4.f, 8.f) );
	

	
	half4 color;
	color.x = 	filtered_shadow_receiveSMH_SB( shadowMap3, texcoord3, shadowRadii3, spot_size );
	color.y = 	filtered_shadow_receiveSMH_SB( shadowMap2, texcoord2, shadowRadii2, spot_size );
	color.z = 	filtered_shadow_receiveSMH_SB( shadowMap1, texcoord1, shadowRadii1, spot_size );
	color.w = 	filtered_shadow_receiveSMH_SB( shadowMap0, texcoord0, shadowRadii0, spot_size );
	
	half4 select = half4( 0.0f, 0.0f, 0.0f, 0.0f);
	
	if (region < 8.5f) select.x = 1.0f;
	if (region >= 8.5f && region < 12.5f) select.y = 1.0f;
	if (region >= 12.5f && region < 14.5f) select.z = 1.0f;
	if (region >= 14.5f && region < 15.5f) select.w = 1.0f;
	
	//half4 color0 = {1.0f, 0.0f, 1.0f, 0.0f};
	//half4 color1 = {1.0f, 0.0f, 0.0f, 0.0f};
	//half4 color2 = {0.0f, 1.0f, 0.0f, 0.0f};
	//half4 color3 = {0.0f, 0.0f, 1.0f, 0.0f};
		
	//return (dot(color, select))*(color0*select.x ); //+ color1*select.y + color2*select.z + color3*select.w);
	
	return (dot(color, select));
		
}	



half4 filtered_quad_shadow_receiveSMH_DB( sampler2D shadowMap0,
									sampler2D shadowMap1,
									sampler2D shadowMap2,
									sampler2D shadowMap3,
									half4 clip0,
									half4 clip1,
									half4 clip2,
									half4 clip3,
									half4 shadowRadii0,
									half4 shadowRadii1,
									half4 shadowRadii2,
									half4 shadowRadii3,
									float4 texcoord0,
									float4 texcoord1,
									float4 texcoord2,
									float4 texcoord3,
									half4 posInScreenS)
									
{
	half4 regionD;
	
	// dot 4 planes into a register
	regionD.x = dot( posInScreenS, clip0 );
	regionD.y = dot( posInScreenS, clip1 );
	regionD.z = dot( posInScreenS, clip2 );
	regionD.w = dot( posInScreenS, clip3 );
	
	half scrZ = saturate( (posInScreenS.z/posInScreenS.w) - 0.991f);
	half spot_size = (scrZ*20.f) + 0.05f;
	
	// classify each plane simulatously and
	// get a unique number indicated which plane this pixel is in (it can only every be in one region at any one time)
	half region = dot( (regionD >= 0.f), float4(1.f, 2.f, 4.f, 8.f) );
	
	

	half shadowterm = 0.f;

	// these are float compares with the numbers being ... in last plane will be 8 = 8, 8+4 = 12, 8+4+2 = 14, 8+4+2+1 = 15
	if( region < 8.5f )
	{
		shadowterm = filtered_shadow_receiveSMH_DB( shadowMap3, texcoord3, shadowRadii3, spot_size ); 
	} else if( region < 12.5f )
	{
		shadowterm = filtered_shadow_receiveSMH_DB( shadowMap2, texcoord2, shadowRadii2, spot_size );
	} else if( region < 14.5f )
	{
		shadowterm = filtered_shadow_receiveSMH_DB( shadowMap1, texcoord1, shadowRadii1, spot_size );
	} else if( region < 15.5f )
	{
		shadowterm = filtered_shadow_receiveSMH_DB( shadowMap0, texcoord0, shadowRadii0, spot_size );
	}
	
	return shadowterm; 
}	


// percent closest filtering of depth maps
float depth_compare( sampler2D depths, float4 texcoord )
{
	float  shadowTestDepth = texcoord.z / texcoord.w;
	float  shadowDepth = tex2Dproj(depths, texcoord);
	float  sample = (shadowTestDepth >= shadowDepth);

	return sample;
}

// percent closest filtering of depth maps
float pcf_depth_compare( sampler2D depths, float4 texcoord, float2 resolution, float2 resolution_rcp )
{      
	float  shadowTestDepth = texcoord.z / texcoord.w;
	
	float4  shadowDepth;
	
	float4 offset = float4(resolution_rcp, 0 ,0) * texcoord.w;
	shadowDepth.x = tex2Dproj(depths, texcoord);
	shadowDepth.y = tex2Dproj(depths, texcoord + offset * float4( 1, 0, 0, 0 ) );
	shadowDepth.z = tex2Dproj(depths, texcoord + offset * float4( 0, 1, 0, 0 ) );
	shadowDepth.w = tex2Dproj(depths, texcoord + offset * float4( 1, 1, 0, 0 ) );
	float4  sample  = (shadowTestDepth <= shadowDepth);

	// blend based on the fractional part
	float2 lerps = frac( texcoord.xy * resolution );
	float2 rows = lerp( sample.xz, sample.yw, lerps.x );
	return lerp( rows.x, rows.y, lerps.y );

}


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

	// Luv color space doesn't like pure black 
	// (as a nice side effect it's now always safe to compute luminance)
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



float4 _LogYUV_to_RGB(float4 yuv)
{

	float gamut = 1.613f;

	// define useful constants
	float3 epsilon = { 0.00001f, 0.00001f, 0.00001f };
	float unit = 1.0f/256.0f;
	float halfunit = 1.0f/512.0f;


	float3x3 mCIE_to_RGB = {
		{ 2.690f, -1.022f, 0.061f },
		{ -1.276f, 1.978f, -0.224f },
		{ -0.414f, 0.044f, 1.163f }
	};

	float3 XYZ;
	XYZ.y = exp2( get_logluminance_LogLuv(yuv) );
	
	
	XYZ.x = (9.0f / gamut)* yuv.x * XYZ.y / ((4.0f / gamut) * yuv.y) ;
	
	XYZ.z = dot(float3(yuv.xy, 1.0f), float3(-3.0f / gamut, -20.0f / gamut, 12.0f)) * XYZ.y / ((4.0f / gamut) * yuv.y) ;

	return float4(mul(XYZ,mCIE_to_RGB) , 0.5f);
}

float4 _NAO24_to_RGB(float4 nao24)
{

	float gamut = 1.613f;

	// define useful constants
	float3 epsilon = { 0.00001f, 0.00001f, 0.00001f };
	float unit = 1.0f/256.0f;
	float halfunit = 1.0f/512.0f;


	float3x3 mCIE_to_RGB = {
		{ 2.690f, -1.022f, 0.061f },
		{ -1.276f, 1.978f, -0.224f },
		{ -0.414f, 0.044f, 1.163f }
	};

	float3 XYZ;
	XYZ.y = exp2( nao24.z * 16.0f - 8.0f );
	
	
	XYZ.x = (9.0f / gamut)* nao24.x * XYZ.y / ((4.0f / gamut) * nao24.y) ;
	
	XYZ.z = dot(float3(nao24.xy, 1.0f), float3(-3.0f / gamut, -20.0f / gamut, 12.0f)) * XYZ.y / ((4.0f / gamut) * nao24.y) ;

	return float4(mul(XYZ,mCIE_to_RGB) , 0.5f);
}

float4 _HDRTexture_to_RGB(float4 yuv)
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
	XYZ.y = exp2((32.0f*(yuv.w  + (yuv.z / 255.0f)) - 16.0f));
	
	
	XYZ.x = (9.0f / gamut)* yuv.x * XYZ.y / ((4.0f / gamut) * yuv.y) ;
	
	XYZ.z = dot(float3(yuv.xy, 1.0f), float3(-3.0f / gamut, -20.0f / gamut, 12.0f)) * XYZ.y / ((4.0f / gamut) * yuv.y) ;

	return float4(mul(XYZ,mCIE_to_RGB) , 0.5f);

	//return float4(XYZ.y, 0, 0, 0);

}


// linear RGB to luminance
float get_luminance( float3 linear_rgb )
{
	return dot( linear_rgb, float3( 0.212671f, 0.715160f, 0.072169f ) );
}

half encode_luminance( half luminance )
{
	half log_luminance = log2( luminance );
	half compress_luminance = (1.0f / (12.0f + 5.0f)) * ( log_luminance + 12.0f );
	
	return compress_luminance;
}

half decode_luminance( half encoded_luminance )
{
	half log_luminance = ( log_luminance * (8.0f + 4.0f) ) - 8.0f;
	half luminance = pow( log_luminance, 2.0f );
	
	return luminance;
}
 
half4 opaque_tonemap( half4 colour, half linear_scale, half white_luminance_rcp_sq )
{
	half3 exposed_colour = linear_scale.xxx * colour.xyz;
	half exposed_luminance = get_luminance( exposed_colour );
	
	half tonemap = ( 1.0f + exposed_luminance * white_luminance_rcp_sq ) / ( 1.0f + exposed_luminance );
	
	return half4( exposed_colour * tonemap.xxx, encode_luminance( get_luminance( exposed_colour ) ) );
}

half4 transparent_tonemap( half4 colour, half linear_scale, half white_luminance_rcp_sq )
{
	half3 exposed_colour = linear_scale.xxx * colour.xyz;
	half exposed_luminance = get_luminance( exposed_colour );
	
	half tonemap = ( 1.0f + exposed_luminance * white_luminance_rcp_sq ) / ( 1.0f + exposed_luminance );
	
	return ( exposed_colour * tonemap.xxx, colour.w );
}	
		 
/*
float4 Windify(float4 position_objectS, float2 vWindAttribs, float4x4 amWindMatrices[4])
{
	return lerp(position_objectS,mul(position_objectS, amWindMatrices[vWindAttribs.x]),vWindAttribs.y);
}
float4 Billboardify(float3 vLeafAttribs, float4 avLeafClusters[48], float4x4 amLeafAngleMatrices[4])
{
	return mul(avLeafClusters[vLeafAttribs.x] * vLeafAttribs.y, amLeafAngleMatrices[vLeafAttribs.z]);
}
*/

// debug
float4 Windify(float4 position_objectS, float2 vWindAttribs, float4x4 amWindMatrices[4])
{
	return lerp(position_objectS,mul(position_objectS, amWindMatrices[vWindAttribs.x]),vWindAttribs.y);
}
float4 Billboardify(float3 vLeafAttribs, float4 avLeafClusters[48], float4x4 amLeafAngleMatrices[4])
{
	return mul(avLeafClusters[vLeafAttribs.x] * vLeafAttribs.y, amLeafAngleMatrices[vLeafAttribs.z]);
}



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

half filtered_shadow_receiveSMHZ( sampler2D shadowMap, float3 texcoord, half4 shadowRadii, half spot_size )
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
		
	color = filtered_shadow_receiveSMHZ( shadowMap, texcoord, radii, 0.04f);


	return  color;
}	


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////HAIR//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float3 ShiftTangent(float3 T, float3 N, float fShift)
{
	float3 shiftedT = T + N * fShift;
	return normalize(shiftedT);
}
float StrandSpecular(float3 T, float3 V, float3 L, float fExponent)
{
	float3 H = normalize(L+V);
	float dotTH = dot(T,H);
	float sinTH = sqrt(1.0f - dotTH*dotTH);
	float dirAtten = smoothstep(-1.0f,0.0f,dotTH);
	return dirAtten * pow(sinTH,fExponent);
}
float StrandSpecular2(float3 T, float3 V, float3 L, float fExponent)
{
/*
	// anisotropic normal
    float3 N = V - dot(V,T) * T;
	normalize(N);
	float3 RV = 2.0 * dot(V,N) * N - V;
	//float powerCoef = 0.5+0.5*dot(RV,L);
	float powerCoef = saturate(0.5+0.5*dot(RV,L)); // debug
	return pow(powerCoef,fExponent);
*/

//	L = normalize(L);
//	T = normalize(T);
//	V = normalize(V);

//	float cos_theta_lt = abs(dot(L,T));
//	float cos_theta_vt = abs(dot(V,T));
	float cos_theta_lt = dot(L,T);
	float cos_theta_vt = dot(V,T);

//	float sin_theta_lt = sqrt(1-(cos_theta_lt*cos_theta_lt));
//	float sin_theta_vt = sqrt(1-(cos_theta_vt*cos_theta_vt));

	float sin_theta_lt = sin(acos(cos_theta_lt));
	float sin_theta_vt = sin(acos(cos_theta_vt));

	float powerCoef = saturate((sin_theta_lt*sin_theta_vt)-(cos_theta_lt*cos_theta_vt));
//	float powerCoef = (sin_theta_lt*sin_theta_vt)-(cos_theta_lt*cos_theta_vt);
//	return max(0,pow(powerCoef,fExponent));
	return pow(powerCoef,fExponent);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void unpackTangentSpace(	in float2 in_tangentspace_short2n,
							out float3 out_normal,
							out float3 out_tangent,
							out float3 out_binormal		)
{
	float normal = in_tangentspace_short2n.x;
	float tangent = in_tangentspace_short2n.y;
	float z_normal_sign = sign(normal);
	float binormal_handednes = sign(tangent);

	normal = abs(normal);
	out_normal.x = 2.0f * normal - 1.0f;
	out_normal.y = 2.0f * frac( normal * 256.0f  ) - 1.0f;
	out_normal.z = z_normal_sign * sqrt(1.0f - dot(out_normal.xy, out_normal.xy));

	//out_normal = 0.5;

	float z_tangent_sign =  2.0f * abs(tangent);
	out_tangent.x = 2.0f * frac( z_tangent_sign ) - 1.0f;
	out_tangent.y = 2.0f * frac( ldexp( z_tangent_sign, 7 ) ) - 1.0f;
	out_tangent.z = (2.0f * round(z_tangent_sign) - 1.0f) * sqrt(1.0f - dot(out_tangent.xy, out_tangent.xy));

	out_binormal = binormal_handednes * cross( out_normal, out_tangent );
} */


float unpackHandnedess ( in float in_packed_handnedess )
{
	return sign( in_packed_handnedess );
//	return 	abs(in_packed_handnedess) / in_packed_handnedess;
}

float3 deriveBinormal	(	in float3 in_normal,
							in float3 in_tangent,
							in float in_handnedess )
{
	return  (in_handnedess * cross( in_normal, in_tangent ));
}

float3 pointToDirection( in float3 in_point_A, in float3 in_point_B)
{
	return in_point_A - in_point_B;
}

float3 fromObjectSpaceToTangentSpace( in float3 in_obj_vector, in float3 in_obj_normal, in float3 in_obj_tangent, in float3 in_obj_binormal)
{
	float3 out_tangent_vector;
	
	out_tangent_vector.x = dot( in_obj_vector, in_obj_tangent );
	out_tangent_vector.y = dot( in_obj_vector, in_obj_binormal );
	out_tangent_vector.z = dot( in_obj_vector, in_obj_normal );		
	
	return out_tangent_vector;
}

void sampleDiffuse( 	in float2 in_diffuse_texcoord,
						in sampler2D in_diffuse_sampler,
						in float4 in_diffuse_colour,
						in half2 in_parallax_offset,
						out half4 out_blender_colour )
{
	out_blender_colour = h4tex2D(in_diffuse_sampler, in_diffuse_texcoord + in_parallax_offset);
	out_blender_colour.xyz *= in_diffuse_colour.xyz;
}

void sampleHdrDiffuse( 	in float2 in_diffuse_texcoord,
						in sampler2D in_diffuse_sampler,
						in float4 in_diffuse_colour,
						in half2 in_parallax_offset,
						out float4 out_blender_colour )
{
	out_blender_colour = tex2D(in_diffuse_sampler, in_diffuse_texcoord + in_parallax_offset);
	out_blender_colour = _HDRTexture_to_RGB(out_blender_colour);
	out_blender_colour.xyz *= in_diffuse_colour.xyz;
}


void getParallaxMapOffset(	in float3 in_eyevector_tangspace,
							in float2 in_normal_map_texcoord,
							in half2 in_parallaxScaleAndBias,
							in sampler2D in_normal_map,							
							out half2 out_parallax_offset)
{
	half pixelHeight = (h4tex2D(in_normal_map, in_normal_map_texcoord).w - 0.5f)*in_parallaxScaleAndBias.x + in_parallaxScaleAndBias.y;
	out_parallax_offset = (pixelHeight.xx * in_eyevector_tangspace.xy);
}


void sampleNormalMap(	in float2 in_normal_map_texcoord,
						in float3 in_world_tangent,
						in float3 in_world_binormal,
						in float3 in_world_normal,
						in sampler2D in_normal_map,
						out half3 out_normal)
{
	out_normal.xy = 2.0f*(h4tex2D(in_normal_map, in_normal_map_texcoord).xw - 0.5f.xx);
	out_normal.z = sqrt(1.0f - dot(out_normal.xy, out_normal.xy));
	out_normal = mul(out_normal, half3x3(in_world_tangent, in_world_binormal, in_world_normal));
	out_normal = normalize(out_normal);
}


void sampleParallaxMap(	in half2 in_parallax_offset,
						in float2 in_normal_map_texcoord,
						in float3 in_world_tangent,
						in float3 in_world_binormal,
						in float3 in_world_normal,
						in sampler2D in_normal_map,
						out half3 out_normal)
{
	in_normal_map_texcoord += in_parallax_offset;
	out_normal = 2.0f.xxx*(h3tex2D(in_normal_map, in_normal_map_texcoord).xyz - 0.5f.xxx);
	out_normal = mul(out_normal, half3x3(in_world_tangent, in_world_binormal, in_world_normal));
	out_normal = normalize(out_normal);
}


void computeFresnelTerm(in float3 in_world_view_dir,
						in float3 in_world_normal,
						in float in_fresnel_strength,
						out half out_fresnel_term)
{
	float dummy = 1.0f + 0.5f * min( dot( in_world_view_dir, in_world_normal ), 0.0f);
	out_fresnel_term = half( lerp( 1.0f, dummy, in_fresnel_strength ) );
}


void lambertPerVertexLighting(	in half4 in_diffuse_fill_and_key_scalar,
								in half in_shadow_term,
								in half3 in_key_dir_colour,
								out half4 out_diffuse_lighting)
{
	out_diffuse_lighting.xyz = in_diffuse_fill_and_key_scalar.xyz;
	out_diffuse_lighting.xyz += saturate(in_diffuse_fill_and_key_scalar.w)*in_key_dir_colour*in_shadow_term;
}


void lambertPerPixelLighting(	in half3 in_normal,
								in half in_shadow_term,
								in half3 in_key_dir_colour,
								in half3 in_key_dir,
								in half3 in_irradiance,
								out half4 out_diffuse_lighting)
{
	out_diffuse_lighting.xyz = saturate(dot(in_normal,in_key_dir)).xxx * in_key_dir_colour * in_shadow_term;
	out_diffuse_lighting.xyz += in_irradiance;
	out_diffuse_lighting.w = 0.0f;	
}

void lambertCombiner(	in half4 in_surface_color,
						in half4 in_diffuse_lighting,
						out half4 out_colour )
{
	out_colour.xyz = in_surface_color.xyz * in_diffuse_lighting.xyz;
	out_colour.w = in_surface_color.w;
}



void metallicPerVertexLighting(	in samplerCUBE in_reflectance_map,
								in half3 in_reflected_colour,
								in half3 in_reflectance_colour,
								in half4 in_diffuse_fill_and_key_scalar,
								in half in_shadow_term,
								in half3 in_key_dir,
								in half3 in_key_dir_colour,
								in float3 in_reflected_dir,
								in half3 in_specular_colour,
								in half in_specular_power,
								out half4 out_diffuse_lighting,
								out half3 out_specular_lighting,
								out half3 out_reflectance_lighting)
{
	out_reflectance_lighting = in_reflected_colour*in_reflectance_colour*(texCUBE(in_reflectance_map, in_reflected_dir).xyz);
	out_reflectance_lighting *= (in_shadow_term * 0.5f) + 0.5f;

	half3 lit_coeffs = lit(in_diffuse_fill_and_key_scalar.w , dot(in_key_dir, normalize(in_reflected_dir)), in_specular_power);
	half3 dummy = in_key_dir_colour * in_shadow_term;

	out_diffuse_lighting.xyz = in_diffuse_fill_and_key_scalar.xyz;
	out_diffuse_lighting.xyz += lit_coeffs.yyy * dummy;
	out_diffuse_lighting.w = 0.0f;

	out_specular_lighting = in_specular_colour * lit_coeffs.zzz * dummy;
}


void metallicPerPixelLighting(in samplerCUBE in_reflectance_map,
							in half3 in_reflected_colour,
							in half3 in_reflectance_colour,	
							in half3 in_normal,
							in half in_shadow_term,
							in half3 in_key_dir_colour,
							in half3 in_key_dir,
							in half3 in_irradiance,							
							in float3 in_world_view_dir,
							in half3 in_specular_colour,
							in half in_specular_power,
							out half4 out_diffuse_lighting,
							out half3 out_specular_lighting,
							out half3 out_reflectance_lighting)
{
	half3 reflected_dir = (reflect( in_world_view_dir, in_normal ));
	
	out_reflectance_lighting = in_reflected_colour*in_reflectance_colour*(texCUBE(in_reflectance_map, reflected_dir).xyz);
	out_reflectance_lighting *= (in_shadow_term * 0.5f) + 0.5f;
	
	half3 lit_coeffs = lit( dot( in_normal, in_key_dir), dot(in_key_dir, normalize(reflected_dir)), in_specular_power);
	half3 dummy = in_key_dir_colour * in_shadow_term;
	
	out_diffuse_lighting.xyz = lit_coeffs.yyy * dummy;
	out_diffuse_lighting.xyz += in_irradiance;
	out_diffuse_lighting.w = 0.0f;
	out_specular_lighting = in_specular_colour * lit_coeffs.zzz * dummy;
}

void metallicCombiner(	in half4 in_surface_color,
						in half4 in_diffuse_lighting,
						in half3 in_specular_lighting,
						in half3 in_reflectance_lighting,
						in half in_fresnel_term,
						out half4 out_colour )
{
	out_colour.xyz = in_surface_color.xyz * in_diffuse_lighting.xyz;
	out_colour.xyz += in_surface_color.w * in_specular_lighting;	
	out_colour.xyz += in_reflectance_lighting * in_fresnel_term * in_surface_color.w;	
	out_colour.w = in_surface_color.w;
}


void metallicCombinerWithSpecularResponse(	in half in_specular_response,
											in half4 in_surface_color,
											in half4 in_diffuse_lighting,
											in half3 in_specular_lighting,
											in half3 in_reflectance_lighting,
											in half in_fresnel_term,
											out half4 out_colour )
{
	out_colour.xyz = lerp( in_diffuse_lighting.xyz, in_reflectance_lighting, in_fresnel_term * in_specular_response);
	out_colour.xyz *= in_surface_color.xyz;
	out_colour.xyz += in_specular_response * in_specular_lighting;
	out_colour.w = in_surface_color.w;
}


void phongPerVertexLighting(	in half4 in_diffuse_fill_and_key_scalar,
								in half in_shadow_term,
								in half3 in_key_dir,
								in half3 in_key_dir_colour,
								in float3 in_reflected_dir,
								in half3 in_specular_colour,
								in half in_specular_power,
								out half4 out_diffuse_lighting,
								out half3 out_specular_lighting)
{
	half3 lit_coeffs = lit(in_diffuse_fill_and_key_scalar.w , dot(in_key_dir, normalize(in_reflected_dir)), in_specular_power);
	half3 dummy = in_key_dir_colour * in_shadow_term;
	out_diffuse_lighting.xyz = in_diffuse_fill_and_key_scalar.xyz;
	out_diffuse_lighting.xyz += lit_coeffs.yyy * dummy;
	out_diffuse_lighting.w = 0.0f;
	out_specular_lighting = in_specular_colour * lit_coeffs.zzz * dummy;
}

void phongPerPixelLighting(	in half3 in_normal,
							in half in_shadow_term,
							in half3 in_key_dir_colour,
							in half3 in_key_dir,
							in half3 in_irradiance,							
							in float3 in_world_view_dir,
							in half3 in_specular_colour,
							in half in_specular_power,
							out half4 out_diffuse_lighting,
							out half3 out_specular_lighting)
{
	half3 reflected_dir = normalize(reflect( in_world_view_dir, in_normal ));
	half3 lit_coeffs = lit( dot( in_normal, in_key_dir), dot(in_key_dir, reflected_dir), in_specular_power);
	half3 dummy = in_key_dir_colour * in_shadow_term;
	out_diffuse_lighting.xyz = lit_coeffs.yyy * dummy;
	out_diffuse_lighting.xyz += in_irradiance;
	out_diffuse_lighting.w = 0.0f;
	out_specular_lighting = in_specular_colour * lit_coeffs.zzz * dummy;
}

void phongCombiner(	in half4 in_surface_color,
					in half4 in_diffuse_lighting,
					in half3 in_specular_lighting,
					out half4 out_colour )
{
	out_colour.xyz = in_surface_color.xyz * in_diffuse_lighting.xyz;
	out_colour.xyz += in_surface_color.w * in_specular_lighting;	
	out_colour.w = in_surface_color.w;
}

void phongCombinerWithSpecularResponse(	in half in_specular_response,
										in half4 in_surface_color,
										in half4 in_diffuse_lighting,
										in half3 in_specular_lighting,
										out half4 out_colour )
{
	out_colour.xyz = in_surface_color.xyz * in_diffuse_lighting.xyz;
	out_colour.xyz += in_specular_response * in_specular_lighting;
	out_colour.w = in_surface_color.w;
}

void snowPerVertexLighting(		in half4 in_diffuse_fill_and_key_scalar,
								in half in_shadow_term,
								in half3 in_key_dir,
								in half3 in_key_dir_colour,
								in float3 in_reflected_dir,
								in half3 in_specular_colour,
								in half in_specular_power,
								in half3 in_snow_normal,								
								in half3 in_snow_diffuse_colour,
								in half3 in_snow_specular_colour,								
								out half4 out_diffuse_lighting,
								out half3 out_specular_lighting,
								out half3 out_snow_diffuse_lighting,
								out half3 out_snow_specular_lighting								
								)
{
	// Phong Phase
	half3 lit_coeffs = lit(in_diffuse_fill_and_key_scalar.w , dot(in_key_dir, normalize(in_reflected_dir)), in_specular_power);
	half3 dummy = in_key_dir_colour * in_shadow_term;
	out_diffuse_lighting.xyz = in_diffuse_fill_and_key_scalar.xyz;
	out_diffuse_lighting.xyz += lit_coeffs.yyy * dummy;
	out_diffuse_lighting.w = 0.0f;
	out_specular_lighting = in_specular_colour * lit_coeffs.zzz * dummy;
	
	// Snow Phase
	out_snow_diffuse_lighting = in_diffuse_fill_and_key_scalar.xyz;
	out_snow_diffuse_lighting += lit_coeffs.yyy * dummy;
	out_snow_specular_lighting = in_snow_specular_colour * lit_coeffs.zzz * dummy;	
}

void snowPerPixelLighting(	in half3 in_normal,
							in half in_shadow_term,
							in half3 in_key_dir_colour,
							in half3 in_key_dir,
							in float4x4 in_fill_sh[3],							
							in half3 in_irradiance,						
							in float3 in_world_view_dir,
							in half3 in_specular_colour,
							in half in_specular_power,
							in half3 in_snow_normal,
							in half3 in_snow_diffuse_colour,
							in half3 in_snow_specular_colour,							
							out half4 out_diffuse_lighting,
							out half3 out_specular_lighting,
							out half3 out_snow_diffuse_lighting,
							out half3 out_snow_specular_lighting
							)
{
	// Phong Phase
	half3 reflected_dir = normalize(reflect( in_world_view_dir, in_normal ));
	half3 lit_coeffs = lit( dot( in_normal, in_key_dir), dot(in_key_dir, reflected_dir), in_specular_power);
	half3 dummy = in_key_dir_colour * in_shadow_term;
	out_diffuse_lighting.xyz = lit_coeffs.yyy * dummy;
	out_diffuse_lighting.xyz += in_irradiance;
	out_diffuse_lighting.w = 0.0f;
	out_specular_lighting = in_specular_colour * lit_coeffs.zzz * dummy;
	
	// Snow Phase
	reflected_dir = normalize(reflect( in_world_view_dir, in_snow_normal ));
	lit_coeffs = lit( dot( in_snow_normal, in_key_dir), dot(in_key_dir, reflected_dir), in_specular_power);	
	out_snow_diffuse_lighting = lit_coeffs.yyy * dummy;
	out_snow_diffuse_lighting += max(sh_lookup(in_snow_normal,in_fill_sh), 0.0f);
	out_snow_specular_lighting = in_specular_colour * lit_coeffs.zzz * dummy;	
	
}

void snowCombiner(	in half4 in_surface_color,
					in half4 in_diffuse_lighting,
					in half3 in_specular_lighting,
					in half4 in_snow_diffuse_lighting,
					in half3 in_snow_specular_lighting,					
					in half2 in_snow_luminance,
					in half in_mask,
					out half4 out_colour )
{
	out_colour.w = in_surface_color.w;

	half3 colorBase;
	colorBase.xyz = in_surface_color.xyz * in_diffuse_lighting.xyz;
	colorBase.xyz += in_surface_color.w * in_specular_lighting;	

	half3 colorSnow;
	colorSnow.xyz = in_snow_luminance.xxx * in_snow_diffuse_lighting.xyz;
	colorSnow.xyz += in_snow_luminance.yyy * in_snow_specular_lighting;	
	
	out_colour.xyz = lerp( colorBase, colorSnow, in_mask );
}


void uberPerVertexLighting(		in half4 in_specular_colour_metallicity,
								in samplerCUBE in_reflectance_map,
								in half3 in_reflected_colour,
								in half3 in_reflectance_colour,
								in half4 in_diffuse_fill_and_key_scalar,
								in half in_shadow_term,
								in half3 in_key_dir,
								in half3 in_key_dir_colour,
								in float3 in_reflected_dir,
								in half in_specular_power,
								out half4 out_diffuse_lighting,
								out half3 out_specular_lighting,
								out half3 out_reflectance_lighting)
{
	out_reflectance_lighting = in_reflected_colour*in_reflectance_colour*(texCUBE(in_reflectance_map, in_reflected_dir).xyz);
	out_reflectance_lighting *= (in_shadow_term * 0.5f) + 0.5f;
	out_reflectance_lighting *= in_specular_colour_metallicity.www;	

	half3 lit_coeffs = lit(in_diffuse_fill_and_key_scalar.w , dot(in_key_dir, normalize(in_reflected_dir)), in_specular_power);
	half3 dummy = in_key_dir_colour * in_shadow_term;

	out_diffuse_lighting.xyz = in_diffuse_fill_and_key_scalar.xyz;
	out_diffuse_lighting.xyz += lit_coeffs.yyy * dummy;
	out_diffuse_lighting.w = 0.0f;

	out_specular_lighting = in_specular_colour_metallicity.xyz * lit_coeffs.zzz * dummy;
}


void uberPerPixelLighting(	in half4 in_specular_colour_metallicity,
							in samplerCUBE in_reflectance_map,
							in half3 in_reflected_colour,
							in half3 in_reflectance_colour,	
							in half3 in_normal,
							in half in_shadow_term,
							in half3 in_key_dir_colour,
							in half3 in_key_dir,
							in half3 in_irradiance,							
							in float3 in_world_view_dir,
							in half in_specular_power,
							out half4 out_diffuse_lighting,
							out half3 out_specular_lighting,
							out half3 out_reflectance_lighting)
{
	half3 reflected_dir = (reflect( in_world_view_dir, in_normal ));
	
	out_reflectance_lighting = in_reflected_colour*in_reflectance_colour*(texCUBE(in_reflectance_map, reflected_dir).xyz);
	out_reflectance_lighting *= (in_shadow_term * 0.5f) + 0.5f;
	out_reflectance_lighting *= in_specular_colour_metallicity.www;
	
	half3 lit_coeffs = lit( dot( in_normal, in_key_dir), dot(in_key_dir, normalize(reflected_dir)), in_specular_power);
	half3 dummy = in_key_dir_colour * in_shadow_term;
	
	out_diffuse_lighting.xyz = lit_coeffs.yyy * dummy;
	out_diffuse_lighting.xyz += in_irradiance;
	out_diffuse_lighting.w = 0.0f;
	out_specular_lighting = in_specular_colour_metallicity.xyz * lit_coeffs.zzz * dummy;
}

void sampleHairSpecShift( 	in half2 in_hairTexcoord,
							in half2 in_shiftFrequency,
							in sampler2D in_s_spectShift,
							out float out_spec_shift)
{
	out_spec_shift = tex2D(in_s_spectShift, in_hairTexcoord * in_shiftFrequency).a - 0.5f;
}

void sampleHairBaseIntensity(	in half2 in_basetexCoord,
								in half2 in_basetexFrequency,
								in sampler2D in_s_texBase,								
								out half out_fBaseIntensity)
{
	out_fBaseIntensity = tex2D(in_s_texBase, in_basetexCoord * in_basetexFrequency);
}

void hairLighting(	in float in_spec_shift, 
					in float3 in_normal_objectS,
					in float3 in_tangent_objectS,
					in float3 in_viewvec_objectS,
					in float3 in_keyDir,
					in half3 in_keyDirColour,
					in half1 in_hairSpecularShift,
					in half1 in_hairSpecularShift2,
					in half1 in_specular_power,
					in half1 in_specular_power2,
					in half3 in_specular_colour,
					in half3 in_specular_colour2,
					in half4 in_diffuseFillAndKeyScalar,
					in half4 in_diffuseColour0,
					in half in_shadowterm,					
					out half4 out_diffuse_lighting,
					out half3 out_specular_lighting)
{
	float3 t1 = ShiftTangent(in_tangent_objectS, in_normal_objectS, in_hairSpecularShift + in_spec_shift);
	float3 t2 = ShiftTangent(in_tangent_objectS, in_normal_objectS, in_hairSpecularShift2 + in_spec_shift);

	float specular1 = StrandSpecular2(t1, in_viewvec_objectS, in_keyDir, in_specular_power);
	float specular2 = StrandSpecular2(t2, in_viewvec_objectS, in_keyDir, in_specular_power2);
	
	out_specular_lighting = (specular1 * in_specular_colour) + (specular2 * in_specular_colour2);
	out_specular_lighting += out_specular_lighting * in_keyDirColour * in_shadowterm.xxx;		

	out_diffuse_lighting.xyz = in_diffuseFillAndKeyScalar.xyz;
	out_diffuse_lighting.xyz += saturate(in_diffuseFillAndKeyScalar.w) * in_keyDirColour * in_shadowterm;
	out_diffuse_lighting.xyz *= in_diffuseColour0;
	out_diffuse_lighting.w = 0;
}

void hairCombiner(	in half4 in_fBaseIntensity,
					in half4 in_diffuse_lighting,
					in half3 in_specular_lighting,
					out half4 out_colour )
{
	out_colour.rgb = (in_diffuse_lighting.xyz + in_specular_lighting) * in_fBaseIntensity.xyz;
	out_colour.a = 1.0f;
}

void sampleIceRefractionCubeMap(in samplerCUBE in_cube_sampler,
								in float3 in_eye_dir,
								in float3 in_normal,
								in float in_warp,
								out half3 out_colour )
{
	float3 ray = - in_eye_dir - in_warp * 0.7f * in_normal;
	out_colour = texCUBE( in_cube_sampler, ray );
}

void iceLighting(	in half3 in_normal,
					in half3 in_world_view_dir,	
					in half in_shadow_term,
					in half3 in_key_dir,
					in half3 in_key_dir_colour,
					in half3 in_irradiance,								
					out half4 out_diffuse_lighting,
					out half3 out_specular_lighting)
{
	half3 reflected_dir = normalize(reflect( in_world_view_dir, in_normal ));
	half3 lit_coeffs = lit( dot( in_normal, in_key_dir), dot(in_key_dir, reflected_dir), 4.0f);
	half3 dummy = in_key_dir_colour * in_shadow_term;

	// diffuse wrap = 2.0	
	out_diffuse_lighting.xyz += ((lit_coeffs.yyy  + 2.0f) / (1.0f + 2.0f)) * dummy;
	
	out_diffuse_lighting.xyz += in_irradiance;
	out_diffuse_lighting.w = 0.0f;
	
	out_specular_lighting = lit_coeffs.zzz * dummy;
}


void iceCombiner(	in half4 in_surface_color,
					in half4 in_specular_colour,	
					in half3 in_constant_specular_colour,	
					in half3 in_refraction_colour,
					in half4 in_diffuse_lighting,
					in half3 in_specular_lighting,
					in half in_facing,
					in half in_frost,
					out half4 out_colour )
{
	// diffuse
	in_surface_color += in_frost;
	in_surface_color *= (1.0f - in_facing);
	out_colour.xyz = in_surface_color.xyz * in_diffuse_lighting.xyz;
	
	// specular
	out_colour.xyz += in_constant_specular_colour*(((in_specular_colour.xyz * in_facing) + (0.25f * in_frost)) * in_specular_lighting);	
	
	// refraction
	out_colour.xyz += max(0.25, saturate(1.0f - in_frost)) * in_refraction_colour;
	
	out_colour.w = in_surface_color.w;
}


half3 facing_term( half in_facing_factor, half3 in_facing_colour, half in_facing_power )
{
	half sine = sqrt( 1.0f - (in_facing_factor * in_facing_factor) );
	return ( pow( sine, in_facing_power ).xxx *  in_facing_colour );
}

void velvetPerVertexLighting(	in half4 in_diffuse_fill_and_key_scalar,
								in half in_roughness,
								in half in_backscatter,
								in half3 in_normal,
								in half in_shadow_term,
								in half3 in_key_dir_colour,
								in half3 in_key_dir,
								in float3 in_world_view_dir,
								in half3 in_retroreflective_color,
								in half3 in_silouette_color,
								in half in_specular_power,
								out half4 out_diffuse_lighting,
								out half3 out_specular_lighting )
{
	out_specular_lighting = 0.0f;
	
	half3 Nf = sign( dot( in_world_view_dir, in_normal ) ) * in_normal;
	in_world_view_dir = -in_world_view_dir;
	
	half diffuse = saturate( dot( in_normal, in_key_dir) );

	half3 dummy = in_key_dir_colour * in_shadow_term;
	out_diffuse_lighting.xyz = saturate(in_diffuse_fill_and_key_scalar.w) * dummy;	
	out_diffuse_lighting.xyz += in_diffuse_fill_and_key_scalar.xyz;
	out_diffuse_lighting.w = 0.0f;
	
	// retro-reflective lobe
	half cosine = saturate( dot( in_key_dir, in_world_view_dir ) );
	out_specular_lighting += pow( cosine, 1.0f / in_roughness ).xxx * in_backscatter.xxx * in_retroreflective_color;
	
	// horizon scattering 
	cosine = saturate( dot( in_normal, in_world_view_dir ) );
	half sine = sqrt( 1.0f - cosine * cosine );
	out_specular_lighting += pow( sine, in_specular_power ).xxx * diffuse.xxx * in_silouette_color;
	out_specular_lighting *= in_key_dir_colour * (0.9 * in_shadow_term + 0.1);
}

void velvetPerPixelLighting(	in half in_roughness,
								in half in_backscatter,
								in half3 in_normal,
								in half in_shadow_term,
								in half3 in_key_dir_colour,
								in half3 in_key_dir,
								in half3 in_irradiance,							
								in float3 in_world_view_dir,
								in half3 in_retroreflective_color,
								in half3 in_silouette_color,
								in half in_specular_power,
								out half4 out_diffuse_lighting,
								out half3 out_specular_lighting )
{
	out_specular_lighting = 0.0f;
	
	half3 Nf = sign( dot( in_world_view_dir, in_normal ) ) * in_normal;
	in_world_view_dir = -in_world_view_dir;
	
	half diffuse = saturate( dot( in_normal, in_key_dir) );


	half3 dummy = in_key_dir_colour * in_shadow_term;
	out_diffuse_lighting.xyz = diffuse * dummy;
	out_diffuse_lighting.xyz += in_irradiance;
	out_diffuse_lighting.w = 0.0f;

	
	// retro-reflective lobe
	half cosine = saturate( dot( in_key_dir, in_world_view_dir ) );
	out_specular_lighting += pow( cosine, 1.0f / in_roughness ).xxx * in_backscatter.xxx * in_retroreflective_color;
	
	// horizon scattering 
	cosine = saturate( dot( in_normal, in_world_view_dir ) );
	half sine = sqrt( 1.0f - cosine * cosine );
	out_specular_lighting += pow( sine, in_specular_power ).xxx * diffuse.xxx * in_silouette_color;
	out_specular_lighting *= in_key_dir_colour * (0.90 * in_shadow_term + 0.1);
}

void emissiveCombiner(	in half4 in_surface_color,
						in half in_emissivity,
						in half4 in_diffuse_lighting,
						in half3 in_specular_lighting,
						out half4 out_colour )
{
	out_colour.xyz = in_surface_color.xyz * (in_diffuse_lighting.xyz + 4.0f.xxx * in_emissivity.xxx);
	out_colour.xyz += in_surface_color.w * in_specular_lighting;	
	out_colour.w = in_surface_color.w;
}


void newSnowCombiner(	in half4 in_surface_color,
						in half4 in_diffuse_lighting,
						in half3 in_specular_lighting,
						in half3 in_glittering,						
						in half3 in_facing_term,												
						out half4 out_colour )
{
	out_colour.xyz = in_surface_color.xyz * in_diffuse_lighting.xyz;
	half specular_luminance = get_luminance( in_specular_lighting );
	out_colour.xyz += in_glittering.xyz * specular_luminance.xxx;	
	out_colour.xyz += in_facing_term;		
	out_colour.w = in_surface_color.w;
}

/* void bsskin_lighting(
	float3 Nn,
	float3 En,
	float3 Ln,
	
	half3 lightColour,
	half3 baseColour,
	float baseOcclusion,
	half3 subColour,
	half3 fuzzColour,	
	half fuzzTightness,
	half diffuseWrap,
	half specularIntensity,
	half specularGlancing,
	half specularFacing,
	
	half shadowTerm,
	half3 sh,
	
	sampler2D diffuseGain,
	sampler2D specularGain,
	
	out half4 out_colour
)
{

	out_colour = float4(0,0,0,1);
	
	
	En = -En;
	
	// reflection coefficients
	float nDotL = dot( Nn, Ln );
	float3 halfDir = normalize( 0.5 * ( Ln + En ) );
	float nDotH = dot( Nn, halfDir );
	float nDotE = dot( Nn, En );
	
	
	// lighting
	float directLightIntensity = saturate( nDotL );
	float3 directLight = directLightIntensity * lightColour;
	float wrapLightIntensity = ( saturate(nDotL) + diffuseWrap )/( 1 + diffuseWrap );
	float3 wrapLight = wrapLightIntensity * lightColour;
	
	// shadowing
	float shadowed = shadowTerm * baseOcclusion;
	
	// diffuse term
	float3 diffuseAlbedo = baseColour; 
	float diffuseFactor = max( nDotL + diffuseWrap, 0 )/( 1 + diffuseWrap );
	float3 diffuseColour = tex2D( diffuseGain, float2(diffuseFactor,0.5) ).xyz;
	//diffuseColour = 1.0.xxx;
	float3 diffuseTerm = diffuseFactor * diffuseAlbedo * diffuseColour;
	out_colour.xyz += diffuseTerm * lightColour * shadowed;
	
	//sh
	out_colour.xyz += sh  * baseOcclusion * baseColour;//
	
	

	// specular term
	float facing =	max( dot( En, Nn ),0 );
	float shininess = lerp( specularGlancing, specularFacing, facing );
	float3 specularColour = tex2D( specularGain, float2(facing,0.5) ).xyz;
	//specularColour = 1.0.xxx;
	float specularCutOff = saturate( 8*nDotL );
	float specularFactor = shadowed * pow( max( nDotH, 0 ), shininess ) * specularCutOff;
	float3 specularTerm = specularFactor * specularColour * specularIntensity * saturate(lightColour);
	out_colour.xyz += specularTerm;
	
	
	// sub-surface term ( note that creaseColour could be just added to diffuseAlbedo if too expensive )
	float ssCutOff = max( 8*nDotL,0  );
	float3 creaseColour = subColour * ( 1.0 - baseOcclusion );
	float3 ssColour =  creaseColour * wrapLight * shadowed;
	out_colour.xyz += ssColour;

	// peach fuzz term ( fuzz + rim )
	float fuzzCutOff = shadowed * abs( 4*nDotH );
	float rimCutOff = abs( 4*nDotE );
	float peachFactor = pow( 1 - facing , fuzzTightness ) * saturate(1-specularIntensity) * baseOcclusion * fuzzColour; 
	out_colour.xyz += peachFactor * fuzzCutOff * lightColour;
	out_colour.xyz += peachFactor * rimCutOff;
}

*/



 void bsskin_lighting_NEW(
	float3 Nn,
	float3 En,
	float3 Ln,
	
	half3 lightColour,
	half3 baseColour,
	float baseOcclusion,
	half3 subColour,
	half3 fuzzColour,	
	half fuzzTightness,
	half diffuseWrap,
	half specularIntensity,
	half specularGlancing,
	half specularFacing,
	
	half shadowTerm,
	half3 sh,
	
	sampler2D diffuseGain,
	sampler2D specularGain,
	
	out half4 out_colour
)
{
	out_colour = half4(0,0,0,1);
		
	En = -En;
	
	// some useful coefficients
	float nDotL = dot( Nn, Ln );
	float3 halfDir = normalize( 0.5 * ( Ln + En ) );
	float nDotH = dot( Nn, halfDir );
	float nDotE = dot( Nn, En );
	float facing =	max( dot( En, Nn ),0 );
	float directLightIntensity = max( nDotL, 0 );
	float3 directLight = directLightIntensity * lightColour;
	float wrapLightIntensity = ( max( nDotL, 0 ) + diffuseWrap )/( 1 + diffuseWrap );
	float3 wrapLight = wrapLightIntensity * lightColour;
	
	// diffuse term
	// note that the diffuse response is controlled by a ram texture. Thus, re-multiplying the lambertian
	// intensity may be too much
	half3 diffuseResponse = h3tex2D( diffuseGain, float2(wrapLightIntensity,0.5) ).xyz;
	half3 diffuseTerm = baseColour * diffuseResponse * shadowTerm * wrapLightIntensity;
	out_colour.xyz += diffuseTerm;
	
	// specular term
	half shininess = lerp( specularGlancing, specularFacing, facing );
	half3 specularColour = h3tex2D( specularGain, float2(facing,0.5) ).xyz;
	half specularFactor = pow( max( nDotH, 0 ), shininess );
	half3 specularTerm = specularFactor * specularColour * specularIntensity * shadowTerm;
	out_colour.xyz += specularTerm;
	
	// peach fuzz term ( fuzz + rim )
	// note the identity pre-multiplier on the dot products. Setting this to something like 4 will make rims thicker and may help 
	// some aliasing issues when fuzz is too tight.
	half fuzzCutOff = max( 1*nDotH, 0 ) * shadowTerm;	// peach fuzz depends on light and eye directions
	half rimCutOff = max( 1*nDotE, 0 );				// rim only depends on eye direction
	half3 peachFactor = pow( 1-facing, fuzzTightness ) * saturate(1-specularIntensity) * fuzzColour; 
	half3 peachTerm = peachFactor * max( fuzzCutOff, rimCutOff );
	out_colour.xyz += saturate(peachTerm); 
	
	// light colour modulation for all of the above
	out_colour.xyz *= lightColour;
	
	//sh
	out_colour.xyz += sh * baseColour;
	
	// occlusion modulation for all of the above
	out_colour.xyz  *= baseOcclusion;
	
	// sub-surface term
	half ssCutOff = max( 8*nDotL, 0  );
	half3 creaseColour = subColour * ( 1.0 - baseOcclusion );
	half3 ssColour =  creaseColour * wrapLight * shadowTerm;
	out_colour.xyz += ssColour;
}