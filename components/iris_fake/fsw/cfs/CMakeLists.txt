project(CFS_IRIS_FAKE C)
include_directories(platform_inc ${hwlib_MISSION_DIR}/fsw/public_inc)
add_cfe_app(iris_fake src/iris_fake_app.c)
