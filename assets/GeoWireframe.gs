#version 330 core
layout(triangles) in;                                                                  
layout(triangle_strip) out;                                                         
layout(max_vertices = 3) out;  
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec3 LightPos;
noperspective out vec3 GEdgeDistance;
in vec2 TexCoord0[];
in vec3 Normal0[];
in vec3 LightPos0[];
in vec3 VertPos0[];
uniform mat4 ViewMtx; // Viewport matrix
void main()
{
	// Transform each vertex into viewport space
	vec3 p0 = vec3(ViewMtx * (gl_in[0].gl_Position / gl_in[0].gl_Position.w));
	vec3 p1 = vec3(ViewMtx * (gl_in[1].gl_Position / gl_in[1].gl_Position.w));
	vec3 p2 = vec3(ViewMtx * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));
	// Find the altitudes (ha, hb and hc)
	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);
	float alpha = acos( (b*b + c*c - a*a) / (2.0*b*c) );
	float beta = acos( (a*a + c*c - b*b) / (2.0*a*c) );
	float ha = abs( c * sin( beta ) );
	float hb = abs( c * sin( alpha ) );
	float hc = abs( b * sin( alpha ) );
	// Send the triangle along with the edge distances
	GEdgeDistance = vec3( ha, 0, 0 );
	Normal = Normal0[0];
	FragPos = VertPos0[0];
	TexCoord = TexCoord0[0];
	LightPos = LightPos0[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	GEdgeDistance = vec3( 0, hb, 0 );
	Normal = Normal0[1];
	FragPos = VertPos0[1];
	TexCoord = TexCoord0[1];
	LightPos = LightPos0[1];
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	GEdgeDistance = vec3( 0, 0, hc );
	Normal = Normal0[2];
	FragPos = VertPos0[2];
	TexCoord = TexCoord0[2];
	LightPos = LightPos0[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	EndPrimitive();
}