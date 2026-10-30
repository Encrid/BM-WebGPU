/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "probe.h"

static const float SHIFT[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
    0.5f, 0, 0, 1 };
static const float SHRINK[16] = { 0.5f, 0, 0, 0, 0, 0.5f, 0, 0, 0, 0, 1, 0,
    -0.5f, 0.5f, 0, 1 };
static const float TURN[16] = { -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0,
    0, 0, 0, 1 };

static const float EDGE[3][3] = { { 0, 0, 0.5f }, { 64, 64, 0.5f },
    { 0, 64, 0.5f } };
static const float ODD[3][3] = { { 8, 32, 0.5f }, { 40, 32, 0.5f },
    { 56, 32, 0.5f } };
static const float EVEN[4][3] = { { 8, 32, 0.5f }, { 40, 32, 0.5f },
    { 56, 32, 0.5f }, { 64, 32, 0.5f } };
static const float CROSS[2][3] = { { 8, 32, -0.5f }, { 56, 32, 0.5f } };
static const float BEHIND[2][3] = { { 8, 32, -0.5f }, { 56, 32, -0.5f } };
static const float CORNER[4][3] = { { 8, 40, 0.5f }, { 40, 40, 0.5f },
    { 40, 40, 0.5f }, { 40, 8, 0.5f } };
static const float CORNER_PAST[4][3] = { { 8, 40, 0.5f }, { 48, 40, 0.5f },
    { 40, 40, 0.5f }, { 40, 8, 0.5f } };

#define RED { 1, 0, 0, 1 }
#define GREEN { 0, 1, 0, 1 }
#define WHITE { 1, 1, 1, 1 }
#define COLORS { { 255, 0, 0, 255 }, { 0, 255, 0, 255 }, \
    { 0, 0, 255, 255 }, { 255, 255, 0, 255 } }

static const item_t UNLIT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 255, 204, 0, 255 } },
    { .kind = DRAW, .mesh = 1, .color = { 0.2f, 1, 1, 1 }, .views = 1 }
};

static const item_t WHITE_MESH[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = { 0.2f, 1, 1, 1 }, .views = 1 }
};

static const item_t VIEW_ONLY[] = {
    { .kind = VIEW, .mask = 1 }
};

static const item_t SOLID[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t STOP[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } } },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 4, .texture = 3, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t HALF_LEFT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t PIPELINES[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 32, 64 }, .z = 0.5f, .views = 1 },
    { .kind = BATCH, .at = { 32, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 0, 255, 0, 255 }, .views = 1, .from = 2 },
    { .kind = BATCH, .at = { 0, 0, 16, 16 }, .z = 0.5f, .views = 1,
        .from = 2 }
};

static const item_t PIPELINES_EARLY[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 32, 64 }, .z = 0.5f, .views = 1 },
    { .kind = BATCH, .at = { 32, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 0, 255, 0, 255 }, .views = 1 },
    { .kind = BATCH, .at = { 0, 0, 16, 16 }, .z = 0.5f, .views = 1 }
};

static const item_t BLEND[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = { 0, 0, 0, 1 }, .views = 1 },
    { .kind = DRAW, .mesh = 1, .color = { 1, 1, 1, 0.5f }, .views = 1,
        .order = 1 }
};

static const item_t BLEND_OPAQUE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = { 0, 0, 0, 1 }, .views = 1 },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .views = 1, .order = 1 }
};

static const item_t TESTED[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = GREEN, .views = 1, .depth = 1 }
};

static const item_t FACES[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = MESH, .at = { 32, 0, 64, 64 }, .z = 0.5f, .clockwise = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 1 }
};

static const item_t REPEAT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 2 }, .texels = COLORS },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 2, 2 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t ERRORS[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t HUSK[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .end = 2,
        .look = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t TEXTURE_HUSK[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } },
        .end = 2, .look = true },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t MESH_LEAVE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .leave = 2,
        .look = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t TEXTURE_LEAVE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } },
        .leave = 2, .look = true },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t DETACH_WAIT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 21, 64 }, .z = 0.5f, .kill = 5,
        .look = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .kill = 5 },
    { .kind = MESH, .at = { 21, 0, 43, 64 }, .z = 0.5f, .kill = 5,
        .look = true },
    { .kind = DRAW, .mesh = 3, .color = RED, .views = 1, .kill = 5 },
    { .kind = MESH, .at = { 43, 0, 64, 64 }, .z = 0.5f, .kill = 5,
        .look = true },
    { .kind = DRAW, .mesh = 5, .color = RED, .views = 1, .kill = 5 }
};

static const item_t TEXTURE_DETACH_WAIT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } },
        .kill = 5, .look = true },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } },
        .kill = 5, .look = true },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } },
        .kill = 5, .look = true }
};

static const item_t REUPLOAD[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 21, 64 }, .z = 0.5f, .leave = 4,
        .back = 5 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = MESH, .at = { 21, 0, 43, 64 }, .z = 0.5f, .leave = 4,
        .back = 5 },
    { .kind = DRAW, .mesh = 3, .color = RED, .views = 1 },
    { .kind = MESH, .at = { 43, 0, 64, 64 }, .z = 0.5f, .leave = 4,
        .back = 5 },
    { .kind = DRAW, .mesh = 5, .color = RED, .views = 1 }
};

static const item_t MESH_LATE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .from = 2 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .from = 2 }
};

static const item_t TEXTURE_LATE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 0, 255, 0, 255 } },
        .from = 2 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1, .from = 2 }
};

static const item_t TEXTURE_EARLY[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 0, 255, 0, 255 } } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1, .from = 2 }
};

static const item_t TWO_MESHES[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = MESH, .at = { 32, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 3, .color = RED, .views = 1 }
};

static const item_t KEEP_UPLOAD[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .reprim = 3,
        .prim_to = 1 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t KEEP_UPLOAD_GEN[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .reprim = 3,
        .prim_to = 1, .regen = 3 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t REGEN[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 255, 0, 0, 255 }, .alter = { 0, 255, 0, 255 },
        .change = 3, .regen = 3 },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .views = 1 }
};

static const item_t SAME[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 255, 0, 0, 255 }, .alter = { 0, 255, 0, 255 },
        .change = 3 },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .views = 1 }
};

static const item_t TEXTURE_REGEN[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } },
        .alter = { 0, 255, 0, 255 }, .change = 3, .regen = 3 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t TEXTURE_SAME[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } },
        .alter = { 0, 255, 0, 255 }, .change = 3 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t LOST_DESTROY[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .kill = 3 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t EARLY_LEAVE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .late = true,
        .kill = 2 }
};

static const item_t EARLY_STAY[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .late = true }
};

static const item_t TEXTURE_ZERO[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 1, .color = GREEN, .shading = 2, .views = 1 }
};

static const item_t TEXTURE_RED[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = GREEN, .shading = 2,
        .views = 1 }
};

static const item_t TEXTURE_GONE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } },
        .end = 2 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t TEXTURE_KEPT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t TEXTURE_BAD[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 255, 0, 0, 255 } },
        .bad = SHORT, .show = true },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t NO_ROOM[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f, .end = 4 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = MESH, .at = { 32, 0, 64, 64 }, .z = 0.5f, .from = 3 },
    { .kind = DRAW, .mesh = 3, .color = RED, .views = 1, .from = 3 }
};

static const item_t NO_ROOM_KEPT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = MESH, .at = { 32, 0, 64, 64 }, .z = 0.5f, .from = 3 },
    { .kind = DRAW, .mesh = 3, .color = RED, .views = 1, .from = 3 }
};

static const item_t TEXTURE_NO_ROOM[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 0, 255, 0, 255 } },
        .from = 3 },
    { .kind = MESH, .at = { 32, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 4, .texture = 3, .color = WHITE, .shading = 2,
        .views = 1, .from = 3 }
};

