#ifndef H_LOG_CUSTOM
#define H_LOG_CUSTOM

// NOTE(hugo): using macros to keep __FILE__ and __LINE__ at the LOG location and not in the function

#define LOG_vec3(variable) LOG_TRACE("vec3(x, y, z) = (%f, %f, %f)", variable.x, variable.y, variable.z)
#define LOG_vec4(variable) LOG_TRACE("vec4(x, y, z, w) = (%f, %f, %f %f)", variable.x, variable.y, variable.z, variable.w)

#define LOG_mat2(variable)                                  \
do{                                                         \
    LOG_TRACE("mat2 = %f %f", variable.data[0], variable.data[2]);    \
    LOG_TRACE("       %f %f", variable.data[1], variable.data[3]);    \
}while(0)

#define LOG_mat3(variable)                                                  \
do{                                                                         \
    LOG_TRACE("mat3 = %f %f %f", variable.data[0], variable.data[3], variable.data[6]);    \
    LOG_TRACE("       %f %f %f", variable.data[1], variable.data[4], variable.data[7]);    \
    LOG_TRACE("       %f %f %f", variable.data[2], variable.data[5], variable.data[8]);    \
}while(0)

#define LOG_mat4(variable)                                                                  \
do{                                                                                         \
    LOG_TRACE("mat4 = %f %f %f %f", variable.data[0], variable.data[4], variable.data[8],  variable.data[12]);  \
    LOG_TRACE("       %f %f %f %f", variable.data[1], variable.data[5], variable.data[9],  variable.data[13]);  \
    LOG_TRACE("       %f %f %f %f", variable.data[2], variable.data[6], variable.data[10], variable.data[14]);  \
    LOG_TRACE("       %f %f %f %f", variable.data[3], variable.data[7], variable.data[11], variable.data[15]);  \
}while(0)

#define LOG_quat(variable) LOG_TRACE("quat(s, i, j, k) = (%f %f %f %f)", variable.s, variable.i, variable.j, variable.k)

#define LOG_Mesh_2D_position(variable)                                                                      \
do{                                                                                                         \
    LOG_TRACE("struct Mesh : nvertices %d", variable.nvertices);                                            \
    for(u32 ivertex = 0; ivertex != variable.nvertices; ++ivertex){                                         \
        LOG_TRACE(" vertices[%d] = %f %f", mesh.vertices[2 * ivertex], mesh.vertices[2 * ivertex + 1]);     \
    }                                                                                                       \
}while(0)

#define LOG_inotify_event(variable_ptr)                                             \
do{                                                                                 \
    LOG_TRACE("struct inotify_event {");                                            \
    LOG_TRACE("    wd      = %d", variable_ptr->wd);                                \
    if(variable_ptr->mask & IN_ACCESS) LOG_TRACE("    IN_ACCESS");                  \
    if(variable_ptr->mask & IN_ATTRIB) LOG_TRACE("    IN_ATTRIB");                  \
    if(variable_ptr->mask & IN_CLOSE_WRITE) LOG_TRACE("    IN_CLOSE_WRITE");        \
    if(variable_ptr->mask & IN_CLOSE_NOWRITE) LOG_TRACE("    IN_CLOSE_NOWRITE");    \
    if(variable_ptr->mask & IN_CREATE) LOG_TRACE("    IN_CREATE");                  \
    if(variable_ptr->mask & IN_DELETE) LOG_TRACE("    IN_DELETE");                  \
    if(variable_ptr->mask & IN_DELETE_SELF) LOG_TRACE("    IN_DELETE_SELF");        \
    if(variable_ptr->mask & IN_MODIFY) LOG_TRACE("    IN_MODIFY");                  \
    if(variable_ptr->mask & IN_MOVE_SELF) LOG_TRACE("    IN_MOVE_SELF");            \
    if(variable_ptr->mask & IN_MOVED_FROM) LOG_TRACE("    IN_MOVED_FROM");          \
    if(variable_ptr->mask & IN_MOVED_TO) LOG_TRACE("    IN_MOVED_TO");              \
    if(variable_ptr->mask & IN_OPEN) LOG_TRACE("    IN_OPEN");                      \
    if(variable_ptr->mask & IN_IGNORED) LOG_TRACE("    IN_IGNORED");                \
    if(variable_ptr->mask & IN_ISDIR) LOG_TRACE("    IN_ISDIR");                    \
    if(variable_ptr->mask & IN_Q_OVERFLOW) LOG_TRACE("    IN_Q_OVERFLOW");          \
    if(variable_ptr->mask & IN_UNMOUNT) LOG_TRACE("    IN_UNMOUNT");                \
    LOG_TRACE("    cookie  = %d", variable_ptr->cookie);                            \
    LOG_TRACE("    len     = %d", variable_ptr->len);                               \
    LOG_TRACE("    name    = %s", variable_ptr->name);                              \
    LOG_TRACE("}");                                                                 \
}while(0)

#define LOG_sarray(variable)                                \
do{                                                         \
    LOG_TRACE("struct sarray {");                           \
    LOG_TRACE("    size     = %d", variable.size);          \
    LOG_TRACE("    capacity = %d", variable.capacity);      \
    for(u32 i = 0; i != variable.size; ++i){                \
        LOG_TRACE("    data[%d] = %d", i, variable[i]);     \
    }                                                       \
    LOG_TRACE("}");                                         \
}while(0)

#define LOG_daryheap(heap)                                                              \
do{                                                                                     \
    LOG_TRACE("struct heap {");                                                         \
    LOG_TRACE("    size     = %d", heap.size);                                          \
    LOG_TRACE("    capacity = %d", heap.capacity);                                      \
    u32 heap_level_current = 0u;                                                        \
    u32 index_current = 0u;                                                             \
    while(index_current != heap->size){                                                 \
        u32 index_end = min(index_current + (1u << heap_level_current++), heap->size);  \
        printf("    LEVEL %d: ", heap_level_current);                                   \
        for(u32 index = index_current; index != index_end; ++index){                    \
            printf("%d ", heap->data[index]);                                           \
        }                                                                               \
        index_current = index_end;                                                      \
        printf("\n");                                                                   \
    }                                                                                   \
    LOG_TRACE("}");                                                                     \
}while(0)

#endif
