# Profiling test

Profiling test is composed of tree components:

* A minimal Agent with just `UAGENT_CED_PROFILE` enable.
* A publisher developed with the Client library.
* A subscriber developed with the Client library.

## Why?

There are some scenarios where the Agent must run over low resource devices, so it is needed to characterize the minimum requirements of the Agent in terms of memory footprint.
In that sense, the Agent profiling with the lowest memory footprint is the one that only use the `CedMiddleware` implementation without logger, cli, and p2p.
Such configuration could be address disabling all the Agent`s profiles except the `UAGENT_CED_PROFILE`.

## How?

```bash
cmake .. -DUXRCE_ENABLE_CLIENT=OFF -DUXRCE_ENABLE_AGENT=OFF -DUXRCE_BUILD_PROFILING=ON
```