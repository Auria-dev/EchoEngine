#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gARM;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uARM;

// https://iquilezles.org/articles/texturerepetition/
vec4 hash4( vec2 p ) { return fract(sin(vec4( 1.0+dot(p,vec2(37.0,17.0)), 
                                              2.0+dot(p,vec2(11.0,47.0)),
                                              3.0+dot(p,vec2(41.0,29.0)),
                                              4.0+dot(p,vec2(23.0,31.0))))*103.0); }

vec4 textureNoTile( sampler2D samp, vec2 uv )
{
    vec2 p = floor( uv );
    vec2 f = fract( uv );
    
    // derivatives (for correct mipmapping)
    vec2 ddx = dFdx( uv );
    vec2 ddy = dFdy( uv );
    
    // voronoi contribution
    vec4 va = vec4( 0.0 );
    float wt = 0.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2( float(i), float(j) );
        vec4 o = hash4( p + g );
        vec2 r = g - f + o.xy;
        float d = dot(r,r);
        float w = exp(-5.0*d );
        vec4 c = textureGrad( samp, uv + o.zw, ddx, ddy );
        va += w*c;
        wt += w;
    }
    
    return va/wt;
}

void main()
{
    gPosition = vec4(fs_in.FragPos, 1.0);

    vec3 tNormal = texture(uNormal, fs_in.TexCoords).rgb;
    tNormal = normalize(tNormal * 2.0 - 1.0);
    vec3 viewNormal = normalize(fs_in.TBN * tNormal);
    
    gNormal = vec4(viewNormal, 1.0);

    vec3 tAlbedo = texture(uAlbedo, fs_in.TexCoords).rgb;
    vec3 tARM    = texture(uARM, fs_in.TexCoords).rgb;

    gAlbedo = vec4(tAlbedo, 1.0);
    gARM    = vec4(tARM, 1.0);
}