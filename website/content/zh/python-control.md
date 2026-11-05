# 使用 Python 控制

按照[第一次仿真](quickstart.md)启动 REXROV，并保持运行。在另一个[准备好的终端](terminals.md)中读取位置，再向一个推进器发送短时指令。

## 订阅位置

将以下程序保存为 `observe.py`，在已加载 ROS 环境的终端中运行 `python3 observe.py`。

{{CODE:python-control:0}}

程序大约每秒输出一次位置。按 Ctrl+C 停止观测程序，仿真仍会继续。

## 发送有时间限制的指令

将以下程序保存为 `pulse_thruster.py`，运行 `python3 pulse_thruster.py`。程序等待订阅者出现，按主机时间向**推进器 1 施加约两秒的 5 N 指令**，最后发送零值。推进器的安装轴决定产生的力和力矩。

{{CODE:python-control:1}}

这里的信号设置让 Ctrl+C 能在 ROS 发布者仍可用时执行清理代码。如果强制终止进程，请停止仿真，或按照[推进器控制](robot-control.md)发送中立指令。

## 观察响应

在另一个准备好的终端中，查看指令期间的角速度：

{{CODE:python-control:2}}

角速度应随指令变化，并在结束后趋近于零。结合里程计观察，可区分执行器响应与整机运动。按 Ctrl+C 停止查看。

## 编写控制器

根据测量状态和控制律，将力分配到 REXROV 的八个推进器。保持话题名称、安装轴和单位一致。实验逻辑使用仿真时间；结束时向所有受控执行器发送中立值。

BlueROV2 使用 [MAVROS／ArduSub 控制链路](robot-control.md)。需要比较实验时，可[记录观测数据](visualization.md)。
