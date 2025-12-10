# Metalium Implementation of Matrix Preferred Divison

Toy example for learning Metalium. Performs the following:

```py
copy cb0, cb1 onto device

allocate tile of 1.0f in dst[0]
dst[0] = dst[0] / cb0 { vector unit }   [ manual reciprocal for 1 / a ]
dst[2] = cb1 * dst[0] { matrix unit }   [ times reciprocal by b ]

return dst[2] to host                   [ c = b / a ]
```
