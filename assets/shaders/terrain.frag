#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in uint textureId;
layout(location = 3) in vec3 camPosition;
layout(location = 4) in vec3 normal;
layout(location = 5) in vec3 FragPosition;
layout(location = 6) in vec4 lightPosition;
layout(location = 7) in float specularPower;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 3) uniform sampler2D texSampler[];

void main() {
   vec3 objectColor = vec3(0.0, 1.0, 0.0);
   vec3 lightColor = vec3(1.0, 1.0, 1.0);

   // Ambient
   vec3 ambient = 0.2 * lightColor;

   // Diffuse
   vec3 norm = normalize(normal);
   vec3 lightDir = normalize(lightPosition.xyz - FragPosition);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 diffuse = diff * lightColor;

   // Specular lighting
   vec3 viewDir = normalize(camPosition - FragPosition);
   vec3 reflectDir = reflect(-lightDir, norm);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularPower);
   vec3 specular = spec * lightColor;

   vec3 result = (ambient + diffuse + specular) * objectColor;

   // Output the final color
   outColor = vec4(result, 1.0);
}
