# Python 3.13t

Video (in spanish): https://www.youtube.com/watch?v=Tq9DX2Ov8N4&t=4s

Python does have real threads.

```
python threads-io.py

# then run
pstree <pid>
```

Install free-threaded version with uv:

```
uv python list

uv python install cpython-3.13.1+freethreaded-linux-x86_64-gnu
```

If we run the io code again in both version of python:

```
❯ uv run --python 3.13 python threads-io.py
work took: 1.000233 s
work took: 1.000189 s
work took: 1.000080 s
work took: 1.000077 s
work took: 1.000143 s
work took: 1.000170 s
work took: 1.000456 s
work took: 1.000439 s
-- main took: 4.002552 s

❯ uv run --python 3.13t python threads-io.py
work took: 1.000431 s
work took: 1.000143 s
work took: 1.000506 s
work took: 1.000333 s
work took: 1.000434 s
work took: 1.000402 s
work took: 1.000445 s
work took: 1.000379 s
-- main took: 4.004804 s
```

IO threads in Python always worked.

What if we had some CPU work? Run `threads-cpu-mem.py`

We can see that the results with 3.13t is not correct. Even worst, everytime is
different!
