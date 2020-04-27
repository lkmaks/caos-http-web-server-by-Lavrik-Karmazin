#ifndef CAOS_HTTP_WEB_SERVER_DEBUG_H
#define CAOS_HTTP_WEB_SERVER_DEBUG_H

#include <iostream>

template <typename T>
void deb(T s) {
#ifdef LOCAL
  std::cout << s << std::endl;
#endif
}

template <typename T1, typename T2>
void deb(T1 s, T2 t) {
#ifdef LOCAL
  std::cout << s << t << std::endl;
#endif
}

#endif //CAOS_HTTP_WEB_SERVER_DEBUG_H
