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
        vec3 emissive;
        float shininess;
    };

    struct TexturedMaterial {
        sampler2D albedo_map;
        vec3 albedo_tint;
        sampler2D specular_map;
        vec3 specular_tint;
        sampler2D ambient_occlusion_map;
        sampler2D roughness_map;
        vec2 roughness_range;
        sampler2D emissive_map;
        vec3 emissive_tint;
    };

    Material sample_material(TexturedMaterial tex_mat, vec2 tex_coord){
        Material mat;
        mat.diffuse = tex_mat.albedo_tint * texture(tex_mat.albedo_map, tex_coord).rgb;
        mat.specular = tex_mat.specular_tint * texture(tex_mat.specular_map, tex_coord).rgb;
        mat.emissive = tex_mat.emissive_tint * texture(tex_mat.emissive_map, tex_coord).rgb;
        mat.ambient = mat.diffuse * texture(tex_mat.ambient_occlusion_map, tex_coord).r;

        float roughness = mix(tex_mat.roughness_range.x, tex_mat.roughness_range.y,
            texture(tex_mat.roughness_map, tex_coord).r);
        mat.shininess = 2.0f/pow(clamp(roughness, 0.001f, 0.999f), 4.0f) - 2.0f;

        return mat;
    }

#endif
