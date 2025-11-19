project(
  'voxels',
  'cpp',
)

epoxy_dependency = dependency('epoxy')
glfw_dependency = dependency('glfw3')
glm_dependency = dependency('glm')
imgui_dependency = dependency('imgui')

executable(
  'voxels',
  dependencies: [
    epoxy_dependency,
    glfw_dependency,
    glm_dependency,
    imgui_dependency,
  ],
  sources: files(
    'src/debug.cpp',
    'src/main.cpp',
  ),
)
