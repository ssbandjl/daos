Next: [Integer Overflow](https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html#Integer-Overflow-Builtins) Builtins , Previous: [__sync Builtins](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html#g_t_005f_005fsync-Builtins) , Up: [C Extensions](https://gcc.gnu.org/onlinedocs/gcc/C-Extensions.html#C-Extensions)  [ [Contents](https://gcc.gnu.org/onlinedocs/gcc/index.html#SEC_Contents) ][ [Index](https://gcc.gnu.org/onlinedocs/gcc/Option-Index.html#Option-Index) ]

------



### 6.55 内存模型感知原子操作的内置函数

以下内置函数大致符合 C++11 内存模型的要求。它们都通过前缀____atomic 来标识__原子' 并且大多数都是重载的，因此它们可以处理多种类型。

这些功能旨在取代传统的 '__sync' 内置程序。主要区别在于请求的内存顺序是函数的参数。新代码应始终使用 '__原子'内置而不是'__同步' 内置程序。

请注意，'__原子' 内置函数假定程序将符合 C++11 内存模型。特别是，他们假设程序没有数据竞争。有关详细要求，请参阅 C++11 标准。

这 '__原子' 内置函数可以与长度为 1、2、4 或 8 字节的任何整数标量或指针类型一起使用。如果 '__you128'（参见[__int128](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fint128.html#g_t_005f_005fint128)）受架构支持。

四个非算术函数（load、store、exchange 和 compare_exchange）也都有一个通用版本。这个通用版本适用于任何数据类型。如果特定的数据类型大小使之成为可能，它会使用无锁内置函数；否则，将在运行时解决外部调用。此外部调用的格式相同，但添加了 '尺寸_t' 参数作为第一个参数插入，指示所指向对象的大小。所有对象的大小必须相同。

可以指定 6 种不同的内存顺序。这些映射到具有相同名称的 C++11 内存顺序，请参阅 C++11 标准或[关于原子同步的 GCC wiki](https://gcc.gnu.org/wiki/Atomic/GCCMM/AtomicSync)了解详细定义。个别目标也可能支持额外的内存订单，以用于特定架构。有关这些的详细信息，请参阅目标文档。

原子操作既可以约束代码运动，又可以映射到硬件指令以实现线程之间的同步（例如，栅栏）。这种情况发生的程度是由记忆顺序控制的，这里列出的记忆顺序大致按强度升序排列。每个内存顺序的描述只是为了粗略说明效果，不是规范；有关精确语义，请参阅 C++11 内存模型。

An atomic operation can both constrain code motion and be mapped to hardware instructions for synchronization between threads (e.g., a fence). To which extent this happens is controlled by the memory orders, which are listed here in approximately ascending order of strength. The description of each memory order is only meant to roughly illustrate the effects and is not a specification; see the C++11 memory model for precise semantics.

- `__ATOMIC_RELAXED`

  意味着没有线程间排序约束。

- `__ATOMIC_CONSUME`

  由于`__ATOMIC_ACQUIRE` C++11 对 `memory_order_consume`.

- `__ATOMIC_ACQUIRE`

  创建从释放（或更强）语义存储到此获取负载的线程间发生前约束。可防止吊码到操作前。

- `__ATOMIC_RELEASE`

  创建线程间发生前约束以获取（或更强）从此版本存储读取的语义负载。可以防止下沉代码到操作后。

- `__ATOMIC_ACQ_REL`

  结合了`__ATOMIC_ACQUIRE`和 的效果`__ATOMIC_RELEASE`。

- `__ATOMIC_SEQ_CST`

  `__ATOMIC_SEQ_CST`对所有其他操作 强制执行总排序。

请注意，在 C++11 内存模型中，*栅栏*（例如，'__atomic_thread_fence') 与特定内存位置上的其他原子操作（例如，原子加载）结合使用；对特定内存位置的操作不一定会以相同的方式影响其他操作。

鼓励目标架构为每个原子内置函数提供自己的模式。如果没有提供目标，则原始的非内存模型集 '__同步' 使用原子内置函数，以及围绕它的任何必需的同步栅栏，以实现正确的行为。这种情况下的执行受到与那些内置函数相同的限制。

如果没有提供无锁指令序列的模式或机制，则会调用具有相同参数的外部例程，以便在运行时解析。

在为这些内置函数实现模式时，只要该模式实现了最严格的`__ATOMIC_SEQ_CST`内存顺序，就可以忽略内存顺序参数。任何其他内存顺序都可以使用此内存顺序正确执行，但它们可能无法像更适当地实现放宽要求时那样高效地执行。

请注意，C++11 标准允许在运行时而不是在编译时确定内存顺序参数。这些内置函数将任何运行时值映射到`__ATOMIC_SEQ_CST`而不是调用运行时库调用或内联 switch 语句。这是符合标准、安全且目前最简单的方法。

内存顺序参数是有符号整数，但只有低 16 位保留用于内存顺序。带符号的 int 的其余部分保留给目标使用，应该为 0。使用预定义的原子值可确保正确使用。

- 内置函数：*type* **__atomic_load_n** *( type \*ptr, int memorder)*

  这个内置函数实现了原子加载操作。它返回的内容。 `*ptr`有效的内存顺序变体是 `__ATOMIC_RELAXED`、`__ATOMIC_SEQ_CST`、`__ATOMIC_ACQUIRE`和`__ATOMIC_CONSUME`。

- 内置函数：*void* **__atomic_load** *( type \*ptr, type \*ret, int memorder)*

  这是原子负载的通用版本。它返回in的内容。 `*ptr``*ret`

- 内置函数：*void* **__atomic_store_n** *( type \*ptr, type val, int memorder)*

  这个内置函数实现了原子存储操作。它 `val`写入. `*ptr`有效的内存顺序变体是 `__ATOMIC_RELAXED`、`__ATOMIC_SEQ_CST`和`__ATOMIC_RELEASE`。

- 内置函数：*void* **__atomic_store** *( type \*ptr, type \*val, int memorder)*

  这是原子存储的通用版本。它存储into的值。 `*val``*ptr`

- 内置函数：*type* **__atomic_exchange_n** *( type \*ptr, type val, int memorder)*

  这个内置函数实现了原子交换操作。它将 val写入，并返回 的先前内容 。 `*ptr``*ptr`所有内存顺序变体均有效。

- 内置函数：*void* **__atomic_exchange** *( type \*ptr, type \*val, type \*ret, int memorder)*

  这是原子交换的通用版本。它存储into的内容。的原始值被复制到. `*val``*ptr``*ptr``*ret`

- 内置函数：*bool* **__atomic_compare_exchange_n** *( type \*ptr, type \*expected, type desired, bool weak, int success_memorder, int failure_memorder)*

  这个内置函数实现了原子比较和交换操作。这会将 的内容与 的内容进行 比较。如果相等，则该操作是*读取-修改-写入* 操作，可将所需写入. 如果它们不相等，则操作为*读取*，并将 的当前内容 写入. weak用于 弱 compare_exchange，它可能会虚假地失败，以及强变化，它永远不会虚假地失败。许多目标只提供强烈的变化而忽略参数。如有疑问，请使用强变化。 `*ptr``*expected``*ptr``*ptr``*expected``true``false`如果需要写入则返回，并且根据success_memorder指定的内存顺序影响内存。此处可以使用的内存顺序没有限制。 `*ptr``true`否则，`false`返回并根据failure_memorder影响内存。此内存顺序不能为 `__ATOMIC_RELEASE`nor `__ATOMIC_ACQ_REL`。它也不能是比success_memorder指定的更强的顺序。

- 内置函数：*bool* **__atomic_compare_exchange** *( type \*ptr, type \*expected, type \*desired, bool weak, int success_memorder, int failure_memorder)*

  这个内置函数实现了 `__atomic_compare_exchange`. 该函数实际上与 相同 `__atomic_compare_exchange_n`，除了所需的值也是一个指针。

- 内置函数：*type* **__atomic_add_fetch** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_sub_fetch** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_and_fetch** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_xor_fetch** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_or_fetch** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_nand_fetch** *( type \*ptr, type val, int memorder)*

  这些内置函数执行名称建议的操作，并返回操作结果。对指针参数的操作就像操作数属于该`uintptr_t`类型一样执行。也就是说，它们不会按指针指向的类型的大小进行缩放。`{ *ptr op = val; 返回 *ptr; } { *ptr = ~(*ptr & val); 返回 *ptr; } // 南德 `第一个参数指向的对象必须是整数或指针类型。它不能是布尔类型。所有内存命令均有效。

- 内置函数：*type* **__atomic_fetch_add** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_fetch_sub** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_fetch_and** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_fetch_xor** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_fetch_or** *( type \*ptr, type val, int memorder)*

- 内置函数：*type* **__atomic_fetch_nand** *( type \*ptr, type val, int memorder)*

  这些内置函数执行名称建议的操作，并返回之前存在的值。对指针参数的操作就像操作数属于该类型一样执行。也就是说，它们不会按指针指向的类型的大小进行缩放。 `*ptr``uintptr_t``{ tmp = *ptr; *ptr op = val; 返回 tmp; } { tmp = *ptr; *ptr = ~(*ptr & val); 返回 tmp; } // 南德 `对参数的约束与对应的 `__atomic_op_fetch`内置函数相同。所有内存命令均有效。

- 内置函数：*bool* **__atomic_test_and_set** *(void \*ptr, int memorder)*

  此内置函数对 at 的字节执行原子测试和设置操作。该字节被设置为某些实现定义的非零“设置”值，并且返回值是当且仅当先前的内容被“设置”时。它应该只用于or类型的操作数。对于其他类型，只能设置部分值。 `*ptr``true``bool``char`所有内存命令均有效。

- 内置函数：*void* **__atomic_clear** *(bool \*ptr, int memorder)*

  此内置函数对 执行原子清除操作 。运算后，包含 0。它只能用于类型为or的操作数，并与 .结合使用。对于其他类型，它可能只能部分清除。如果类型不 喜欢使用. `*ptr``*ptr``bool``char``__atomic_test_and_set``bool``__atomic_store`有效的内存顺序变体是 `__ATOMIC_RELAXED`、`__ATOMIC_SEQ_CST`和 `__ATOMIC_RELEASE`。

- 内置函数：*void* **__atomic_thread_fence** *(int memorder)*

  此内置函数根据指定的内存顺序充当线程之间的同步围栏。所有内存命令均有效。

- 内置函数：*void* **__atomic_signal_fence** *(int memorder)*

  这个内置函数充当线程和基于同一线程的信号处理程序之间的同步围栏。所有内存命令均有效。

- 内置函数：*bool* **__atomic_always_lock_free** *(size_t size, void \*ptr)*

  `true`如果大小字节的对象总是为目标体系结构生成无锁原子指令，则 此内置函数返回。size必须解析为编译时常量，并且结果也解析为编译时常量。ptr是指向可用于确定对齐的对象的可选指针。值 0 表示应使用典型对齐方式。编译器也可能忽略此参数。`if (__atomic_always_lock_free (sizeof (long long), 0)) `

- 内置函数：*bool* **__atomic_is_lock_free** *(size_t size, void \*ptr)*

  `true`如果大小字节的对象总是为目标体系结构生成无锁原子指令，则此内置函数返回。如果不知道内置函数是无锁的，则调用名为 的运行时例程`__atomic_is_lock_free`。ptr是指向可用于确定对齐的对象的可选指针。值 0 表示应使用典型对齐方式。编译器也可能忽略此参数。

------

Next: [Integer Overflow](https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html#Integer-Overflow-Builtins) Builtins , Previous: [__sync Builtins](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html#g_t_005f_005fsync-Builtins) , Up: [C Extensions](https://gcc.gnu.org/onlinedocs/gcc/C-Extensions.html#C-Extensions)  [ [Contents](https://gcc.gnu.org/onlinedocs/gcc/index.html#SEC_Contents) ][ [Index](https://gcc.gnu.org/onlinedocs/gcc/Option-Index.html#Option-Index) ]