#version 330 core
out vec4 FragColor;
// 材质属性
struct Material {
    sampler2D diffuse; // 漫反射
    sampler2D specular;// 镜面反射
    float shininess;   // 反光度
}; 
// 定向光
struct DirLight {
    vec3 direction; // 方向：从光源至片段
	
    vec3 ambient;   // 漫反射
    vec3 diffuse;   // 镜面反射
    vec3 specular;  // 反光度
};
// 点光源
struct PointLight {
    vec3 position;  // 位置
    
    float constant; // 衰减值常数项
    float linear;   // 衰减值一阶项
    float quadratic;// 衰减值二阶项
    vec3 ambient;   // 漫反射
    vec3 diffuse;   // 镜面反射
    vec3 specular;  // 反光度
};
// 聚光源
struct SpotLight {
    vec3 position;  // 位置
    vec3 direction; // 方向
    float cutOff;   // 近切光角的余弦值
    float outerCutOff;// 远切光角余弦值
  
    float constant; // 衰减值常数项
    float linear;   // 衰减值一阶项
    float quadratic;// 衰减值二阶项
  
    vec3 ambient;   // 漫反射
    vec3 diffuse;   // 镜面反射
    vec3 specular;  // 反光度  
};

#define NR_POINT_LIGHTS 4   // 点光源数量

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    // properties
    // 属性
    vec3 norm = normalize(Normal);              // 法向量
    vec3 viewDir = normalize(viewPos - FragPos);// 视线向量
    
    // phase 1: directional lighting
    // 定向光
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: point lights
    // 点光
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
    // phase 3: spot light
    // 聚光
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);// 光照方向取反：现从片段至光源
    // diffuse shading
    // 计算漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    // 计算镜面反射
    vec3 reflectDir = reflect(-lightDir, normal); // 得到反射向量
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    // 合并结果
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // 得到光照向量
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal); // 得到反射
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);// 距离
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));// 计算衰减系数  
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // 光照向量
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    // 内切和外切的余弦值差
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);// clamp函数：将第一个参数约束到第二/三个参数之间
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}