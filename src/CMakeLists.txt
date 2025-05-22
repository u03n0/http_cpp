# Define the core data library
add_library(socket_data
    socket/socket.cpp
)

add_library(request_data
  request/send_request.cpp
  request/receive_request.cpp
)

add_library(processing_data
  processing/processing.cpp
)

target_link_libraries(socket_data)
target_link_libraries(request_data)
target_link_libraries(processing_data)

add_executable(socket_app socket/socket_demo.cpp)
add_executable(send_request_app request/send_request_demo.cpp)
add_executable(receive_request_app request/receive_request_demo.cpp)
add_executable(processing_app processing/processing_demo.cpp)

target_link_libraries(socket_app PRIVATE socket_data)
target_link_libraries(send_request_app PRIVATE request_data socket_data)
target_link_libraries(receive_request_app PRIVATE request_data socket_data)
target_link_libraries(processing_app PRIVATE processing_data)
