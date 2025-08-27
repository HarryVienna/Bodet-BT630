# main/CMakeLists.txt

idf_component_register(SRCS "main.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES flip_clock wifi_provisioner)