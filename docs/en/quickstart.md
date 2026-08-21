# PhysFX Quickstart

PhysFX is an Apache-2.0 video world editor. The local engine is complete and remains available
when cloud or billing features are disabled.

```text
cmake -S . -B build/off -DPHYSFX_BUILD_TESTS=ON
cmake --build build/off
build/off/apps/cli/physfx --version
```

Use `examples/empty_pipeline.json` for a dependency-free smoke run. FFmpeg, ONNX Runtime and
Taichi are optional and report their actual fallback backend. SAM, XMem, ProPainter and Depth
Anything are permanent documented limitations, not silently substituted neural models.
