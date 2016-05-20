#基于包过滤的简易防火墙实现

首先复习一下:

> 问：防火墙是什么？

> 答：防火墙是由不燃烧体构成，耐火极限不低于3h。为减小或避免建筑、结构、设备遭受热辐射危害和防止火灾蔓延，设置的竖向分隔体或直接设置在建筑物基础上或钢筋混凝土框架上具有耐火性的墙。

![壮哉我大长城](https://raw.githubusercontent.com/familyld/Simple_Firewall_Based_on_Packet_Filtering/master/graph/gfw.jpg)

这都信，you are too young.

在网络安全中，我们所说的防火墙是指一种 `置于不同网络之间` `执行访问控制策略` 从而 `保护本地网络安全` 的 `一组硬件和软件系统`，它是**不同网络间通信流的唯一通道**，能根据企业有关的安全政策控制进出网络的访问行为（如允许、拒绝、监视、记录等）。

> 问：那防火墙具体能做什么呢？

> 答：大致如下

> - 实现安全机制，监控安全相关行为
>       - 过滤进出网络的数据包
>       - 管理进出网络的访问行为并封堵某些禁止的访问行为
>       - 记录通过防火墙的信息内容和活动（所以你做了什么不可描述的事情，CIO是可以知道的噢）
>       - 对网络攻击进行检测和告警

> - 提供强有力的身份验证
>       - 辨别用户身份并根据其身份提供网络服务
>       - 比方说Boss上班可以看看youtube，你上班就只能老老实实工作

> - 提供VPN(virtual private networks) 服务
>       - 在不安全的互联网上提供一个虚拟专用网络，使得远程访问时，能够保证企业机密数据的安全

特别地，防火墙提供的是一种双向的保护，既阻止外部网络对本地网络的非法访问和入侵，又控制了本地网络对外部不良网络的访问或未经允许的数据输出，从而有效地防止内部信息的泄露。

> 问：那防火墙什么时候会失手呢？

> 答：一是绕开防火墙的攻击，也即旁路(bypass)；二是内部威胁，比方说公司内部出了个内奸，它把机密数据拷到U盘上带出去，这是防火墙阻止不了的；三是病毒，防火墙检查一般是检查源、目的地址和端口号，所以如果病毒在传输的数据上做文章，防火墙是难以防御的。
