# Conan build support

This branch adds an optional Conan path for pulling in Boost, alongside the
existing tarball/system-Boost path.

- `conanfile.py` — libossia package descriptor
- `cmake/OssiaDeps.cmake` — defines `OSSIA_USE_CONAN` (default `True`)
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
