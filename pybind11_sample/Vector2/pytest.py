from pybindings import Vector2

vec = Vector2(4.0, 5.0)
vec2 = Vector2(5.5, 6.5)

print vec, vec2

print vec + vec2
vec += vec2
print vec

print 2 * vec
vec *= 3
print vec

