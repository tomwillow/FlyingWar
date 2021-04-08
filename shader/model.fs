#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
//uniform vec3 texture_normal1;

uniform vec3 viewPos;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 lightColor1;
uniform vec3 lightColor2;

struct Material
{
    vec3 specular;
    float shininess;
};
uniform Material material;

void main()
{    

  float ambientStrength=0.1f;
  vec3 ambient=ambientStrength*(lightColor1+lightColor2);
  
  vec3 norm=normalize(Normal);//
  vec3 lightDir1=normalize(lightPos1-FragPos);
  vec3 lightDir2=normalize(lightPos2-FragPos);
  float diff1 = max(dot(norm, lightDir1), 0.0);
  float diff2 = max(dot(norm, lightDir2), 0.0);
  vec3 diffuse1 = diff1 * lightColor1;
  vec3 diffuse2 = diff2 * lightColor2;
  vec3 diffuse = (diffuse1+diffuse2)*vec3(texture(texture_diffuse1,TexCoord)+texture(texture_diffuse2,TexCoord));
  
  float specularStrength=1;
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir1 = reflect(-lightDir1, norm);
  vec3 reflectDir2 = reflect(-lightDir2, norm);
  float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), material.shininess);
  float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), material.shininess);
  vec3 specular = specularStrength * (spec1+spec2) * (lightColor1+lightColor2) * material.specular;
  
  vec3 result=ambient+diffuse+specular;
  color = vec4(result,1.0f);
}