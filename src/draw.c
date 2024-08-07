#include <math.h>

#include "state.h"
#include "passthrough.h"
#include "log.h"

#include <GLES/gl.h>

// Shaders
#define REAL_GL_FRAGMENT_SHADER 0x8b30
#define REAL_GL_VERTEX_SHADER 0x8b31
#define REAL_GL_INFO_LOG_LENGTH 0x8b84
#define REAL_GL_COMPILE_STATUS 0x8b81
GL_FUNC(glUseProgram, void, (GLuint program));
GL_FUNC(glGetUniformLocation, GLint, (GLuint program, const GLchar *name));
GL_FUNC(glUniformMatrix4fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
GL_FUNC(glUniform1i, void, (GLint location, GLint v0));
GL_FUNC(glUniform1f, void, (GLint location, GLfloat v0));
GL_FUNC(glUniform3f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2));
GL_FUNC(glUniform4f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3));
GL_FUNC(glGetAttribLocation, GLint, (GLuint program, const GLchar *name));
GL_FUNC(glEnableVertexAttribArray, void, (GLuint index));
GL_FUNC(glDisableVertexAttribArray, void, (GLuint index));
GL_FUNC(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer));
GL_FUNC(glVertexAttrib3f, void, (GLuint index, GLfloat v0, GLfloat v1, GLfloat v2));
GL_FUNC(glVertexAttrib4f, void, (GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3));
GL_FUNC(glCreateShader, GLuint, (GLenum type));
GL_FUNC(glShaderSource, void, (GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length));
GL_FUNC(glCompileShader, void, (GLuint shader));
GL_FUNC(glCreateProgram, GLuint, ());
GL_FUNC(glAttachShader, void, (GLuint program, GLuint shader));
GL_FUNC(glLinkProgram, void, (GLuint program));
GL_FUNC(glGetShaderiv, void, (GLuint shader, GLenum pname, GLint *params));
GL_FUNC(glGetShaderInfoLog, void, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog));

// Compile Shader
static void log_shader(GLuint shader, const char *name) {
    // Log
    GLint log_length = 0;
    real_glGetShaderiv()(shader, REAL_GL_INFO_LOG_LENGTH, &log_length);
    GLchar *log = malloc(log_length * sizeof (GLchar));
    ALLOC_CHECK(log);
    real_glGetShaderInfoLog()(shader, log_length, &log_length, log);
    if (log_length > 0) {
        if (log_length > 1 && log[log_length - 1] == '\n') {
            log[log_length - 1] = '\0';
        }
        DEBUG("%s Shader Compile Log: %s", name, log);
    }
    free(log);

    // Check Status
    GLint is_compiled = 0;
    real_glGetShaderiv()(shader, REAL_GL_COMPILE_STATUS, &is_compiled);
    if (!is_compiled) {
        ERR("Failed To Compile %s Shader", name);
    }
}
static GLuint compile_shader(const char *vertex_shader_text, const int vertex_shader_length, const char *fragment_shader_text, const int fragment_shader_length) {
    // Vertex Shader
    const GLuint vertex_shader = real_glCreateShader()(REAL_GL_VERTEX_SHADER);
    real_glShaderSource()(vertex_shader, 1, &vertex_shader_text, &vertex_shader_length);
    real_glCompileShader()(vertex_shader);
    log_shader(vertex_shader, "Vertex");

    // Fragment Shader
    const GLuint fragment_shader = real_glCreateShader()(REAL_GL_FRAGMENT_SHADER);
    real_glShaderSource()(fragment_shader, 1, &fragment_shader_text, &fragment_shader_length);
    real_glCompileShader()(fragment_shader);
    log_shader(fragment_shader, "Fragment");

    // Link
    GLuint program = real_glCreateProgram()();
    real_glAttachShader()(program, vertex_shader);
    real_glAttachShader()(program, fragment_shader);
    real_glLinkProgram()(program);

    // Return
    return program;
}

