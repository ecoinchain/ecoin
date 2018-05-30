
set(AUTOMOC FALSE)
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DENABLE_WALLET)

file(GLOB UIS *.ui)

include_directories(../..)

qt5_wrap_ui(FORMS_SRC ${UIS})

add_library(rcoin-qt-uis STATIC ${FORMS_SRC})

set_property(SOURCE ${FORMS_SRC} PROPERTY SKIP_AUTOMOC ON)
