file(GLOB_RECURSE sources	"sources/*.cpp")
file(GLOB_RECURSE headers	"headers/*.h")
file(GLOB_RECURSE external	"include/*.h")

include_directories("include")
include_directories("headers")

source_group("Sources" ${sources})
source_group("Headers" ${headers})
source_group("External" ${external})
