# Conan build support

This branch adds an optional Conan path for pulling in Boost, alongside the
existing tarball/system-Boost path.

- `conanfile.py` — libossia package descriptor
- `cmake/OssiaDeps.cmake` — defines `OSSIA_USE_CONAN` (default `False`, so a
  plain cmake build never needs conan installed)
- `conanfile.py` — passes `-DOSSIA_USE_CONAN=True`, so the conan path is used
  exactly when libossia is built as a conan package
- `cmake/deps/boost.cmake` — bootstraps cmake-conan and resolves Boost via
  Conan; falls back to the tarball download when `OSSIA_USE_CONAN` is `OFF`

## This is Conan 1.x only

The setup will **not** work on Conan 2. Every API in use is 1.x:

| Location | 1.x API | Conan 2 equivalent |
| --- | --- | --- |
| `conanfile.py` | `from conans import ...` | `from conan import ...` (the `conans` package no longer exists — hard import error) |
| `conanfile.py` | `generators = "cmake"` | `CMakeToolchain` + `CMakeDeps` |
| `conanfile.py` | `cmake.definitions[...]` | `tc.variables[...]` |
| `conanfile.py` | `self.copy(...)` | `conan.tools.files.copy` |
| `conanfile.py` | `tools.Git`, `tools.collect_libs` | legacy `tools` module removed |
| `conanfile.py` | *(no `layout()`)* | `layout()` effectively required |
| `boost.cmake` | `conan_check(VERSION 1.29.0 REQUIRED)` | rejects 2.x outright |
| `boost.cmake` | `GENERATORS cmake_find_package` | `CMakeDeps` |
| `boost.cmake` | `conan_cmake_configure` / `conan_cmake_install` | dependency provider (`conan_provider.cmake` via `CMAKE_PROJECT_TOP_LEVEL_INCLUDES`) |
| `boost.cmake` | `boost:shared=False` | `boost/*:shared=False` |

Porting to Conan 2 means rewriting both files, not tweaking them.

## Why Boost is pinned to 1.86

`BOOST_MINOR_CONAN` is `86` because **1.86.0 is the newest Boost recipe
available on the Conan 1.x ConanCenter remote.** Verified against the live
remotes:

```
center.conan.io   (Conan 1)   boost/1.85.0 200   1.86.0 200   1.87.0 404   1.88.0 404   1.90.0 404
center2.conan.io  (Conan 2)   ... 1.87.0, 1.88.0, 1.89.0, 1.90.0, 1.91.0 all present
```

ConanCenter froze Conan-1-compatible recipes; 1.86.0 (Aug 2024) was the last
Boost published there. Everything newer is Conan 2 only. There is no way to get
Boost > 1.86 through this path without migrating to Conan 2.

Only the Conan path is constrained. `BOOST_MINOR_LATEST` stays at `90` for the
tarball/system-Boost path — the two were deliberately decoupled so the pin
doesn't hold back non-Conan builds.

## `BOOST_MINOR_MINIMAL` is 86, upstream is 87

This is a deliberate divergence from `feature/remove-websocketpp`.

The 87 floor was introduced by two upstream commits, both of which also bumped
the `3rdparty/websocketpp` submodule:

- `4923aad87` "websocket: final touches for boost 1.87 support" → `LATEST 87`
- `7e9b16e88` "ci: further fixes for boost 1.87 support" → `MINIMAL 87`

That work was entirely about **removing Asio APIs that Boost 1.87 deleted**:

| Removed in 1.87 | Replacement used | Available since |
| --- | --- | --- |
| `asio::io_service` | `asio::io_context` | 1.66 |
| `asio::io_service::work` | `asio::executor_work_guard<...>` | 1.66 |
| `sock.get_io_service().post(f)` | `asio::post(ex, f)` | 1.66 |

Every replacement predates 1.86 by roughly twenty releases, so the resulting
code compiles on 1.86 *and* 1.87+. The floor was housekeeping, and the component
that actually strained against 1.87 — websocketpp — is deleted by this branch.

