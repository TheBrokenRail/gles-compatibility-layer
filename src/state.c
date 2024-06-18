#include <string.h>

#include "log.h"

#include "state.h"
#include "passthrough.h"

// GL State
#define init_array_pointer \
    { \
        .enabled = 0 \
    }
#define init_light_source \
    .enabled = 0, \
    .position = { \
        .x = 0, \
        .y = 0, \
        .z = 1 \
    }
static gl_state_t init_gl_state = {
    .color = {
        .red = 1,
        .green = 1,
        .blue = 1,
        .alpha = 1
    },
    .normal = {
        .x = 0,
        .y = 0,
        .z = 1
    },
    .rescale_normal = 0,
    .array_pointers = {
        .vertex = init_array_pointer,
        .color = init_array_pointer,
        .tex_coord = init_array_pointer,
        .normal = init_array_pointer
    },
    .matrix_stacks = {
        .mode = GL_MODELVIEW
    },
    .alpha_test = 0,
    .texture_2d = 0,
    .fog = {
        .enabled = 0,
        .mode = GL_LINEAR,
        .color = {
            .red = 0,
            .green = 0,
            .blue = 0,
            .alpha = 0
        },
        .start = 0,
        .end = 1
    },
    .lighting = {
        .enabled = 0,
        .light_sources = {
            {
                init_light_source,
                .diffuse = {
                    .red = 1,
                    .green = 1,
                    .blue = 1,
                    .alpha = 1
                }
            },
            {
                init_light_source,
                .diffuse = {
                    .red = 0,
                    .green = 0,
                    .blue = 0,
                    .alpha = 0
                }
            }
        },
        .ambient = {
            .red = 0.2f,
            .green = 0.2f,
            .blue = 0.2f,
            .alpha = 0.2f
        }
    }
};
gl_state_t gl_state;
void _init_gles_compatibility_layer_state() {
    gl_state = init_gl_state;
    _init_gles_compatibility_matrix_stacks();
}

// Change Color
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    gl_state.color.red = red;
    gl_state.color.green = green;
    gl_state.color.blue = blue;
    gl_state.color.alpha = alpha;
}

// Change Normal
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
    gl_state.normal.x = nx;
    gl_state.normal.y = ny;
    gl_state.normal.z = nz;
}

// Array Pointer Storage
#define ARRAY_POINTER_FUNC(func, name) \
    void func(GLint size, GLenum type, GLsizei stride, const void *pointer) { \
        gl_state.array_pointers.name.size = size; \
        gl_state.array_pointers.name.type = type; \
        gl_state.array_pointers.name.stride = stride; \
        gl_state.array_pointers.name.pointer = pointer; \
    }
ARRAY_POINTER_FUNC(glVertexPointer, vertex)
ARRAY_POINTER_FUNC(glColorPointer, color)
ARRAY_POINTER_FUNC(glTexCoordPointer, tex_coord)
static ARRAY_POINTER_FUNC(fake_glNormalPointer, normal)
void glNormalPointer(GLenum type, GLsizei stride, const void *pointer) {
    fake_glNormalPointer(0, type, stride, pointer);
}
static array_pointer_t *get_array_pointer(GLenum array) {
    switch (array) {
        case GL_VERTEX_ARRAY: {
            return &gl_state.array_pointers.vertex;
        }
        case GL_COLOR_ARRAY: {
            return &gl_state.array_pointers.color;
        }
        case GL_TEXTURE_COORD_ARRAY: {
            return &gl_state.array_pointers.tex_coord;
        }
        case GL_NORMAL_ARRAY: {
            return &gl_state.array_pointers.normal;
        }
        default: {
            ERR("Unsupported Array Pointer: %i", array);
        }
    }
}
void glEnableClientState(GLenum array) {
    get_array_pointer(array)->enabled = 1;
}
void glDisableClientState(GLenum array) {
    get_array_pointer(array)->enabled = 0;
}

