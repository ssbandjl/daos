'在线渲染: https://www.planttext.com/'
'使用文档: https://plantuml.com/zh/sequence-diagram'


@startuml

title OP发送和回复

client -> librados: 客户端已创建EQ, 任务及回调已封装到OP(如 op_submit(*task))



@enduml