// Reset Static Variables
typedef struct {
    GLint *variable;
    GLint default_value;
} static_variable_t;
static static_variable_t variables_to_reset[64];
static int variables_to_reset_size = 0;
static void mark_for_reset(GLint *variable) {
    static_variable_t variable_obj;
    variable_obj.variable = variable;
    variable_obj.default_value = *variable;
    variables_to_reset[variables_to_reset_size++] = variable_obj;
}
static void reset_variables() {
    for (int i = 0; i < variables_to_reset_size; i++) {
        static_variable_t variable_obj = variables_to_reset[i];
        *variable_obj.variable = variable_obj.default_value;
    }
    variables_to_reset_size = 0;
}

// Shader
extern unsigned char main_vsh[];
extern size_t main_vsh_len;
extern unsigned char main_fsh[];
extern size_t main_fsh_len;
static GLuint get_shader() {
    static GLuint program = 0;
    if (program == 0) {
        mark_for_reset((GLint *) &program);
        program = compile_shader((const char *) main_vsh, main_vsh_len, (const char *) main_fsh, main_fsh_len);
    }
    return program;
}

// Init
void init_gles_compatibility_layer(getProcAddress_t new_getProcAddress) {
    // Setup Passthrough
    getProcAddress = new_getProcAddress;

    // State
    _init_gles_compatibility_layer_state();

    // Reset Static Variables
    reset_variables();

    // Load Shader
    GLuint program = get_shader();
    real_glUseProgram()(program);
}

