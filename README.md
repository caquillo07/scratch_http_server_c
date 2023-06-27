# scratch_http_server_c

Very basic, bare bones, single threaded HTTP server written in C.

The purpose of this was to learn and demystify what an HTTP server
actually does, and practice some C along the way. This code will only
run in UNIX-based systems, not tested for Windows.

Educational purposes only, nothing from this code should be considered
production quality.

## How to Run
```shell
# using make file
make run

# using clang
clang -o main main.c
./main
```

on a separate terminal, can check with cURL
```shell
curl localhost:8080/index.html -v
```
