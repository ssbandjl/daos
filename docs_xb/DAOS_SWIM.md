# SWIM(可扩展弱一致性组员协议): scalable weakly-consistent infection-style process group membership protocol

https://ieeexplore.ieee.org/document/1028914



# 可扩展的弱一致性感染型进程组成员协议



**抽象的：**

几个分布式对等应用程序需要所有参与进程的进程组成员信息的弱一致知识。SWIM 是一个通用软件模块，为大型流程组提供此服务。SWIM 努力的动机是传统心跳协议的不可扩展性，这些协议要么施加随组大小二次增长的网络负载，要么在检测过程崩溃时牺牲响应时间或误报频率。本文报告了 SWIM 子系统在大型商用 PC 集群上的设计、实施和性能。与传统的心跳协议不同，SWIM 将成员协议的故障检测和成员更新分发功能分开。通过有效的对等定期随机探测协议监控流程。首次检测到每个进程故障的预期时间和每个成员的预期消息负载都不会随组大小而变化。有关成员资格更改的信息，例如进程加入、退出和失败，通过 ping 消息和确认的捎带传播。这导致了强大而快速的感染方式（也是流行或八卦方式）的传播。通过修改协议以允许组成员在将进程声明为失败之前对其进行怀疑，从而降低了 SWIM 系统中的错误故障检测率——这允许系统发现并纠正错误的故障检测。最后，该协议保证了检测故障的确定性时间限制。展示了 SWIM 原型的实验结果。我们讨论了设计在 WAN 范围内的可扩展性。