static const item_t MANY_VIEWS[] = {
    { .kind = VIEW, .mask = 1, .repeat = 16 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t DEPTH_KEEP[] = {
    { .kind = VIEW, .mask = 1 }
};

static const item_t NO_DEPTH[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.25f },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.75f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 3 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 1, .depth = 1,
        .order = 1 }
};

static const item_t VIEW_ORDER[] = {
    { .kind = VIEW, .mask = 1, .order = 1 },
    { .kind = VIEW, .mask = 2, .order = 0 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 2, .color = RED, .views = 1 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 2 }
};

static const item_t VIEW_ORDER_SWAP[] = {
    { .kind = VIEW, .mask = 1, .order = 0 },
    { .kind = VIEW, .mask = 2, .order = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 2, .color = RED, .views = 1 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 2 }
};

static const item_t DRAW_BATCH[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .order = 5 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 0, 255, 0, 255 }, .views = 1, .order = -5 }
};

static const item_t DRAW_BATCH_OFF[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .order = 5 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 0, 255, 0, 255 }, .views = 2, .order = -5 }
};

static const item_t VIEW_MASK[] = {
    { .kind = VIEW, .mask = 2 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = MESH, .at = { 32, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 3 }
};

static const item_t VIEW_MASK_ONE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = MESH, .at = { 32, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 3 }
};

static const item_t MSAA[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .verts = EDGE, .vert_count = 3, .views = 1 }
};

static const item_t VIEW_MATRIX[] = {
    { .kind = VIEW, .mask = 1, .matrix = SHRINK },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t CLIP_OUT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = -0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t RECT[] = {
    { .kind = VIEW, .mask = 1, .rect = { 0, 0, 32, 32 } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t WHOLE_RECT[] = {
    { .kind = VIEW, .mask = 1, .rect = { 8, 8, 0, 16 } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t RECT_16[] = {
    { .kind = VIEW, .mask = 1, .rect = { 8, 8, 16, 16 } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t PAST_EDGE[] = {
    { .kind = VIEW, .mask = 1, .rect = { 32, 0, 64, 64 } },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t INSIDE[] = {
    { .kind = VIEW, .mask = 1, .rect = { 0, 0, 64, 64 } },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t OUT_VIEW[] = {
    { .kind = VIEW, .mask = 1, .rect = { 64, 0, 64, 64 }, .show = true },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t IN_VIEW[] = {
    { .kind = VIEW, .mask = 1, .rect = { 32, 0, 32, 64 }, .show = true },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t WIDE_VIEW[] = {
    { .kind = VIEW, .mask = 1, .rect = { 0, 0, 65, 64 }, .show = true },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t FIT_VIEW[] = {
    { .kind = VIEW, .mask = 1, .rect = { 0, 0, 64, 64 }, .show = true },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t FAR_VIEW[] = {
    { .kind = VIEW, .mask = 1, .rect = { 4294967280u, 0, 32, 64 },
        .show = true },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t NEAR_VIEW[] = {
    { .kind = VIEW, .mask = 1, .rect = { 0, 0, 32, 64 }, .show = true },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t BELOW_VIEW[] = {
    { .kind = VIEW, .mask = 1, .rect = { 0, 64, 64, 64 }, .show = true },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t FLAT_VIEW[] = {
    { .kind = VIEW, .mask = 1, .rect = { 0, 0, 64, 0 }, .show = true },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t LINES[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 8.5f, 31.5f, 56.5f, 31.5f }, .z = 0.5f,
        .primitive = 1, .views = 1 }
};

static const item_t POINTS[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 32.5f, 31.5f, 32.5f, 31.5f }, .z = 0.5f,
        .primitive = 2, .views = 1 }
};

static const item_t TRIANGLES[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t ODD_LINE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .verts = ODD, .vert_count = 3, .primitive = 1,
        .views = 1, .line = 3 }
};

static const item_t EVEN_LINE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .verts = EVEN, .vert_count = 4, .primitive = 1,
        .views = 1, .line = 3 }
};

static const item_t TEXTURED[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 2 }, .texels = COLORS },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 7,
        .normal = { 0, 0, 1 }, .uv = { 0, 0, 1, 1 },
        .rgba = { 255, 255, 255, 255 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t TEXTURED_FLIP[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 2 }, .texels = { { 0, 0, 255, 255 },
        { 255, 255, 0, 255 }, { 255, 0, 0, 255 }, { 0, 255, 0, 255 } } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 7,
        .normal = { 0, 0, 1 }, .uv = { 0, 0, 1, 1 },
        .rgba = { 255, 255, 255, 255 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t LINEAR[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 1 }, .filter = 1,
        .texels = { { 0, 0, 0, 255 }, { 255, 255, 255, 255 } } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0.5f, 0.5f, 0.5f, 0.5f } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t NEAREST_MID[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 1 }, .filter = 0,
        .texels = { { 0, 0, 0, 255 }, { 255, 255, 255, 255 } } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0.5f, 0.5f, 0.5f, 0.5f } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t NEAREST[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 1 }, .filter = 0,
        .texels = { { 0, 0, 0, 255 }, { 255, 255, 255, 255 } } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0.4f, 0.5f, 0.4f, 0.5f } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t LINEAR_NEAR[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 1 }, .filter = 1,
        .texels = { { 0, 0, 0, 255 }, { 255, 255, 255, 255 } } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0.4f, 0.5f, 0.4f, 0.5f } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t MODEL[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .matrix = SHIFT, .color = RED, .views = 1 }
};

static const item_t LAMBERT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f, .layout = 1,
        .normal = { 0, 0, 1 } },
    { .kind = MESH, .at = { 32, 0, 64, 64 }, .z = 0.5f, .layout = 1,
        .normal = { 0, 0, -1 } },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .shading = 1, .views = 1 },
    { .kind = DRAW, .mesh = 2, .color = WHITE, .shading = 1, .views = 1 }
};

static const item_t LAMBERT_FLIP[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f, .layout = 1,
        .normal = { 0, 0, -1 } },
    { .kind = MESH, .at = { 32, 0, 64, 64 }, .z = 0.5f, .layout = 1,
        .normal = { 0, 0, 1 } },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .shading = 1, .views = 1 },
    { .kind = DRAW, .mesh = 2, .color = WHITE, .shading = 1, .views = 1 }
};

static const item_t TURNED[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .layout = 1,
        .normal = { 0, 0, -1 } },
    { .kind = DRAW, .mesh = 1, .matrix = TURN, .color = WHITE, .shading = 1,
        .views = 1 }
};

static const item_t UNTURNED[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .layout = 1,
        .normal = { 0, 0, -1 } },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .shading = 1, .views = 1 }
};

static const item_t LAMBERT_ALPHA[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 1,
        .normal = { 0, 0, 1 } },
    { .kind = DRAW, .mesh = 1, .color = { 1, 1, 1, 0.5f }, .shading = 1,
        .views = 1 }
};

static const item_t LAMBERT_OPAQUE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 1,
        .normal = { 0, 0, 1 } },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .shading = 1, .views = 1 }
};

static const item_t NO_NORMAL[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .shading = 1, .views = 1 }
};

static const item_t NO_UV[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 2 }, .texels = COLORS },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 5,
        .normal = { 0, 0, 1 }, .rgba = { 255, 255, 255, 255 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t DRAW_ORDER[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .order = 1 },
    { .kind = DRAW, .mesh = 1, .color = GREEN, .views = 1, .order = 0 }
};

static const item_t DRAW_ORDER_SWAP[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .order = 0 },
    { .kind = DRAW, .mesh = 1, .color = GREEN, .views = 1, .order = 1 }
};

static const item_t SAME_ORDER[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 },
    { .kind = DRAW, .mesh = 1, .color = GREEN, .views = 1 }
};

