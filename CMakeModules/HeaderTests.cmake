INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)

MACRO(CHECK_INCLUDE_FILE_DEFINE HEADER VAR)
CHECK_INCLUDE_FILE(${HEADER} ${VAR})
IF(${VAR})
  ADD_DEFINE("${VAR} 1")
ENDIF(${VAR})
ENDMACRO(CHECK_INCLUDE_FILE_DEFINE)

MACRO(CHECK_FUNCTION_DEFINE HEADERS FUNC PARAM VAR)
CHECK_C_SOURCE_COMPILES(
"
${HEADERS}
int main(){
${FUNC} ${PARAM};
}
" ${VAR})
IF(${VAR})
  ADD_DEFINE("${VAR} 1")
ENDIF(${VAR})
ENDMACRO(CHECK_FUNCTION_DEFINE HEADERS)
