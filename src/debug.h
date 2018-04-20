#ifndef DM510_DEBUG_H
#define DM510_DEBUG_H
#ifdef DEBUG
# define DEBUG_CODE(code) do {code} while (0);
#else
# define DEBUG_CODE(code) do {} while (0)
#endif
# define dprintf(...) DEBUG_CODE(fprintf(stderr, __VA_ARGS__ );)
#endif /* end of include guard: DM510_DEBUG_H */