// Enable/Disable State
GL_FUNC(glEnable, void, (GLenum cap));
void glEnable(GLenum cap) {
    switch (cap) {
        case GL_ALPHA_TEST: {
            gl_state.alpha_test = 1;
            break;
        }
        case GL_TEXTURE_2D: {
            gl_state.texture_2d = 1;
            break;
        }
        case GL_RESCALE_NORMAL: {
            gl_state.rescale_normal = 1;
            break;
        }
        case GL_COLOR_MATERIAL: {
            // Ignore
            break;
        }
        case GL_FOG: {
            gl_state.fog.enabled = 1;
            break;
        }
        case GL_LIGHTING: {
            gl_state.lighting.enabled = 1;
            break;
        }
        case GL_LIGHT0:
        case GL_LIGHT1: {
            gl_state.lighting.light_sources[cap - GL_LIGHT0].enabled = 1;
            break;
        }
        default: {
            real_glEnable()(cap);
            break;
        }
    }
}
GL_FUNC(glDisable, void, (GLenum cap));
void glDisable(GLenum cap) {
    switch (cap) {
        case GL_ALPHA_TEST: {
            gl_state.alpha_test = 0;
            break;
        }
        case GL_TEXTURE_2D: {
            gl_state.texture_2d = 0;
            break;
        }
        case GL_RESCALE_NORMAL: {
            gl_state.rescale_normal = 0;
            break;
        }
        case GL_COLOR_MATERIAL: {
            // Ignore
            break;
        }
        case GL_FOG: {
            gl_state.fog.enabled = 0;
            break;
        }
        case GL_LIGHTING: {
            gl_state.lighting.enabled = 0;
            break;
        }
        case GL_LIGHT0:
        case GL_LIGHT1: {
            gl_state.lighting.light_sources[cap - GL_LIGHT0].enabled = 0;
            break;
        }
        default: {
            real_glDisable()(cap);
            break;
        }
    }
}
void glAlphaFunc(GLenum func, GLclampf ref) {
    if (func != GL_GREATER && ref != 0.1f) {
        ERR("Unsupported Alpha Function");
    }
}

// Fog
#define UNSUPPORTED_FOG() ERR("Unsupported Fog Configuration")
void glFogfv(GLenum pname, const GLfloat *params) {
    if (pname == GL_FOG_COLOR) {
        gl_state.fog.color.red = params[0];
        gl_state.fog.color.green = params[1];
        gl_state.fog.color.blue = params[2];
        gl_state.fog.color.alpha = params[3];
    } else {
        UNSUPPORTED_FOG();
    }
}
void glFogx(GLenum pname, GLfixed param) {
    if (pname == GL_FOG_MODE && (param == GL_LINEAR || param == GL_EXP)) {
        gl_state.fog.mode = param;
    } else {
        UNSUPPORTED_FOG();
    }
}
void glFogf(GLenum pname, GLfloat param) {
    switch (pname) {
        case GL_FOG_DENSITY:
        case GL_FOG_START: {
            gl_state.fog.start = param;
            break;
        }
        case GL_FOG_END: {
            gl_state.fog.end = param;
            break;
        }
        default: {
            UNSUPPORTED_FOG();
            break;
        }
    }
}

// Get Matrix Data
GL_FUNC(glGetFloatv, void, (GLenum pname, GLfloat *params));
void glGetFloatv(GLenum pname, GLfloat *params) {
    switch (pname) {
        case GL_MODELVIEW_MATRIX: {
            memcpy((void *) params, gl_state.matrix_stacks.model_view.stack[gl_state.matrix_stacks.model_view.i].data, MATRIX_DATA_SIZE);
            break;
        }
        case GL_PROJECTION_MATRIX: {
            memcpy((void *) params, gl_state.matrix_stacks.projection.stack[gl_state.matrix_stacks.projection.i].data, MATRIX_DATA_SIZE);
            break;
        }
        default: {
            real_glGetFloatv()(pname, params);
            break;
        }
    }
}

// Configure Light Sources
void glLightfv(GLenum light, GLenum pname, const GLfloat *params) {
    light_source_t *light_source = &gl_state.lighting.light_sources[light - GL_LIGHT0];
    if (pname == GL_DIFFUSE) {
        light_source->diffuse.red = params[0];
        light_source->diffuse.green = params[1];
        light_source->diffuse.blue = params[2];
        light_source->diffuse.alpha = params[3];
    } else if (pname == GL_POSITION) {
        // Transform Position By Modelview Matrix
        matrix_t model_view = gl_state.matrix_stacks.model_view.stack[gl_state.matrix_stacks.model_view.i];
        GLfloat out[4];
        for (int i = 0; i < 4; i++) {
            GLfloat result = 0;
            for (int j = 0; j < 4; j++) {
                result += model_view.data[j][i] * params[j];
            }
            out[i] = result;
        }
        // Store
        light_source->position.x = out[0];
        light_source->position.y = out[1];
        light_source->position.z = out[2];
    }
}

// Global Ambient Lighting
void glLightModelfv(GLenum pname, const GLfloat *params) {
    if (pname == GL_LIGHT_MODEL_AMBIENT) {
        gl_state.lighting.ambient.red = params[0];
        gl_state.lighting.ambient.green = params[1];
        gl_state.lighting.ambient.blue = params[2];
        gl_state.lighting.ambient.alpha = params[3];
    }
}