static const item_t SAME_ORDER_SWAP[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = GREEN, .views = 1 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t EXTREMES[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1,
        .order = INT32_MAX },
    { .kind = DRAW, .mesh = 1, .color = GREEN, .views = 1,
        .order = INT32_MIN }
};

static const item_t EXTREMES_SWAP[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1,
        .order = INT32_MIN },
    { .kind = DRAW, .mesh = 1, .color = GREEN, .views = 1,
        .order = INT32_MAX }
};

static const item_t DEPTH_TEST[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.25f },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.75f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 2 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 1, .depth = 1,
        .order = 1 }
};

static const item_t DEPTH_NO_TEST[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.25f },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.75f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 2 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 1, .depth = 0,
        .order = 1 }
};

static const item_t DEPTH_NO_WRITE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.25f },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.75f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 0 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 1, .depth = 1,
        .order = 1 }
};

static const item_t OTHER_BITS[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.25f },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.75f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 6 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 1, .depth = 5,
        .order = 1 }
};

static const item_t OTHER_BITS_OFF[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.25f },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.75f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 6 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 1, .depth = 4,
        .order = 1 }
};

static const item_t BIAS[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = MESH, .at = { 8.5f, 31.5f, 56.5f, 31.5f }, .z = 0.6f,
        .primitive = 1 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 3 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 1, .depth = 1,
        .order = 1, .bias = 0.2f }
};

static const item_t NO_BIAS[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = MESH, .at = { 8.5f, 31.5f, 56.5f, 31.5f }, .z = 0.6f,
        .primitive = 1 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 3 },
    { .kind = DRAW, .mesh = 2, .color = GREEN, .views = 1, .depth = 1,
        .order = 1 }
};

static const item_t THIN_POINT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 32.5f, 31.5f, 32.5f, 31.5f }, .z = 0.5f,
        .primitive = 2, .views = 1, .line = 0.5f }
};

static const item_t WIDE_POINT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 32.5f, 31.5f, 32.5f, 31.5f }, .z = 0.5f,
        .primitive = 2, .views = 1, .line = 5 }
};

static const item_t AWAY_POINT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 96.5f, 31.5f, 96.5f, 31.5f }, .z = 0.5f,
        .primitive = 2, .views = 1, .line = 5 }
};

static const item_t WIDE_LINE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 8.5f, 31.5f, 56.5f, 31.5f }, .z = 0.5f,
        .primitive = 1, .views = 1, .line = 3 }
};

static const item_t THIN_LINE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 8.5f, 31.5f, 56.5f, 31.5f }, .z = 0.5f,
        .primitive = 1, .views = 1, .line = 1 }
};

static const item_t TRIM[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .verts = CROSS, .vert_count = 2, .primitive = 1,
        .views = 1, .line = 3 }
};

static const item_t TRIM_AWAY[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .verts = BEHIND, .vert_count = 2, .primitive = 1,
        .views = 1, .line = 3 }
};

static const item_t BATCH_ROUND[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 255, 0, 0, 255 }, .alter = { 0, 255, 0, 255 },
        .change = 2, .views = 1 }
};

static const item_t BATCH_STILL[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 255, 0, 0, 255 }, .alter = { 0, 255, 0, 255 },
        .views = 1 }
};

static const item_t BATCH_SIDES[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .matrix = SHIFT, .color = RED, .views = 1 },
    { .kind = BATCH, .at = { 0, 0, 16, 64 }, .z = 0.5f, .layout = 5,
        .normal = { 0, 0, -1 }, .rgba = { 0, 255, 0, 255 }, .views = 1 },
    { .kind = BATCH, .at = { 48, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t BATCH_SWAP[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 32, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .matrix = SHIFT, .color = RED, .views = 1 },
    { .kind = BATCH, .at = { 0, 0, 16, 64 }, .z = 0.5f, .layout = 1,
        .normal = { 0, 0, -1 }, .views = 1 },
    { .kind = BATCH, .at = { 48, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 0, 0, 255, 255 }, .views = 1 }
};

static const item_t BATCH_ORDER[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 255, 0, 0, 255 }, .views = 1, .order = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 0, 255, 0, 255 }, .views = 1, .order = 0 }
};

static const item_t BATCH_ORDER_SWAP[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 255, 0, 0, 255 }, .views = 1, .order = 0 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 4,
        .rgba = { 0, 255, 0, 255 }, .views = 1, .order = 1 }
};

static const item_t BATCH_DEPTH[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.25f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 2 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.75f, .layout = 4,
        .rgba = { 0, 255, 0, 255 }, .views = 1, .depth = 1 }
};

static const item_t BATCH_DEPTH_OFF[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.25f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .depth = 2 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.75f, .layout = 4,
        .rgba = { 0, 255, 0, 255 }, .views = 1, .depth = 0 }
};

static const item_t BATCH_VIEWS[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 2 }
};

static const item_t BATCH_VIEWS_ONE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1 }
};

static const item_t WHOLE_DATA[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .show = true }
};

static const item_t OUTSIDE_DATA[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .bad = OUTSIDE,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t BELOW_DATA[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .bad = BELOW,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t WRAP_DATA[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .bad = WRAP,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t CUT_DATA[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .bad = CUT,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t EMPTY_DATA[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .bad = EMPTY,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t TEXTURE_WHOLE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 2 }, .texels = COLORS, .show = true },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t TEXTURE_SHORT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 2 }, .texels = COLORS, .bad = SHORT,
        .show = true },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t NO_MESH[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 0, .color = RED, .views = 1, .show = true }
};

static const item_t PAST_MESH[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = PAST, .color = RED, .views = 1, .show = true }
};

static const item_t DEAD_MESH[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .kill = 2 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1, .show = true }
};

static const item_t PRIMITIVE_3[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .primitive = 3,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t LAYOUT_8[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 8,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t SHADING_3[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .shading = 3, .views = 1,
        .show = true }
};

static const item_t FILTER_2[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 2, 2 }, .texels = COLORS, .filter = 2,
        .show = true },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t WIDE_TEXTURE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 65, 1 }, .texels = { { 255, 0, 0, 255 } },
        .show = true },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t FIT_TEXTURE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 64, 1 }, .texels = { { 255, 0, 0, 255 } },
        .show = true },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t BIG_MESH[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .count = 342,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t SMALL_MESH[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .count = 6,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t BATCH_CUT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .bad = CUT,
        .views = 1, .show = true }
};

static const item_t BATCH_WHOLE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .views = 1,
        .show = true }
};

static const item_t BATCH_LAYOUT_8[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 8,
        .views = 1, .show = true }
};

static const item_t BATCH_PRIMITIVE_3[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .primitive = 3,
        .views = 1, .show = true }
};

static const item_t RETRY[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .bad = CUT,
        .fix = 3 },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t RETRY_NEVER[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .bad = CUT },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t MASS_LEAVE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .repeat = 300,
        .kill = 2 }
};

static const item_t DRAW_POINT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 32.5f, 31.5f, 32.5f, 31.5f }, .z = 0.5f,
        .primitive = 2 },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .views = 1, .line = 5 }
};

static const item_t DRAW_POINT_THIN[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 32.5f, 31.5f, 32.5f, 31.5f }, .z = 0.5f,
        .primitive = 2 },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .views = 1, .line = 1 }
};

static const item_t PAST_BOTTOM[] = {
    { .kind = VIEW, .mask = 1, .rect = { 0, 32, 64, 64 } },
    { .kind = MESH, .at = { 0, 0, 64, 32 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t INSIDE_TOP[] = {
    { .kind = VIEW, .mask = 1, .rect = { 0, 0, 64, 64 } },
    { .kind = MESH, .at = { 0, 0, 64, 32 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .views = 1 }
};

static const item_t BENT[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 1,
        .normal = { 0, 0, 1 }, .bend = true },
    { .kind = DRAW, .mesh = 1, .color = WHITE, .shading = 1, .views = 1 }
};

static const item_t GRAY[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = { 0.5f, 0.5f, 0.5f, 1 }, .views = 1 }
};

static const item_t GRAY_TEXEL[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = TEXTURE, .size = { 1, 1 }, .texels = { { 128, 128, 128, 255 } } },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .layout = 2,
        .uv = { 0, 0, 1, 1 } },
    { .kind = DRAW, .mesh = 2, .texture = 1, .color = WHITE, .shading = 2,
        .views = 1 }
};

