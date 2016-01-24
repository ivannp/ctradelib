SET(GTEST_ROOT C:/3rdparty/gtest CACHE PATH "GTEST library location")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/tradelib/include)
INCLUDE_DIRECTORIES(${GTEST_ROOT}/include)
LINK_DIRECTORIES(${GTEST_ROOT}/msvc/x64/Debug)

ADD_EXECUTABLE(
   gtests	
   GtestDataFeed.cpp
   GtestMath.cpp
   GtestPortfolio.cpp
   GtestTypes.cpp
   main.cpp)
   
TARGET_LINK_LIBRARIES(gtests tradelib)
TARGET_LINK_LIBRARIES(gtests debug gtestd)
TARGET_LINK_LIBRARIES(gtests debug gtest_maind)