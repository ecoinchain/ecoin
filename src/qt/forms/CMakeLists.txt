
cmake_policy(SET CMP0020 NEW)
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DENABLE_WALLET)

file(GLOB UIS *.ui)

include_directories(../..)

qt5_wrap_ui(FORMS_SRC ${UIS})

add_library(rcoin-qt-uis STATIC ${FORMS_SRC})