static const item_t CORNER_LINE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .verts = CORNER, .vert_count = 4, .primitive = 1,
        .views = 1, .line = 8 }
};

static const item_t CORNER_PAST_LINE[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = BATCH, .verts = CORNER_PAST, .vert_count = 4, .primitive = 1,
        .views = 1, .line = 8 }
};

static const item_t VIEW_AFTER_DRAW[] = {
    { .kind = VIEW, .mask = 1, .rect = { 64, 0, 64, 64 }, .show = true },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f },
    { .kind = DRAW, .mesh = 1, .color = RED, .shading = 3, .views = 1,
        .show = true }
};

static const item_t TWO_BAD[] = {
    { .kind = VIEW, .mask = 1 },
    { .kind = MESH, .at = { 0, 0, 64, 64 }, .z = 0.5f, .bad = CUT,
        .show = true },
    { .kind = DRAW, .mesh = 1, .color = RED, .shading = 3, .views = 1,
        .from = 2, .show = true }
};

static const gpu_spec_t PLAIN = { WGPUTextureFormat_RGBA8Unorm, 1,
    WGPUTextureFormat_Depth24PlusStencil8 };
static const gpu_spec_t DEPTH32 = { WGPUTextureFormat_RGBA8Unorm, 1,
    WGPUTextureFormat_Depth32Float };
static const gpu_spec_t FLAT = { WGPUTextureFormat_RGBA8Unorm, 1, 0 };
static const gpu_spec_t MULTI = { WGPUTextureFormat_RGBA8Unorm, 4,
    WGPUTextureFormat_Depth24PlusStencil8 };
static const gpu_spec_t SWAPPED = { WGPUTextureFormat_BGRA8Unorm, 1, 0 };
static const gpu_spec_t SRGB = { WGPUTextureFormat_RGBA8UnormSrgb, 1,
    WGPUTextureFormat_Depth24PlusStencil8 };
static const gpu_spec_t STENCIL32 = { WGPUTextureFormat_RGBA8Unorm, 1,
    WGPUTextureFormat_Depth32FloatStencil8 };

