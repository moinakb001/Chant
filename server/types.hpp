template <unsigned long long x>
using u = unsigned _BitInt(x);

template <unsigned long long x>
using s = signed _BitInt(x);

#define unwrap(a, b) using a ## b = a<b>

unwrap(u, 8);
unwrap(u, 16);
unwrap(u, 32);
unwrap(u, 64);
unwrap(u, 128);

unwrap(s, 8);
unwrap(s, 16);
unwrap(s, 32);
unwrap(s, 64);
unwrap(s, 128);

