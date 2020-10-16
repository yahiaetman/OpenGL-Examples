#ifndef OUR_LIGHT_COMMON_GLSL_INCLUDED
#define OUR_LIGHT_COMMON_GLSL_INCLUDED

    float calculate_lambert(vec3 normal, vec3 light_direction){
        return max(0.0f, dot(normal, -light_direction));
    }

    float calculate_phong(vec3 normal, vec3 light_direction, vec3 view, float shininess){
        vec3 reflected = reflect(light_direction, normal);
        return pow(max(0.0f, dot(view, reflected)), shininess);
    }

    struct Material {
        vec3 diffuse;
        vec3 specular;
        vec3 ambient;
        float shininess;
    };

#endif
