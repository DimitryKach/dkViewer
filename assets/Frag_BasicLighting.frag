#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;
noperspective in vec3 GEdgeDistance;

out vec4 color;

uniform sampler2D texture1;
uniform bool hasTexture;
uniform bool doWire;
uniform vec3 lightColor;
uniform vec3 wireColor;
uniform float gWireframeWidth = 0.001;
//uniform vec3 viewPos;

vec2 FlipTexCoord;

void main()
{
    FlipTexCoord = vec2(TexCoord.x, 1.0 - TexCoord.y);
    color = vec4(0.5, 0.5, 0.5, 1.0);
    if (hasTexture)
    {
        color = texture(texture1, TexCoord);
    }
    //color = mix(tx1, tx2, tx2.w*0.5);

    // Lets shade this bad boy!
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float ambientStrength = 0.1;
    float specStr = 1.0;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * diff;
    vec3 ambient = lightColor * ambientStrength;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specStr * spec * lightColor;

    vec3 result = (diffuse + ambient + specular) * vec3(color);
    if (doWire)
    {
        if (GEdgeDistance.x >= 0)
        {
	        float d = min( GEdgeDistance.x, min(GEdgeDistance.y, GEdgeDistance.z ));

            float mixVal = 0.0;
            if (d < gWireframeWidth)
            {
                mixVal = 1.0;
            } else if (d > gWireframeWidth)
            {
                mixVal = 0.0;
            } else
            {
                float x = d - gWireframeWidth;
                mixVal = exp2(-2.0 * x * x);
            }
            result = mix(result, wireColor, mixVal);
        }        
    }

    color = vec4(result, 1.0);
}