**发表于：**[Proceedings International Conference on Dependable Systems and Networks](https://ieeexplore.ieee.org/xpl/conhome/7991/proceeding)



**Abstract:**

Several distributed peer-to-peer applications require weakly-consistent knowledge of process group membership information at all participating processes. SWIM is a generic software module that offers this service for large scale process groups. The SWIM effort is motivated by the unscalability of traditional heart-beating protocols, which either impose network loads that grow quadratically with group size, or compromise response times or false positive frequency w.r.t. detecting process crashes. This paper reports on the design, implementation and performance of the SWIM sub-system on a large cluster of commodity PCs. Unlike traditional heart beating protocols, SWIM separates the failure detection and membership update dissemination functionalities of the membership protocol. Processes are monitored through an efficient peer-to-peer periodic randomized probing protocol. Both the expected time to first detection of each process failure, and the expected message load per member do not vary with group size. Information about membership changes, such as process joins, drop-outs and failures, is propagated via piggybacking on ping messages and acknowledgments. This results in a robust and fast infection style (also epidemic or gossip-style) of dissemination. The rate of false failure detections in the SWIM system is reduced by modifying the protocol to allow group members to suspect a process before declaring it as failed - this allows the system to discover and rectify false failure detections. Finally, the protocol guarantees a deterministic time bound to detect failures. Experimental results from the SWIM prototype are presented. We discuss the extensibility of the design to a WAN-wide scale.

**Published in:** [Proceedings International Conference on Dependable Systems and Networks](https://ieeexplore.ieee.org/xpl/conhome/7991/proceeding)

**Date of Conference:** 23-26 June 2002

**Date Added to IEEE \*Xplore\*:** 10 December 2002

**ISBN Information:**

**INSPEC Accession Number:** 7396105

**DOI:** [10.1109/DSN.2002.1028914](https://doi.org/10.1109/DSN.2002.1028914)

**Publisher:** IEEE

**Conference Location:** Washington, DC, USA



![image-20220913215418405](/Users/xb/Library/Application Support/typora-user-images/image-20220913215418405.png)

### **heartbeats**

> 传统的诸如heartbeats这种membership protocols，每个node周期性地向网络中的所有其他节点发送heartbeat来表示自己是alive的，如果peer超过指定interval没有收到node的heartbeart则该node被认定为dead。这种方式适用于小型网络，其发送的heartbeart数量为O(n^2)，当网络中有成千上万的node时则会造成巨大的网络负担；SWIM采用Infection-Style dissemination感染型传播来解决这个问题





## **小结**

- SWIM的全称是Scalable, Weakly-Consistent, Infection-Style, Processes Group Membership Protocol；与传统的heartbeats相比，SWIM将整个过程分为Failure Detection及Membership update Dissemination两个task
- SWIM的failure detection过程分为两个部分，一个是direct ping，一个是indirect ping
- Infection-Style的方式进行dissemination，即利用Failure Detection的ping机制，将需要dissemination的消息piggyback在ping/ack上(传播携带的数据)，来实现类似gossip的消息传播，从而减少额外的单独信息传递开销



# [SWIM 会员协议](https://prakhar.me/articles/swim/)

 2015 年2 月 13 日 在 [分布式系统](https://prakhar.me/tags/distributed-systems)

假设您要求构建一个类似于[Cassandra](http://cassandra.apache.org/)的分布式数据库。您的存储系统将存储和处理在大量商品服务器上运行的大量数据。换句话说，您的系统将依靠 100 多个节点的力量来管理数据。

在这个规模上，失败将是常态，而不是例外。即使我们假设一个节点持续 1000 天（大约 3 年），在 500 个节点的集群中，每 2 天就会出现一次故障。

为了处理这种情况，您需要一个故障检测服务，它除了检测故障节点外，还使所有非故障进程与处于活动状态的进程保持同步。在这篇博文中，我们将介绍一种称为 SWIM 的协议并了解其内部工作原理。

### 游泳

SWIM 或可扩展**弱**一致感染**型**进程组**M成员****身份**协议是用于维护分布式系统中进程之间成员资格的协议。

> 成员协议为组中的每个进程提供了一个本地维护的列表，称为**成员列表**，该列表包含组中的其他非故障进程。

因此，该协议执行两项重要任务 -

- 检测失败，即如何识别哪个进程失败以及
- 传播信息，即如何将这些故障通知系统中的其他进程。

不言而喻，成员协议在检测故障方面应该是可扩展的、可靠的和快速的。成员协议的可扩展性和效率主要由以下属性决定

- 完整性：每个失败的进程**最终**都会被检测到吗？
- 故障检测速度：故障与非故障进程检测到的平均时间间隔是多少？
- 准确性：流程被归类为失败的频率，实际上是非故障的（称为误报率）？
- 消息负载：每个节点产生多少网络负载，是否均匀分布？

理想情况下，人们会想要一个完全 100% 准确的协议，这意味着检测到每个错误的过程，*没有*误报。然而，与分布式系统中的其他权衡一样，存在一个[不可能的结果](http://www.ecommons.cornell.edu/bitstream/1813/7192/1/95-1535.pdf)，即无法通过异步网络保证 100% 的完整性和准确性。因此，大多数成员协议（包括 SWIM）以准确性换取完整性，并试图将误报率保持在尽可能低的水平。

### SWIM 故障检测器

SWIM 故障检测器使用两个参数——一个协议周期`T`和一个整数`k`，它是故障检测子组的大小。

![img](https://prakhar.me/images/swim.png)SWIM 故障检测

上图显示了协议的工作原理。在每个`T`时间单位之后，进程 M i从其成员列表中选择一个随机进程 - 比如说 M j - 并向它发送一个*ping*。然后它等待来自 M j的*ack*。如果在预先指定的超时时间内没有收到*确认，M* i通过随机选择目标间接探测 M j并使用它们向M j发送*ping*。然后，这些目标中的每一个都代表 M i向 M j发送一个*ping* ，并在收到一个*ack时*`k``k`通知 M i。如果由于某种原因，这些进程都没有收到*ack*，则 M i将 M j声明为失败并将更新移交给传播组件（如下所述）。

SWIM 与其他心跳/八卦协议之间的关键区别因素是 SWIM 如何使用其他目标到达 M j以避免在 M i和 M j之间的网络路径上出现任何拥塞。

### SWIM 传播组件

传播组件只是将故障更新多播到组的其余部分。所有收到此消息的成员都将 M j从其本地成员列表中删除。关于新成员或自愿离开的信息以类似的方式被多播成员。新加入成员或自愿离开成员的信息以类似方式多播。

### 改进

**感染式传播**——在更健壮的 SWIM 版本中，传播组件不依赖于不可靠和低效的多播，而是在故障检测器协议发送的*ping*和*ack消息上捎带成员资格更新。*这种方法被称为*感染式*（因为这类似于八卦或人群中的流行病）的传播，其优点是具有较低的数据包丢失和更好的延迟。

**怀疑机制**- 尽管 SWIM 协议通过 ping`k`节点来防止两个节点之间出现拥塞的情况，但仍然有可能完全健康的进程 M j变得缓慢（高负载）或由于周围的网络分区而暂时不可用本身，因此被协议标记为失败。

每当基本 SWIM 检测到故障时，SWIM 通过运行称为 Suspicion 子协议的子协议来缓解这种情况。在该协议中，当 M i发现 M j没有响应（直接和间接）时，它将 M j标记为*嫌疑人*，而不是将其标记为失败。然后它使用传播组件将此消息 M j :发送`suspect`到其他节点（感染式）。任何后来发现 M j响应*ping的进程都会取消标记怀疑并用 M* j :`alive`消息感染系统。

**有时间限制的完整性**——基本的 SWIM 协议在平均恒定数量的协议周期内检测故障。虽然保证*最终*检测到每个故障进程，但由于目标节点的随机选择，在将 ping 发送到故障节点之前可能会有相当大的延迟。

SWIM 建议的一个简单改进是通过维护一组已知成员并以循环方式选择*ping目标。*数组完全遍历后，随机打乱，继续处理。这为同一目标的连续选择之间的时间单位提供了有限的上限。

### 结论

SWIM 协议已在许多分布式系统中使用。一个使用 SWIM 的流行开源系统是[Serf](https://www.serfdom.io/)，它是[Hashicorp](https://www.hashicorp.com/)为集群成员提供的分散式解决方案。该[文档](https://www.serfdom.io/docs/internals/gossip.html)对底层架构进行了相当清晰的演练。Hashicorp 的好心人也在[Github](https://github.com/hashicorp/memberlist)上开源了他们的实现。对于那些通过阅读代码可以更好地理解的人，请务必检查一下。

最后，这篇博文特意保留了数学以使高级思想变得简单，但如果您有兴趣深入研究，请务必阅读[本文](http://www.cs.cornell.edu/~asdas/research/dsn02-SWIM.pdf)以更好地了解误报率的上限、检测故障的平均时间和网络负载。

我希望这篇博文能让您了解流行的会员协议 SWIM 是如何工作的。如果您有任何疑问，请在下面的评论中告诉我。