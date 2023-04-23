在编译时出现了一些错误。

第一个错误是关于结构体成员名的错误，似乎是 StringBuffer、IntBuffer、CharBuffer 和 ByteBuffer 结构体中没有名为 data 的成员。错误建议您将其更改为 datas，这可能是结构体的实际成员名。

第二个错误是一个拼写错误，似乎是在 include/utils.c 文件中的 symbolTableClear 函数中，将 SymbolTbale 写成了 SymbolTable。建议将其更正为正确的拼写。

最后，有一个警告，提示您的 errorReport 函数中有一行代码可能会导致控制流“穿透”（即从一个 case 分支“掉落”到下一个 case 分支），

可能会导致您的代码不按预期执行。建议您在需要控制流穿透的地方添加 break 语句来避免这种情况的发生。