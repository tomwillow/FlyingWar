#version 330 core
out vec4 FragColor;
in vec3 fragPos;
in vec3 normal;
in float mountainHeight;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{
    float ambientStrength=0.25;
    vec3 ambient=ambientStrength*lightColor;
    
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 norm = normalize(normal);
    float diff = max(dot(norm,lightDir),0.0);
    vec3 diffuse = diff * lightColor;
    
    float specularStrength=0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir,reflectDir),0.0),8);
    vec3 specular = specularStrength*spec*lightColor;
    
    vec3 grassColor = vec3(0.535,0.562,0.140);
    vec3 rockColor = vec3(0.414,0.277,0.207);
    vec3 snowColor = vec3(1);
    float snowLine = 150;
    float grassLine = 100;
    vec3 objectColor;
    if (mountainHeight>=snowLine)
      objectColor = rockColor;
    else
      if (mountainHeight<=grassLine)
        objectColor = grassColor;
      else
        objectColor = (rockColor-grassColor)/(snowLine-grassLine)*mountainHeight+grassColor;
    //objectColor.y += random()*0.4-0.2;
    //objectColor = grassColor;
    
    vec3 result = (ambient+diffuse+specular)*objectColor;//
    
    FragColor = vec4(result,1.0);
} 