The tree is clean of all four removed APIs. (`network/resolve.cpp:156` has a
local variable *named* `io_service`; its type is `io_context`.)

## Verification

Every Beast/WebSocket file on this branch was compiled `-fsyntax-only` against
the real Boost 1.86 headers:

```
PASS  network/sockets/websocket_client_beast.cpp
PASS  network/sockets/websocket_server_beast.cpp
PASS  network/sockets/websocket_client_interface.cpp
PASS  network/sockets/websocket_server_interface.cpp
PASS  network/sockets/websocket_factory.cpp
PASS  network/sockets/websocket_header_client.hpp
PASS  network/http/http_client.hpp, http_client_request.hpp
PASS  network/oscquery/oscquery_server.cpp, oscquery_mirror.cpp
PASS  protocols/oscquery/oscquery_server_asio.cpp
PASS  protocols/oscquery/oscquery_mirror_asio.cpp, oscquery_mirror_asio_dense.cpp
PASS  protocols/socketio/socketio_client.cpp, socketio_server.cpp, boost_json_impl.cpp
```

Controls: `static_assert(BOOST_VERSION == 108600)`, `clang++ -H` confirming
`boost/beast/websocket.hpp` resolved to the 1.86 tree, and no system/homebrew
Boost present that could shadow it.

Newer-Boost features the code relies on all landed before 1.86:

| Feature | Since |
| --- | --- |
| `asio::as_tuple` (promoted out of `experimental`) | 1.84 |
| `unordered::concurrent_flat_map` | 1.83 |
| `asio::cancellation_signal` | 1.77 |
| Boost.JSON | 1.75 |
| coroutines / `co_spawn` / `awaitable` | 1.70 |
| Beast `tcp_stream`, `get_lowest_layer`, `stream_base::timeout` | 1.70 |

**Not covered:** this was syntax-only on macOS/clang. It does not cover
link-time symbols, MSVC/gcc, or runtime behavior, and
`websocket_client_emscripten.cpp` was not tested (needs the emsdk toolchain).
CI on 1.86 would close those gaps.

## Boost.MQTT5

`boost/mqtt5.hpp` is **absent** from the Conan 1.86 Boost package, even though
Boost.MQTT5 shipped in upstream 1.86. This is handled — `mqtt_protocol.cpp`
guards it:

```cpp
#if __has_include(<boost/mqtt5.hpp>)
  #include <boost/mqtt5.hpp>
#else
  #include <async_mqtt5.hpp>   // 3rdparty/async-mqtt5 submodule
#endif
```

`cmake/deps/mqtt.cmake` wires the vendored submodule by default regardless, so
the fallback path is the normal one. No action needed.

## Gotchas

**cmake-conan is downloaded unpinned.** `boost.cmake` fetches `conan.cmake` from
the `master` branch of `conan-io/cmake-conan` at configure time. That still
serves the v1-style file today (`conan_check`, `conan_cmake_configure`,
`conan_cmake_install` all present), but `master` is where Conan 2 provider work
lands. If it is ever retargeted, configure breaks with no version to fall back
on. Worth pinning the URL to a tag such as `0.18.1`.

**Conan 1 is end-of-life** and can be awkward to install on modern Python. A
pinned venv is the safest route:

```sh
python3 -m venv ~/.venvs/conan1
~/.venvs/conan1/bin/pip install 'conan<2'
```

On Apple Silicon, watch for a Conan installed under an x86_64 Python — its
native `charset_normalizer` module will fail to load with an "incompatible
architecture" error. Reinstall under an arm64 Python, or run the build under
Rosetta.

**`git submodule sync` after rebasing.** Upstream changed the
`3rdparty/asio` submodule URL (now `jcelerier/asio-2`). A stale local override
in `.git/config` causes `Fetched in submodule path '3rdparty/asio', but it did
not contain <sha>`. Run `git submodule sync --recursive` after any rebase that
touches `.gitmodules`.

