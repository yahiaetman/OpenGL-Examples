#ifndef OUR_UTILS_IMGUI_H
#define OUR_UTILS_IMGUI_H

#include <functional>
#include <iterator>
#include <optional>
#include <map>
#include <iterator>
#include <type_traits>

#include <glad/gl.h>
#include <imgui.h>
#include <glm/common.hpp>

namespace our {

    namespace gl_enum_options {
        typedef std::map<GLenum, const char*> const EnumMap;

        inline EnumMap primitives = {
                {GL_POINTS, "GL_POINTS"},
                {GL_LINES, "GL_LINES"},
                {GL_LINE_STRIP, "GL_LINE_STRIP"},
                {GL_LINE_LOOP, "GL_LINE_LOOP"},
                {GL_TRIANGLES, "GL_TRIANGLES"},
                {GL_TRIANGLE_STRIP, "GL_TRIANGLE_STRIP"},
                {GL_TRIANGLE_FAN, "GL_TRIANGLE_FAN"},
        };

        inline EnumMap polygon_modes = {
                {GL_POINT, "GL_POINT"},
                {GL_LINE, "GL_LINE"},
                {GL_FILL, "GL_FILL"}
        };

        inline EnumMap comparison_functions = {
                {GL_ALWAYS, "GL_ALWAYS"},
                {GL_NEVER, "GL_NEVER"},
                {GL_EQUAL, "GL_EQUAL"},
                {GL_NOTEQUAL, "GL_NOTEQUAL"},
                {GL_LESS, "GL_LESS"},
                {GL_LEQUAL, "GL_LEQUAL"},
                {GL_GREATER, "GL_GREATER"},
                {GL_GEQUAL, "GL_GEQUAL"}
        };

        inline EnumMap face_windings = {
                {GL_CCW, "GL_CCW"},
                {GL_CW, "GL_CW"}
        };

        inline EnumMap facets = {
                {GL_FRONT, "GL_FRONT"},
                {GL_BACK, "GL_BACK"},
                {GL_FRONT_AND_BACK, "GL_FRONT_AND_BACK"}
        };

        inline EnumMap texture_magnification_filters = {
                {GL_NEAREST, "GL_NEAREST"},
                {GL_LINEAR, "GL_LINEAR"}
        };

        inline EnumMap texture_minification_filters = {
                {GL_NEAREST, "GL_NEAREST"},
                {GL_LINEAR, "GL_LINEAR"},
                {GL_NEAREST_MIPMAP_NEAREST, "GL_NEAREST_MIPMAP_NEAREST"},
                {GL_LINEAR_MIPMAP_NEAREST, "GL_LINEAR_MIPMAP_NEAREST"},
                {GL_NEAREST_MIPMAP_LINEAR, "GL_NEAREST_MIPMAP_LINEAR"},
                {GL_LINEAR_MIPMAP_LINEAR, "GL_LINEAR_MIPMAP_LINEAR"}
        };

        inline EnumMap texture_wrapping_modes = {
                {GL_CLAMP_TO_EDGE, "GL_CLAMP_TO_EDGE"},
                {GL_CLAMP_TO_BORDER, "GL_CLAMP_TO_BORDER"},
                {GL_REPEAT, "GL_REPEAT"},
                {GL_MIRRORED_REPEAT, "GL_MIRRORED_REPEAT"},
                {GL_MIRROR_CLAMP_TO_EDGE, "GL_MIRROR_CLAMP_TO_EDGE"}
        };

        inline EnumMap blend_functions = {
                {GL_ZERO, "GL_ZERO"},
                {GL_ONE, "GL_ONE"},
                {GL_SRC_COLOR, "GL_SRC_COLOR"},
                {GL_ONE_MINUS_SRC_COLOR, "GL_ONE_MINUS_SRC_COLOR"},
                {GL_DST_COLOR, "GL_DST_COLOR"},
                {GL_ONE_MINUS_DST_COLOR, "GL_ONE_MINUS_DST_COLOR"},
                {GL_SRC_ALPHA, "GL_SRC_ALPHA"},
                {GL_ONE_MINUS_SRC_ALPHA, "GL_ONE_MINUS_SRC_ALPHA"},
                {GL_DST_ALPHA, "GL_DST_ALPHA"},
                {GL_ONE_MINUS_DST_ALPHA, "GL_ONE_MINUS_DST_ALPHA"},
                {GL_CONSTANT_COLOR, "GL_CONSTANT_COLOR"},
                {GL_ONE_MINUS_CONSTANT_COLOR, "GL_ONE_MINUS_CONSTANT_COLOR"},
                {GL_CONSTANT_ALPHA, "GL_CONSTANT_ALPHA"},
                {GL_ONE_MINUS_CONSTANT_ALPHA, "GL_ONE_MINUS_CONSTANT_ALPHA"}
        };

