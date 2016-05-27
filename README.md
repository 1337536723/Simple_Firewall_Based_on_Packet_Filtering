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
>       - 辨别用户身份并根据其身份提供网络设备
>       - 比方说Boss上班可以看看youtube，你上班就只能老老实实工作

> - 提供VPN(virtual private networks) 设备
>       - 在不安全的互联网上提供一个虚拟专用网络，使得远程访问时，能够保证企业机密数据的安全

特别地，防火墙提供的是一种双向的保护，既阻止外部网络对本地网络的非法访问和入侵，又控制了本地网络对外部不良网络的访问或未经允许的数据输出，从而有效地防止内部信息的泄露。

> 问：那防火墙什么时候会失手呢？

> 答：一是绕开防火墙的攻击，也即旁路(bypass)；二是内部威胁，比方说公司内部出了个内奸，它把机密数据拷到U盘上带出去，这是防火墙阻止不了的；三是病毒，防火墙检查一般是检查源、目的地址和端口号，所以如果病毒在传输的数据上做文章，防火墙是难以防御的。

#项目架构分析
原项目是由一位网名为sudhirmangla的印度工程师和他的朋友共同完成并开源的，[项目地址](http://www.codeproject.com/Articles/5602/Simple-Packet-Filter-Firewall?msg=1635599#xx1635599xx)。界面如下：

![完整版界面](http://www.csc.villanova.edu/~vbhardwa/netclass/Firewall_files/image002.jpg)

但这次实验采用的是~~阉割版~~的代码，要求在Windows XP下实现一个基于MFC的包过滤防火墙，包含以下功能：

- 直接过滤进、出本机所有的数据包；
- 能够选择过滤进、出本机的某一种类数据包；
- 用户可以自己添加过滤规则：对指定端口、指定源地址、指定目的地址的指定类型的数据包进行指定的操作；
- 程序具备良好的健壮性、可用性；
- 除以下功能外再加入一些创新的功能；

架构图如下：

![架构](https://raw.githubusercontent.com/familyld/Simple_Firewall_Based_on_Packet_Filtering/master/graph/How_it_Works.png)

其中Hook可以理解为用户模式到网卡驱动之间一个沟通的渠道，或者说是一个系统设备。当我们需要让网卡驱动做用户设定的某件任务(比如开始过滤)时，首先要把任务转化为一个系统设备，然后把这个系统设备在系统数据库中进行注册，然后开启网卡驱动的IO流，最后把设备写进去使其生效。

整个项目可以分为底层文件和应用层文件两部分，底层文件负责和底层驱动的交互，应用层文件则负责与用户界面的交互。因为应用层功能实现上的代码被~~阉割~~了，所以需要对缺失函数进行填充才能实现功能。

##底层文件：

###1.  DrvFltIp.h：

定义了设备控制代码的CTL_CODE（采用函数指针的方法实现，为windows内部用于进行设备控制的宏定义，在这里面标识了设备的类型、访问控制方法、功能和IO口对于内存的访问方式），同时也定义了用于过滤的`filter`结构体以及`TCP header`、`IP header`、`UDP header`的结构体

#### CTL_CODE
CTL_CODE函数用于创建一个唯一的**32位系统I/O控制代码**，这个控制代码包括4部分组成：DeviceType（设备类型，高16位（16-31位）），Access（访问限制，14-15位），Function（功能，2-13位），Method（I/O访问内存使用方式，0-1位）。本项目中定义了5个设备控制代码，分别是`开始过滤`、`停止过滤`、`添加过滤规则`、 `清除过滤规则` 和 `读取过滤规则`

```cpp
#define  CTL_CODE(DeviceType, Function, Method, Access) (
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method)
)
```

**Notice:** 传入函数的参数顺序和控制代码中的位置顺序不是一致的

具体参考：http://blog.csdn.net/lujunql/article/details/2532362

###2.  TDriver.h：

与底层驱动交互有关的代码，需要借助于`DrvFltIp.h`所得到的CTL_CODE。其包含的变量及函数如下：

####a) 私有变量driverHandle
HANDLE（句柄）类型，句柄可以简单理解成一个指针（在这里是用作控制驱动的指针，在VC中HANDLE类型定义在`Winnt.h`文件内，通过typedef PVOID类型得到。PVOID是一个**无指定类型的指针**，也就是**可指向任意类型的指针**）

####b) 私有变量driverName、driverPath、driverDosName
LPTSTR（指向可能的Unicode字符串的长指针）类型，LPTSTR是一个32-bit指针，其指向一个字符串，在字符串中每个字符可能占1字节或2字节（取决于Unicode是否定义）。在这里面driverName为驱动名，driverPath为驱动所在路径，而driverDosName估计是驱动在Dos系统下的名字

####c)  私有变量initialized、started、loaded和removable
BOOL类型，用作设备状态的标识符，initialized标识设备是否被初始化，started标识设备是否被启动，loaded标识设备是否被加载，而removable标识设备是否可以被移除

####d)  私有函数OpenDevice
OpenDevice主要进行**设备句柄的开启**(注意不是设备的开启)：其会通过CreateFile函数打开用于操作设备的输入输出流（沟通用户模式和内核模式的桥梁，这里也即Hook）。其参数依次为：文件对象名(driverDosName)、访问模式、共享模式、销毁方式、创建方法、文件属性和模板文件句柄

####e)  构造函数和析构函数
构造函数中对各私有变量进行了初始化，而析构函数中主要进行资源的释放（比如将可能打开的驱动进行关闭）

####f)  一些私有变量的get函数和set函数

####g)  InitDriver
InitDriver主要进行设备的检查和初始化。首先会检查设备是否在占用，如是则先退出设备（如果退出失败会报错），然后根据用户的输入得到driverName、driverPath和driverDosName，并且对它们进行检查，如果不符合规则就返回错误代码，否则对设备进行初始化并返回初始化成功的代码。

####h)  LoadDriver
LoadDriver主要进行设备的加载，其关键是创建一个对应的防火墙系统设备。首先看一下有没有被初始化（如果没有先要进行初始化），然后再创建一个SC_HANDLE类型的SCManager变量，并将该变量作为参数用于注册系统设备（**要将创建的设备放到该数据库中才可正常执行**）

- SCManager变量通过OpenSCManager函数进行创建，该函数的功能是**建立一个到设备控制管理器的连接**并**打开指定的数据库**。如果创建成功将返回指定的设备控制管理器数据库的句柄，错误时返回NULL。共有有三个参数：

    * 第一个参数指目标计算机名称（NULL为自己）；
    * 第二个参数为要打开的设备控制管理器数据库的名称（NULL为默认为SERVICE_ACTIVE_DATABASE）；
    * 第三个参数为权限

- SCService变量通过CreateService函数进行创建，该函数的功能是**创建一个放于指定设备控制管理器数据库的系统对象**，也即把设备加载到数据库。如果创建成功将返回设备的句柄，错误时返回NULL。一共有13个参数：

    * 第一个参数为设备控制管理器数据库的句柄；
    * 第二个参数为要安装的设备名称；
    * 第三个参数为对被用户界面来识别设备的显示名称；
    * 第四个参数为对设备的访问权限；
    * 第五个参数为设备类型（代码中的SERVICE_KERNEL_DRIVER说明为驱动设备程序）；
    * 第六个参数为设备启动选项（代码中的SERVICE_DEMAND_START说明为设备控制管理器启动）；
    * 第七个参数为当该设备启动失败时产生错误的严重程度及保护措施（代码中的SERVICE_ERROR_NORMAL说明将设备记录到日志中并继续执行）；
    * 第八个参数为该设备程序的路径；
    * 第九个参数为该设备程序所属的注册表组的名字（NULL指不属于任何组）；
    * 第十个参数与第九个参数配合使用，说明该设备在第九个参数中指定的组中的标记值；
    * 第十一个参数说明设备在该组中的执行先决条件；
    * 第十二个参数为可执行该设备的账户名（NULL说明本地账户）；
    * 第十三个参数为执行该设备的密码

####i)  UnloadDriver
UnloadDriver主要进行设备的注销，其会对现有的设备进行依次的检查。一般来说会经历初始化->加载->启动三步，因此在这里先要弄清楚设备启动了没有，如果有就要先关闭设备（StopDriver函数）。然后再直接通过函数DeleteService注销设备。

####j)  StartDriver
StartDriver主要进行设备的启动(前面加载好了，还得启动才能正式起作用），主要通过系统提供的StartService函数进行。在这个函数中需要传入三个参数:

- 第一个参数要传入设备对应的指针，该指针可通过OpenService函数得到；
- 第二个参数和第三个参数为启动该设备的参数（argc和argv）。然后调用OpenDevice函数打开对应的系统设备用于和用户进行交互（会返回对应设备操作的句柄）。
- **Notice：** 在OpenService函数中需要传入设备所在设备控制管理器数据库的名称，要启动的设备名和访问类型。

####k)  StopDriver
StopDriver主要进行设备的关闭，主要通过系统提供的ControlService函数进行（因为这里只是将设备停掉，没有将设备完全干掉）。在这个函数中需要传入三个参数：

- 第一个参数要传入设备对应的指针，该指针可通过OpenService函数得到；
- 第二个参数为要对对应设备做的操作(这里是关闭)；
- 第三个必须传入一个参数为系统设备状态的结构体（其实这个也是函数返回值，但是由于返回值只能有一个，因此只能用这种办法）。

同时还要关闭设备，可以通过向CloseHandle函数传入句柄(私有变量driverHandle)实现

####l)  WriteIo、ReadIo和RawIo
WriteIo、ReadIo和RawIo主要进行对于设备的控制指令的交互，其会通过DeviceIoControl函数进行控制，功能是发送和接收对应设备驱动程序的控制代码。该函数的参数依次为：设备句柄(driverHandle)、控制命令CTL_CODE代码、给驱动程序传递的数据的所在地址、给驱动程序传递的数据的大小（字节）、驱动程序传回的数据的所在地址、驱动程序传回的数据的大小（字节）、驱动程序实际传回的数据的大小（字节）、是否并行操作

##应用层文件：

###1.  AddRuleDlg.h：

基于底层文件进行过滤规则的加入

####a)  构造函数
通过LoadDriver函数进行了设备的注册、启动和设备的开启(要使设备生效，三个条件缺一不可)

####b)  中间有一些跟MFC有关的用于前端交互的函数，不重要，先略去

####c)  AddFilter
AddFilter函数用于创建一条新的防火墙过滤规则并且写入设备文件

####d)  Verify (待填充)
Verify函数用于验证用户的ip地址输入的有效准确（包括格式正确和地址规则正确）

####e)  OnAddSave (待填充)
OnAddSave函数主要需要实现下面几个功能：

- 从前端界面获取用户输入的防火墙规则

- 将获取到的规则进行整理后写入文件（文件名为saved.rul，可借助本文件中提供的文件读写函数进行）

- 创建对应的防火墙过滤规则并且写入设备文件（借助AddFilter函数）

####f)  文件中还提供了一些可能会用到的文件读写函数（可在其上进行优化，或者直接使用）

###2.  fireView.h：
基于底层文件进行过滤规则的查看与控制

####a)  构造函数
主要用于初始化变量，变量的功能比较好理解，就不细说了

####b)  PreCreateWindow
PreCreateWindow指的是初始化界面时的行为，这里注册并启动了需要用到的设备

####c)  接下来还是几个跟MFC有关的用于前端交互的函数

####d)  OnAddrule
OnAddrule指点击了AddRule按钮后的行为

####e)  OnStart
OnStart指点击了Start或者Stop按钮后的行为，对于hook进行启动和中止的控制

####f)  OnBlockping (待填充)
OnBlockping指点击了Block Ping按钮后的行为，功能是加入一条对ping包完全block的规则

####g)  OnBlockAll (待填充)
OnBlockAll指点击了Block All按钮后的行为，功能是加入一条对所有包完全block的规则

####h)  OnAllowAll (待填充)
OnAllowAll指点击了Allow All按钮后的行为，功能是无效化所有的block规则

####i)  ImplementRule (待填充)
ImplementRule在OnViewrules函数中进行了调用，指点击了View Rules按钮后的行为，功能是把所有的规则显示在前端界面上（利用本文件中已有的跟MFC有关的用于前端交互的函数，比如：AddItem函数）

####j)  ParseToIp
ParseToIp函数可直接删除（如果真的要删除要把对应.h文件的函数声明也删除），这个函数主要是分割出一部分代码，避免ImplementRule函数写得太长。

####k)  后面都是跟MFC有关的用于前端交互的函数，可不修改

###3.  fire.h：
跟MFC有关的用于前端交互的文件，可不修改

###4.  MainFrm.h：
跟MFC有关的用于前端交互的文件，可不修改

###5.  stdafx.h：
跟MFC有关的用于前端交互的文件，可不修改

###6.  SystemTray.h：
跟MFC有关的用于前端交互的文件，可不修改

###7.  fireDoc.h：
用于文件读写的辅助文件，可不修改

###8.  sockutil.h：
提供了一些用于ip地址转换的工具函数

###目标
1. 实现AddRuleDlg.cpp中的Verify函数和OnAddSave函数；
2. 实现fireView.cpp中的OnBlockping函数，OnBlockall函数，OnAllowall函数，以及ImplementRule函数。

注：如果把ImplementRule函数分割成ImplementRule函数和ParseToIP函数两个函数，则还需实现ParseToIp函数。

#实验配置

本次实验使用到三台机器，两台虚拟机分别装载Windows XP系统和SEEDUbuntu系统，我们在Windows Xp系统下实现包过滤防火墙，然后使用SEEDUbuntu虚拟机进行网络攻击测试。另外，本机装载win10系统，可用于验证。详见[实验配置流程](https://github.com/familyld/Simple_Firewall_Based_on_Packet_Filtering/blob/master/%E5%AE%9E%E9%AA%8C%E9%85%8D%E7%BD%AE.md)。

#界面优化

由于原实现实在不美观，操作上也不够人性化，所以我适当地进行了一些界面调整，包括扩展窗体大小，调整过滤规则列表的列宽和整体宽度，使得IP地址和掩码可以完整显示，等等。 详见[界面优化流程](https://github.com/familyld/Simple_Firewall_Based_on_Packet_Filtering/blob/master/%E7%95%8C%E9%9D%A2%E4%BC%98%E5%8C%96.md)。

#程序流程梳理

在[程序流程梳理](https://github.com/familyld/Simple_Firewall_Based_on_Packet_Filtering/blob/master/%E7%A8%8B%E5%BA%8F%E6%B5%81%E7%A8%8B%E6%A2%B3%E7%90%86.md)中对这个防火墙程序的启动以及各点击时间触发的函数进行了梳理。

#函数填充

在[函数填充](https://github.com/familyld/Simple_Firewall_Based_on_Packet_Filtering/blob/master/%E7%A8%8B%E5%BA%8F%E6%B5%81%E7%A8%8B%E6%A2%B3%E7%90%86.md)中详细剖析了怎样把前面提到的7个大坑填好。
