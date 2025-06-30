project('cib', ['cpp'], version: '0.1.0', default_options: ['cpp_std=c++20'])

cmake = import('cmake')

boostmp11_sub = subproject('boostmp11')
boostmp11_dep = boostmp11_sub.get_variable('boostmp11_dep')

fmt_sub = subproject('fmt')
fmt_dep = fmt_sub.get_variable('fmt_header_only_dep')

baremetal_concurrency_sub = subproject('cpp-baremetal-concurrency')
baremetal_concurrency_dep = baremetal_concurrency_sub.get_variable(
    'baremetal_concurrency_dep',
)

stdx_sub = subproject('cpp-std-extensions')
stdx_dep = stdx_sub.get_variable('stdx_dep')

async_sub = subproject('cpp-baremetal-senders-and-receivers')
async_dep = async_sub.get_variable('async_dep')

# check test

inc = include_directories('include')

cib_dep = declare_dependency(
    include_directories: inc,
    dependencies: [
        boostmp11_dep,
        fmt_dep,
        baremetal_concurrency_dep,
        stdx_dep,
        async_dep,
    ],
)
