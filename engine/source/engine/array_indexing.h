#ifndef H_BUFFER
#define H_BUFFER

// NOTE(hugo): converts 2D coordinates into an index in a 1D array
//             row major    i2D(x, y, width)
//             column major i2D(y, x, height)
uint index2D(uint major_axis_coord, uint minor_axis_coord, uint major_axis_size);

// NOTE(hugo): converts a 1D array index into 2D coordinates
//             row major    c2D(index, width, x, y)
//             column major c2D(index, height, y, x)
void coord2D(uint index, uint major_axis_size, uint& major_axis_coord, uint& minor_axis_coord);

// NOTE(hugo): out_neighbor must have a size of 4
//             row major    neigh4_i2D(index, width, width * height, data);
//             column major neigh4_i2D(index, height, width * height, data);
uint neigh4_index2D(uint index, uint major_axis_size, uint max_index, uint* out_neighbor);

// NOTE(hugo): out_neighbor must have a size of 2 * 4 = 8
//             row major    neigh4_c2D(x, y, width, height, data);
//             column major neigh4_c2D(y, x, height, width, data);
uint neigh4_coord2D(uint major_axis_coord, uint minor_axis_coord, uint major_axis_size, uint minor_axis_size, uint* out_neighbor);

#endif
