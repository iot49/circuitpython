# IoTPython

IoTPython is a fork of [CircuitPhython](https://github.com/adafruit/circuitpython/) with a added features:

* Module `msgpack`



# Fork of CircuitPython

Branches:
- `main`: tracks `circuitpython/main`
- `iot`: main branch of this fork with features merged

Feature braches (off `main`):
- `flow`: support for flow control in nrf52 uart driver
- `msgpack`: subset of CPython msgpack (PR)

In addition, branch `iot` adds a module with the same name. Do

```python
import iot
print(dir(iot))
```

to get a list of features.

Note: the same repo contains also a fork of `MicroPython`. Switch to branch `mp` for more information.
