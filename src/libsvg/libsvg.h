#ifndef LIBSVG_LIBSVG_H
#define LIBSVG_LIBSVG_H

#include "path.h"
#include "rect.h"
#include "shape.h"
#include "svgpage.h"

namespace libsvg {

typedef std::vector<shape *> shapes_list_t;

shapes_list_t *libsvg_read_file(const char *filename);

void libsvg_free(shapes_list_t *shapes);

}  // namespace libsvg

#endif /* LIBSVG_LIBSVG_H */