static const scene_t SCENES[] = {
    { .name = "unlit", .rounds = 2, .spec = PLAIN, .items = UNLIT,
        .item_count = COUNT(UNLIT), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "white", .rounds = 2, .spec = PLAIN, .items = WHITE_MESH,
        .item_count = COUNT(WHITE_MESH), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "clear", .rounds = 1, .spec = PLAIN, .items = VIEW_ONLY,
        .item_count = COUNT(VIEW_ONLY), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "reload", .rounds = 1, .spec = PLAIN },
    { .name = "stop", .rounds = 2, .spec = PLAIN, .counts = true,
        .items = STOP, .item_count = COUNT(STOP) },
    { .name = "empty", .rounds = 2, .spec = PLAIN, .counts = true,
        .items = VIEW_ONLY, .item_count = COUNT(VIEW_ONLY) },
    { .name = "noframe", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = SOLID, .item_count = COUNT(SOLID),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "nodevice", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = SOLID, .item_count = COUNT(SOLID),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "skipped", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = SOLID, .item_count = COUNT(SOLID),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "steady", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = SOLID, .item_count = COUNT(SOLID),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "first", .rounds = 2, .spec = PLAIN, .counts = true,
        .items = VIEW_ONLY, .item_count = COUNT(VIEW_ONLY) },
    { .name = "pipelines", .rounds = 2, .spec = PLAIN, .counts = true,
        .items = PIPELINES, .item_count = COUNT(PIPELINES) },
    { .name = "pipelinesearly", .rounds = 2, .spec = PLAIN, .counts = true,
        .items = PIPELINES_EARLY, .item_count = COUNT(PIPELINES_EARLY) },
    { .name = "newdevice", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = SOLID, .item_count = COUNT(SOLID),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "newformat", .rounds = 4, .spec = FLAT, .items = SOLID,
        .item_count = COUNT(SOLID), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "newsamples", .rounds = 4, .spec = PLAIN, .items = SOLID,
        .item_count = COUNT(SOLID), .points = { { 32, 40 } },
        .point_count = 1 },
    { .name = "newdepth", .rounds = 4, .spec = PLAIN, .items = SOLID,
        .item_count = COUNT(SOLID), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "noscene", .rounds = 1, .spec = PLAIN, .none = true,
        .items = VIEW_ONLY, .item_count = COUNT(VIEW_ONLY),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "blendnext", .rounds = 5, .spec = PLAIN, .items = BLEND,
        .item_count = COUNT(BLEND), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "comparenext", .rounds = 5, .spec = PLAIN, .items = TESTED,
        .item_count = COUNT(TESTED), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "cullnext", .rounds = 5, .spec = PLAIN, .items = SOLID,
        .item_count = COUNT(SOLID), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "clockwisenext", .rounds = 5, .spec = PLAIN, .items = SOLID,
        .item_count = COUNT(SOLID), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "repeatnext", .rounds = 5, .spec = PLAIN, .items = REPEAT,
        .item_count = COUNT(REPEAT), .points = { { 40, 8 } },
        .point_count = 1 },
    { .name = "loadnext", .rounds = 2, .spec = PLAIN, .items = VIEW_ONLY,
        .item_count = COUNT(VIEW_ONLY), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "cleardepthnext", .rounds = 2, .spec = DEPTH32,
        .items = VIEW_ONLY, .item_count = COUNT(VIEW_ONLY),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "memorynext", .rounds = 2, .spec = PLAIN, .items = SOLID,
        .item_count = COUNT(SOLID), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "errors", .rounds = 2, .spec = SWAPPED, .items = ERRORS,
        .item_count = COUNT(ERRORS), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "husk", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = HUSK, .item_count = COUNT(HUSK) },
    { .name = "texhusk", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = TEXTURE_HUSK, .item_count = COUNT(TEXTURE_HUSK) },
    { .name = "meshleave", .rounds = 3, .spec = PLAIN, .counts = true,
        .items = MESH_LEAVE, .item_count = COUNT(MESH_LEAVE) },
    { .name = "texleave", .rounds = 3, .spec = PLAIN, .counts = true,
        .items = TEXTURE_LEAVE, .item_count = COUNT(TEXTURE_LEAVE) },
    { .name = "meshstay", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = SOLID, .item_count = COUNT(SOLID) },
    { .name = "texstay", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = TEXTURE_KEPT, .item_count = COUNT(TEXTURE_KEPT) },
    { .name = "detachwait", .rounds = 6, .spec = PLAIN,
        .items = DETACH_WAIT, .item_count = COUNT(DETACH_WAIT) },
    { .name = "reupload", .rounds = 5, .spec = PLAIN, .items = REUPLOAD,
        .item_count = COUNT(REUPLOAD),
        .points = { { 10, 32 }, { 32, 32 }, { 54, 32 } },
        .point_count = 3 },
    { .name = "firstmesh", .rounds = 2, .spec = PLAIN, .items = SOLID,
        .item_count = COUNT(SOLID), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "latemesh", .rounds = 2, .spec = PLAIN, .items = MESH_LATE,
        .item_count = COUNT(MESH_LATE), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "firsttexture", .rounds = 3, .spec = PLAIN,
        .items = TEXTURE_LATE, .item_count = COUNT(TEXTURE_LATE),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "earlytexture", .rounds = 3, .spec = PLAIN,
        .items = TEXTURE_EARLY, .item_count = COUNT(TEXTURE_EARLY),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "attachwait", .rounds = 3, .spec = PLAIN, .items = TWO_MESHES,
        .item_count = COUNT(TWO_MESHES),
        .points = { { 16, 32 }, { 48, 32 } }, .point_count = 2 },
    { .name = "keepupload", .rounds = 3, .spec = PLAIN,
        .items = KEEP_UPLOAD, .item_count = COUNT(KEEP_UPLOAD),
        .points = { { 16, 16 } }, .point_count = 1 },
    { .name = "keepuploadgen", .rounds = 3, .spec = PLAIN,
        .items = KEEP_UPLOAD_GEN, .item_count = COUNT(KEEP_UPLOAD_GEN),
        .points = { { 16, 16 } }, .point_count = 1 },
    { .name = "regen", .rounds = 3, .spec = PLAIN, .counts = true,
        .items = REGEN, .item_count = COUNT(REGEN),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "same", .rounds = 3, .spec = PLAIN, .counts = true,
        .items = SAME, .item_count = COUNT(SAME),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "texregen", .rounds = 3, .spec = PLAIN, .items = TEXTURE_REGEN,
        .item_count = COUNT(TEXTURE_REGEN), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "texsame", .rounds = 3, .spec = PLAIN, .items = TEXTURE_SAME,
        .item_count = COUNT(TEXTURE_SAME), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "lostdestroy", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = LOST_DESTROY, .item_count = COUNT(LOST_DESTROY) },
    { .name = "keptdestroy", .rounds = 4, .spec = PLAIN, .counts = true,
        .items = LOST_DESTROY, .item_count = COUNT(LOST_DESTROY) },
    { .name = "earlyleave", .rounds = 3, .spec = PLAIN, .counts = true,
        .items = EARLY_LEAVE, .item_count = COUNT(EARLY_LEAVE) },
    { .name = "earlystay", .rounds = 3, .spec = PLAIN, .counts = true,
        .items = EARLY_STAY, .item_count = COUNT(EARLY_STAY) },
    { .name = "texzero", .rounds = 2, .spec = PLAIN, .items = TEXTURE_ZERO,
        .item_count = COUNT(TEXTURE_ZERO), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "texred", .rounds = 2, .spec = PLAIN, .items = TEXTURE_RED,
        .item_count = COUNT(TEXTURE_RED), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "texgone", .rounds = 3, .spec = PLAIN, .items = TEXTURE_GONE,
        .item_count = COUNT(TEXTURE_GONE), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "texkept", .rounds = 3, .spec = PLAIN, .items = TEXTURE_KEPT,
        .item_count = COUNT(TEXTURE_KEPT), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "texbad", .rounds = 2, .spec = PLAIN, .items = TEXTURE_BAD,
        .item_count = COUNT(TEXTURE_BAD), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "noroom", .rounds = 5, .spec = PLAIN, .items = NO_ROOM,
        .item_count = COUNT(NO_ROOM),
        .points = { { 16, 32 }, { 48, 32 } }, .point_count = 2 },
    { .name = "noroomkept", .rounds = 5, .spec = PLAIN,
        .items = NO_ROOM_KEPT, .item_count = COUNT(NO_ROOM_KEPT),
        .points = { { 16, 32 }, { 48, 32 } }, .point_count = 2 },
    { .name = "texnoroom", .rounds = 4, .spec = PLAIN,
        .items = TEXTURE_NO_ROOM, .item_count = COUNT(TEXTURE_NO_ROOM),
        .points = { { 16, 32 }, { 48, 32 } }, .point_count = 2 },
    { .name = "twomeshes", .rounds = 2, .spec = PLAIN, .items = TWO_MESHES,
        .item_count = COUNT(TWO_MESHES),
        .points = { { 16, 32 }, { 48, 32 } }, .point_count = 2 },
    { .name = "twomeshesmore", .rounds = 3, .spec = PLAIN,
        .items = TWO_MESHES, .item_count = COUNT(TWO_MESHES),
        .points = { { 16, 32 }, { 48, 32 } }, .point_count = 2 },
    { .name = "manyviews", .rounds = 2, .spec = PLAIN, .small = true,
        .items = MANY_VIEWS, .item_count = COUNT(MANY_VIEWS),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "oneview", .rounds = 2, .spec = PLAIN, .small = true,
        .items = SOLID, .item_count = COUNT(SOLID),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "halfleft", .rounds = 2, .spec = PLAIN, .items = HALF_LEFT,
        .item_count = COUNT(HALF_LEFT),
        .points = { { 16, 32 }, { 48, 32 } }, .point_count = 2 },
    { .name = "depthclear", .rounds = 1, .spec = DEPTH32,
        .items = VIEW_ONLY, .item_count = COUNT(VIEW_ONLY),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "depthkeep", .rounds = 1, .spec = DEPTH32, .prefill = true,
        .prefill_depth = 0.25f, .items = DEPTH_KEEP,
        .item_count = COUNT(DEPTH_KEEP), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "stencil", .rounds = 1, .spec = PLAIN, .prefill = true,
        .prefill_depth = 1.0f, .prefill_stencil = 5, .items = VIEW_ONLY,
        .item_count = COUNT(VIEW_ONLY), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "stencil32", .rounds = 1, .spec = STENCIL32, .prefill = true,
        .prefill_depth = 1.0f, .prefill_stencil = 5, .items = VIEW_ONLY,
        .item_count = COUNT(VIEW_ONLY), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "blend", .rounds = 2, .spec = PLAIN, .items = BLEND,
        .item_count = COUNT(BLEND), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "blendopaque", .rounds = 2, .spec = PLAIN,
        .items = BLEND_OPAQUE, .item_count = COUNT(BLEND_OPAQUE),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "compare", .rounds = 2, .spec = PLAIN, .items = TESTED,
        .item_count = COUNT(TESTED), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "faces", .rounds = 2, .spec = PLAIN, .items = FACES,
        .item_count = COUNT(FACES), .points = { { 16, 32 }, { 48, 32 } },
        .point_count = 2 },
    { .name = "repeat", .rounds = 2, .spec = PLAIN, .items = REPEAT,
        .item_count = COUNT(REPEAT), .points = { { 40, 8 } },
        .point_count = 1 },
    { .name = "nodepth", .rounds = 2, .spec = FLAT, .items = NO_DEPTH,
        .item_count = COUNT(NO_DEPTH), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "withdepth", .rounds = 2, .spec = PLAIN, .items = NO_DEPTH,
        .item_count = COUNT(NO_DEPTH), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "vieworder", .rounds = 2, .spec = PLAIN, .items = VIEW_ORDER,
        .item_count = COUNT(VIEW_ORDER), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "vieworderswap", .rounds = 2, .spec = PLAIN,
        .items = VIEW_ORDER_SWAP, .item_count = COUNT(VIEW_ORDER_SWAP),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "drawbatch", .rounds = 2, .spec = PLAIN, .items = DRAW_BATCH,
        .item_count = COUNT(DRAW_BATCH), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "drawbatchoff", .rounds = 2, .spec = PLAIN,
        .items = DRAW_BATCH_OFF, .item_count = COUNT(DRAW_BATCH_OFF),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "viewmask", .rounds = 2, .spec = PLAIN, .items = VIEW_MASK,
        .item_count = COUNT(VIEW_MASK), .points = { { 16, 32 }, { 48, 32 } },
        .point_count = 2 },
    { .name = "viewmaskone", .rounds = 2, .spec = PLAIN,
        .items = VIEW_MASK_ONE, .item_count = COUNT(VIEW_MASK_ONE),
        .points = { { 16, 32 }, { 48, 32 } }, .point_count = 2 },
    { .name = "msaa", .rounds = 1, .spec = MULTI, .dark = true, .items = MSAA,
        .item_count = COUNT(MSAA), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "msaaone", .rounds = 1, .spec = PLAIN, .dark = true,
        .items = MSAA, .item_count = COUNT(MSAA), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "viewmatrix", .rounds = 2, .spec = PLAIN, .items = VIEW_MATRIX,
        .item_count = COUNT(VIEW_MATRIX),
        .points = { { 16, 16 }, { 48, 48 } }, .point_count = 2 },
    { .name = "solid", .rounds = 2, .spec = PLAIN, .items = SOLID,
        .item_count = COUNT(SOLID), .points = { { 16, 16 }, { 48, 48 },
        { 63, 63 } }, .point_count = 3 },
    { .name = "clipout", .rounds = 2, .spec = PLAIN, .items = CLIP_OUT,
        .item_count = COUNT(CLIP_OUT), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "rect", .rounds = 2, .spec = PLAIN, .items = RECT,
        .item_count = COUNT(RECT), .points = { { 16, 16 }, { 48, 48 } },
        .point_count = 2 },
    { .name = "wholerect", .rounds = 2, .spec = PLAIN, .items = WHOLE_RECT,
        .item_count = COUNT(WHOLE_RECT), .points = { { 63, 63 } },
        .point_count = 1 },
    { .name = "rect16", .rounds = 2, .spec = PLAIN, .items = RECT_16,
        .item_count = COUNT(RECT_16), .points = { { 63, 63 } },
        .point_count = 1 },
    { .name = "pastedge", .rounds = 2, .spec = PLAIN, .items = PAST_EDGE,
        .item_count = COUNT(PAST_EDGE), .points = { { 16, 32 }, { 40, 32 } },
        .point_count = 2 },
    { .name = "inside", .rounds = 2, .spec = PLAIN, .items = INSIDE,
        .item_count = COUNT(INSIDE), .points = { { 16, 32 }, { 40, 32 } },
        .point_count = 2 },
    { .name = "outview", .rounds = 1, .spec = PLAIN, .items = OUT_VIEW,
        .item_count = COUNT(OUT_VIEW), .points = { { 40, 32 } },
        .point_count = 1 },
    { .name = "inview", .rounds = 1, .spec = PLAIN, .items = IN_VIEW,
        .item_count = COUNT(IN_VIEW), .points = { { 40, 32 } },
        .point_count = 1 },
    { .name = "wideview", .rounds = 1, .spec = PLAIN, .small = true,
        .items = WIDE_VIEW, .item_count = COUNT(WIDE_VIEW),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "fitview", .rounds = 1, .spec = PLAIN, .small = true,
        .items = FIT_VIEW, .item_count = COUNT(FIT_VIEW),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "farview", .rounds = 1, .spec = PLAIN, .items = FAR_VIEW,
        .item_count = COUNT(FAR_VIEW), .points = { { 16, 32 } },
        .point_count = 1 },
    { .name = "nearview", .rounds = 1, .spec = PLAIN, .items = NEAR_VIEW,
        .item_count = COUNT(NEAR_VIEW), .points = { { 16, 32 } },
        .point_count = 1 },
    { .name = "belowview", .rounds = 1, .spec = PLAIN, .items = BELOW_VIEW,
        .item_count = COUNT(BELOW_VIEW), .points = { { 40, 32 } },
        .point_count = 1 },
    { .name = "flatview", .rounds = 1, .spec = PLAIN, .items = FLAT_VIEW,
        .item_count = COUNT(FLAT_VIEW), .points = { { 40, 32 } },
        .point_count = 1 },
    { .name = "lines", .rounds = 1, .spec = PLAIN, .items = LINES,
        .item_count = COUNT(LINES), .points = { { 16, 31 }, { 33, 31 },
        { 32, 40 } }, .point_count = 3 },
    { .name = "points", .rounds = 1, .spec = PLAIN, .items = POINTS,
        .item_count = COUNT(POINTS), .points = { { 16, 31 }, { 32, 31 },
        { 33, 31 }, { 32, 40 } }, .point_count = 4 },
    { .name = "triangles", .rounds = 1, .spec = PLAIN, .items = TRIANGLES,
        .item_count = COUNT(TRIANGLES), .points = { { 32, 40 } },
        .point_count = 1 },
    { .name = "oddline", .rounds = 1, .spec = PLAIN, .items = ODD_LINE,
        .item_count = COUNT(ODD_LINE), .points = { { 20, 32 }, { 60, 32 } },
        .point_count = 2 },
    { .name = "evenline", .rounds = 1, .spec = PLAIN, .items = EVEN_LINE,
        .item_count = COUNT(EVEN_LINE), .points = { { 20, 32 }, { 60, 32 } },
        .point_count = 2 },
    { .name = "textured", .rounds = 2, .spec = PLAIN, .items = TEXTURED,
        .item_count = COUNT(TEXTURED), .points = { { 16, 16 }, { 48, 16 },
        { 16, 48 }, { 48, 48 } }, .point_count = 4 },
    { .name = "texturedflip", .rounds = 2, .spec = PLAIN,
        .items = TEXTURED_FLIP, .item_count = COUNT(TEXTURED_FLIP),
        .points = { { 16, 16 }, { 48, 16 }, { 16, 48 }, { 48, 48 } },
        .point_count = 4 },
    { .name = "linear", .rounds = 2, .spec = PLAIN, .items = LINEAR,
        .item_count = COUNT(LINEAR), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "nearestmid", .rounds = 2, .spec = PLAIN, .items = NEAREST_MID,
        .item_count = COUNT(NEAREST_MID), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "nearest", .rounds = 2, .spec = PLAIN, .items = NEAREST,
        .item_count = COUNT(NEAREST), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "linearnear", .rounds = 2, .spec = PLAIN, .items = LINEAR_NEAR,
        .item_count = COUNT(LINEAR_NEAR), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "model", .rounds = 2, .spec = PLAIN, .items = MODEL,
        .item_count = COUNT(MODEL), .points = { { 8, 32 }, { 40, 32 } },
        .point_count = 2 },
    { .name = "lambert", .rounds = 2, .spec = PLAIN, .items = LAMBERT,
        .item_count = COUNT(LAMBERT), .points = { { 16, 32 }, { 48, 32 } },
        .point_count = 2 },
    { .name = "lambertflip", .rounds = 2, .spec = PLAIN,
        .items = LAMBERT_FLIP, .item_count = COUNT(LAMBERT_FLIP),
        .points = { { 16, 32 }, { 48, 32 } }, .point_count = 2 },
    { .name = "turned", .rounds = 2, .spec = PLAIN, .items = TURNED,
        .item_count = COUNT(TURNED), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "unturned", .rounds = 2, .spec = PLAIN, .items = UNTURNED,
        .item_count = COUNT(UNTURNED), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "lambertalpha", .rounds = 2, .spec = PLAIN,
        .items = LAMBERT_ALPHA, .item_count = COUNT(LAMBERT_ALPHA),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "lambertopaque", .rounds = 2, .spec = PLAIN,
        .items = LAMBERT_OPAQUE, .item_count = COUNT(LAMBERT_OPAQUE),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "nonormal", .rounds = 2, .spec = PLAIN, .items = NO_NORMAL,
        .item_count = COUNT(NO_NORMAL), .points = { { 16, 32 } },
        .point_count = 1 },
    { .name = "nouv", .rounds = 2, .spec = PLAIN, .items = NO_UV,
        .item_count = COUNT(NO_UV), .points = { { 48, 48 } },
        .point_count = 1 },
    { .name = "draworder", .rounds = 2, .spec = PLAIN, .items = DRAW_ORDER,
        .item_count = COUNT(DRAW_ORDER), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "draworderswap", .rounds = 2, .spec = PLAIN,
        .items = DRAW_ORDER_SWAP, .item_count = COUNT(DRAW_ORDER_SWAP),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "sameorder", .rounds = 2, .spec = PLAIN, .items = SAME_ORDER,
        .item_count = COUNT(SAME_ORDER), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "sameorderswap", .rounds = 2, .spec = PLAIN,
        .items = SAME_ORDER_SWAP, .item_count = COUNT(SAME_ORDER_SWAP),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "extremes", .rounds = 2, .spec = PLAIN, .items = EXTREMES,
        .item_count = COUNT(EXTREMES), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "extremesswap", .rounds = 2, .spec = PLAIN,
        .items = EXTREMES_SWAP, .item_count = COUNT(EXTREMES_SWAP),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "depthtest", .rounds = 2, .spec = PLAIN, .items = DEPTH_TEST,
        .item_count = COUNT(DEPTH_TEST), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "depthnotest", .rounds = 2, .spec = PLAIN,
        .items = DEPTH_NO_TEST, .item_count = COUNT(DEPTH_NO_TEST),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "depthnowrite", .rounds = 2, .spec = PLAIN,
        .items = DEPTH_NO_WRITE, .item_count = COUNT(DEPTH_NO_WRITE),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "otherbits", .rounds = 2, .spec = PLAIN, .items = OTHER_BITS,
        .item_count = COUNT(OTHER_BITS), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "otherbitsoff", .rounds = 2, .spec = PLAIN,
        .items = OTHER_BITS_OFF, .item_count = COUNT(OTHER_BITS_OFF),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "bias", .rounds = 2, .spec = PLAIN, .items = BIAS,
        .item_count = COUNT(BIAS), .points = { { 32, 31 } },
        .point_count = 1 },
    { .name = "nobias", .rounds = 2, .spec = PLAIN, .items = NO_BIAS,
        .item_count = COUNT(NO_BIAS), .points = { { 32, 31 } },
        .point_count = 1 },
    { .name = "thinpoint", .rounds = 1, .spec = PLAIN, .items = THIN_POINT,
        .item_count = COUNT(THIN_POINT), .points = { { 32, 31 }, { 34, 31 },
        { 30, 31 }, { 32, 29 } }, .point_count = 4 },
    { .name = "widepoint", .rounds = 1, .spec = PLAIN, .items = WIDE_POINT,
        .item_count = COUNT(WIDE_POINT), .points = { { 32, 31 }, { 34, 31 },
        { 30, 31 }, { 32, 29 } }, .point_count = 4 },
    { .name = "awaypoint", .rounds = 1, .spec = PLAIN, .items = AWAY_POINT,
        .item_count = COUNT(AWAY_POINT), .points = { { 32, 31 }, { 34, 31 },
        { 30, 31 }, { 32, 29 } }, .point_count = 4 },
    { .name = "wideline", .rounds = 1, .spec = PLAIN, .items = WIDE_LINE,
        .item_count = COUNT(WIDE_LINE), .points = { { 32, 30 }, { 32, 32 } },
        .point_count = 2 },
    { .name = "thinline", .rounds = 1, .spec = PLAIN, .items = THIN_LINE,
        .item_count = COUNT(THIN_LINE), .points = { { 32, 30 }, { 32, 32 } },
        .point_count = 2 },
    { .name = "trim", .rounds = 1, .spec = PLAIN, .items = TRIM,
        .item_count = COUNT(TRIM), .points = { { 16, 32 }, { 48, 32 } },
        .point_count = 2 },
    { .name = "trimaway", .rounds = 1, .spec = PLAIN, .items = TRIM_AWAY,
        .item_count = COUNT(TRIM_AWAY), .points = { { 16, 32 }, { 48, 32 } },
        .point_count = 2 },
    { .name = "batchround", .rounds = 2, .spec = PLAIN, .items = BATCH_ROUND,
        .item_count = COUNT(BATCH_ROUND), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "batchstill", .rounds = 2, .spec = PLAIN, .items = BATCH_STILL,
        .item_count = COUNT(BATCH_STILL), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "batchsides", .rounds = 2, .spec = PLAIN, .items = BATCH_SIDES,
        .item_count = COUNT(BATCH_SIDES), .points = { { 8, 32 }, { 32, 32 },
        { 56, 32 } }, .point_count = 3 },
    { .name = "batchswap", .rounds = 2, .spec = PLAIN, .items = BATCH_SWAP,
        .item_count = COUNT(BATCH_SWAP), .points = { { 8, 32 }, { 32, 32 },
        { 56, 32 } }, .point_count = 3 },
    { .name = "batchorder", .rounds = 1, .spec = PLAIN, .items = BATCH_ORDER,
        .item_count = COUNT(BATCH_ORDER), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "batchorderswap", .rounds = 1, .spec = PLAIN,
        .items = BATCH_ORDER_SWAP, .item_count = COUNT(BATCH_ORDER_SWAP),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "batchdepth", .rounds = 2, .spec = PLAIN, .items = BATCH_DEPTH,
        .item_count = COUNT(BATCH_DEPTH), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "batchdepthoff", .rounds = 2, .spec = PLAIN,
        .items = BATCH_DEPTH_OFF, .item_count = COUNT(BATCH_DEPTH_OFF),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "batchviews", .rounds = 1, .spec = PLAIN, .items = BATCH_VIEWS,
        .item_count = COUNT(BATCH_VIEWS), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "batchviewsone", .rounds = 1, .spec = PLAIN,
        .items = BATCH_VIEWS_ONE, .item_count = COUNT(BATCH_VIEWS_ONE),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "whole", .rounds = 3, .spec = PLAIN, .items = WHOLE_DATA,
        .item_count = COUNT(WHOLE_DATA), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "outside", .rounds = 2, .spec = PLAIN, .items = OUTSIDE_DATA,
        .item_count = COUNT(OUTSIDE_DATA), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "below", .rounds = 2, .spec = PLAIN, .items = BELOW_DATA,
        .item_count = COUNT(BELOW_DATA), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "wrap", .rounds = 2, .spec = PLAIN, .items = WRAP_DATA,
        .item_count = COUNT(WRAP_DATA), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "cut", .rounds = 2, .spec = PLAIN, .items = CUT_DATA,
        .item_count = COUNT(CUT_DATA), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "zerolength", .rounds = 2, .spec = PLAIN, .items = EMPTY_DATA,
        .item_count = COUNT(EMPTY_DATA), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "texwhole", .rounds = 2, .spec = PLAIN, .items = TEXTURE_WHOLE,
        .item_count = COUNT(TEXTURE_WHOLE), .points = { { 16, 16 } },
        .point_count = 1 },
    { .name = "texshort", .rounds = 2, .spec = PLAIN, .items = TEXTURE_SHORT,
        .item_count = COUNT(TEXTURE_SHORT), .points = { { 16, 16 } },
        .point_count = 1 },
    { .name = "nomesh", .rounds = 2, .spec = PLAIN, .items = NO_MESH,
        .item_count = COUNT(NO_MESH), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "pastmesh", .rounds = 2, .spec = PLAIN, .items = PAST_MESH,
        .item_count = COUNT(PAST_MESH), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "deadmesh", .rounds = 3, .spec = PLAIN, .items = DEAD_MESH,
        .item_count = COUNT(DEAD_MESH), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "primitive3", .rounds = 2, .spec = PLAIN, .items = PRIMITIVE_3,
        .item_count = COUNT(PRIMITIVE_3), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "layout8", .rounds = 2, .spec = PLAIN, .items = LAYOUT_8,
        .item_count = COUNT(LAYOUT_8), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "shading3", .rounds = 2, .spec = PLAIN, .items = SHADING_3,
        .item_count = COUNT(SHADING_3), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "filter2", .rounds = 2, .spec = PLAIN, .items = FILTER_2,
        .item_count = COUNT(FILTER_2), .points = { { 16, 16 } },
        .point_count = 1 },
    { .name = "widetexture", .rounds = 2, .spec = PLAIN, .small = true,
        .items = WIDE_TEXTURE, .item_count = COUNT(WIDE_TEXTURE),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "fittexture", .rounds = 2, .spec = PLAIN, .small = true,
        .items = FIT_TEXTURE, .item_count = COUNT(FIT_TEXTURE),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "bigmesh", .rounds = 2, .spec = PLAIN, .small = true,
        .items = BIG_MESH, .item_count = COUNT(BIG_MESH),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "smallmesh", .rounds = 2, .spec = PLAIN, .small = true,
        .items = SMALL_MESH, .item_count = COUNT(SMALL_MESH),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "batchcut", .rounds = 1, .spec = PLAIN, .items = BATCH_CUT,
        .item_count = COUNT(BATCH_CUT), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "batchwhole", .rounds = 1, .spec = PLAIN, .items = BATCH_WHOLE,
        .item_count = COUNT(BATCH_WHOLE), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "batchlayout8", .rounds = 1, .spec = PLAIN,
        .items = BATCH_LAYOUT_8, .item_count = COUNT(BATCH_LAYOUT_8) },
    { .name = "batchprimitive3", .rounds = 1, .spec = PLAIN,
        .items = BATCH_PRIMITIVE_3, .item_count = COUNT(BATCH_PRIMITIVE_3) },
    { .name = "retry", .rounds = 3, .spec = PLAIN, .items = RETRY,
        .item_count = COUNT(RETRY), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "retrynever", .rounds = 3, .spec = PLAIN, .items = RETRY_NEVER,
        .item_count = COUNT(RETRY_NEVER), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "massleave", .rounds = 3, .spec = PLAIN, .items = MASS_LEAVE,
        .item_count = COUNT(MASS_LEAVE) },
    { .name = "drawpoint", .rounds = 2, .spec = PLAIN, .items = DRAW_POINT,
        .item_count = COUNT(DRAW_POINT), .points = { { 32, 31 }, { 34, 31 } },
        .point_count = 2 },
    { .name = "drawpointthin", .rounds = 2, .spec = PLAIN,
        .items = DRAW_POINT_THIN, .item_count = COUNT(DRAW_POINT_THIN),
        .points = { { 32, 31 }, { 34, 31 } }, .point_count = 2 },
    { .name = "pastbottom", .rounds = 2, .spec = PLAIN, .items = PAST_BOTTOM,
        .item_count = COUNT(PAST_BOTTOM), .points = { { 32, 16 }, { 32, 40 } },
        .point_count = 2 },
    { .name = "insidetop", .rounds = 2, .spec = PLAIN, .items = INSIDE_TOP,
        .item_count = COUNT(INSIDE_TOP), .points = { { 32, 16 }, { 32, 40 } },
        .point_count = 2 },
    { .name = "unmoved", .rounds = 2, .spec = PLAIN, .items = HALF_LEFT,
        .item_count = COUNT(HALF_LEFT), .points = { { 8, 32 }, { 40, 32 } },
        .point_count = 2 },
    { .name = "bent", .rounds = 2, .spec = PLAIN, .items = BENT,
        .item_count = COUNT(BENT), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "srgb", .rounds = 2, .spec = SRGB, .items = GRAY,
        .item_count = COUNT(GRAY), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "unormgray", .rounds = 2, .spec = PLAIN, .items = GRAY,
        .item_count = COUNT(GRAY), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "graytexel", .rounds = 2, .spec = PLAIN, .items = GRAY_TEXEL,
        .item_count = COUNT(GRAY_TEXEL), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "twobad", .rounds = 2, .spec = PLAIN, .items = TWO_BAD,
        .item_count = COUNT(TWO_BAD), .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "corner", .rounds = 1, .spec = PLAIN, .items = CORNER_LINE,
        .item_count = COUNT(CORNER_LINE), .points = { { 24, 40 }, { 42, 42 } },
        .point_count = 2 },
    { .name = "cornerpast", .rounds = 1, .spec = PLAIN,
        .items = CORNER_PAST_LINE, .item_count = COUNT(CORNER_PAST_LINE),
        .points = { { 24, 40 }, { 42, 42 } }, .point_count = 2 },
    { .name = "noview", .rounds = 1, .spec = PLAIN, .points = { { 32, 32 } },
        .point_count = 1 },
    { .name = "viewafterdraw", .rounds = 1, .spec = PLAIN,
        .items = VIEW_AFTER_DRAW, .item_count = COUNT(VIEW_AFTER_DRAW),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "texdetachwait", .rounds = 6, .spec = PLAIN,
        .items = TEXTURE_DETACH_WAIT,
        .item_count = COUNT(TEXTURE_DETACH_WAIT) },
    { .name = "lostagain", .rounds = 5, .spec = PLAIN, .counts = true,
        .items = SOLID, .item_count = COUNT(SOLID),
        .points = { { 32, 32 } }, .point_count = 1 },
    { .name = "lostwait", .rounds = 6, .spec = PLAIN,
        .items = DETACH_WAIT, .item_count = COUNT(DETACH_WAIT) }
};

