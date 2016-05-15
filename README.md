# 简易分布式kv存储系统
## 进程基础框架
进程标识：type.id这样标识，type为字符串，用于表示不同的进程，有dir，proxy，svr；id是同一个类型下进程的编号，比如dir.2表示为编号为2的dir进程。

## 进程的唯一性：
每个进程只能启动一个，通过<code>/opt/kv_storage/pid/</code>下面的.pid文件来唯一识别进程，如果有进程存在，不能启动。具体实现的方式是在<code>/opt/kv_storage/pid/</code>下面生成当前进程id为表示的pid文件，进程启动后，将自己的pid写入这个文件，并把这个文件锁住（flock或fcntl都可以实现），这样其他进程读取这个文件会失败，这样来达到唯一性的目的。

## 定时器
进程还需要实现一个定时器

## 配置文件
使用xml作为配置文件，配置文件共同需要涉及的配置是进程表示。
<identity>dir.1<identity>
## 进程通信协议
使用protobuf来生成协议文件，并且使用tcp进行通信
涉及范围：
Dir和proxy
上报内容id，ip
proxy和server
api和proxy，api和dir，server和dir
## Dir目录服务
Dir目录服务，客服户端定期从这里拉取proxy的列表，所有proxy向这里注册以及注销。
另外，所有的server也要向dir注册和注销，proxy向这里请求server的信息。
进程模型：
单进程单线程，守护进程
## Proxy代理
proxy作为负载均衡使用，proxy使用key做hash，将请求转发到不同的server。每个proxy跟每个server都要连接，这样才能保证请求能转发到任意一个server，这样proxy内部需要维护一个server的路由表。而路由表同样也是从dir中获取。
内部维护两个server队列，已连接队列，正在连接队列，将server首先放在正在连接队列，连接完成后放在已连接队列，否则继续呆在待连接队列，每隔一段时间会去轮询正在连接队列，尝试连接队列中所有未连接的描述符。
进程模型：单进程单线程，守护进程
## Server存储
为了简化，暂时key和value都为string类型。暂时支持4个命令字，insert，replace，get，delete
进程模型：单进程多线程，守护进程
## API客户端
暂时先做一个proxy进程+server进程，客户端直接连接proxy，然后均衡转发消息，server返回，不实现dir进程，proxy中server的列表现在proxy的配置中预先配置。

