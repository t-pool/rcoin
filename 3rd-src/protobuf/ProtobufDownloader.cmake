macro(protobuf_extract PROTOBUF_ARCHIVE VERSION_STRING PROTOBUF_SOURCE_PATH)

  if(EXISTS "${PROTOBUF_ARCHIVE}")
    set(${PROTOBUF_SOURCE_PATH} "${PROJECT_BINARY_DIR}/protobuf-${VERSION_STRING}")
    message(STATUS "Extracting Protobuf sources to '${${PROTOBUF_SOURCE_PATH}}'...")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xzf "${PROTOBUF_ARCHIVE}"
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
  endif()

endmacro(protobuf_extract)

macro(protobuf_download VERSION_STRING PROTOBUF_ARCHIVE)

  set(PROTOBUF_ARCHIVE_URL
    "http://mirrors.ustc.edu.cn/gentoo/distfiles/protobuf-${VERSION_STRING}.tar.bz2")

  set(PROTOBUF_DOWNLOAD_PATH
    "${PROJECT_BINARY_DIR}/protobuf-${VERSION_STRING}.tar.bz2")

  if(EXISTS "${PROTOBUF_DOWNLOAD_PATH}")
    set(${PROTOBUF_ARCHIVE} "${PROTOBUF_DOWNLOAD_PATH}")
	message(STATUS "file already exist")
  else()
  
	message(STATUS "Downloading Protobuf version ${VERSION_STRING} from '${PROTOBUF_ARCHIVE_URL}' to ${PROTOBUF_DOWNLOAD_PATH} ...")


    file(DOWNLOAD "${PROTOBUF_ARCHIVE_URL}" "${PROTOBUF_DOWNLOAD_PATH}"
      STATUS status)

    list(GET status 0 error_code)

    if(error_code)
      file(REMOVE "${PROTOBUF_DOWNLOAD_PATH}")
      list(GET status 1 error_msg)
      message(FATAL_ERROR
        "Failed to download Protobuf source archive '${PROTOBUF_ARCHIVE_URL}': ${error_msg}")
    else()
      set(${PROTOBUF_ARCHIVE} "${PROTOBUF_DOWNLOAD_PATH}")
      message(STATUS "Successfully downloaded Protobuf version ${VERSION_STRING}.")
    endif()
  endif()
endmacro(protobuf_download)

macro(protobuf_download_and_extract VERSION_STRING PROTOBUF_SOURCE_PATH)
  protobuf_download(${VERSION_STRING} PROTOBUF_ARCHIVE)
  protobuf_extract(${PROTOBUF_ARCHIVE} ${VERSION_STRING} ${PROTOBUF_SOURCE_PATH})
endmacro(protobuf_download_and_extract)

