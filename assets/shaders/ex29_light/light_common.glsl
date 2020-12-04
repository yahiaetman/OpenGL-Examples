#ifndef OUR_LIGHT_COMMON_GLSL_INCLUDED
#define OUR_LIGHT_COMMON_GLSL_INCLUDED

    // These are some common functions and data structures for all types of lights so we wrote them in a single file to be included in the other files.

    // This will be used to compute the diffuse factor.
    float calculate_lambert(vec3 normal, vec3 light_direction){
        return max(0.0f, dot(normal, -light_direction));
    }

    // This will be used to compute the phong specular.
    float calculate_phong(vec3 normal, vec3 light_direction, vec3 view, float shininess){
        vec3 reflected = reflect(light_direction, normal);
        return pow(max(0.0f, dot(view, reflected)), shininess);
    }

    // This contains all the material properties in a single struct.
    struct Material {
        vec3 diffuse;
        vec3 specular;
        vec3 ambient;
        float shininess;
    };

#endif
