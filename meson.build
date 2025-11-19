project(
  'voxels',
  'cpp',
)

epoxy_dependency = dependency('epoxy')
glfw_dependency = dependency('glfw3')
glm_dependency = dependency('glm')

executable(
  'voxels',
  dependencies: [
    epoxy_dependency,
    glfw_dependency,
    glm_dependency,
  ],
  sources: files(
    'src/main.cpp',
  ),
)