// Array Pointer Drawing
#define lazy_handle(function, name) \
    static GLint name##_handle = -1; \
    if (name##_handle == -1) { \
        mark_for_reset(&name##_handle); \
        name##_handle = real_##function()(program, #name); \
        if (name##_handle == -1) { \
            ERR("Unable To Find: %s", #name); \
        } \
    }
#define lazy_uniform(name) lazy_handle(glGetUniformLocation, name)
#define lazy_attrib(name) lazy_handle(glGetAttribLocation, name)
static void draw(void (*func)(const void *), const void *data) {
    // Verify
    if (gl_state.array_pointers.vertex.size != 3 || !gl_state.array_pointers.vertex.enabled || gl_state.array_pointers.vertex.type != GL_FLOAT) {
        ERR("Unsupported Vertex Conifguration");
    }

    // Check
    GLint current_buffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_buffer);
    if (current_buffer == 0) {
        return;
    }

    // Check Mode
    const int use_color_pointer = gl_state.array_pointers.color.enabled;
    if (use_color_pointer && (gl_state.array_pointers.color.size != 4 || gl_state.array_pointers.color.type != GL_UNSIGNED_BYTE)) {
        ERR("Unsupported Color Configuration");
    }
    const int use_texture = gl_state.texture_2d && gl_state.array_pointers.tex_coord.enabled;
    if (use_texture && (gl_state.array_pointers.tex_coord.size != 2 || gl_state.array_pointers.tex_coord.type != GL_FLOAT)) {
        ERR("Unsupported Texture Configuration");
    }

    // Get Shader
    const GLuint program = get_shader();

    // Projection Matrix
    lazy_uniform(u_projection);
    matrix_t *projection = &gl_state.matrix_stacks.projection.stack[gl_state.matrix_stacks.projection.i];
    real_glUniformMatrix4fv()(u_projection_handle, 1, 0, (GLfloat *) &projection->data[0][0]);

    // Model View Matrix
    lazy_uniform(u_model_view);
    matrix_t *model_view = &gl_state.matrix_stacks.model_view.stack[gl_state.matrix_stacks.model_view.i];
    real_glUniformMatrix4fv()(u_model_view_handle, 1, 0, (GLfloat *) &model_view->data[0][0]);

    // Has Texture
    lazy_uniform(u_has_texture);
    real_glUniform1i()(u_has_texture_handle, use_texture);

    // Texture Matrix
    lazy_uniform(u_texture);
    matrix_t *texture = &gl_state.matrix_stacks.texture.stack[gl_state.matrix_stacks.texture.i];
    real_glUniformMatrix4fv()(u_texture_handle, 1, 0, (GLfloat *) &texture->data[0][0]);

    // Texture Unit
    lazy_uniform(u_texture_unit);
    real_glUniform1i()(u_texture_unit_handle, 0);

    // Alpha Test
    lazy_uniform(u_alpha_test);
    real_glUniform1i()(u_alpha_test_handle, gl_state.alpha_test);

    // Color
    lazy_attrib(a_color);
    if (use_color_pointer) {
        real_glVertexAttribPointer()(a_color_handle, gl_state.array_pointers.color.size, gl_state.array_pointers.color.type, 1, gl_state.array_pointers.color.stride, gl_state.array_pointers.color.pointer);
        real_glEnableVertexAttribArray()(a_color_handle);
    } else {
        real_glVertexAttrib4f()(a_color_handle, gl_state.color.red, gl_state.color.green, gl_state.color.blue, gl_state.color.alpha);
    }

    // Fog
    lazy_uniform(u_fog);
    real_glUniform1i()(u_fog_handle, gl_state.fog.enabled);
    if (gl_state.fog.enabled) {
        lazy_uniform(u_fog_color);
        real_glUniform4f()(u_fog_color_handle, gl_state.fog.color.red, gl_state.fog.color.green, gl_state.fog.color.blue, gl_state.fog.color.alpha);
        lazy_uniform(u_fog_is_linear);
        real_glUniform1i()(u_fog_is_linear_handle, gl_state.fog.mode == GL_LINEAR);
        lazy_uniform(u_fog_start);
        real_glUniform1f()(u_fog_start_handle, gl_state.fog.start);
        lazy_uniform(u_fog_end);
        real_glUniform1f()(u_fog_end_handle, gl_state.fog.end);
    }

    // Vertices
    lazy_attrib(a_vertex_coords);
    real_glVertexAttribPointer()(a_vertex_coords_handle, gl_state.array_pointers.vertex.size, gl_state.array_pointers.vertex.type, 0, gl_state.array_pointers.vertex.stride, gl_state.array_pointers.vertex.pointer);
    real_glEnableVertexAttribArray()(a_vertex_coords_handle);

    // Texture Coordinates
    lazy_attrib(a_texture_coords);
    if (use_texture) {
        real_glVertexAttribPointer()(a_texture_coords_handle, gl_state.array_pointers.tex_coord.size, gl_state.array_pointers.tex_coord.type, 0, gl_state.array_pointers.tex_coord.stride, gl_state.array_pointers.tex_coord.pointer);
        real_glEnableVertexAttribArray()(a_texture_coords_handle);
    } else {
        real_glVertexAttrib3f()(a_texture_coords_handle, 0, 0, 0);
    }

    // Draw
    func(data);

    // Cleanup
    if (use_color_pointer) {
        real_glDisableVertexAttribArray()(a_color_handle);
    }
    real_glDisableVertexAttribArray()(a_vertex_coords_handle);
    if (use_texture) {
        real_glDisableVertexAttribArray()(a_texture_coords_handle);
    }
}

// glDrawArrays
struct cmd_glDrawArrays {
    GLenum mode;
    GLint first;
    GLsizei count;
};
GL_FUNC(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count));
static void do_glDrawArrays(const void *data) {
    const struct cmd_glDrawArrays *cmd = data;
    real_glDrawArrays()(cmd->mode, cmd->first, cmd->count);
}
void glDrawArrays(const GLenum mode, const GLint first, const GLsizei count) {
    const struct cmd_glDrawArrays cmd = {
        .mode = mode,
        .first = first,
        .count = count
    };
    draw(do_glDrawArrays, &cmd);
}

// glMultiDrawArrays
struct cmd_glMultiDrawArrays {
    GLenum mode;
    const GLint *first;
    const GLsizei *count;
    GLsizei drawcount;
};
GL_FUNC(glMultiDrawArraysEXT, void, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount));
static void do_glMultiDrawArrays(const void *data) {
    const struct cmd_glMultiDrawArrays *cmd = data;
    real_glMultiDrawArraysEXT()(cmd->mode, cmd->first, cmd->count, cmd->drawcount);
}
void glMultiDrawArrays(const GLenum mode, const GLint *first, const GLsizei *count, const GLsizei drawcount) {
    const struct cmd_glMultiDrawArrays cmd = {
        .mode = mode,
        .first = first,
        .count = count,
        .drawcount = drawcount
    };
    draw(do_glMultiDrawArrays, &cmd);
}