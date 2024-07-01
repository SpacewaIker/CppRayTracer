cmake:
  -[ -d build ] && rm -r build
  mkdir build
  cd build && cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build:
  ninja -C build

run:
  ./build/RayTracer/RayTracer.exe > /dev/null 2>&1