**`3rdparty/asio` is the Steinberg ASIO audio SDK, not Boost.Asio.** The name
collision is a trap. `jcelerier/asio-2` is a redistribution mirror of
Steinberg's SDK (used by the Windows ASIO audio backend via
`cmake/deps/asio-sdk.cmake`), *not* a fork of chriskohlhoff/asio. Do not
"fix" the submodule URL to point at chriskohlhoff/asio, and ignore
ConanCenter's `asio` package — it is the unrelated networking library. Boost.Asio
comes from Boost, as usual.

---

# Conan 2 migration

Supporting Conan 2 is viable, and cross-compilation would end up better
supported than the current setup. The work is structural rather than large.

## Boost is consumed header-only

This shapes everything below. `src/ossia_setup.cmake` sets `BOOST_ALL_NO_LIB=1`,
`cmake/deps/boost.cmake` only ever sets `INTERFACE_INCLUDE_DIRECTORIES`, and
nothing links `Boost::system` / `thread` / `filesystem` / etc. Conan is being
used purely as a header downloader for Boost.

ConanCenter's boost recipe has a `header_only` option whose `package_id()` calls
`self.info.clear()` — one package, with no arch/OS/compiler in the id. So a
header-only Boost package is consumed identically by macOS, iOS, Android and
wasm builds. **Cross-compiling Boost is a non-problem.**

This is already applied on the Conan 1 path — `cmake/deps/boost.cmake` passes
`boost:header_only=True` and nothing else. Verified with Conan 1.61:

```
boost/1.86.0:5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9 - Download   # prebuilt, nothing compiled
```

Re-resolving the same reference with `-s os=iOS -s os.version=14.0 -s arch=armv8`
yields the *identical* package id and reports `Already installed!`, confirming
the package is architecture-independent in practice and not just on paper. The
generated `FindBoost.cmake` sets both `Boost_INCLUDE_DIRS` and the singular
`Boost_INCLUDE_DIR` that `boost.cmake` consumes, and ossia's Beast/WebSocket
sources compile against the resulting headers.

## Cross-compilation with our toolchain files

Conan 2 supports user-supplied CMake toolchain files:

```ini
[conf]
tools.cmake.cmaketoolchain:user_toolchain=["3rdparty/ios-cmake/ios.toolchain.cmake"]
```

Conan emits `include(...)` for each of these as the *first* lines of the
generated `conan_toolchain.cmake`, and variables the user file defines
(`CMAKE_SYSTEM_NAME`, `CMAKE_SYSTEM_PROCESSOR`, ...) are respected rather than
overwritten. `tools.cmake.cmaketoolchain:enabled_blocks` narrows what Conan
emits if the two fight.

**The structural catch:** that flow requires `conan install` to run *first*,
producing `conan_toolchain.cmake`, which then becomes `CMAKE_TOOLCHAIN_FILE`.
The current setup calls Conan from *inside* CMake configure
(`cmake/deps/boost.cmake`), after the toolchain has already been processed.
That ordering is backwards and cannot be patched in place — a Conan 2 port
means moving Conan invocation outside of CMake.

cmake-conan's `conan_provider.cmake` is not a way around this. Its own docs
state that autodetection "only supports the main compilers of the mainstream
platforms" (Windows+MSVC, Linux+gcc, Apple+clang); anything else needs an
explicit profile and a separate `conan install`.

The concrete case to design against is the iOS job in
`.github/workflows/ossia-unity.yml`:

```sh
-DCMAKE_TOOLCHAIN_FILE=${GITHUB_WORKSPACE}/3rdparty/ios-cmake/ios.toolchain.cmake
-DPLATFORM=OS64 -DENABLE_BITCODE=1
```

Note this only works with the current Conan path by accident: `PROFILE_HOST`
resolves to `default` (macOS), and it gets away with it because Boost headers
are architecture-independent.

## Boost can be upgraded

| Remote | Newest Boost |
| --- | --- |
| `center.conan.io` (Conan 1) | 1.86.0 |
| `center2.conan.io` (Conan 2) | **1.91.0** |

