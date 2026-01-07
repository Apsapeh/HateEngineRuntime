#include "node3d.h"
#include <core/math/mat4.h>
#include <user/object/node/node.h>
#include <user/object/object_types.h>
#include <core/platform/memory.h>

void node3d_init(Node3D* this, enum ObjectType object_type, const char* name) {
    node_init(&this->node, object_type, name);
    this->transform = MAT4_ONE_M;
}

void node3d_free(Node3D* node3d) {
    node_free(&node3d->node);
}


Node3D* node3d_new(const char* name) {
    Node3D* node3d = tmalloc(sizeof(Node3D));
    node3d_init(node3d, ObjectTypeNode3D, name);
    return node3d;
}
