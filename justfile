alias br := build_run

build_run profile="release":
  @just build {{profile}}
  @just run {{profile}}

cmake:
  -[ -d build ] && rm -r build
  cmake --preset=debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --preset=release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cp build/debug/compile_commands.json compile_commands.json

build profile="release":
  @if [ {{profile}} != "debug" ] && [ {{profile}} != "release" ]; then echo "Invalid profile"; exit 1; fi
  ninja -C "build/{{profile}}"

run profile="release":
  @if [ {{profile}} != "debug" ] && [ {{profile}} != "release" ]; then echo "Invalid profile"; exit 1; fi
  ./build/{{profile}}/RayTracer/RayTracer.exe