        inline EnumMap blend_equations = {
                {GL_FUNC_ADD, "GL_FUNC_ADD"},
                {GL_FUNC_SUBTRACT, "GL_FUNC_SUBTRACT"},
                {GL_FUNC_REVERSE_SUBTRACT, "GL_FUNC_REVERSE_SUBTRACT"},
                {GL_MIN, "GL_MIN"},
                {GL_MAX, "GL_MAX"}
        };
    }

    template<typename T>
    static void OptionMapCombo(const char* label, T& selected, const std::map<T, const char*>& options){
        if(ImGui::BeginCombo(label, options.at(selected))){
            for(auto const& [key, value] : options){
                bool is_selected = selected == key;
                if(ImGui::Selectable(value, is_selected))
                    selected = key;
                if(is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    template<typename It>
    static void IteratorCombo(const char* label, std::string& selected, It begin, It end){
        if(ImGui::BeginCombo(label, selected.c_str())){
            for(It it = begin; it != end; ++it){
                auto& key = *it;
                bool is_selected = selected == key;
                if(ImGui::Selectable(key.c_str(), is_selected))
                    selected = key;
                if(is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    template<typename It>
    static void PairIteratorCombo(const char* label, std::string& selected, It begin, It end){
        if(ImGui::BeginCombo(label, selected.c_str())){
            for(It it = begin; it != end; ++it){
                auto& [key, value] = *it;
                bool is_selected = selected == key;
                if(ImGui::Selectable(key.c_str(), is_selected))
                    selected = key;
                if(is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    static bool ColorEdit4U8(const char* label, uint8_t* color, ImGuiColorEditFlags flags = 0){
        float color_f32[4] = { color[0]/255.0f, color[1]/255.0f, color[2]/255.0f, color[3]/255.0f };
        bool result = ImGui::ColorEdit4(label, color_f32, flags);
        color[0] = 255 * color_f32[0];
        color[1] = 255 * color_f32[1];
        color[2] = 255 * color_f32[2];
        color[3] = 255 * color_f32[3];
        return result;
    }

    template<typename It>
    static void ReorderableList(It begin, It end,
                                std::function<void(size_t,typename std::iterator_traits<It>::value_type&)> item_gui,
                                std::function<void(size_t)> item_add,
                                std::function<void(size_t)> item_delete,
                                size_t starting_index = 0){
        size_t index = starting_index;
        size_t distance = std::distance(begin, end);
        size_t last = starting_index + distance - 1;
        std::optional<size_t> item_to_delete, item_to_add;
        std::optional<std::tuple<It, It>> items_to_swap;
        float item_spacing = ImGui::GetStyle().ItemSpacing.x, window_width = ImGui::GetWindowWidth();
        float full_button_width = window_width - 2 * item_spacing;
        float third_button_width = (window_width - 4 * item_spacing) / 3;
        for(It it = begin, prev; it != end; prev = it, it++, ++index){
            ImGui::PushID(index);

            ImGui::PushStyleColor(ImGuiCol_Button, {0.25,0.65,0.15,1});
            if(ImGui::Button("+", ImVec2(full_button_width, 0)))
                item_to_add = index;
            ImGui::PopStyleColor();
            ImGui::Separator();

            item_gui(index, *it);

            ImGui::PushStyleColor(ImGuiCol_Button, {0.65,0.15,0.25,1});
            if(ImGui::Button("DELETE", ImVec2(third_button_width, 0)))
                item_to_delete = index;
            ImGui::PopStyleColor();
            ImGui::SameLine();
            bool up_enabled = index > starting_index;
            if (!up_enabled)
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            if(ImGui::Button("UP", ImVec2(third_button_width, 0)) && up_enabled)
                items_to_swap = { prev, it };
            if (!up_enabled)
                ImGui::PopStyleVar();
            ImGui::SameLine();
            bool down_enabled = index < last;
            if (!down_enabled)
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            if(ImGui::Button("DOWN", ImVec2(third_button_width, 0)) && down_enabled)
                items_to_swap = { it, it + 1 };
            if (!down_enabled)
                ImGui::PopStyleVar();
            ImGui::Separator();
            ImGui::PopID();
        }
        ImGui::PushStyleColor(ImGuiCol_Button, {0.25,0.65,0.15,1});
        if(ImGui::Button("+", ImVec2(full_button_width, 0)))
            item_to_add = index;
        ImGui::PopStyleColor();

        if(item_to_add.has_value())
            item_add(item_to_add.value());
        else if(item_to_delete.has_value())
            item_delete(item_to_delete.value());
        else if(items_to_swap.has_value()){
            auto [it1, it2] = items_to_swap.value();
            auto temp = *it1;
            *it1 = *it2;
            *it2 = temp;
        }

    }

}

#endif //OUR_UTILS_IMGUI_H
