# Define the core data library
add_library(socket_data
    socket/socket.cpp
)

# Link dependencies
target_link_libraries(socket_data)
#target_link_libraries(models_bayesian PRIVATE core_data)
#target_link_libraries(models_knn PRIVATE core_data)

# Create the main executable for the naive bayes example
#add_executable(naive_bayes_app models/bayesian/main.cpp)
#add_executable(knn_app models/knn/main.cpp)
add_executable(socket_app socket/socket_demo.cpp)
target_link_libraries(socket_app PRIVATE socket_data)
#target_link_libraries(naive_bayes_app PRIVATE models_bayesian core_data)
#target_link_libraries(knn_app PRIVATE models_knn core_data)
