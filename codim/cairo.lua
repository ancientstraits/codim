-- `cairo.lua` provides bindings to the `libcairo` C library for drawing images.
local ffi = require('ffi')

-- The declarations of `cairo` functions.
ffi.cdef [[
typedef struct cairo_surface cairo_surface_t;
typedef struct cairo cairo_t;
typedef enum _cairo_format {
    CAIRO_FORMAT_INVALID   = -1,
    CAIRO_FORMAT_ARGB32    = 0,
    CAIRO_FORMAT_RGB24     = 1,
    CAIRO_FORMAT_A8        = 2,
    CAIRO_FORMAT_A1        = 3,
    CAIRO_FORMAT_RGB16_565 = 4,
    CAIRO_FORMAT_RGB30     = 5
} cairo_format_t;
typedef enum _cairo_font_slant {
    CAIRO_FONT_SLANT_NORMAL,
    CAIRO_FONT_SLANT_ITALIC,
    CAIRO_FONT_SLANT_OBLIQUE
} cairo_font_slant_t;
typedef enum _cairo_font_weight {
    CAIRO_FONT_WEIGHT_NORMAL,
    CAIRO_FONT_WEIGHT_BOLD
} cairo_font_weight_t;
typedef enum _cairo_status {
    CAIRO_STATUS_SUCCESS = 0,

    CAIRO_STATUS_NO_MEMORY,
    CAIRO_STATUS_INVALID_RESTORE,
    CAIRO_STATUS_INVALID_POP_GROUP,
    CAIRO_STATUS_NO_CURRENT_POINT,
    CAIRO_STATUS_INVALID_MATRIX,
    CAIRO_STATUS_INVALID_STATUS,
    CAIRO_STATUS_NULL_POINTER,
    CAIRO_STATUS_INVALID_STRING,
    CAIRO_STATUS_INVALID_PATH_DATA,
    CAIRO_STATUS_READ_ERROR,
    CAIRO_STATUS_WRITE_ERROR,
    CAIRO_STATUS_SURFACE_FINISHED,
    CAIRO_STATUS_SURFACE_TYPE_MISMATCH,
    CAIRO_STATUS_PATTERN_TYPE_MISMATCH,
    CAIRO_STATUS_INVALID_CONTENT,
    CAIRO_STATUS_INVALID_FORMAT,
    CAIRO_STATUS_INVALID_VISUAL,
    CAIRO_STATUS_FILE_NOT_FOUND,
    CAIRO_STATUS_INVALID_DASH,
    CAIRO_STATUS_INVALID_DSC_COMMENT,
    CAIRO_STATUS_INVALID_INDEX,
    CAIRO_STATUS_CLIP_NOT_REPRESENTABLE,
    CAIRO_STATUS_TEMP_FILE_ERROR,
    CAIRO_STATUS_INVALID_STRIDE,
    CAIRO_STATUS_FONT_TYPE_MISMATCH,
    CAIRO_STATUS_USER_FONT_IMMUTABLE,
    CAIRO_STATUS_USER_FONT_ERROR,
    CAIRO_STATUS_NEGATIVE_COUNT,
    CAIRO_STATUS_INVALID_CLUSTERS,
    CAIRO_STATUS_INVALID_SLANT,
    CAIRO_STATUS_INVALID_WEIGHT,
    CAIRO_STATUS_INVALID_SIZE,
    CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED,
    CAIRO_STATUS_DEVICE_TYPE_MISMATCH,
    CAIRO_STATUS_DEVICE_ERROR,
    CAIRO_STATUS_INVALID_MESH_CONSTRUCTION,
    CAIRO_STATUS_DEVICE_FINISHED,
    CAIRO_STATUS_JBIG2_GLOBAL_MISSING,
    CAIRO_STATUS_PNG_ERROR,
    CAIRO_STATUS_FREETYPE_ERROR,
    CAIRO_STATUS_WIN32_GDI_ERROR,
    CAIRO_STATUS_TAG_ERROR,

    CAIRO_STATUS_LAST_STATUS
} cairo_status_t;
cairo_surface_t *cairo_image_surface_create(cairo_format_t format, int width, int height);
void cairo_surface_destroy (cairo_surface_t *surface);
cairo_t *cairo_create(cairo_surface_t *target);
void cairo_destroy(cairo_t *cr);

void cairo_set_source_rgb(cairo_t *cr, double red, double green, double blue);
void cairo_set_source_rgba(cairo_t *cr, double red, double green, double blue, double alpha);
void cairo_set_line_width(cairo_t *cr, double width);
void cairo_select_font_face(cairo_t *cr, const char *family, cairo_font_slant_t slant, cairo_font_weight_t weight);
void cairo_set_font_size(cairo_t *cr, double size);
void cairo_move_to(cairo_t *cr, double x, double y);
void cairo_show_text(cairo_t *cr, const char *utf8);

void cairo_rectangle(cairo_t *cr, double x, double y, double width, double height);
void cairo_stroke_preserve(cairo_t *cr);
void cairo_surface_flush(cairo_surface_t *surface);
void cairo_fill(cairo_t *cr);

unsigned char *cairo_image_surface_get_data(cairo_surface_t *surface);
int cairo_image_surface_get_height(cairo_surface_t *surface);
int cairo_image_surface_get_width(cairo_surface_t *surface);
int cairo_image_surface_get_stride(cairo_surface_t *surface);
cairo_status_t cairo_surface_write_to_png(cairo_surface_t *surface, const char *filename);
]]

-- Load the `cairo` library.
local _cairo_lib = ffi.load('cairo')

-- The return value is a table makes it conventent to use the `cairo` library.
-- Whenever the table is indexed, the prefix is automatically added.
-- This means that if the return table is assigned to a variable `cairo`,
-- `cairo_show_text()` can be called with `cairo.show_text()`
-- instead of `cairo.cairo_show_text()`.
-- It also handles constants, such as `cairo.FONT_WEIGHT_BOLD`.
return setmetatable({}, {
    __index = function(_, k)
        return _cairo_lib[(string.match(k:sub(1,1), '%u') and 'CAIRO_' or 'cairo_') .. k]
    end,
})