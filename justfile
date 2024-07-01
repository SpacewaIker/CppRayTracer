alias br := build_run

build_run profile="debug":
  @just build {{profile}}
  @just run {{profile}}

cmake:
  -[ -d build ] && rm -r build
  cmake --preset=debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --preset=release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build profile="debug":
  @if [ {{profile}} != "debug" ] && [ {{profile}} != "release" ]; then echo "Invalid profile"; exit 1; fi
  ninja -C "build/{{profile}}"

run profile="debug":
  @if [ {{profile}} != "debug" ] && [ {{profile}} != "release" ]; then echo "Invalid profile"; exit 1; fi
  ./build/{{profile}}/RayTracer/RayTracer.exe > /dev/null 2>&1