static const step_t STEPS[] = {
    { "reload", 1, RELOAD, 0 },
    { "noframe", 3, GONE, 0 },
    { "noframe", 4, BACK, 0 },
    { "nodevice", 3, STATE, WEBGPU_FRAME_NO_DEVICE },
    { "nodevice", 4, STATE, WEBGPU_FRAME_READY },
    { "skipped", 3, STATE, WEBGPU_FRAME_SKIPPED },
    { "skipped", 4, STATE, WEBGPU_FRAME_READY },
    { "first", 1, STATE, WEBGPU_FRAME_SKIPPED },
    { "first", 2, STATE, WEBGPU_FRAME_READY },
    { "newdevice", 3, DEVICE, 0 },
    { "newformat", 3, FORMAT, WGPUTextureFormat_BGRA8Unorm },
    { "newsamples", 3, SAMPLES, 4 },
    { "newdepth", 3, DEPTH, WGPUTextureFormat_Depth32Float },
    { "blendnext", 2, BLEND_TO, 0 },
    { "blendnext", 4, DEVICE, 0 },
    { "comparenext", 2, COMPARE_TO, WEBGPU_COMPARE_GREATER },
    { "comparenext", 4, DEVICE, 0 },
    { "cullnext", 2, CULL_TO, WEBGPU_CULL_FRONT },
    { "cullnext", 4, DEVICE, 0 },
    { "clockwisenext", 2, CLOCKWISE_TO, 1 },
    { "clockwisenext", 4, DEVICE, 0 },
    { "repeatnext", 2, REPEAT_TO, 0 },
    { "repeatnext", 4, DEVICE, 0 },
    { "loadnext", 2, LOAD_TO, WEBGPU_LOAD_KEEP },
    { "cleardepthnext", 2, CLEAR_DEPTH_TO, 250 },
    { "memorynext", 2, GPU_MEMORY_TO, 100 },
    { "errors", 1, LIE, WGPUTextureFormat_RGBA8Unorm },
    { "lostdestroy", 3, STATE, WEBGPU_FRAME_NO_DEVICE },
    { "lostdestroy", 4, STATE, WEBGPU_FRAME_READY },
    { "lostagain", 3, STATE, WEBGPU_FRAME_NO_DEVICE },
    { "lostagain", 5, STATE, WEBGPU_FRAME_READY },
    { "lostwait", 5, STATE, WEBGPU_FRAME_NO_DEVICE }
};

const scene_t *probe_find_scene(const char *name)
{
    for (uint32_t i = 0; i < COUNT(SCENES); i++)
        if (!strcmp(SCENES[i].name, name))
            return (&SCENES[i]);
    return (nullptr);
}

const step_t *probe_get_steps(uint32_t *count)
{
    *count = COUNT(STEPS);
    return (STEPS);
}
