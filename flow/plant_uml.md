@startuml

Client -> HttpServer: request

HttpServer -> WebApp: application_callable()
note right: application_callable(environ, start_response)

WebApp -> HttpServer: start_response()
note right: start_response(status, headers, exc_info)

WebApp -> HttpServer: return iterator

HttpServer -> Client: dddds

@enduml
