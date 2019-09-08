# pycinot

This is a simple Python package to the Linux kernel inotify API, written in C.

It's been developed for Python 3 and tested with Python 3.6.

### Status

This package is absolutely not finished.

It lacks comments, documentations, examples, documentation, and a distutil file to build and install it. It could also probably benefit from some code improvements.

The core code is functionnal. Future work may extend, improve or modify the API.

### Build and install

#### Building the package

Simply run make.

```sh
make
```

Or if you want some useless debug information, you can run:

```sh
make DEBUG=1
```

#### Installing pycinot

You will find the package in `dist/cinot`. Simply copy the directory `cinot` wherever you want and add this path to your `PYTHONPATH`.

A better solution would be to install it in `~/.local/lib/python3.X/site-packages` (which should be in your ` PYTHONPATH`).

### Usage

```python
import cinot
import select

i = cinot.inot()
e = select.epoll()
# Ensure the file to watch exists; create it if needed.
open("WATCH", "w").close()
watched_fd = i.add_watch("WATCH", cinot.IN_MODIFY)
e.register(i.fd(), select.EPOLLIN | select.EPOLLET)
e.poll()
...
e.unregister(i.fd())
i.rm_watch(watched_fd)
```

### Help

See `inotyfi(7)` for details on the kernel API.

The documentation for the `select` module (`poll`, `epoll`, ...) is at https://docs.python.org/3/library/select.htm