Conan 2 removes the cause of the 1.86 pin entirely; `BOOST_MINOR_CONAN` could
then track `BOOST_MINOR_LATEST`.

## Other dependencies on ConanCenter

Queried live against `center2.conan.io`:

| Available | Version | | Not available |
| --- | --- | --- | --- |
| boost | 1.91.0 | | kfr |
| fmt | 12.2.0 *(matches our submodule)* | | libremidi |
| spdlog | 1.17.0 | | oscpack |
| re2 | 20251105 *(matches upstream's bump)* | | verdigris |
| abseil | 20260526.0 | | whereami |
| catch2 | 3.15.3 | | rubberband |
| magic_enum / ctre / mdspan | 0.9.8 / 3.11.0 / 0.6.0 | | tcb-span, dno, rnd, Flicks |
| concurrentqueue / readerwriterqueue | 1.0.5 / 1.0.6 | | SmallFunction, tuplet |
| unordered_dense / rapidfuzz / rapidjson | 4.9.2 / 3.3.3 / 1.1.0 | | nano-signal-slot, Servus |
| portaudio / libsamplerate / sdl / fftw | 19.7 / 0.2.2 / 3.4.14 / 3.3.10 | | wiiuse, libartnet, rapidhash |
| pybind11 / exprtk / libcoap / miniaudio | 3.0.1 / 0.0.3 / 4.3.3 / 0.11.22 | | async-mqtt5 |

Roughly 18 of our ~44 submodules have ConanCenter equivalents, so Conan can
*supplement* the submodule set but never replace it. Every dual-sourced
dependency then needs both paths kept in version sync.

Ignore ConanCenter's `asio` package — see the note on the Steinberg ASIO SDK
below. Pure name collision.

## Arguments against a large push

**Upstream is moving toward vendoring, not away from it.** Recent commits
`68b193ef8` (vendor Abseil, update re2), `3327acdae`, `c9934ac61` and
`180d6f61d` all go the other direction. A proposal making Conan primary is
unlikely to land upstream; Conan as an opt-in path is a far easier sell.

**~~This branch currently breaks CI, independent of the Conan version.~~**
*Fixed.* `OSSIA_USE_CONAN` used to default to `True` while
`cmake/deps/boost.cmake` does a hard `conan_check(VERSION 1.29.0 REQUIRED)`
and no workflow installs conan — so every job, including iOS, would have
failed at configure. The default is now `False`, and `conanfile.py` passes
`-DOSSIA_USE_CONAN=True` so the conan path is taken exactly when libossia is
built as a conan package.

## Suggested plan

1. ~~**Default `OSSIA_USE_CONAN=OFF`.**~~ *Done* — the cmake default is `False`
   and `conanfile.py` turns it on. Fixes CI and makes the feature opt-in,
   which is also what makes it upstreamable.
2. **Move Conan out of CMake:** `conan install` → `conan_toolchain.cmake` →
   `cmake -DCMAKE_TOOLCHAIN_FILE=...`. Required for cross-compilation, and it
   also removes the unpinned `cmake-conan/master` download.
3. ~~**Use `boost/*:header_only=True`.**~~ *Already done on the Conan 1 path*;
   carry it over with the Conan 2 option syntax (`boost/*:header_only=True`).
4. **Add an iOS profile** with `user_toolchain` pointing at
   `ios.toolchain.cmake`; validate against the existing unity job.
5. **Do not dual-source the other 18 dependencies** initially. Boost alone
   justifies the work; the rest is sync burden for little gain.

`conanfile.py` is ~100 lines and the CMake side ~40, so the rewrite is modest.
The real work is steps 2 and 4.

## References

- [CMakeToolchain reference](https://docs.conan.io/2/reference/tools/cmake/cmaketoolchain.html)
- [Cross-building with Conan](https://docs.conan.io/2/tutorial/consuming_packages/cross_building_with_conan.html)
- [cmake-conan](https://github.com/conan-io/cmake-conan)
- [ConanCenter boost recipe](https://github.com/conan-io/conan-center-index/blob/master/recipes/boost/all/conanfile.py)
