cmake_minimum_required(VERSION 3.28)

project("ACL")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ACL)

target_compile_features(ACL PUBLIC cxx_std_20)

target_sources(ACL PUBLIC FILE_SET CXX_MODULES FILES
    src/ACL.cppm
    # XML
    src/Xml/Xml.cppm
    src/Xml/Parsing.cppm
    src/Xml/Element.cppm
    # STRING
    src/String/String.cppm
    src/String/Conversion.cppm
    src/String/Instance.cppm
    src/String/Symbol.cppm
    # MAIN
    src/Main/Main.cppm
    src/Main/Array.cppm
    src/Main/Many.cppm
    src/Main/Range.cppm
    src/Main/Tuple.cppm
    src/Main/Union.cppm
    src/Main/Heap.cppm
    src/Main/Optional.cppm
    src/Main/Test.cppm
    src/Main/Traits.cppm
    src/Main/Types.cppm
)